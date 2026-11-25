#include "aclnn_obfuscation_setup.h"
#include "aclnn_obfuscation_setup_v2.h"
int use_both(void* a) { (void)aclnnObfuscationSetupGetWorkspaceSize; (void)aclnnObfuscationSetupV2GetWorkspaceSize; return 0; }
