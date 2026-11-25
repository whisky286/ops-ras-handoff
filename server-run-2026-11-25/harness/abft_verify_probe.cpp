// ops-ras #118 device reproduction harness (written on the server for this handoff).
// A/B ~ U(-0.5,0.5) rounded to the target dtype, C computed on host in FP64 from the
// rounded values, then verified with aclnnMatmulAbftVerify; compRow is decoded per row.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#include "acl/acl.h"
#include "aclnn_matmul_abft_verify.h"

#define CK(cond, msg) do { if (!(cond)) { std::printf("FAIL %s\n", msg); return 1; } } while (0)

static aclrtStream gStream = nullptr;

static uint16_t FloatToBf16(float v) {
    uint32_t bits; std::memcpy(&bits, &v, 4);
    bits += 0x7FFFu + ((bits >> 16U) & 1U);
    return static_cast<uint16_t>(bits >> 16U);
}
static float Bf16ToFloat(uint16_t v) {
    uint32_t bits = static_cast<uint32_t>(v) << 16U; float f; std::memcpy(&f, &bits, 4); return f;
}
static uint16_t FloatToFp16(float v) { __fp16 h = static_cast<__fp16>(v); uint16_t u; std::memcpy(&u, &h, 2); return u; }
static float Fp16ToFloat(uint16_t u) { __fp16 h; std::memcpy(&h, &u, 2); return static_cast<float>(h); }

struct DType { const char* name; aclDataType acl; size_t esize; };

