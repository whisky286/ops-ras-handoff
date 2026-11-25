#include "aclnn_obfuscation_calculate_v2.h"
#include "aclnn_obfuscation_calculate.h"
int use_both2(void* a) { (void)aclnnObfuscationCalculateV2GetWorkspaceSize; (void)aclnnObfuscationCalculateGetWorkspaceSize; return 0; }
