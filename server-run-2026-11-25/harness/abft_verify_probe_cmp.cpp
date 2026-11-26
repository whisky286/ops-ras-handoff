// #118 harness v2. Hardened: sentinel-initialized compRow, every ACL return checked,
// per-case internal-failure accounting, exact fault-row decoding.
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

static aclrtStream gStream = nullptr;
static int gInternalFailures = 0;
static int gCases = 0;

static uint16_t FloatToBf16(float v) { uint32_t b; std::memcpy(&b,&v,4); b += 0x7FFFu + ((b>>16U)&1U); return (uint16_t)(b>>16U); }
static float Bf16ToFloat(uint16_t v) { uint32_t b=(uint32_t)v<<16U; float f; std::memcpy(&f,&b,4); return f; }
static uint16_t FloatToFp16(float v) { __fp16 h=(__fp16)v; uint16_t u; std::memcpy(&u,&h,2); return u; }
static float Fp16ToFloat(uint16_t u) { __fp16 h; std::memcpy(&h,&u,2); return (float)h; }

struct DType { const char* name; aclDataType acl; size_t esize; };

static int MakeTensor(const void* host, int64_t count, std::vector<int64_t> shape, aclDataType dt,
                      size_t es, void** dev, aclTensor** t) {
    size_t bytes = (size_t)count * es;
    if (aclrtMalloc(dev, bytes ? bytes : 1, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) return -1;
    if (bytes && aclrtMemcpy(*dev, bytes, host, bytes, ACL_MEMCPY_HOST_TO_DEVICE) != ACL_SUCCESS) return -2;
    std::vector<int64_t> st(shape.size(), 1);
    for (int64_t i = (int64_t)shape.size()-2; i >= 0; --i) st[i] = shape[i+1]*st[i+1];
    *t = aclCreateTensor(shape.data(), shape.size(), dt, st.data(), 0, ACL_FORMAT_ND,
                         shape.data(), shape.size(), *dev);
    return *t ? 0 : -3;
}

// mode: 0 no fault, 1 abs +0.5, 2 rel +10%, 3 +0.5*rowSum
static void RunCase(const DType& dt, int64_t M, int64_t N, int64_t K, unsigned seed,
                    double eMax, int mode, int injectRow, int injectCol) {
    ++gCases;
    const unsigned kSentinel = 0xA5;
    const int64_t splitN = (N + 255) / 256;
    const int64_t compBytes = ((M + 7) / 8) * splitN;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
    const bool isHalf = (dt.esize == 2 && dt.acl == ACL_FLOAT16);
    const bool isBf = (dt.esize == 2 && dt.acl == ACL_BF16);

    std::vector<uint16_t> aRaw(M*K), bRaw(K*N), wRaw(N, 0);
    std::vector<float> aF(M*K), bF(K*N), wF(N, 1.0f);
    for (int64_t i = 0; i < M*K; ++i) {
        float v = dist(rng);
        if (isHalf) { aRaw[i]=FloatToFp16(v); aF[i]=Fp16ToFloat(aRaw[i]); }
        else if (isBf) { aRaw[i]=FloatToBf16(v); aF[i]=Bf16ToFloat(aRaw[i]); }
        else std::memcpy(&aF[i],&v,4);
    }
    for (int64_t i = 0; i < K*N; ++i) {
        float v = dist(rng);
        if (isHalf) { bRaw[i]=FloatToFp16(v); bF[i]=Fp16ToFloat(bRaw[i]); }
        else if (isBf) { bRaw[i]=FloatToBf16(v); bF[i]=Bf16ToFloat(bRaw[i]); }
        else std::memcpy(&bF[i],&v,4);
    }
    for (int64_t i = 0; i < N; ++i) {
        if (isHalf) wRaw[i]=FloatToFp16(1.0f);
        else if (isBf) wRaw[i]=FloatToBf16(1.0f);
    }
    std::vector<float> c(M*N);
    for (int64_t i = 0; i < M; ++i)
        for (int64_t j = 0; j < N; ++j) {
            double acc = 0.0;
            for (int64_t k = 0; k < K; ++k) acc += (double)aF[i*K+k]*(double)bF[k*N+j];
            c[i*N+j] = (float)acc;
        }
    if (mode != 0 && injectRow >= 0) {
        float& v = c[injectRow*N + injectCol];
        if (mode == 1) v += 0.5f;
        else if (mode == 2) v *= 1.10f;
        else { double rs = 0.0; for (int64_t j = 0; j < N; ++j) rs += (double)c[injectRow*N+j]; v += (float)(std::fabs(rs)*0.5); }
    }
    std::vector<uint8_t> comp((size_t)compBytes, (uint8_t)kSentinel);
    const void* aHost = (dt.esize == 4) ? (const void*)aF.data() : (const void*)aRaw.data();
    const void* bHost = (dt.esize == 4) ? (const void*)bF.data() : (const void*)bRaw.data();
    const void* wHost = (dt.esize == 4) ? (const void*)wF.data() : (const void*)wRaw.data();

    void *dA=nullptr,*dB=nullptr,*dC=nullptr,*dW=nullptr,*dComp=nullptr,*ws=nullptr;
    aclTensor *tA=nullptr,*tB=nullptr,*tC=nullptr,*tW=nullptr,*tComp=nullptr;
    int cr = 0;
    cr |= MakeTensor(aHost,M*K,{M,K},dt.acl,dt.esize,&dA,&tA);
    cr |= MakeTensor(bHost,K*N,{K,N},dt.acl,dt.esize,&dB,&tB);
    cr |= MakeTensor(c.data(),M*N,{M,N},ACL_FLOAT,4,&dC,&tC);
    cr |= MakeTensor(wHost,N,{N},dt.acl,dt.esize,&dW,&tW);
    cr |= MakeTensor(comp.data(),compBytes,{compBytes},ACL_UINT8,1,&dComp,&tComp);

    uint64_t wsSize = 0; aclOpExecutor* exec = nullptr;
    aclnnStatus s1 = ACL_ERROR_FAILURE, s2 = ACL_ERROR_FAILURE;
    int syncRc = -1, memRc = -1, written = -1, bad = -1, faultBit = -1;
    std::string hex;
    if (cr != 0) {
        ++gInternalFailures;
        std::printf("{\"dtype\":\"%s\",\"K\":%lld,\"seed\":%u,\"mode\":%d,\"status\":\"tensor_setup_failed\",\"rc\":%d}\n",
                    dt.name,(long long)K,seed,mode,cr);
    } else {
        s1 = aclnnMatmulAbftVerifyGetWorkspaceSize(tA,tB,tC,tW,eMax,tComp,&wsSize,&exec);
        if (s1 == ACL_SUCCESS) {
            if (wsSize > 0 && aclrtMalloc(&ws,wsSize,ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) ws = nullptr;
            s2 = aclnnMatmulAbftVerify(ws,wsSize,exec,gStream);
            if (s2 == ACL_SUCCESS) syncRc = (int)aclrtSynchronizeStream(gStream);
        }
        if (s1 == ACL_SUCCESS && s2 == ACL_SUCCESS && syncRc == 0) {
            memRc = (int)aclrtMemcpy(comp.data(),compBytes,dComp,compBytes,ACL_MEMCPY_DEVICE_TO_HOST);
            if (memRc == 0) {
                written = 0;
                for (int64_t i = 0; i < compBytes; ++i) if (comp[i] != (uint8_t)kSentinel) { written = 1; break; }
                if (written == 1) {
                    bad = 0;
                    for (int64_t r = 0; r < M; ++r) {
                        int64_t byte = (r/8)*splitN;
                        int bit = (int)(r%8);
                        int set = (comp[byte] >> bit) & 1;
                        if (mode != 0 && r == injectRow) faultBit = set;
                        if (!set) ++bad;
                    }
                    char buf[4];
                    for (int64_t i = 0; i < compBytes; ++i) { std::snprintf(buf,sizeof buf,"%02x",comp[i]); hex += buf; }
                }
            }
        }
        bool ok = (s1==ACL_SUCCESS && s2==ACL_SUCCESS && syncRc==0 && memRc==0 && written==1);
        if (!ok) ++gInternalFailures;
        std::printf("{\"dtype\":\"%s\",\"K\":%lld,\"seed\":%u,\"mode\":%d,\"getws\":%d,\"exec\":%d,\"sync\":%d,"
                    "\"memcpy\":%d,\"written\":%d,\"bad\":%d,\"faultBit\":%d,\"compRowHex\":\"%s\"}\n",
                    dt.name,(long long)K,seed,mode,(int)s1,(int)s2,syncRc,memRc,written,bad,faultBit,hex.c_str());
    }
    if (std::getenv("ABFT_DEBUG_DUMP") != nullptr && ws != nullptr && mode == 0 && dt.acl != ACL_FLOAT) {
        const size_t userOff = 16UL * 1024UL * 1024UL;
        std::vector<float> dbg(464, 0.0f);
        if (aclrtMemcpy(dbg.data(), 1856, (char*)ws + userOff, 1856, ACL_MEMCPY_DEVICE_TO_HOST) == ACL_SUCCESS) {
            std::printf("DBG %s K=%lld zRow ", dt.name, (long long)K);
            for (int i = 0; i < 64; ++i) std::printf("%.8g ", dbg[i]);
            std::puts("");
            std::printf("DBG %s K=%lld dRow ", dt.name, (long long)K);
            for (int i = 64; i < 128; ++i) std::printf("%.8g ", dbg[i]);
            std::puts("");
            std::printf("DBG %s K=%lld thre ", dt.name, (long long)K);
            for (int i = 128; i < 192; ++i) std::printf("%.8g ", dbg[i]);
            std::puts("");

            if (K <= 4) {
                int64_t splitN2 = (N + 255) / 256;
                std::vector<double> hostRow(M, 0.0);
                for (int64_t i = 0; i < M; ++i) {
                    double acc = 0.0;
                    for (int64_t j = 0; j < N; ++j) acc += (double)c[i*N+j] * (double)wF[j];
                    hostRow[i] = acc;
                }
                int nviol = 0, nbit0 = 0, maxdd_row = -1; double maxdd = 0.0;
                std::printf("CMP %s K=%lld rows_bad_from_bits:", dt.name, (long long)K);
                for (int64_t r = 0; r < M; ++r) {
                    int set = (comp[(r/8)*splitN2] >> (r%8)) & 1;
                    if (!set) { std::printf(" %lld", (long long)r); ++nbit0; }
                }
                std::puts("");
                for (int64_t r = 0; r < M; ++r) {
                    double z = dbg[r], d = dbg[64+r], t = dbg[128+r];
                    double rel = std::fabs(z - d);
                    bool vi = (rel > t);
                    if (vi) ++nviol;
                    double dd = d - hostRow[r];
                    if (std::fabs(dd) > maxdd) { maxdd = std::fabs(dd); maxdd_row = (int)r; }
                    int set = (comp[(r/8)*splitN2] >> (r%8)) & 1;
                    if (vi || !set || std::fabs(dd) > 1e-3)
                        std::printf("ROW %s K=%lld r=%lld bit=%d z=%.9g d=%.9g |z-d|=%.3g thre=%.3g viol=%d hostRow=%.9g d-host=%.3g\n",
                            dt.name, (long long)K, (long long)r, set, z, d, rel, t, (int)vi, hostRow[r], dd);
                }
                std::printf("CMP %s K=%lld nviol=%d nbit0=%d maxAbsdHost=%.3g at_row=%d\n",
                    dt.name, (long long)K, nviol, nbit0, maxdd, maxdd_row);
                if (K <= 3) {
                    std::printf("BST %s K=1 bMeanAbs=%.7g bMeanSquare=%.7g bVar=%.7g\n",
                        dt.name, dbg[768/4], dbg[832/4], dbg[896/4]);
                    for (int64_t r = 0; r < M; ++r) {
                        int set = (comp[(r/8)*splitN2] >> (r%8)) & 1;
                        double t = dbg[128+r];
                        bool bad = !set || std::isnan(t) || std::isinf(t);
                        double hmax = -1e30, hmin = 1e30;
                        for (int64_t kk = 0; kk < K; ++kk) { double v = (double)aF[r*K+kk]; if (v > hmax) hmax = v; if (v < hmin) hmin = v; }
                        if (r == 0 || bad || r == 2 || r == 13 || r == 30 || r == 32 || r == 44 || r == 54)
                            std::printf("AST %s K=1 r=%lld bit=%d thre=%.7g aMax=%.7g aMean=%.7g aMin=%.7g aHost=%.7g hostMax=%.7g hostMin=%.7g\n",
                                dt.name, (long long)r, set, t, dbg[1088/4+r], dbg[1344/4+r], dbg[1600/4+r], hmax, hmin, (double)aF[r*K]);
                    }
                }
            }
        } else {
            std::puts("DBG copy_failed");
        }
    }
    if (exec) aclDestroyAclOpExecutor(exec);
    for (aclTensor* t : {tA,tB,tC,tW,tComp}) if (t) aclDestroyTensor(t);
    for (void* p : {dA,dB,dC,dW,dComp,ws}) if (p) aclrtFree(p);
}

int main(int argc, char** argv) {
    int dev = (argc > 1) ? std::atoi(argv[1]) : 0;
    int mode = (argc > 2) ? std::atoi(argv[2]) : 0;
    aclrtContext ctx = nullptr;
    if (aclInit(nullptr) != ACL_SUCCESS) { std::printf("{\"fatal\":\"aclInit\"}\n"); return 2; }
    if (aclrtSetDevice(dev) != ACL_SUCCESS) { std::printf("{\"fatal\":\"setDevice\"}\n"); return 2; }
    if (aclrtCreateContext(&ctx,dev) != ACL_SUCCESS) { std::printf("{\"fatal\":\"createContext\"}\n"); return 2; }
    if (aclrtSetCurrentContext(ctx) != ACL_SUCCESS) { std::printf("{\"fatal\":\"setCurrentContext\"}\n"); return 2; }
    if (aclrtCreateStream(&gStream) != ACL_SUCCESS) { std::printf("{\"fatal\":\"createStream\"}\n"); return 2; }

    DType dts[3] = {{"float32",ACL_FLOAT,4},{"float16",ACL_FLOAT16,2},{"bfloat16",ACL_BF16,2}};
    const int64_t Ks[] = {1,2,3,5,9};
    const unsigned seeds[] = {42u};
    const int64_t M = 64, N = 256;
    for (const auto& dt : dts) {
        double emax = (dt.acl == ACL_BF16) ? 0.001 : 0.00002;
        for (int64_t K : Ks)
            for (unsigned seed : seeds)
                RunCase(dt,M,N,K,seed,emax,mode,5,3);
    }
    std::printf("{\"summary\":{\"cases\":%d,\"internal_failures\":%d,\"mode\":%d}}\n", gCases, gInternalFailures, mode);
    aclrtDestroyStream(gStream); aclrtDestroyContext(ctx); aclrtResetDevice(dev); aclFinalize();
    return gInternalFailures == 0 ? 0 : 3;
}
