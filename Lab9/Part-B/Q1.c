#include <stdio.h>
#include <openssl/bio.h>
#include <string.h>

int main() {
    FILE *fp;
    BIO *file_bio;
    char buffer[100];
    int bytes_read;
    
    // (i)
    fp = fopen("Sample.txt","w+");
    // (ii)
    file_bio = BIO_new_fp(fp,BIO_CLOSE);
    
    // (iii)
    BIO_write(file_bio,"Hello World",11);
    BIO_flush(file_bio);
    
    // (iv)
    BIO_seek(file_bio,0);
    bytes_read = BIO_read(file_bio,buffer,sizeof(buffer)-1);
    if(bytes_read>0){
        buffer[bytes_read]='\0';
        printf("BIO_read output: %s\n",buffer);
    }
    
    // (v)
    fseek(fp,0,SEEK_SET);
    if(fgets(buffer,sizeof(buffer),fp)){
        printf("fgets output: %s\n", buffer);
    }
    
    // (vi)
    BIO_free(file_bio);
    return 0;
}