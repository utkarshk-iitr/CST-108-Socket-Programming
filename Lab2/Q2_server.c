#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 4096

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen);

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;

    if(argc!=2){
        printf("Usage: %s <Port>\n",argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        perror("socket error");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd,(struct sockaddr*) &servaddr,sizeof(servaddr))<0){
        perror("bind error");
        exit(1);
    }

    printf("UDP File Server listening on port %d\n",atoi(argv[1]));
    dg_echo(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
    return 0;
}

void dg_echo(int sockfd,struct sockaddr* pcliaddr,socklen_t clilen){
    int n;
    socklen_t len;
    char mesg[BUFFER_SIZE];
    char filename[BUFFER_SIZE];
    char filebuf[BUFFER_SIZE];
    FILE *fp;
    size_t bytes_read;

    while(1){
        len = clilen;
        n = recvfrom(sockfd,mesg,BUFFER_SIZE,0,pcliaddr,&len);
        if(n<0){
            perror("recvfrom error");
            continue;
        }

        mesg[n] = '\0';
        strcpy(filename, mesg);
        if (filename[strlen(filename)-1]=='\n')
            filename[strlen(filename)-1] = '\0';

        printf("\nClient requested file: %s\n",filename);
        printf("Client address: %s:%d\n",inet_ntoa(((struct sockaddr_in*)pcliaddr)->sin_addr),ntohs(((struct sockaddr_in*)pcliaddr)->sin_port));
        fp = fopen(filename, "rb");

        if(fp==NULL){
            strcpy(filebuf,"ERROR: File not found");
            sendto(sockfd,filebuf,strlen(filebuf),0,pcliaddr,len);
            printf("File not found: %s\n",filename);
            continue;
        }

        fseek(fp,0,SEEK_END);
        long file_size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        
        sprintf(filebuf,"SIZE:%ld",file_size);
        sendto(sockfd,filebuf,strlen(filebuf),0,pcliaddr,len);        
        printf("Sending file: %s (Size: %ld bytes)\n",filename,file_size);

        while((bytes_read=fread(filebuf,1,BUFFER_SIZE-1,fp))>0){
            if(sendto(sockfd,filebuf,bytes_read,0,pcliaddr,len)!=bytes_read){
                perror("send to error");
                break;
            }
        }

        fclose(fp);
        strcpy(filebuf,"EOF");
        sendto(sockfd,filebuf,3,0,pcliaddr,len);
        printf("File transfer completed for: %s\n\n",filename);
    }
}
