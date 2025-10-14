#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>

#define BUFFER_SIZE 4096

void str_cli(BIO *sockfd, char *filename);
void diff(char *file1, char *file2);

int main(int argc, char **argv) {
    BIO *connect_bio;
    
    if (argc != 4) {
        printf("Usage: %s <IP Address> <Port> <FileName>\n", argv[0]);
        exit(1);
    }
    
    connect_bio = BIO_new(BIO_s_connect());
    BIO_set_conn_hostname(connect_bio, argv[1]);
    BIO_set_conn_port(connect_bio, argv[2]);
    BIO_do_connect(connect_bio);
    
    printf("Connected to server %s:%s\n", argv[1], argv[2]);
    str_cli(connect_bio, argv[3]);
    BIO_free(connect_bio);
    return 0;
}

void str_cli(BIO *sockfd, char *filename) {
    char sendline[BUFFER_SIZE], recvline[BUFFER_SIZE];
    char out_file[BUFFER_SIZE];
    FILE *output_fp;
    int n;
    
    snprintf(out_file, sizeof(out_file), "recv_%s", filename);
    snprintf(sendline, sizeof(sendline), "%s\n", filename);
    
    BIO_write(sockfd, sendline, strlen(sendline));
    BIO_flush(sockfd);
    
    n = BIO_read(sockfd, recvline, BUFFER_SIZE - 1);
    if (n <= 0) {
        printf("Server closed connection\n");
        return;
    }
    
    recvline[n] = '\0';
    if (strncmp(recvline, "ERROR", 5) == 0) {
        printf("%s", recvline);
        return;
    }
    
    output_fp = fopen(out_file, "w");
    if (output_fp == NULL) {
        perror("Cannot create output file");
        return;
    }
    
    printf("Receiving file and saving as: %s\n", out_file);
    
    if (strcmp(recvline, "OK\n") != 0) {
        fputs(recvline, output_fp);
    }
    
    while ((n = BIO_read(sockfd, recvline, BUFFER_SIZE - 1)) > 0) {
        recvline[n] = '\0';
        fputs(recvline, output_fp);
    }
    
    fclose(output_fp);
    printf("File transfer completed\n");
    diff(filename, out_file);
}

void diff(char *file1, char *file2) {
    char command[512];
    int result;
    
    snprintf(command, sizeof(command), "diff \"%s\" \"%s\"", file1, file2);
    printf("Executing: %s\n", command);
    result = system(command);
    
    if (result == 0) {
        printf("Files are identical\n");
    } else if (result == 256) {
        printf("Files differ\n");
    } else {
        printf("Error occurred (exit code: %d)\n", result);
    }
}
