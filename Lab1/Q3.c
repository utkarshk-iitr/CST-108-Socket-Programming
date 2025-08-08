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

void alarm_handler(int sig){}

int main(void){
    signal(SIGCHLD,handle_sigchld);

    for(int i=0;i<5;i++){
        pid_t pid = fork();
        if(pid<0){
            perror("Fork failed");
            exit(1);
        }
        if(pid==0){
            signal(SIGALRM,alarm_handler);
            if(i){
                alarm(i*10);
                pause();
            }
            exit(i);
        }
    }

    while(killed<5)
        pause();
    printf("All children killed, parent exiting.\n");

/* 
    Yes the output of Question 3 is similar to Question 2, the only difference
    is that in Question 3 the output the delayed whereas in Question 2 the output
    is not delayed.
*/
    return 0;
}
