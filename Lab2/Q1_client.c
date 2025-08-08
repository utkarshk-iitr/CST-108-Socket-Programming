#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 4096
void str_cli(FILE *fp,int sockfd,char *filename);

void diff(char *file1,char *file2){
    char command[512];
    int result;
    snprintf(command,sizeof(command),"diff \"%s\" \"%s\"",file1,file2);
    printf("Executing: %s\n",command);
    result = system(command);

    if(result==0) printf("Files are identical\n");
    else if(result==256) printf("Files differ\n");
    else printf("Error occurred (exit code: %d)\n",result);
}

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr, localaddr;
    socklen_t addrlen;
    
    if(argc!=4){
        printf("Usage: %s <IP Address> <Port> <FileName>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("Connection failed");
        exit(1);
    }

    addrlen = sizeof(localaddr);
    if(getsockname(sockfd,(struct sockaddr*)&localaddr,&addrlen)==0){
        printf("Client connected from local port: %d\n", ntohs(localaddr.sin_port));
    } else {
        perror("getsockname failed");
    }

    str_cli(NULL,sockfd,argv[3]);
    close(sockfd);
    return 0;
}

void str_cli(FILE *fp, int sockfd, char *filename){
    char sendline[BUFFER_SIZE], recvline[BUFFER_SIZE];
    char out_file[BUFFER_SIZE];
    FILE *output_fp;
    ssize_t n;
    
    snprintf(out_file,sizeof(out_file),"recv_%s",filename);
    snprintf(sendline,sizeof(sendline),"%s\n",filename);
    write(sockfd,sendline,strlen(sendline));
    
    if((n=read(sockfd,recvline,BUFFER_SIZE-1))<=0){
        printf("Server closed connection\n");
        return;
    }
    
    recvline[n] = '\0';
    
    if(strncmp(recvline,"ERROR",5)==0){
        printf("%s",recvline);
        return;
    }
    
    if((output_fp=fopen(out_file,"w"))==NULL) {
        perror("Cannot create output file");
        return;
    }
    
    printf("Receiving file and saving as: %s\n",out_file);
    if(strcmp(recvline,"OK\n")!=0){
        fputs(recvline,output_fp);
    }
    
    while((n=read(sockfd,recvline,BUFFER_SIZE-1))>0){
        recvline[n]='\0';
        fputs(recvline,output_fp);
    }
    
    fclose(output_fp);
    printf("File transfer completed\n");
    diff(filename,out_file);
}
