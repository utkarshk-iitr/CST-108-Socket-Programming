#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void diff(const char *file1,const char *file2){
    char command[512];
    int result;
    snprintf(command,sizeof(command),"diff \"%s\" \"%s\"",file1,file2);
    printf("Executing: %s\n",command);
    result = system(command);

    if(result==0) printf("Files are identical\n");
    else if(result==256) printf("Files differ\n");
    else printf("Error occurred (exit code: %d)\n",result);
}

void encrypt_file(const char *input_file,const char *output_file,const unsigned char *key,const unsigned char *iv,const EVP_CIPHER *cipher) {
    EVP_CIPHER_CTX *ctx;
    FILE *infile, *outfile;
    unsigned char inbuf[BUFFER_SIZE];
    unsigned char outbuf[BUFFER_SIZE + EVP_CIPHER_block_size(cipher)];
    int bytes_read, len;
    int ct_len = 0;
    
    infile = fopen(input_file,"rb");
    if(!infile){
        fprintf(stderr,"ERROR: Cannot open input file\n");
        return;
    }
    
    outfile = fopen(output_file,"wb");
    if(!outfile){
        fprintf(stderr,"ERROR: Cannot open output file\n");
        fclose(infile);
        return;
    }
    
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx,cipher, NULL, key, iv);

    while((bytes_read = fread(inbuf,1,BUFFER_SIZE, infile))>0){
        EVP_EncryptUpdate(ctx,outbuf,&len,inbuf,bytes_read);
        fwrite(outbuf,1,len,outfile);
        ct_len += len;
    }
    
    EVP_EncryptFinal_ex(ctx,outbuf,&len);
    if(len>0){
        fwrite(outbuf,1,len,outfile);
        ct_len += len;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    fclose(infile);
    fclose(outfile);
}

void decrypt_file(const char *input_file,const char *output_file,const unsigned char *key,const unsigned char *iv,const EVP_CIPHER *cipher) {
    EVP_CIPHER_CTX *ctx;
    FILE *infile, *outfile;
    unsigned char inbuf[BUFFER_SIZE];
    unsigned char outbuf[BUFFER_SIZE + EVP_CIPHER_block_size(cipher)];
    int bytes_read,len;
    int pt_len = 0;
    
    infile = fopen(input_file,"rb");
    if(!infile){
        fprintf(stderr,"ERROR: Cannot open input file\n");
        return;
    }
    
    outfile = fopen(output_file,"wb");
    if(!outfile){
        fprintf(stderr,"ERROR: Cannot open output file\n");
        fclose(infile);
        return;
    }
    
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);
    
    while((bytes_read = fread(inbuf, 1, BUFFER_SIZE, infile))>0){
        EVP_DecryptUpdate(ctx, outbuf, &len, inbuf, bytes_read);
        fwrite(outbuf, 1, len, outfile);
        pt_len += len;
    }
    
    EVP_DecryptFinal_ex(ctx, outbuf, &len);    
    if(len>0){
        fwrite(outbuf, 1, len, outfile);
        pt_len += len;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    fclose(infile);
    fclose(outfile);
}


int main(int argc, char *argv[]) {
    unsigned char key[32];
    unsigned char iv[16];
    const char *input_file = "plaintext.txt";
    const char *encrypted_file = "ciphertext.bin";
    const char *decrypted_file = "decrypted.txt";

    clock_t start, end;
    double time_taken;
    
    printf("\nUsing AES-256-CBC:\n");
    RAND_bytes(key,32);
    RAND_bytes(iv,16);
    printf("Generated Key and IV\n");
    start = clock();
    encrypt_file(input_file, encrypted_file, key, iv, EVP_aes_256_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Encryption: %f seconds\n", time_taken);
    start = clock();
    decrypt_file(encrypted_file, decrypted_file, key, iv,EVP_aes_256_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Decryption: %f seconds\n", time_taken);
    diff(input_file, decrypted_file);

    printf("\nUsing AES-128-CBC:\n");
    RAND_bytes(key,16);
    RAND_bytes(iv,16);
    printf("Generated Key and IV\n");
    start = clock();
    encrypt_file(input_file, encrypted_file, key, iv, EVP_aes_128_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Encryption: %f seconds\n", time_taken);
    start = clock();
    decrypt_file(encrypted_file, decrypted_file, key, iv,EVP_aes_128_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Decryption: %f seconds\n", time_taken);
    diff(input_file, decrypted_file);

    printf("\nUsing AES-192-CBC:\n");
    RAND_bytes(key,24);
    RAND_bytes(iv,16);
    printf("Generated Key and IV\n");
    start = clock();
    encrypt_file(input_file, encrypted_file, key, iv, EVP_aes_192_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Encryption: %f seconds\n", time_taken);
    start = clock();
    decrypt_file(encrypted_file, decrypted_file, key, iv,EVP_aes_192_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Decryption: %f seconds\n", time_taken);
    diff(input_file, decrypted_file);

    printf("\nUsing 3-DES:\n");
    RAND_bytes(key,24);
    RAND_bytes(iv,8);
    printf("Generated Key and IV\n");
    start = clock();
    encrypt_file(input_file, encrypted_file, key, iv, EVP_des_ede3_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Encryption: %f seconds\n", time_taken);
    start = clock();
    decrypt_file(encrypted_file, decrypted_file, key, iv,EVP_des_ede3_cbc());
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Decryption: %f seconds\n", time_taken);
    diff(input_file, decrypted_file);

    OPENSSL_cleanse(key, 32);
    OPENSSL_cleanse(iv, 16);
    return 0;
}
