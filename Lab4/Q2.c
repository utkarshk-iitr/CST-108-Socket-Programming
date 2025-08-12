#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

int main(){
    int raw_socket;
    char buffer[65536];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);
    
    raw_socket = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if(raw_socket<0){
        perror("Failed to create raw socket");
        return 1;
    }
    
    printf("ICMP packet capture started. Press Ctrl+C to stop.\n");
    printf("%-15s %-15s %-10s %-10s\n","Source IP","IP Hdr Len","ICMP Type","ICMP Code");
    printf("--------------------------------------------------------\n");
    
    while (1){
        int packet_size = recvfrom(raw_socket,buffer,sizeof(buffer),0,(struct sockaddr*)&source_addr,&addr_len);
        if(packet_size<0){
            perror("Failed to receive packet");
            continue;
        }
        
        struct iphdr *ip_header = (struct iphdr*)buffer;
        int ip_len = ip_header->ihl*4;
        if(packet_size<ip_len+sizeof(struct icmphdr)){
            continue;
        }
        
        struct icmphdr *icmp_header = (struct icmphdr*)(buffer+ip_len);
        struct in_addr source_ip;
        source_ip.s_addr = ip_header->saddr;
        char *source_ip_str = inet_ntoa(source_ip);

        printf("%-15s %-15d %-10d %-10d\n",source_ip_str,ip_len,icmp_header->type,icmp_header->code);
        fflush(stdout);
    }
    
    close(raw_socket);
    return 0;
}