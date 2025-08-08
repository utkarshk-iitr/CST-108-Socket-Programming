#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

void print_hex(unsigned char* data,int len){
    for(int i=0;i<len;i++)
        printf("%02x",data[i]);
}

int main(){
    unsigned char s[256];
    unsigned char rndm[16];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    printf("Enter a string: ");
    if(fgets(s,sizeof(s),stdin)==NULL){
        fprintf(stderr,"Error reading input\n");
        return 1;
    }
    
    int len = strlen(s);
    if(len>0 && s[len-1]=='\n'){
        s[len-1]='\0';
        len--;
    }
    
    if(RAND_bytes(rndm,16)!=1) {
        fprintf(stderr,"Error generating random bytes\n");
        return 1;
    }
    
    unsigned char* cnct = (unsigned char*)malloc(len+16);
    if(cnct==NULL){
        fprintf(stderr,"Memory allocation failed\n");
        return 1;
    }
    
    memcpy(cnct,s,len);
    memcpy(cnct+len,rndm,16);
    SHA256(cnct,len+16,hash);
    
    printf("\nUser String: %s\n",s);
    printf("Random Number: ");
    print_hex(rndm,16);
    printf("\nSHA256 Hash: ");
    print_hex(hash,SHA256_DIGEST_LENGTH);
    printf("\n");
    
    free(cnct);
    return 0;
}
