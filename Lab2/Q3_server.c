#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096
#define MAX_FILENAME 255

typedef struct {
    int seq_num;
    int ack_num;
    int data_len;
    char type[10];
    char data[BUFFER_SIZE];
} packet_t;

void handle_file_request(int sockfd, struct sockaddr_in *client_addr);
int send_data(int sockfd, struct sockaddr_in *addr, char *data, int len, int seq);
int wait_for_ack(int sockfd, int exp);
void set_timeout(int sockfd, int seconds);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr, client_addr;
    
    if(argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket error");
        exit(1);
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    
    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        exit(1);
    }
    
    set_timeout(sockfd, 2);
    printf("Reliable UDP File Server listening on port %d\n", atoi(argv[1]));
    
    while(1) {
        handle_file_request(sockfd, &client_addr);
    }
    
    return 0;
}

void handle_file_request(int sockfd, struct sockaddr_in *client_addr) {
    packet_t packet;
    socklen_t len = sizeof(*client_addr);
    int n, seq_num = 1;
    FILE *fp;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)client_addr, &len);
    if(n < 0) return;
    
    printf("DEBUG: Received request: seq=%d, type=%s, len=%d\n", 
           packet.seq_num, packet.type, packet.data_len);
    
    if(strcmp(packet.type, "REQUEST") != 0) {
        printf("Invalid packet type: %s\n", packet.type);
        return;
    }
    
    if(packet.data_len < 0 || packet.data_len >= MAX_FILENAME) {
        printf("Invalid filename length: %d\n", packet.data_len);
        return;
    }
    
    packet_t ack_packet;
    ack_packet.seq_num = 0;
    ack_packet.ack_num = packet.seq_num;
    strcpy(ack_packet.type, "ACK");
    ack_packet.data_len = 0;
    sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)client_addr, len);
    printf("Sent ACK for filename request\n");
    
    char filename[MAX_FILENAME + 1];
    strncpy(filename, packet.data, packet.data_len);
    filename[packet.data_len] = '\0';
    
    printf("Opening file: %s\n", filename);
    fp = fopen(filename, "rb");
    if(fp == NULL) {
        packet_t error_packet;
        error_packet.seq_num = seq_num;
        strcpy(error_packet.type, "ERROR");
        strcpy(error_packet.data, "File not found");
        error_packet.data_len = strlen(error_packet.data);
        sendto(sockfd, &error_packet, sizeof(error_packet), 0, (struct sockaddr*)client_addr, len);
        printf("File not found, sent error\n");
        return;
    }
    
    usleep(100000);
    
    printf("Sending file contents...\n");
    while((bytes_read = fread(buffer, 1, BUFFER_SIZE - 100, fp)) > 0) {
        if(send_data(sockfd, client_addr, buffer, bytes_read, seq_num++) < 0) {
            printf("Failed to send data packet seq=%d\n", seq_num - 1);
            break;
        }
        printf("Sent data packet seq=%d (%zu bytes)\n", seq_num - 1, bytes_read);
    }
    
    fclose(fp);
    
    if(send_data(sockfd, client_addr, "", 0, seq_num++) >= 0) {
        printf("File transfer completed successfully\n");
    }
    
    printf("Connection closed\n\n");
}

int send_data(int sockfd, struct sockaddr_in *addr, char *data, int len, int seq) {
    packet_t packet;
    int retries = 0;
    
    packet.seq_num = seq;
    packet.data_len = len;
    
    if(len == 0) {
        strcpy(packet.type, "FIN");
    } else {
        strcpy(packet.type, "DATA");
    }
    
    if(len > 0) {
        memcpy(packet.data, data, len);
    }
    
    printf("DEBUG: Sending packet seq=%d, type=%s, len=%d\n", 
           packet.seq_num, packet.type, packet.data_len);
    
    while(retries < 5) {
        sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)addr, sizeof(*addr));
        if(wait_for_ack(sockfd, seq) == 0) {
            return 0;
        }
        retries++;
        printf("Retransmitting seq=%d (attempt %d)\n", seq, retries);
    }
    
    return -1;
}

int wait_for_ack(int sockfd, int exp) {
    packet_t ack_packet;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    
    int n = recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&addr, &len);
    if(n < 0) return -1;
    
    if(strcmp(ack_packet.type, "ACK") == 0 && ack_packet.ack_num == exp) {
        return 0;
    }
    
    return -1;
}

void set_timeout(int sockfd, int seconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}