static int CreateTensor(const void* host, int64_t count, const std::vector<int64_t>& shape,
                        aclDataType dt, size_t esize, void** dev, aclTensor** t) {
    size_t bytes = static_cast<size_t>(count) * esize;
    if (aclrtMalloc(dev, bytes, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) return -1;
    if (bytes && aclrtMemcpy(*dev, bytes, host, bytes, ACL_MEMCPY_HOST_TO_DEVICE) != ACL_SUCCESS) return -1;
    std::vector<int64_t> strides(shape.size(), 1);
    for (int64_t i = static_cast<int64_t>(shape.size()) - 2; i >= 0; --i) strides[i] = shape[i + 1] * strides[i + 1];
    *t = aclCreateTensor(shape.data(), shape.size(), dt, strides.data(), 0, ACL_FORMAT_ND,
                         shape.data(), shape.size(), *dev);
    return *t ? 0 : -2;
}

static int RunCase(const DType& dt, int64_t M, int64_t N, int64_t K, unsigned seed,
                   double eMax, int injectRow, int injectCol, double injectDelta, double injectRel) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
    const int64_t splitN = (N + 255) / 256;
    const int64_t compBytes = ((M + 7) / 8) * splitN;

    std::vector<uint16_t> aRaw(M * K), bRaw(K * N), wRaw(N);
    std::vector<float> aF(M * K), bF(K * N), wF(N, 1.0f);
    const bool isHalf = (dt.esize == 2 && dt.acl == ACL_FLOAT16);
    const bool isBf = (dt.esize == 2 && dt.acl == ACL_BF16);
    for (int64_t i = 0; i < M * K; ++i) {
        float v = dist(rng);
        if (isHalf) { aRaw[i] = FloatToFp16(v); aF[i] = Fp16ToFloat(aRaw[i]); }
        else if (isBf) { aRaw[i] = FloatToBf16(v); aF[i] = Bf16ToFloat(aRaw[i]); }
        else { std::memcpy(&aF[i], &v, 4); }
    }
    for (int64_t i = 0; i < K * N; ++i) {
        float v = dist(rng);
        if (isHalf) { bRaw[i] = FloatToFp16(v); bF[i] = Fp16ToFloat(bRaw[i]); }
        else if (isBf) { bRaw[i] = FloatToBf16(v); bF[i] = Bf16ToFloat(bRaw[i]); }
        else { std::memcpy(&bF[i], &v, 4); }
    }
    for (int64_t i = 0; i < N; ++i) {
        if (isHalf) wRaw[i] = FloatToFp16(1.0f);
        else if (isBf) wRaw[i] = FloatToBf16(1.0f);
    }

    std::vector<float> c(M * N);
    for (int64_t i = 0; i < M; ++i) {
        for (int64_t j = 0; j < N; ++j) {
            double acc = 0.0;
            for (int64_t k = 0; k < K; ++k) acc += static_cast<double>(aF[i * K + k]) * static_cast<double>(bF[k * N + j]);
            c[i * N + j] = static_cast<float>(acc);
        }
    }
    if (injectRow >= 0) {
        if (injectRel < 0.0) {
            double rowSum = 0.0;
            for (int64_t j = 0; j < N; ++j) rowSum += static_cast<double>(c[injectRow * N + j]);
            c[injectRow * N + injectCol] += static_cast<float>(std::fabs(rowSum) * 0.5);
        } else {
            c[injectRow * N + injectCol] = c[injectRow * N + injectCol] * static_cast<float>(1.0 + injectRel) + static_cast<float>(injectDelta);
        }
    }

    std::vector<uint8_t> comp(compBytes, 0xFF);
    const void* aHost = (dt.esize == 4) ? static_cast<const void*>(aF.data()) : static_cast<const void*>(aRaw.data());
    const void* bHost = (dt.esize == 4) ? static_cast<const void*>(bF.data()) : static_cast<const void*>(bRaw.data());
    const void* wHost = (dt.esize == 4) ? static_cast<const void*>(wF.data()) : static_cast<const void*>(wRaw.data());

    void *devA = nullptr, *devB = nullptr, *devC = nullptr, *devW = nullptr, *devComp = nullptr, *ws = nullptr;
    aclTensor *tA = nullptr, *tB = nullptr, *tC = nullptr, *tW = nullptr, *tComp = nullptr;
    int rc = 0;
    rc |= CreateTensor(aHost, M * K, {M, K}, dt.acl, dt.esize, &devA, &tA);
    rc |= CreateTensor(bHost, K * N, {K, N}, dt.acl, dt.esize, &devB, &tB);
    rc |= CreateTensor(c.data(), M * N, {M, N}, ACL_FLOAT, 4, &devC, &tC);
    rc |= CreateTensor(wHost, N, {N}, dt.acl, dt.esize, &devW, &tW);
    rc |= CreateTensor(comp.data(), compBytes, {compBytes}, ACL_UINT8, 1, &devComp, &tComp);
    if (rc != 0) { std::printf("{\"error\":\"tensor_create\",\"rc\":%d}\n", rc); return 1; }

    uint64_t wsSize = 0;
    aclOpExecutor* exec = nullptr;
    aclnnStatus s1 = aclnnMatmulAbftVerifyGetWorkspaceSize(tA, tB, tC, tW, eMax, tComp, &wsSize, &exec);
    aclnnStatus s2 = ACL_ERROR_FAILURE;
    if (s1 != ACL_SUCCESS) {
        const char* em = aclGetRecentErrMsg();
        std::printf("recentErr=%s\n", em ? em : "");
    }
    int syncRc = -1;
    if (s1 == ACL_SUCCESS) {
        if (wsSize > 0) {
            if (aclrtMalloc(&ws, wsSize, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) ws = nullptr;
        }
        s2 = aclnnMatmulAbftVerify(ws, wsSize, exec, gStream);
        if (s2 == ACL_SUCCESS) syncRc = aclrtSynchronizeStream(gStream);
    }
    if (s2 == ACL_SUCCESS) {
        aclrtMemcpy(comp.data(), compBytes, devComp, compBytes, ACL_MEMCPY_DEVICE_TO_HOST);
    }
    int bad = -1;
    std::string hex;
    if (s2 == ACL_SUCCESS) {
        bad = 0;
        for (int64_t r = 0; r < M; ++r) {
            int64_t byte = (r / 8) * splitN;      // N-block 0 (N<=256 in these cases)
            int bit = static_cast<int>(r % 8);
            if (((comp[byte] >> bit) & 1) == 0) ++bad;
        }
        char buf[4];
        for (int64_t i = 0; i < compBytes; ++i) { std::snprintf(buf, sizeof buf, "%02x", comp[i]); hex += buf; }
    }
    std::printf("{\"dtype\":\"%s\",\"M\":%lld,\"N\":%lld,\"K\":%lld,\"seed\":%u,\"eMax\":%.8g,"
                "\"injectRow\":%d,\"injectCol\":%d,\"injectDelta\":%.6g,"
                "\"getWorkspace\":%d,\"exec\":%d,\"sync\":%d,\"bad\":%d,\"compRowHex\":\"%s\"}\n",
                dt.name, (long long)M, (long long)N, (long long)K, seed, eMax,
                injectRow, injectCol, injectDelta, (int)s1, (int)s2, syncRc, bad, hex.c_str());

    if (exec) aclDestroyAclOpExecutor(exec);
    for (aclTensor* t : {tA, tB, tC, tW, tComp}) { if (t) aclDestroyTensor(t); }
    for (void* p : {devA, devB, devC, devW, devComp, ws}) { if (p) aclrtFree(p); }
    return 0;
}

int main(int argc, char** argv) {
    int devId = 10;
    int mode = 0; // 0 = no-fault sweep, 1 = single-element fault injection
    if (argc > 1) devId = std::atoi(argv[1]);
    if (argc > 2) mode = std::atoi(argv[2]);
    aclrtContext ctx = nullptr;
    CK(aclInit(nullptr) == ACL_SUCCESS, "aclInit");
    CK(aclrtSetDevice(devId) == ACL_SUCCESS, "setDevice");
    CK(aclrtCreateContext(&ctx, devId) == ACL_SUCCESS, "createContext");
    CK(aclrtSetCurrentContext(ctx) == ACL_SUCCESS, "setCurrentContext");
    CK(aclrtCreateStream(&gStream) == ACL_SUCCESS, "createStream");

    DType dts[3] = {{"float32", ACL_FLOAT, 4}, {"float16", ACL_FLOAT16, 2}, {"bfloat16", ACL_BF16, 2}};
    const int64_t Ks[] = {1, 2, 3, 5, 9, 17, 33, 65, 129, 255, 256, 257, 512};
    const unsigned seeds[] = {42u, 1234u, 777777u};
    const int64_t M = 64, N = 256;
    for (const auto& dt : dts) {
        double emax = (dt.acl == ACL_BF16) ? 0.001 : 0.00002;
        for (int64_t K : Ks) {
            for (unsigned seed : seeds) {
                if (mode == 0) RunCase(dt, M, N, K, seed, emax, -1, 0, 0.0, 0.0);
                else if (mode == 1) RunCase(dt, M, N, K, seed, emax, 5, 3, 0.5, 0.0);
                else if (mode == 2) RunCase(dt, M, N, K, seed, emax, 5, 3, 0.0, 0.10); // +10% relative fault
                else RunCase(dt, M, N, K, seed, emax, 5, 3, 0.0, -1.0); // +half row sum fault
            }
        }
    }
    std::printf("{\"harness\":\"done\"}\n");
    aclrtDestroyStream(gStream);
    aclrtDestroyContext(ctx);
    aclrtResetDevice(devId);
    aclFinalize();
    return 0;
}
