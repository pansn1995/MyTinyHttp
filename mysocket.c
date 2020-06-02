#include "mysocket.h"


void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}

int Socket(int __domain, int __type, int __protocol)
{
    int listenfd=socket(__domain,__type,__protocol);
    if(listenfd==-1)
        error_die("socket");
    return listenfd;
}

int Bind (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    int sock=bind(__fd,__addr,__len);
    if(sock<0)
        error_die("bind");
    return sock;
}

int Getsockname(int __fd, __SOCKADDR_ARG __addr,socklen_t *__restrict __len)
{
    int get=getsockname(__fd,__addr,__len);
    if(get==-1)
        error_die("getsockname");
    return get;
}

int Listen (int __fd, int __n)
{
    int get=listen(__fd,__n);
    if(get<0)
        error_die("listen");
    return get;
}

int Accept(int __fd, __SOCKADDR_ARG __addr,socklen_t *__restrict __addr_len)
{
    int get=accept(__fd,__addr,__addr_len);
    if(accept==-1)
        error_die("accept");
    return get;
}

int startup(u_short *port)
{
    int listenfd=0;
    struct sockaddr_in servaddr;

    listenfd=Socket(AF_INET,SOCK_STREAM,0);//初始化套接字,指定协议类型ipv4,流
    
    bzero(&servaddr,sizeof(servaddr));//清零
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(*port);

    Bind(listenfd,(SA* )(&servaddr),sizeof(servaddr));

    if(*port==0)//如果端口随机指定的，获取此时的端口
    {
        socklen_t  len = sizeof(servaddr);
        Getsockname(listenfd, (SA *)&servaddr, &len);
        *port = ntohs(servaddr.sin_port);
    }
    Listen(listenfd, 5) ;//监听，此时最大连接数为5
    return listenfd;
}
