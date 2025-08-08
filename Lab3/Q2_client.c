#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 512

ssize_t read_line(int fd,char *buf,size_t sz){
    ssize_t n,tot = 0;
    while(tot+1<(ssize_t)sz && (n=read(fd,buf+tot,1))==1){
        if(buf[tot]=='\n') break;
        tot++;
    }
    if(n<0) return -1;
    buf[tot] = '\0';
    return tot;
}

int main(int argc,char **argv) {
    if (argc!=3){
        fprintf(stderr,"Usage: %s <Server_IP> <Port>\n",argv[0]);
        exit(1);
    }

    int sfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET,argv[1],&sa.sin_addr);

    if(connect(sfd,(struct sockaddr*)&sa,sizeof(sa))<0){
        perror("connect"); exit(1);
    }

    printf("\n=========== MENU ===========\n"
           "1. Register new user\n"
           "2. Fetch file (auth required)\n"
           "============================\n"
           "Choice: ");

    int choice;
    scanf("%d", &choice); 
    getchar();

    char user[64],pass[64],file[256],resp[MAXLINE];

    if(choice == 1){
        printf("New login    : "); 
        fgets(user,sizeof(user),stdin);
        printf("New password : "); 
        fgets(pass,sizeof(pass),stdin);
        user[strcspn(user,"\n")] = '\0';
        pass[strcspn(pass,"\n")] = '\0';

        dprintf(sfd,"REGISTER\n%s\n%s\n",user,pass);

        if(read_line(sfd,resp,sizeof(resp))>0)
            printf("> %s\n", resp);
        close(sfd);
        return 0;
    }

    printf("Login ID : "); 
    fgets(user,sizeof(user),stdin);
    printf("Password : "); 
    fgets(pass,sizeof(pass),stdin);
    printf("Filename : "); 
    fgets(file,sizeof(file),stdin);
    user[strcspn(user,"\n")] = '\0';
    pass[strcspn(pass,"\n")] = '\0';
    file[strcspn(file,"\n")] = '\0';

    dprintf(sfd,"GET\n%s\n%s\n%s\n",user,pass,file);

    if(read_line(sfd,resp,sizeof(resp))<=0){
        puts("No reply"); 
        close(sfd); 
        exit(1);
    }

    if(strcmp(resp,"OK")){
        if(read_line(sfd,resp,sizeof(resp))>0)
            fprintf(stderr,"> %s\n",resp);
        close(sfd); 
        exit(1);
    }

    char out_file[512];
    snprintf(out_file,sizeof(out_file),"recv_%s",file);
    FILE *fd = fopen(out_file,"wb");
    if(fd<0){ 
        perror("open"); 
        close(sfd); 
        exit(1); 
    }

    char buf[1024];
    ssize_t n;
    while((n = read(sfd,buf,sizeof(buf)))>0) 
        fwrite(buf,1,n,fd);

    printf("> File '%s' received ",out_file);
    if(n==0) printf("successfully \n");
    else printf("failed \n");

    fclose(fd);
    close(sfd);
    return 0;
}
