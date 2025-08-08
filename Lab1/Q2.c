#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>

int killed=0;
void handle_sigchld(int sig){
    int status;
    pid_t pid;

    while((pid=waitpid(-1,&status,WNOHANG))>0){
        if(WIFEXITED(status)){
            killed++;
            printf("Killed child %d, exit status = %d\n",pid,WEXITSTATUS(status));
        }
    }
}

int main(void){
    signal(SIGCHLD,handle_sigchld);

    for(int i=0;i<5;i++){
        pid_t pid = fork();
        if(pid<0){
            perror("Fork failed");
            exit(1);
        }
        if(pid==0){
            exit(i);
        }
    }

    while(killed<5)
        pause();
    printf("All children killed, parent exiting.\n");

/* 
    The output is that, each child is created one by one and destroyed 
    sequentially. Also it is likely possible that the childs are assigned
    pid that are consecutive in nature.
*/
    return 0;
}
