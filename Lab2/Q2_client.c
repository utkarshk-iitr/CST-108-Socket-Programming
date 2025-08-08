#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 4096
void dg_cli(FILE *fp,int sockfd,const struct sockaddr* pservaddr,socklen_t servlen,char *filename);

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

int main(int argc,char **argv){
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc != 4){
        printf("Usage: %s <IP Address> <Port> <FileName>\n",argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        perror("socket error");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr)<=0){
        printf("inet_pton error for %s\n",argv[1]);
        exit(1);
    }

    dg_cli(NULL,sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr),argv[3]);
    close(sockfd);
    return 0;
}

void dg_cli(FILE *fp,int sockfd,const struct sockaddr* pservaddr,socklen_t servlen,char *filename){
    int n;
    char sendline[BUFFER_SIZE],recvline[BUFFER_SIZE];
    char out_file[BUFFER_SIZE];
    FILE* output_fp;
    socklen_t len;
    struct sockaddr *preply_addr;
    long exp_size=0, recv_bytes=0;

    snprintf(out_file,sizeof(out_file),"udp_recv_%s",filename);
    preply_addr = malloc(servlen);
    sendto(sockfd,filename,strlen(filename),0,pservaddr,servlen);
    printf("Requested file: %s\n",filename);

    output_fp = fopen(out_file,"wb");
    if(output_fp==NULL){
        perror("Cannot create output file");
        free(preply_addr);
        return;
    }

    printf("Receiving file and saving as: %s\n", out_file);

    while(1){
        len = servlen;
        n = recvfrom(sockfd,recvline,BUFFER_SIZE,0,preply_addr,&len);
        if(n<0){
            perror("recvfrom error");
            break;
        }

        recvline[n] = '\0';
        if(strncmp(recvline,"ERROR",5)==0){
            printf("Server error: %s\n",recvline);
            fclose(output_fp);
            unlink(out_file);
            free(preply_addr);
            return;
        }

        if(strncmp(recvline,"SIZE:",5)==0){
            exp_size = atol(recvline+5);
            printf("Expected file size: %ld bytes\n",exp_size);
            continue;
        }

        if(strncmp(recvline,"EOF",3)==0){
            printf("File transfer completed.\n");
            break;
        }
        
        if(fwrite(recvline,1,n,output_fp)!=n){
            perror("fwrite error");
            break;
        }

        recv_bytes += n;
        printf("Received: %ld/%ld bytes\r",recv_bytes,exp_size);
        fflush(stdout);
    }

    fclose(output_fp);
    free(preply_addr);
    printf("\nFile saved as: %s\n",out_file);
    diff(filename,out_file);
}