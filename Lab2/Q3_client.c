#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096
#define MAX_RETRIES 5

typedef struct {
    int seq_num;
    int ack_num;
    int data_len;
    char type[10];
    char data[BUFFER_SIZE];
} packet_t;

int send_reliable(int sockfd, struct sockaddr_in *addr, char *data, int len, int seq);
int wait_for_ack(int sockfd, int exp);
void set_timeout(int sockfd, int seconds);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in channel_addr;
    char filename[240];
    char out_file[256];
    FILE *output_fp;
    packet_t packet;
    int n, seq_num = 0;
    int total_packets = 0;
    int expected_seq = 1;
    
    if(argc != 4) {
        printf("Usage: %s <IP> <Port> <Filename>\n", argv[0]);
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket error");
        exit(1);
    }
    
    channel_addr.sin_family = AF_INET;
    channel_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &channel_addr.sin_addr);
    
    if(strlen(argv[3]) >= sizeof(filename)) {
        printf("Filename too long (max %zu characters)\n", sizeof(filename) - 1);
        exit(1);
    }
    
    strcpy(filename, argv[3]);
    snprintf(out_file, sizeof(out_file), "reliable_%s", filename);
    
    set_timeout(sockfd, 2);
    
    printf("Requesting file: %s through channel\n", filename);
    if(send_reliable(sockfd, &channel_addr, filename, strlen(filename), seq_num++) < 0) {
        printf("Failed to send filename request\n");
        exit(1);
    }
    
    total_packets++;
    
    output_fp = fopen(out_file, "wb");
    if(output_fp == NULL) {
        perror("Cannot create output file");
        exit(1);
    }
    
    printf("Receiving file and saving as: %s\n", out_file);
    
    while(1) {
        socklen_t len = sizeof(channel_addr);
        n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&channel_addr, &len);
        
        if(n < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Timeout waiting for data\n");
                break;
            }
            perror("recvfrom error");
            continue;
        }
        
        printf("DEBUG: Received packet seq=%d, expected=%d, type=%s, len=%d\n", 
               packet.seq_num, expected_seq, packet.type, packet.data_len);
        
        if(packet.data_len < 0 || packet.data_len > BUFFER_SIZE) {
            printf("Invalid data length: %d\n", packet.data_len);
            continue;
        }
        
        if(strcmp(packet.type, "ERROR") == 0) {
            printf("Server error: %s\n", packet.data);
            break;
        }
        
        if(strcmp(packet.type, "FIN") == 0) {
            printf("File transfer completed\n");
            packet_t ack;
            ack.seq_num = 0;
            ack.ack_num = packet.seq_num;
            strcpy(ack.type, "ACK");
            ack.data_len = 0;
            sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr*)&channel_addr, sizeof(channel_addr));
            break;
        }
        
        if(strcmp(packet.type, "DATA") == 0) {
            if(packet.seq_num == expected_seq) {
                if(packet.data_len > 0) {
                    fwrite(packet.data, 1, packet.data_len, output_fp);
                }
                expected_seq++;
                printf("Processed new packet seq=%d\n", packet.seq_num);
            } else {
                printf("Duplicate or out-of-order packet seq=%d (expected %d)\n",
                       packet.seq_num, expected_seq);
            }
            
            packet_t ack;
            ack.seq_num = 0;
            ack.ack_num = packet.seq_num;
            strcpy(ack.type, "ACK");
            ack.data_len = 0;
            sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr*)&channel_addr, sizeof(channel_addr));
            printf("Sent ACK for seq=%d\n", packet.seq_num);
        }
    }
    
    fclose(output_fp);
    close(sockfd);
    
    printf("\nStatistics:\n");
    printf("Total packets sent: %d\n", total_packets);
    printf("File saved as: %s\n", out_file);
    
    return 0;
}

int send_reliable(int sockfd, struct sockaddr_in *addr, char *data, int len, int seq) {
    packet_t packet;
    int retries = 0;
    
    packet.seq_num = seq;
    packet.data_len = len;
    strcpy(packet.type, "REQUEST");
    memcpy(packet.data, data, len);
    
    while(retries < 5) {
        sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)addr, sizeof(*addr));
        printf("Sent packet seq=%d (attempt %d)\n", seq, retries + 1);
        
        if(wait_for_ack(sockfd, seq) == 0) {
            printf("Received ACK for seq=%d\n", seq);
            return 0;
        }
        
        retries++;
        printf("Timeout or invalid ACK, retrying...\n");
    }
    
    printf("Max retries exceeded for seq=%d\n", seq);
    return -1;
}

int wait_for_ack(int sockfd, int exp) {
    packet_t ack;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    
    int n = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr*)&addr, &len);
    if(n < 0) return -1;
    
    if(strcmp(ack.type, "ACK") == 0 && ack.ack_num == exp) {
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
