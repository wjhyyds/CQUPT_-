#include "csapp.h"
void echo(int connfd){

//    thread_arg_t *thread_arg = (thread_arg_t *) arg;
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio,connfd);
    while ((n= Rio_readlineb(&rio,buf,MAXLINE))!=0){
        printf("server received %d bytes\n",(int)n);
        // printf("%s", buf);

        // // 检查是否为退出命令
        // if (strcmp(buf, "bye\n") == 0) {
        //     break;
        // }

        Rio_writen(connfd, buf, strlen(buf));
    }
}