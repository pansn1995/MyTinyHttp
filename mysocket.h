#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#define SA struct sockaddr

void error_die(const char *sc);//出错显示

int Socket(int , int , int );//封装socket

int Bind (int , __CONST_SOCKADDR_ARG , socklen_t );//封装bind

int Getsockname(int , __SOCKADDR_ARG ,socklen_t * );//封装getsocketname

int Listen (int , int );//封装listen

int Accept(int , __SOCKADDR_ARG ,socklen_t * );//封装accept

int startup(u_short *);//socket初始化工作