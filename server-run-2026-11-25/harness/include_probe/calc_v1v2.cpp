#include "aclnn_obfuscation_calculate.h"
#include "aclnn_obfuscation_calculate_v2.h"
int use_both(void* a) { (void)aclnnObfuscationCalculateGetWorkspaceSize; (void)aclnnObfuscationCalculateV2GetWorkspaceSize; return 0; }
