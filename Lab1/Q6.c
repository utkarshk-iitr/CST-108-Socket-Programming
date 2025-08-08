#include <stdio.h>
#include <stdlib.h>                                                                                                                                                                                                                                             
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int sigflag = 0;
void sig_usr(int signo){
    sigflag = 1;
}

void WAIT_PARENT(){
    while(!sigflag)
        pause();
}

void TELL_CHILD(pid_t pid){
    kill(pid,SIGUSR1);
}

int main(){
    signal(SIGUSR1,sig_usr);
    pid_t pid;

    if((pid=fork())<0)
        printf("Fork failed\n");

    else if(pid==0){
        WAIT_PARENT();
        printf("Child: Received signal from parent\n");
        exit(0);
    }

    printf("Parent: Sending signal to child\n");
    TELL_CHILD(pid);
    wait(NULL);
    printf("Parent: Signal sent to child, exiting\n");
    return 0;
}