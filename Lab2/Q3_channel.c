#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

#define BUFFER_SIZE 4096
#define LOSS_RATE 5

typedef struct {
    int client_sock;
    int server_sock;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    struct sockaddr_in act_client_addr;
    struct sockaddr_in act_server_addr;
} channel_t;

void simulate_packet_loss(channel_t *channel);

int main(int argc, char **argv) {
    channel_t channel;
    
    if(argc != 4) {
        printf("Usage: %s <ClientPort> <ServerPort> <ActualServerPort>\n", argv[0]);
        exit(1);
    }
    
    srand(time(NULL));
    
    channel.client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    channel.server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(channel.client_sock < 0 || channel.server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    channel.client_addr.sin_family = AF_INET;
    channel.client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.client_addr.sin_port = htons(atoi(argv[1]));
    
    channel.server_addr.sin_family = AF_INET;
    channel.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.server_addr.sin_port = htons(atoi(argv[2]));
    
    if(bind(channel.client_sock, (struct sockaddr*)&channel.client_addr, sizeof(channel.client_addr)) < 0) {
        perror("Client bind failed");
        exit(1);
    }
    
    if(bind(channel.server_sock, (struct sockaddr*)&channel.server_addr, sizeof(channel.server_addr)) < 0) {
        perror("Server bind failed");
        exit(1);
    }
    
    channel.act_server_addr.sin_family = AF_INET;
    channel.act_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    channel.act_server_addr.sin_port = htons(atoi(argv[3]));
    
    printf("Channel Simulator started\n");
    printf("Client Port: %d, Server Port: %d, Actual Server: %d\n",
           atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    printf("Packet Loss Rate: 1 out of %d packets (%.1f%%)\n", LOSS_RATE, 100.0/LOSS_RATE);
    
    simulate_packet_loss(&channel);
    
    return 0;
}

void simulate_packet_loss(channel_t *channel) {
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    int n, max_fd;
    socklen_t len;
    int packet_recv = 0, packet_drop = 0, packet_fwd = 0;
    
    max_fd = (channel->client_sock > channel->server_sock) ? channel->client_sock : channel->server_sock;
    
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(channel->client_sock, &readfds);
        FD_SET(channel->server_sock, &readfds);
        
        if(select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select error");
            continue;
        }
        
        if(FD_ISSET(channel->client_sock, &readfds)) {
            len = sizeof(channel->act_client_addr);
            n = recvfrom(channel->client_sock, buffer, BUFFER_SIZE, 0,
                        (struct sockaddr*)&channel->act_client_addr, &len);
            
            if(n > 0) {
                packet_recv++;
                printf("Received packet from client (%d bytes)\n", n);
                
                if(rand() % LOSS_RATE == 0) {
                    packet_drop++;
                    printf("*** PACKET DROPPED (Client->Server) ***\n");
                } 
                else {
                    sendto(channel->server_sock, buffer, n, 0,
                          (struct sockaddr*)&channel->act_server_addr, sizeof(channel->act_server_addr));
                    packet_fwd++;
                    printf("Forwarded packet to server\n");
                }
            }
        }
        
        if(FD_ISSET(channel->server_sock, &readfds)) {
            len = sizeof(channel->act_server_addr);
            n = recvfrom(channel->server_sock, buffer, BUFFER_SIZE, 0,
                        (struct sockaddr*)&channel->act_server_addr, &len);
            
            if(n > 0) {
                packet_recv++;
                printf("Received packet from server (%d bytes)\n", n);
                
                if(rand() % LOSS_RATE == 0) {
                    packet_drop++;
                    printf("*** PACKET DROPPED (Server->Client) ***\n");
                } 
                else {
                    sendto(channel->client_sock, buffer, n, 0,
                          (struct sockaddr*)&channel->act_client_addr, sizeof(channel->act_client_addr));
                    packet_fwd++;
                    printf("Forwarded packet to client\n");
                }
            }
        }
        
        if(packet_recv > 0 && packet_recv % 10 == 0) {
            printf("\n=== STATISTICS ===\n");
            printf("Packets Received: %d\n", packet_recv);
            printf("Packets Dropped: %d (%.1f%%)\n", packet_drop, 100.0 * packet_drop / packet_recv);
            printf("Packets Forwarded: %d\n", packet_fwd);
            printf("==================\n\n");
        }
    }
}
