#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void charatatime(char *);
int main(void){
    pid_t pid;
    pid = fork();
    if (pid == 0) charatatime("output from child\n");
    else charatatime("output from parent\n");
    exit(0);
}
static void charatatime(char *str){
    char *ptr;
    int c;
    setbuf(stdout, NULL);
    for(ptr = str; c = *ptr++; )
        putc(c, stdout);
}