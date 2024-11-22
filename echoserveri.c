#include "csapp.h"
void echo(int connfd);

int main(int argc,char **argv){
    int listenfd,connfd;
    socklen_t clientlen;
    char client_hostname[MAXLINE],client_port[MAXLINE];
    struct sockaddr_storage clientaddr;

    if (argc!=2){
        fprintf(stderr,"usage:%s <host> <port>\n",argv[0]);
        exit(0);
    }


    listenfd = Open_listenfd(argv[1]);
//    printf("Server listening on port %d\n", port);


    while(1){
        clientlen= sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd,(SA *)&clientaddr,&clientlen);
        Getnameinfo((SA*)&clientaddr,clientlen,client_hostname,MAXLINE,client_port,MAXLINE,0);
        printf("%s:%s connects the server\n",client_hostname,client_port);

        echo(connfd);
        Close(connfd);

    }
    return 0;
}