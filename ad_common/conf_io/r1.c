#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cJSON.h"

#define _AD_SHELL   "test.json"

int open_file(int *pf)
{
    char    fileName[1024];

    memset(fileName, 0, sizeof(fileName));
//    sprintf(fileName, "%s/ad_shells/%s", getenv("HOME"), _AD_SHELL);
    sprintf(fileName, "%s/%s", ".", _AD_SHELL);

    *pf = open(fileName, O_RDONLY);
    if(*pf < 0)
        return -1;

    return 0;
}

int main(int argc, char *argv[])
{
    int pf = 0;
    char strjson[102400];
    memset(strjson, 0, sizeof(strjson));

    if(open_file(&pf) < 0)
        return -1;

    read(pf,strjson,sizeof(strjson));

	//get string
	cJSON *root = cJSON_Parse(strjson); 

	//get item
	cJSON *name = cJSON_GetObjectItem(root, argv[1]);
	if(!name)
	{
    	printf("No name !\n");
    	return -1;
	}
	
	//print item
	printf("name type is %d\n",name->type);
	printf("name is %s\n",name->valuestring);

	//free
	if (root)
  		cJSON_Delete(root);

}
