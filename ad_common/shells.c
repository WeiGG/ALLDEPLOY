#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define _AD_SHELL	".ad_shells"

int dupfile(int *pf)
{
	char    fileName[1024];

    memset(fileName, 0, sizeof(fileName));
//    sprintf(fileName, "%s/ad_shells/%s", getenv("HOME"), _AD_SHELL);
    sprintf(fileName, "%s/%s", ".", _AD_SHELL);

    *pf = open(fileName, O_WRONLY|O_CREAT|O_APPEND,0666);
    if(*pf < 0)
        return -1;

    return 0;
}


int main(int argc, char *argv[])
{
	char str[10240];
	memset(str, 0, 10240);
	int ad_strlen = argc-1;
	int retN = 0;
	int tmpret = 0;
	
	int num = 1;

	while(ad_strlen--)
	{
		printf("argv[num]: %s \n",argv[num]);
		if((tmpret = sprintf(str+retN,"%s ",argv[num++])) < 0)
			printf("error \n");

		retN += tmpret;
		printf("str: %s \n",str);
	}

	int pf = 0;
	int ret = dupfile(&pf);
	if(ret < 0)
		return -1;

	dup2(pf,STDOUT_FILENO);

	system(str);
}

