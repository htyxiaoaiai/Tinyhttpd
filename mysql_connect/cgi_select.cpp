#include "sql_api.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE 1024

void myselect()
{
	sql_api * sql = new sql_api;
	 sql->my_connect_mysql();
	 sql->my_select();

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

	myselect();

	return 0;

}
