#include <stdio.h>
#include <openssl/bio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd;
    BIO *file_bio;
    char buffer[100];
    int bytes_read;
    
    // (i)
    fd = open("Sample.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    
    // (ii)
    file_bio = BIO_new_fd(fd, BIO_NOCLOSE);
    
    // (iii)
    BIO_write(file_bio,"Hello World",11);
    
    // (iv)
    BIO_seek(file_bio,0);
    bytes_read = BIO_read(file_bio,buffer,sizeof(buffer)-1);
    if (bytes_read>0){
        buffer[bytes_read]='\0';
        printf("BIO_read output: %s\n",buffer);
    }
    
    // (v)
    lseek(fd,0,SEEK_SET);
    bytes_read = read(fd,buffer,sizeof(buffer)-1);
    if (bytes_read > 0){
        buffer[bytes_read]='\0';
        printf("read output: %s\n",buffer);
    }
    
    // (vi)
    BIO_free(file_bio);
    close(fd);
    return 0;
}