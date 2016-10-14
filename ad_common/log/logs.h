
/*
########################################


########################################
*/

#define _NO_LOG			0
#define _DEBUG_LOG		1
#define _INFO_LOG		2
#define _WARNING_LOG	3
#define _ERROR_LOG		4

extern int logLevel[5];
void ad_log(const char *file, int line, int level, int status, const char *fmt, ...);
