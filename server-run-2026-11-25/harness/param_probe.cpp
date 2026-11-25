// Parameter-validation probe v2 for #56 (dtype/shape) and #57 (eMax): GetWorkspaceSize + execute.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>
#include "acl/acl.h"
#include "aclnn_matmul_abft_verify.h"

static aclrtStream gSt = nullptr;

static int CreateT(void* host, int64_t count, std::vector<int64_t> shape, aclDataType dt, size_t es, void** dev, aclTensor** t) {
    size_t bytes = static_cast<size_t>(count) * es;
    if (aclrtMalloc(dev, bytes, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) return -1;
    if (bytes) aclrtMemcpy(*dev, bytes, host, bytes, ACL_MEMCPY_HOST_TO_DEVICE);
    std::vector<int64_t> st(shape.size(), 1);
    for (int64_t i = static_cast<int64_t>(shape.size()) - 2; i >= 0; --i) st[i] = shape[i+1]*st[i+1];
    *t = aclCreateTensor(shape.data(), shape.size(), dt, st.data(), 0, ACL_FORMAT_ND, shape.data(), shape.size(), *dev);
    return *t ? 0 : -2;
}
static int64_t Prod(const std::vector<int64_t>& s) { int64_t p=1; for (auto v : s) p*=v; return p; }

static void Case(const char* name, int64_t M, int64_t N, int64_t K,
                 aclDataType aT, aclDataType bT, aclDataType wT,
                 std::vector<int64_t> cShape, std::vector<int64_t> wShape, double eMax) {
    std::vector<float> aF(M*K, 0.1f), bF(K*N, 0.1f), cF(static_cast<size_t>(Prod(cShape)), 0.1f), wF(static_cast<size_t>(Prod(wShape)), 1.0f);
    std::vector<uint8_t> comp(8, 0xFF);
    void *da=nullptr,*db=nullptr,*dc=nullptr,*dw=nullptr,*dcomp=nullptr;
    aclTensor *ta=nullptr,*tb=nullptr,*tc=nullptr,*tw=nullptr,*tcomp=nullptr;
    std::vector<uint16_t> aH(M*K, 0x2E66), bH(K*N, 0x2E66), wH(static_cast<size_t>(Prod(wShape)), 0x3C00);
    auto es = [](aclDataType t) { return t == ACL_FLOAT ? sizeof(float) : sizeof(uint16_t); };
    CreateT(aT==ACL_FLOAT ? (void*)aF.data() : (void*)aH.data(), M*K, {M,K}, aT, es(aT), &da, &ta);
    CreateT(bT==ACL_FLOAT ? (void*)bF.data() : (void*)bH.data(), K*N, {K,N}, bT, es(bT), &db, &tb);
    CreateT(cF.data(), Prod(cShape), cShape, ACL_FLOAT, 4, &dc, &tc);
    CreateT(wT==ACL_FLOAT ? (void*)wF.data() : (void*)wH.data(), Prod(wShape), wShape, wT, es(wT), &dw, &tw);
    CreateT(comp.data(), 8, {8}, ACL_UINT8, 1, &dcomp, &tcomp);
    uint64_t ws=0; aclOpExecutor* ex=nullptr;
    aclnnStatus s = aclnnMatmulAbftVerifyGetWorkspaceSize(ta, tb, tc, tw, eMax, tcomp, &ws, &ex);
    int execStatus = -99, syncStatus = -99;
    void* wp = nullptr;
    if (s == ACL_SUCCESS) {
        if (ws > 0) aclrtMalloc(&wp, ws, ACL_MEM_MALLOC_HUGE_FIRST);
        execStatus = (int)aclnnMatmulAbftVerify(wp, ws, ex, gSt);
        if (execStatus == 0) syncStatus = (int)aclrtSynchronizeStream(gSt);
    }
    const char* em = aclGetRecentErrMsg();
    std::printf("{\"case\":\"%s\",\"getws\":%d,\"exec\":%d,\"sync\":%d,\"msg\":\"%.140s\"}\n",
                name, (int)s, execStatus, syncStatus, em ? em : "");
    if (wp) aclrtFree(wp);
    if (ex) aclDestroyAclOpExecutor(ex);
    for (aclTensor* t : {ta,tb,tc,tw,tcomp}) if (t) aclDestroyTensor(t);
    for (void* p : {da,db,dc,dw,dcomp}) if (p) aclrtFree(p);
}

int main(int argc, char** argv) {
    int dev = 0; if (argc > 1) dev = std::atoi(argv[1]);
    aclrtContext ctx=nullptr;
    aclInit(nullptr); aclrtSetDevice(dev); aclrtCreateContext(&ctx, dev); aclrtSetCurrentContext(ctx); aclrtCreateStream(&gSt);
    const int64_t M=64,N=256,K=5;
    Case("valid_fp16_control", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, 0.00002);
    Case("A_fp16_B_fp32_mismatch", M,N,K, ACL_FLOAT16,ACL_FLOAT,ACL_FLOAT16, {M,N},{N}, 0.00002);
    Case("C_rank1", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M*N},{N}, 0.00002);
    Case("C_col_mismatch", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N-1},{N}, 0.00002);
    Case("W_len_mismatch", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N-1}, 0.00002);
    Case("W_rank2", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{1,N}, 0.00002);
    Case("eMax_negative", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, -1.0);
    Case("eMax_zero", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, 0.0);
    Case("eMax_nan", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, std::numeric_limits<double>::quiet_NaN());
    Case("eMax_pos_inf", M,N,K, ACL_FLOAT16,ACL_FLOAT16,ACL_FLOAT16, {M,N},{N}, std::numeric_limits<double>::infinity());
    std::printf("{\"probe\":\"done\"}\n");
    aclrtDestroyStream(gSt); aclrtDestroyContext(ctx); aclrtResetDevice(dev); aclFinalize();
    return 0;
}
