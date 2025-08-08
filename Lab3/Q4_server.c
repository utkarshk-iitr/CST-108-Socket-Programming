#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 4096
#define LISTENQ 1024
void sig_chld(int signo);
void str_echo(int sockfd);

int main(int argc, char **argv){
    int listenfd, connfd;
    pid_t pid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    
    if(argc!=2){
        printf("Usage: %s <Port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd, LISTENQ);
    
    signal(SIGCHLD, sig_chld);
    
    printf("Server listening on port %d\n",atoi(argv[1]));
    
    while(1){
        clilen = sizeof(cliaddr);
        if((connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen))<0){
            if(errno==EINTR) continue;
            else{
                perror("accept error");
                exit(1);
            }
        }
        
        if((pid=fork())==0){
            close(listenfd);  
            str_echo(connfd);
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
}

void str_echo(int sockfd){
    ssize_t n;
    char filename[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    FILE *fp;
    
    if((n=read(sockfd,filename,BUFFER_SIZE-1))<=0){
        perror("read filename error");
        return;
    }
    
    filename[n-1]='\0';
    printf("Child %d: Client requested file: %s\n", getpid(), filename);

    if((fp=fopen(filename,"r"))==NULL) {
        strcpy(line,"ERROR: File not found\n");
        write(sockfd,line,strlen(line));
        return;
    }
    
    strcpy(line,"OK\n");
    write(sockfd,line,strlen(line));

    while(fgets(line,BUFFER_SIZE,fp)!=NULL){
        if(write(sockfd,line,strlen(line))!=strlen(line)){
            perror("write error");
            break;
        }
    }
    fclose(fp);
    printf("Child %d: File transfer completed for: %s\n", getpid(), filename);
}

void sig_chld(int signo){
    pid_t pid;
    int stat;
    
    while((pid=waitpid(-1,&stat,WNOHANG))>0){
        printf("Child process %d terminated with status %d\n\n", pid, WEXITSTATUS(stat));
    }
    signal(SIGCHLD, sig_chld);
    return;
}