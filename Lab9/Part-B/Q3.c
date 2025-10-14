#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

int main() {
    BIO *file_bio, *buffer_bio, *cipher_bio, *bio_chain;
    unsigned char key[32] = "01234567890123456789012345678901";
    unsigned char iv[16] = "0123456789012345";
    
    // (i)
    file_bio = BIO_new_file("Sample.bin","wb");
    
    // (ii)
    buffer_bio = BIO_new(BIO_f_buffer());
    cipher_bio = BIO_new(BIO_f_cipher());
    
    // (iii)
    BIO_set_cipher(cipher_bio,EVP_aes_256_cbc(),key,iv,1);
    
    // (iv)
    BIO_push(buffer_bio,file_bio);
    BIO_push(cipher_bio,buffer_bio);
    bio_chain = cipher_bio;
    
    // (v)
    BIO_write(bio_chain,"Hello World!",12);
    BIO_flush(bio_chain);
    
    BIO_free_all(bio_chain);    
    printf("Encrypted data written to Sample.bin\n");
    return 0;
}