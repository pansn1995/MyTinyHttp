#include "http.h"

void accept_request(void* arg)
{
    int connfd=(intptr_t)arg;
    char buf[1024];//第一行报文内容
    int numchars;//第一行报文长度
    char method[255];//方法——GET/POST
    char url[255];//URI
    char path[512];//路径
    size_t i, j;
    struct stat st;
    int cgi = 0;  //cgi程序

    char *query_string = NULL;
    //根据上面的Get请求，可以看到这边就是取第一行
    //这边都是在处理第一条http信息
    //"GET / HTTP/1.1\n"
    numchars = get_line(connfd, buf, sizeof(buf));//读取方法、URI和协议版本，返回读取的字符数，写入缓冲区buf
    i = 0; j = 0;
    
    //提取方法
     while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
    {
        method[i] = buf[j];
        i++; j++;
    }
    method[i] = '\0';
     //判断是Get还是Post
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))//都不是
    {
        unimplemented(connfd);
        return ;
    }

    if (strcasecmp(method, "POST") == 0)//如果是POST方法cgi置1
        cgi=1;

    i = 0;
    while (ISspace(buf[j]) && (j < sizeof(buf)))//跳过空格
        j++;
    
     while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))//获取URI
    {
        url[i] = buf[j];
        i++; j++;
    }
    url[i] = '\0';

     if (strcasecmp(method, "GET") == 0)//如果是GET方法
    {
        query_string = url;
        while ((*query_string != '?') && (*query_string != '\0'))
            query_string++;
        if (*query_string == '?')
        {
            cgi = 1;
            *query_string = '\0';
            query_string++;
        }
    }
    sprintf(path, "htdocs%s", url);

    //默认地址，解析到的路径如果为/，则自动加上index.html
    if (path[strlen(path) - 1] == '/')
        strcat(path, "index.html");
    
    //没有找到文件
     if (stat(path, &st) == -1) {

        //把所有http信息读出然后丢弃
        while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
            numchars = get_line(connfd, buf, sizeof(buf));

        //发送未找到数据包给客户端
        not_found(connfd);
    }else{
        //如果是当前路径个目录，加上index.html
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            strcat(path, "/index.html");
        
        //如果你的文件默认是有执行权限的，自动解析成cgi程序，如果有执行权限但是不能执行，会接受到报错信号
        if ((st.st_mode & S_IXUSR) ||(st.st_mode & S_IXGRP) ||(st.st_mode & S_IXOTH) )
            cgi = 1;
        
        if (!cgi)
            serve_file(connfd, path);//接读取文件返回给请求的http客户端
        else
            execute_cgi(connfd, path, method, query_string);//执行cgi文件
    }
    close(connfd);
}  

int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);//读取一个字节
        //printf("%c", c); 
        if (n > 0)
        {
            if (c == '\r')
            {
                //窥看一个字节，如果是\n就读走
                n = recv(sock, &c, 1, MSG_PEEK);
                /* DEBUG printf("%02X\n", c); */
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';//不是\n（读到下一行的字符）或者没读到，置c为\n 跳出循环,完成一行读取
            }
            buf[i] = c;
            i++;
        }else//读取错误直接跳出循环或者没有数据
            c = '\n';
    }
    buf[i] = '\0';
    
    return (i);
}

void unimplemented(int connfd)
{
    char buf[1024];//发送缓冲区

    //协议版本、状态码、状态码的原因状语
    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(connfd, buf, strlen(buf), 0);

    //响应首部字段
    sprintf(buf, SERVER_STRING);
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(connfd, buf, strlen(buf), 0);

    //空行
    sprintf(buf, "\r\n");
    send(connfd, buf, strlen(buf), 0);

    //html主体内容
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(connfd, buf, strlen(buf), 0);
}

void not_found(int connfd)
{
    char buf[1024];//发送缓冲区

    //协议版本、状态码、状态码的原因状语
    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(connfd, buf, strlen(buf), 0);

    //响应首部字段
    sprintf(buf, SERVER_STRING);
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(connfd, buf, strlen(buf), 0);

    //空行
    sprintf(buf, "\r\n");
    send(connfd, buf, strlen(buf), 0);

    //html主体内容
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(connfd, buf, strlen(buf), 0);
}

void serve_file(int connfd, const char *filename)
{
    FILE *resource = NULL;
    int numchars = 1;
    char buf[1024];//缓冲区

    buf[0] = 'A'; buf[1] = '\0';
    //把所有http信息读出然后丢弃
    while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
        numchars = get_line(connfd, buf, sizeof(buf));

    resource = fopen(filename, "r");
    if (resource == NULL)
        not_found(connfd);
    else
    {
        headers(connfd, filename);
        cat(connfd, resource);
    }
    fclose(resource);
}

