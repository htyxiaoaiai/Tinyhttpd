#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SIZE 1024
//输出错误信息
static void echo_errno(int sock)
{
	printf("sock :%d\n",sock);
}

void Usge(const char* proc)
{
	printf("Usge %s [IP] [PORT] \n",proc);
}

//create socket
int startUp(char* _ip,char* _port)
{
	int sock =socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(atoi(_port));
	local.sin_addr.s_addr = inet_addr(_ip);

	int opt =1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind");
		exit(2);
	}

	if(listen(sock,5) < 0)
	{
		perror("listen");
		exit(3);
	}

	return sock;
}

//读取一行
static int get_line(int sock, char buf[],int len)
{
	if(sock < 0 || !buf)
	{
		echo_errno(sock);
		return -1;
	}

	int i = 0;
	char c ='\0';
	int ret = 0;
	//\n->\n  \r->\n \r\n->\n
	while(i < len-1 && c != '\n')
	{
		ret = recv(sock,&c,1,0);
		if(ret > 0)
		{
			if(c == '\r')
			{
				ret = recv(sock,&c,1,MSG_PEEK);//窥探功能，只查看不取走
				if(ret > 0 && c == '\n')//\r\n -> \n
					recv(sock,&c,1,0);
				else//\r -> \n
					c = '\n';
			}

			buf[i++] = c; 
		}
		else
		{
			c = '\n';//break
		}
	}

	buf[i] = '\0';
	return i;
}

//清理头部信息，只剩下请求内容
static void clear_header(int sock)
{
	char buf[SIZE];
	int len = SIZE;
	int ret = 0;
	do
	{
		ret = get_line(sock,buf,len);
	}while(ret > 0 && strcmp(buf,"\n")!=0 );
}

//输出页面，作出响应
static void echo_www(int sock, const char* path,ssize_t size)
{
	int fd = open(path,O_RDONLY);
	if(fd < 0)
	{
		echo_errno(sock);
		return;
	}

	char status_line[SIZE];
	sprintf(status_line,"HTTP/1.0 200 OK\r\n\r\n");
	send(sock,status_line,strlen(status_line),0);

	if(sendfile(sock,fd,NULL,size) < 0)
	{
		echo_errno(sock);
		close(fd);
		return;
	}
	close(fd);
}

//运行程序
static void exe_cgi(int sock,const char* method, const char* path,const char* query_string)
{
	char buf[SIZE];
	int content_length = - 1;
	int ret = -1;
	//相对于子进程来说，建立双向管道
	int cgi_input[2];
	int cgi_output[2];
	//GET的参数在URL中
	if(strcasecmp(method,"GET") == 0)
	{
		clear_header(sock);
	}
	//POST的参数在消息内
	else
	{
		do
		{
			ret = get_line(sock,buf,sizeof(buf));
			if(strncasecmp(buf,"Content-Length: ",16) == 0)
			{
				//从第16个开始比较Content-Length: 占16个字节
				content_length = atoi(&buf[16]);
			}
		}while(ret > 0 && strcmp(buf,"\n") != 0);

		printf("content_length%d\n",content_length);

		if(content_length == -1)
		{
			echo_errno(sock);
		}
	}

	//作出响应
	sprintf(buf,"HTTP/1.0 200 OK\r\n\r\n");
	send(sock,buf,strlen(buf),0);
	
	//存储环境变量
	char method_env[SIZE];
	char query_string_env[SIZE];
	char content_length_env[SIZE];
	
	if(pipe(cgi_input) < 0)
	{
		echo_errno(sock);
		return;
	}

	if(pipe(cgi_output) < 0)
	{
		echo_errno(sock);
		return;
	}

	//创建子进程
	pid_t id = fork();
	if(id == 0)
	{
		//子进程
		close(cgi_input[1]);
		close(cgi_output[0]);
		
		//将输入输出重定向到标准输入，标准输出
		dup2(cgi_input[0],0);
		dup2(cgi_output[1],1);
		
		//将方法，参数等放入到环境变量，让程序可以运行
		sprintf(method_env,"REQUEST_METHOD=%s",method);
		putenv(method_env);

		if(strcasecmp(method,"GET") == 0)
		{
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);
		}
		else
		{
			sprintf(content_length_env,"CONTENT_LENGTH=%d",content_length);
			putenv(content_length_env);
		}
		//程序替换，执行程序
		execl(path,path,NULL);
		exit(1);
	}
	else if(id > 0)
	{
		//父进程
		close(cgi_input[0]);
		close(cgi_output[1]);
		//读取content_length数据发送给子进程

		char c ='\0';
		int i = 0;


		if(strcasecmp(method,"POST") == 0)
		{
			for(; i<content_length; i++)
			{
				recv(sock,&c,1,0);
				printf("%c",c);
				write(cgi_input[1],&c,1);
			}
		}

		printf("\n");
		//读取子进程的运行结果
		int ret = 0;
		while(ret = read(cgi_output[0],&c,1) > 0)
		{
			send(sock,&c,1,0);
		}

		waitpid(id,NULL,0);
	}
}

