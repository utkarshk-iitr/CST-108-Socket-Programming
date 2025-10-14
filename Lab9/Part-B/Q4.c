#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

int main() {
    BIO *file_bio, *buffer_bio, *cipher_bio, *bio_chain;
    unsigned char key[32] = "01234567890123456789012345678901";
    unsigned char iv[16] = "0123456789012345";
    char decrypted_buffer[100];
    int bytes_read;
    
    file_bio = BIO_new_file("Sample.bin","rb");
    
    buffer_bio = BIO_new(BIO_f_buffer());
    cipher_bio = BIO_new(BIO_f_cipher());

    BIO_set_cipher(cipher_bio,EVP_aes_256_cbc(),key,iv,0);
    BIO_push(buffer_bio,file_bio);
    BIO_push(cipher_bio,buffer_bio);
    bio_chain = cipher_bio;
    
    bytes_read = BIO_read(bio_chain,decrypted_buffer,sizeof(decrypted_buffer)-1);
    if(bytes_read>0){
        decrypted_buffer[bytes_read]='\0';
        printf("Decrypted string: %s\n",decrypted_buffer);
    }
    
    BIO_free_all(bio_chain);
    return 0;
}