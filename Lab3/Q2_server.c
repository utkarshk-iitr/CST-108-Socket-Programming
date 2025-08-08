#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DB_FILE "users.db"
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

void to_hex(unsigned char *in,size_t len,char *out){
    for(size_t i=0;i<len;i++)
        sprintf(out+2*i,"%02x",in[i]);
    out[2*len] = '\0';
}

void handle_client(int cfd){
    char cmd[16],user[64],pass[64],file[256];

    if(read_line(cfd,cmd,sizeof(cmd))<=0){
        close(cfd);
        exit(0);
    }

    if(!strcmp(cmd,"REGISTER")){
        read_line(cfd,user,sizeof(user));
        read_line(cfd,pass,sizeof(pass));

        unsigned char salt[16], hash[SHA256_DIGEST_LENGTH];
        RAND_bytes(salt,sizeof(salt));

        size_t plen = strlen(pass);
        unsigned char *tmp = malloc(plen+sizeof(salt));
        memcpy(tmp,pass,plen);
        memcpy(tmp+plen,salt,sizeof(salt));
        SHA256(tmp,plen+sizeof(salt),hash);
        free(tmp);

        char salt_hex[33],hash_hex[65];
        to_hex(salt,sizeof(salt),salt_hex);
        to_hex(hash,SHA256_DIGEST_LENGTH,hash_hex);

        FILE *db = fopen(DB_FILE,"a");
        if(!db){
            dprintf(cfd,"ERROR\nCannot open DB\n");
        } 
        else{
            fprintf(db,"%s:%s:%s\n",user,salt_hex,hash_hex);
            fclose(db);
            dprintf(cfd,"OK\nRegistered %s\n",user);
        }
        printf("User %s registered successfully\n",user);

        close(cfd);
        exit(0);
    }

    if(!strcmp(cmd,"GET")){
        read_line(cfd,user,sizeof(user));
        read_line(cfd,pass,sizeof(pass));
        read_line(cfd,file,sizeof(file));

        FILE *db = fopen(DB_FILE,"r");
        if(!db){ 
            dprintf(cfd,"ERROR\nNo DB\n");
            close(cfd);
            exit(0);
        }

        int ok = 0;
        char line[MAXLINE],dbu[64],salt_hex[33],hash_hex[65];

        while(fgets(line,sizeof(line),db)){
            if(sscanf(line,"%63[^:]:%32[^:]:%64s",dbu,salt_hex,hash_hex)==3 && !strcmp(dbu,user)){
                unsigned char salt[16],expect[SHA256_DIGEST_LENGTH];
                for (int i=0;i<16;i++)
                    sscanf(salt_hex+2*i,"%2hhx",&salt[i]);
                for (int i=0;i<SHA256_DIGEST_LENGTH;i++)
                    sscanf(hash_hex+2*i,"%2hhx",&expect[i]);

                size_t plen = strlen(pass);
                unsigned char *tmp = malloc(plen+sizeof(salt));
                memcpy(tmp,pass,plen);
                memcpy(tmp+plen,salt,sizeof(salt));
                unsigned char got[SHA256_DIGEST_LENGTH];
                SHA256(tmp,plen+sizeof(salt),got);
                free(tmp);

                if(!memcmp(got,expect,SHA256_DIGEST_LENGTH)) ok = 1;
                break;
            }
        }
        fclose(db);

        if(!ok){
            dprintf(cfd,"ERROR\nBad credentials\n"); 
            close(cfd);
            exit(0);
        }
        
        int fd = open(file,O_RDONLY);
        if (fd<0){
            dprintf(cfd, "ERROR\nCannot open file\n");
            close(cfd);
            exit(0);
        }

        dprintf(cfd,"OK\n");
        char buf[1024];
        ssize_t n;
        while ((n=read(fd,buf,sizeof(buf)))>0) 
            write(cfd,buf,n);

        printf("File %s sent successfully\n",file);
        close(fd);
        exit(0);
    }

    dprintf(cfd,"ERROR\nUnknown command\n");
}

int main(int argc,char** argv){
    if(argc!=2){
        printf("Usage: %s <Port>\n",argv[0]);
        exit(1);
    }
    
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    if(lfd<0){
        perror("socket"); 
        exit(1); 
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(lfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("bind"); 
        exit(1);
    }

    listen(lfd,10);
    printf("Server ready on port %d \n", atoi(argv[1]));

    while(1){
        int cfd = accept(lfd,NULL,NULL);
        if(cfd<0){ 
            if(errno == EINTR) continue; 
            perror("accept"); 
            continue; 
        }
        if(!fork()){ 
            close(lfd); 
            handle_client(cfd); 
        }
        close(cfd);
    }
}
