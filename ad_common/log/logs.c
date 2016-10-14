
/*
########################################


########################################
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "logs.h"

#define _DEBUG_FILE 	"ad_debug_logs.log"
#define _MAX_STRING_LEN 		10240

#define _NO_LOG         0
#define _DEBUG_LOG      1
#define _INFO_LOG       2
#define _WARNING_LOG    3
#define _ERROR_LOG		4

int logLevel[5] = {_NO_LOG, _DEBUG_LOG, _INFO_LOG, _WARNING_LOG, _ERROR_LOG};
char logName[5][20] = {"NO_LOG", "DEBUG_LOG", "INFO_LOG", "WARNING_LOG", "ERROR_LOG"};

static int log_get_time(char *strTime)
{
	struct tm*	tmTime = NULL;
	size_t 		tmLen = 0;
	time_t 		tTimeN = 0;

	tTimeN = time(NULL);
	tmTime = localtime(&tTimeN);
	tmLen = strftime(strTime, 33, "%Y.%m.%d %H:%M:%S", tmTime);

	return tmLen;
}

static int log_open_file(int *pf)
{
	char 	fileName[1024];

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "%s/log/%s", getenv("HOME"), _DEBUG_FILE);

	*pf = open(fileName, O_WRONLY|O_CREAT|O_APPEND,0666);
	if(*pf < 0)
		return -1;
	
	return 0;
}

static void err_core(const char *file, int line, int level, int status, const char *fmt, va_list args)
{
	char	str[_MAX_STRING_LEN];
	int		strLen = 0;
	char 	tmpStr[64];
	int 	tmpStrLen = 0;
	int		pf = 0;

	memset(str, 0, _MAX_STRING_LEN);
	memset(tmpStr, 0, 64);

	tmpStrLen = log_get_time(tmpStr);
	tmpStrLen = sprintf(str+strLen, "%s ", logName[level]);
	strLen += tmpStrLen;

	if(status != 0)
		tmpStrLen = sprintf(str+strLen, "ERROR is %d ", status);
	else
		tmpStrLen = sprintf(str+strLen, "SUCCESS ! ");

	strLen += tmpStrLen;

	tmpStrLen = vsprintf(str+strLen, fmt, args);
	strLen += tmpStrLen;

	tmpStrLen = sprintf(str+strLen, "%s ", file);
	strLen += tmpStrLen;

	tmpStrLen = sprintf(str+strLen, "%d\n", line);
	strLen += tmpStrLen;

	if(log_open_file(&pf))
		return;

	write(pf, str, strLen);

	close(pf);

	return;
}

void ad_log(const char *file, int line, int level, int status, const char *fmt, ...)
{
	va_list args;
	
	if(level == _NO_LOG)
		return;

	va_start(args, fmt);
	err_core(file, line, level, status, fmt, args);
	va_end(args);

	return;
}

int main(void)
{
	return 0;
}
