#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <openssl/bio.h>

#define BUFFER_SIZE 4096

void sig_chld(int signo);
void str_echo(BIO *client_bio);

int main(int argc, char **argv) {
    BIO *accept_bio, *client_bio;
    pid_t pid;
    
    if (argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        exit(1);
    }
    
    signal(SIGCHLD, sig_chld);
    
    accept_bio = BIO_new_accept(argv[1]);    
    BIO_set_bind_mode(accept_bio, BIO_BIND_REUSEADDR);
    BIO_do_accept(accept_bio);
    
    printf("Server listening on port %s\n", argv[1]);

    while (1){
        if(BIO_do_accept(accept_bio)<=0){
            break;
        }
        
        client_bio = BIO_pop(accept_bio);
        if (!client_bio) {
            fprintf(stderr, "Error: BIO_pop returned NULL\n");
            continue;
        }
        
        printf("New connection accepted\n");
        
        pid = fork();
        if(pid < 0){
            perror("Fork error");
            BIO_free(client_bio);
            continue;
        }
        
        if (pid == 0) {
            BIO_free(accept_bio);
            str_echo(client_bio);
            BIO_free(client_bio);
            exit(0);
        }
        else {
            BIO_free(client_bio);
        }
    }
    
    BIO_free(accept_bio);
    return 0;
}

void str_echo(BIO *client_bio) {
    int n;
    char filename[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    FILE *fp;
    
    n = BIO_read(client_bio, filename, BUFFER_SIZE - 1);
    if (n <= 0) {
        fprintf(stderr, "Child %d: Error reading filename\n", getpid());
        return;
    }
    
    filename[n - 1] = '\0';
    printf("Child %d: Client requested file: %s\n", getpid(), filename);
    
    fp = fopen(filename, "r");
    if (fp == NULL) {
        strcpy(line, "ERROR: File not found\n");
        BIO_write(client_bio, line, strlen(line));
        BIO_flush(client_bio);
        return;
    }
    
    strcpy(line, "OK\n");
    BIO_write(client_bio, line, strlen(line));
    BIO_flush(client_bio);
    
    while (fgets(line, BUFFER_SIZE, fp) != NULL) {
        if (BIO_write(client_bio, line, strlen(line)) != strlen(line)) {
            fprintf(stderr, "Child %d: Write error\n", getpid());
            break;
        }
    }
    
    BIO_flush(client_bio);
    fclose(fp);
    printf("Child %d: File transfer completed for: %s\n", getpid(), filename);
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("Child process %d terminated with status %d\n\n", 
               pid, WEXITSTATUS(stat));
    }

    signal(SIGCHLD, sig_chld);
}
