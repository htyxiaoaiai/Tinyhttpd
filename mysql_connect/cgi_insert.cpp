#include "sql_api.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE 1024

void myinsert(char *arg)
{
	sql_api * sql = new sql_api;

	string cols = "(name, school, hobby)";

	char *buf[4];
	buf[3] = NULL;
	int index = 2;

	 char * end = arg + strlen(arg)-1;
	 while(end > arg)
	 {
		 if(*end == '=')
		 {
			 buf[index--] = end+1;
		 }
		 if(*end == '&')
		 {
			 *end = '\0';
		 }
		 end--;
	 }

	 string data = "(\"";
	 data += buf[0];
	 data += "\",\"";
	 data += buf[1];
	 data += "\",\"";
	 data += buf[2];
	 data += "\")";


	 sql->my_connect_mysql();
	 sql->my_insert(cols,data);

	 delete sql;
}

int main()
{
	char method[SIZE];
	char arg[SIZE];
	char content_length[SIZE];
	int len = 0;
	if(getenv("REQUEST_METHOD"))
	{
		strcpy(method,getenv("REQUEST_METHOD"));
	}

	if(strcasecmp(method,"GET") == 0)
	{
		if(getenv("QUERY_STRING"))
			strcpy(arg,getenv("QUERY_STRING"));
	}
	else
	{
		if(getenv("CONTENT_LENGTH"))
		{
			strcpy(content_length,getenv("CONTENT_LENGTH"));
			len = atoi(content_length);
		}
	//获取参数
		int i = 0;
		for(; i<len; i++)
		{
			read(0,&arg[i],1);
		}
		arg[i]= '\0';
	}

	myinsert(arg);

	return 0;

}
