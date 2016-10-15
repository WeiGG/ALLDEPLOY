#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cJSON.h"
#include "ad_file.h"

#define _AD_SHELL   "server-cfg.json"

//string 16
//int 8


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
    	printf("get name err!\n");
    	return -1;
	}
	
	//print item
	printf("name type is %d\n",name->type);
	printf("name is %s\n",name->valuestring);

	//free
	if (root)
  		cJSON_Delete(root);

}
