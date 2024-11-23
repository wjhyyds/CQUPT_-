#include "csapp.h"
//void echo(int connfd);

typedef struct {
    int connfd;
} thread_arg_t;
void *echo(void *arg) {
    thread_arg_t *thread_arg = (thread_arg_t *) arg;
    int connfd = thread_arg->connfd;
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    socklen_t clientlen;


    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n",(int)n);
        printf("client: %s", buf);

        // 检查是否为退出命令
        if (strcmp(buf, "bye\n") == 0) {
            printf("client quits...\n");
            break;
        }

        Rio_writen(connfd, buf, n);
    }

    close(connfd);
    free(arg);

    return NULL;
}

int main(int argc,char **argv){
    int listenfd,connfd;
    char buf[MAXLINE];
    rio_t rio;
    socklen_t clientlen;
    char client_hostname[MAXLINE],client_port[MAXLINE];
    struct sockaddr_storage clientaddr;
    pthread_t tid; //create a pthread
    if (argc!=2){
        fprintf(stderr,"usage:%s <host> <port>\n",argv[0]);
        exit(0);
    }
    int port = atoi(argv[1]);

    listenfd = Open_listenfd(argv[1]);
//    printf("Server listening on port %d\n", port);


    while(1){
        clientlen= sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd,(SA *)&clientaddr,&clientlen);
        Getnameinfo((SA*)&clientaddr,clientlen,client_hostname,MAXLINE,client_port,MAXLINE,0);
        printf("%s:%s connects the server\n",client_hostname,client_port);
        thread_arg_t *thread_arg = malloc(sizeof(thread_arg_t));
        thread_arg->connfd = connfd;

        pthread_create(&tid, NULL, echo, thread_arg);
        pthread_detach(tid);

    }
    return 0;
}