static void * accept_request(void *arg)
{
	int sock = (int)arg;
	int ret = -1;
	char buf[SIZE];
	int len = sizeof(buf)/sizeof(buf[0]);

	char method[SIZE/10];
	char url[SIZE];
	char path[SIZE];
/*#ifdef _DEBUG_
	do
	{
		ret = get_line(sock,buf,len);
		printf("%s",buf);
		fflush(stdout);
	}while(ret > 0 && strcmp(buf,'\n') ! = 0);
	printf("client # %d\n",sock);

#endif
//上方用于测试get_line
*/

	//http请求行
	ret = get_line(sock,buf, len);
	if(ret < 0)
	{
		echo_errno(sock);
		return (void *)1;
	}
	int i = 0;//method index
	int j = 0;//buf index
	int cgi = 0;
	char * query_string = NULL;
	//获取method
	
	while((i < sizeof(method) -1) && (j <sizeof(buf)) && (!isspace(buf[j])))
	{
		method[i++] = buf[j++];
	}
	method[i] = '\0';

	printf("get method:%s\n",method);
	//判获取的method是否有效
	if(0 != strcasecmp(method,"GET") && (0 != strcasecmp(method,"POST")))
	{
		echo_errno(sock);
		return (void*)2;
	}

	//去除空格
	while(isspace(buf[j]))
	{
		j++;
	}
	 i = 0;
	//获取URL
	 while((i <sizeof(url)-1) && j<sizeof(buf) && (!isspace(buf[j])))
	 {
		 url[i] = buf[j];
		 i++;j++;
	 }
	 url[i] = '\0';

	 //POST模式参数在空行之后，消息体
	 if(strcasecmp(method,"POST") == 0)
	 {
		 cgi = 1;
	 }

	 //GET参数在URL中,需要提取
	 if(strcasecmp(method,"GET") == 0)
	 {
		 query_string = url;
		 while(*query_string != '\0' && *query_string != '?')
		 {
			 query_string++;
		 }
	 
		 if(*query_string == '?')
		 {
			 cgi = 1;
			 *query_string = '\0';
			 query_string++;
		 }
	 }

	sprintf(path,"htdoc%s",url);

	//根目录,则返回主页面
		 if(path[strlen(path)-1] == '/' )
		 {
			 strcat(path,"index.html");
		 }

		 //判断path是不是存在

		 struct stat st;
		 if(stat(path,&st) < 0)
		 {
			 echo_errno(sock);
		 }
		 else
		 {
			 //判断path是不是目录
			 if(S_ISDIR(st.st_mode))
			 {
				 strcat(path,"index.html");
			 }
			 //path是可执行文件
			 else if(S_IXGRP&st.st_mode || S_IXUSR&st.st_mode || S_IXOTH&st.st_mode)
			 {
				 cgi = 1;
			 }

			 //判断cgi
			 if(cgi == 1)
			 {
				 printf("begin runing exe_cgi...\n");
				 printf("path :%s\n",path);
				 exe_cgi(sock,method,path,query_string);
			 }
			 else
			 {
				 clear_header(sock);
				 printf("begin runing echo_www....\n");
				 echo_www(sock,path,st.st_size);
			 }
		 }

	 printf("accept_request %s\n",method);

	 close(sock);
	 return (void*)3;
}

int main(int argc,char* argv[])
{

	if(argc != 3)
	{
		Usge(argv[0]);
	}
	int listen_sock = startUp(argv[1],argv[2]);

	int done = 0;
	while(!done)
	{
		struct sockaddr_in peer;
		socklen_t len = sizeof(peer);
		int new_sock = accept(listen_sock,(struct sockaddr*)&peer,&len);
		if(new_sock < 0)
		{
			perror("accept");
			exit(1);
		}

		pthread_t id = 0;
		pthread_create(&id,NULL,accept_request,(void*)new_sock);
		pthread_detach(id);
	}
	return 0;
}
