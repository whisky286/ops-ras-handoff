// #56/#57 parameter probe v2. Invalid inputs are stopped at GetWorkspaceSize:
// executing malformed tensors would be undefined behaviour and is not needed to
// decide whether the phase-1 contract is enforced.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>
#include "acl/acl.h"
#include "aclnn_matmul_abft_verify.h"

static int MakeT(void* host, int64_t count, std::vector<int64_t> shape, aclDataType dt, size_t es,
                 void** dev, aclTensor** t) {
    size_t bytes = (size_t)count * es;
    if (aclrtMalloc(dev, bytes ? bytes : 1, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) return -1;
    if (bytes && aclrtMemcpy(*dev, bytes, host, bytes, ACL_MEMCPY_HOST_TO_DEVICE) != ACL_SUCCESS) return -2;
    std::vector<int64_t> st(shape.size(), 1);
    for (int64_t i = (int64_t)shape.size()-2; i >= 0; --i) st[i] = shape[i+1]*st[i+1];
    *t = aclCreateTensor(shape.data(), shape.size(), dt, st.data(), 0, ACL_FORMAT_ND,
                         shape.data(), shape.size(), *dev);
    return *t ? 0 : -3;
}
static int64_t Prod(const std::vector<int64_t>& s) { int64_t p = 1; for (auto v : s) p *= v; return p; }

static int gInvalidAccepted = 0;
static int gInternalFailures = 0;
static int gControlOk = 0;
static aclrtStream gSt = nullptr;

// invalid=true means the call must be rejected at phase 1; it is never executed.
static void Case(const char* name, int64_t M, int64_t N, int64_t K,
                 aclDataType aT, aclDataType bT, aclDataType wT,
                 std::vector<int64_t> cShape, std::vector<int64_t> wShape,
                 double eMax, bool invalid) {
    std::vector<float> cF((size_t)Prod(cShape), 0.1f), wF((size_t)Prod(wShape), 1.0f);
    std::vector<uint16_t> aH(M*K, 0x2E66), bH(K*N, 0x2E66), wH((size_t)Prod(wShape), 0x3C00);
    std::vector<float> aF(M*K, 0.1f), bF(K*N, 0.1f);
    std::vector<uint8_t> comp(8, 0xFF);
    void *da=nullptr,*db=nullptr,*dc=nullptr,*dw=nullptr,*dcomp=nullptr;
    aclTensor *ta=nullptr,*tb=nullptr,*tc=nullptr,*tw=nullptr,*tcomp=nullptr;
    auto es = [](aclDataType t) { return t == ACL_FLOAT ? sizeof(float) : sizeof(uint16_t); };
    int cr = 0;
    cr |= MakeT(aT==ACL_FLOAT ? (void*)aF.data() : (void*)aH.data(), M*K, {M,K}, aT, es(aT), &da, &ta);
    cr |= MakeT(bT==ACL_FLOAT ? (void*)bF.data() : (void*)bH.data(), K*N, {K,N}, bT, es(bT), &db, &tb);
    cr |= MakeT(cF.data(), Prod(cShape), cShape, ACL_FLOAT, 4, &dc, &tc);
    cr |= MakeT(wT==ACL_FLOAT ? (void*)wF.data() : (void*)wH.data(), Prod(wShape), wShape, wT, es(wT), &dw, &tw);
    cr |= MakeT(comp.data(), 8, {8}, ACL_UINT8, 1, &dcomp, &tcomp);
    uint64_t ws=0; aclOpExecutor* ex=nullptr;
    if (cr != 0) ++gInternalFailures;
    int s1 = -999;
    if (cr == 0) s1 = (int)aclnnMatmulAbftVerifyGetWorkspaceSize(ta, tb, tc, tw, eMax, tcomp, &ws, &ex);
    bool accepted = (s1 == 0);
    int execStatus = -99;
    if (accepted && !invalid) {           // execute only the valid control
        void* wp = nullptr;
        if (ws > 0) aclrtMalloc(&wp, ws, ACL_MEM_MALLOC_HUGE_FIRST);
        execStatus = (int)aclnnMatmulAbftVerify(wp, ws, ex, gSt);
        if (execStatus == 0) execStatus = (int)aclrtSynchronizeStream(gSt);
        if (wp) aclrtFree(wp);
        if (execStatus != 0) ++gInternalFailures;
        else gControlOk = 1;
    }
    if (invalid && accepted) ++gInvalidAccepted;
    const char* em = aclGetRecentErrMsg();
    std::printf("{\"case\":\"%s\",\"expected\":\"%s\",\"getws\":%d,\"accepted\":%s,\"exec\":%d,\"msg\":\"%.110s\"}\n",
                name, invalid ? "reject" : "accept", s1, accepted ? "true" : "false",
                execStatus, em ? em : "");
    if (ex) aclDestroyAclOpExecutor(ex);
    for (aclTensor* t : {ta,tb,tc,tw,tcomp}) if (t) aclDestroyTensor(t);
    for (void* p : {da,db,dc,dw,dcomp}) if (p) aclrtFree(p);
}

int main(int argc, char** argv) {
    int dev = (argc > 1) ? std::atoi(argv[1]) : 0;
    const char* mode = (argc > 2) ? argv[2] : "defect";   // "defect" | "fix"
    aclrtContext ctx = nullptr;
    if (aclInit(nullptr) != ACL_SUCCESS) { std::printf("{\"fatal\":\"aclInit\"}\n"); return 3; }
    if (aclrtSetDevice(dev) != ACL_SUCCESS) { std::printf("{\"fatal\":\"setDevice\"}\n"); return 3; }
    if (aclrtCreateContext(&ctx, dev) != ACL_SUCCESS) { std::printf("{\"fatal\":\"createContext\"}\n"); return 3; }
    if (aclrtSetCurrentContext(ctx) != ACL_SUCCESS) { std::printf("{\"fatal\":\"setCurrentContext\"}\n"); return 3; }
    if (aclrtCreateStream(&gSt) != ACL_SUCCESS) { std::printf("{\"fatal\":\"createStream\"}\n"); return 3; }
    const int64_t M=64,N=256,K=5;
    Case("valid_fp16_control", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, 0.00002, false);
    Case("A_fp16_B_fp32_mismatch", M,N,K, ACL_FLOAT16,ACL_FLOAT,ACL_FLOAT16, {M,N},{N}, 0.00002, true);
    Case("C_rank1", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M*N},{N}, 0.00002, true);
    Case("C_col_mismatch", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N-1},{N}, 0.00002, true);
    Case("W_len_mismatch", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N-1}, 0.00002, true);
    Case("W_rank2", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{1,N}, 0.00002, true);
    Case("eMax_negative", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, -1.0, true);
    Case("eMax_zero_allowed", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, 0.0, false);
    Case("eMax_nan", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, std::numeric_limits<double>::quiet_NaN(), true);
    Case("eMax_pos_inf", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, std::numeric_limits<double>::infinity(), true);
    std::printf("{\"summary\":{\"mode\":\"%s\",\"invalid_cases_accepted\":%d,\"control_ok\":%d,"
                "\"internal_failures\":%d}}\n", mode, gInvalidAccepted, gControlOk, gInternalFailures);
    int rc = 0;
    if (gInternalFailures > 0) rc = 3;                       // harness/environment error, never a verdict
    else if (std::strcmp(mode, "fix") == 0)
        rc = (gInvalidAccepted == 0 && gControlOk) ? 0 : 5;   // 5 = fix not effective
    else
        rc = (gInvalidAccepted > 0 && gControlOk) ? 0 : 4;    // 4 = defect not reproduced
    aclrtDestroyStream(gSt); aclrtDestroyContext(ctx); aclrtResetDevice(dev); aclFinalize();
    return rc;
}
