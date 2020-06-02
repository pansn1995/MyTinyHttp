#include <sys/types.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include "mysocket.h"
#include "http.h"

int main(int argc, char *argv[]){
    int listenfd,connfd;
    u_short port=0;
    struct sockaddr_in clientaddr;
    if(argc>1)//通过命令行指定运行端口
        port=atoi(argv[1]);

    listenfd=startup(&port);
    printf("当前服务器运行在端口: %d\n", port);
    
    pthread_t newthread;//线程
    socklen_t len=sizeof(clientaddr);
    while(1)
    {
        connfd=Accept(listenfd,(SA*)&clientaddr,&len);//三次握手成功
        
        //建立线程处理客户端请求
        if(pthread_create(&newthread,NULL,accept_request,(void *)(intptr_t)connfd)!=0)
            perror("pthread_create");
    }
    return 0;
}