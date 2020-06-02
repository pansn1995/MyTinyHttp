#include <sys/socket.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
//是否为空格
#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"

void accept_request(void* );//解析客户端请求

int get_line(int , char *, int );//从连接中读获取一行数据

void unimplemented(int );//读取未识别的方法——不是get也不是post

void not_found(int );//未在本地找到请求的web文件

void serve_file(int , const char *);//发送文件内容给客户端

void headers(int , const char *);//加入http的首部

void cat(int, FILE *);//读取文件

void execute_cgi(int , const char *,const char *, const char *);//执行cgi脚本文件

void bad_request(int );//在cgi中的post请求中没有找到content-length，发送报错信息

void cannot_execute(int );//在cgi中无法建立管道或者fork失败