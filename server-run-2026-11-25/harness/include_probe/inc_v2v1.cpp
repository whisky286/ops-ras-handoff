#include "aclnn_obfuscation_setup_v2.h"
#include "aclnn_obfuscation_setup.h"
int use_both2(void* a) { (void)aclnnObfuscationSetupV2GetWorkspaceSize; (void)aclnnObfuscationSetupGetWorkspaceSize; return 0; }
