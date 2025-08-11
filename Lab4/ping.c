#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;

    while(len>1){
        sum += *buf++;
        len -= 2;
    }
    if(len==1) 
        sum += *(unsigned char*)buf;
    
    sum = (sum>>16) + (sum&0xFFFF);
    sum += (sum>>16);
    return (unsigned short)(~sum);
}

int main(int argc,char *argv[]){
    if(argc!=2){
        fprintf(stderr,"Usage: %s <hostname>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    struct hostent* host = gethostbyname(argv[1]);
    if(!host){
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *(struct in_addr*)host->h_addr;

    printf("PING %s (%s): %d bytes of data.\n",argv[1],inet_ntoa(addr.sin_addr),64);

    int sockfd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if(sockfd<0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    for(int i=0;i<6;i++){
        char packet[64];
        memset(packet,0,sizeof(packet));

        struct icmp *icmp_hdr = (struct icmp*)packet;
        icmp_hdr->icmp_type = ICMP_ECHO;
        icmp_hdr->icmp_code = 0;
        icmp_hdr->icmp_id = getpid()&0xFFFF;
        icmp_hdr->icmp_seq = i+1;
        icmp_hdr->icmp_cksum = checksum(icmp_hdr,sizeof(packet));

        struct timeval start,end;
        gettimeofday(&start,NULL);

        if(sendto(sockfd,packet,sizeof(packet),0,(struct sockaddr*)&addr,sizeof(addr))<=0){
            perror("sendto");
            continue;
        }

        char recv_buf[1024];
        socklen_t addrlen = sizeof(addr);
        int bytes = recvfrom(sockfd,recv_buf,sizeof(recv_buf),0,(struct sockaddr*)&addr,&addrlen);

        if(bytes>0){
            gettimeofday(&end,NULL);
            double rtt = (end.tv_sec-start.tv_sec)*1000.0 + (end.tv_usec-start.tv_usec)/1000.0;
            printf("%d bytes from %s: icmp_seq=%d time=%.3f ms\n",bytes,inet_ntoa(addr.sin_addr),i+1,rtt);
        } 
        else{
            printf("Request timed out.\n");
        }

        sleep(1);
    }

    close(sockfd);
    return 0;
}