void headers(int connfd, const char *filename)
{
    char buf[1024];//缓冲区

    //协议版本、状态码、状态码的原因状语
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(connfd, buf, strlen(buf), 0);

    //响应首部字段
    strcpy(buf, SERVER_STRING);
    send(connfd, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(connfd, buf, strlen(buf), 0);

    //空行
    strcpy(buf, "\r\n");
    send(connfd, buf, strlen(buf), 0);
}

void cat(int connfd, FILE *resource)
{
    char buf[1024];//发送缓冲区

    fgets(buf, sizeof(buf), resource);
    //循环读，并发送给客户端
    while (!feof(resource))
    {
        send(connfd, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), resource);
    }
}

void execute_cgi(int connfd, const char *path, const char *method, const char *query_string)
{
    char buf[1024];//缓冲区

    //管道——进程间通信
    int cgi_output[2];
    int cgi_input[2];

    //进程pid和状态
    pid_t pid;
    int status;

    int i;
    char c;

    //读取的字符数
    int numchars = 1;

    //http的content_length
    int content_length = -1;

    //默认字符
    buf[0] = 'A'; buf[1] = '\0';

    if (strcasecmp(method, "GET") == 0)
    {
        //把所有http信息读出然后丢弃
        while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
            numchars = get_line(connfd, buf, sizeof(buf));
    }else{
        numchars = get_line(connfd, buf, sizeof(buf));
        //把所有http信息读出得到content-length
        while ((numchars > 0) && strcmp("\n", buf))
        {
            //如果是POST请求，就需要得到Content-Length，Content-Length：这个字符串一共长为15位，所以
            //取出头部一句后，将第16位设置结束符，进行比较
            //第16位置为结束
            buf[15] = '\0';
            //内存从第17位开始就是长度，将17位开始的所有字符串转成整数就是content_length
            if (strcasecmp(buf, "Content-Length:") == 0)
                content_length = atoi(&(buf[16]));
            numchars = get_line(connfd, buf, sizeof(buf));
        }
        //没有找到content-length
        if (content_length == -1) {
            bad_request(connfd);
            return;
        }
    }

    //协议版本、状态码、状态码的原因状语
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(connfd, buf, strlen(buf), 0);

     //建立output管道
    if (pipe(cgi_output) < 0) {
        cannot_execute(connfd);
        return;
    }
    //建立input管道
    if (pipe(cgi_input) < 0) {
        cannot_execute(connfd);
        return;
    }

    //fork失败
    if ( (pid = fork()) < 0 ) {
        cannot_execute(connfd);
        return;
    }

    if (pid == 0)  /* child: CGI script 子进程*/
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        //子进程输出重定向到output管道的1端
        dup2(cgi_output[1], 1);
        //子进程输入重定向到input管道的0端
        dup2(cgi_input[0], 0);

        //关闭无用管道口
        close(cgi_output[0]);
        close(cgi_input[1]);

        //CGI环境变量
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);
        if (strcasecmp(method, "GET") == 0) {
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }else {   /* POST */
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }
        //执行path
        execl(path, path, NULL);
        //int m = execl(path, path, NULL);
        //如果path有问题，例如将html网页改成可执行的，但是执行后m为-1
        //退出子进程，管道被破坏，但是父进程还在往里面写东西，触发Program received signal SIGPIPE, Broken pipe.
        exit(0);
    }else {    /* parent */
        //关闭无用管道口
        close(cgi_output[1]);
        close(cgi_input[0]);
        if (strcasecmp(method, "POST") == 0)
            for (i = 0; i < content_length; i++) {
                //得到post请求数据，写到input管道中，供子进程使用
                recv(connfd, &c, 1, 0);
                write(cgi_input[1], &c, 1);
            }
    
        //从output管道读到子进程处理后的信息，然后send出去
        while (read(cgi_output[0], &c, 1) > 0)
            send(connfd, &c, 1, 0);

        //完成操作后关闭管道
        close(cgi_output[0]);
        close(cgi_input[1]);

        //等待子进程返回
        waitpid(pid, &status, 0);
    }
}

void bad_request(int connfd)
{
    char buf[1024];//缓冲区

    //协议版本、状态码、状态码的原因状语
    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(connfd, buf, sizeof(buf), 0);

    //响应首部字段
    sprintf(buf, "Content-type: text/html\r\n");
    send(connfd, buf, sizeof(buf), 0);

    //空行
    sprintf(buf, "\r\n");
    send(connfd, buf, sizeof(buf), 0);

    //html主体内容
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(connfd, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(connfd, buf, sizeof(buf), 0);
}

void cannot_execute(int connfd)
{
    char buf[1024];

    //协议版本、状态码、状态码的原因状语
    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(connfd, buf, strlen(buf), 0);

    //响应首部字段
    sprintf(buf, "Content-type: text/html\r\n");
    send(connfd, buf, strlen(buf), 0);

    //空行
    sprintf(buf, "\r\n");
    send(connfd, buf, strlen(buf), 0);

    //html主体内容
    sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
    send(connfd, buf, strlen(buf), 0);
}