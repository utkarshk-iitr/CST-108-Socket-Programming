#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>

int killed=0;
void print_rusage(){
    struct rusage self, child;
    getrusage(RUSAGE_SELF,&self);
    getrusage(RUSAGE_CHILDREN,&child);

    double user = self.ru_utime.tv_sec + self.ru_utime.tv_usec/1e6;
    double sys  = self.ru_stime.tv_sec + self.ru_stime.tv_usec/1e6;
    double child_user = child.ru_utime.tv_sec + child.ru_utime.tv_usec/1e6;
    double child_sys  = child.ru_stime.tv_sec + child.ru_stime.tv_usec/1e6;

    printf("\nResource usage statistics:\n");
    printf("Parent:   user = %.6fs, system = %.6fs\n", user, sys);
    printf("Children: user = %.6fs, system = %.6fs\n", child_user, child_sys);
}

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
    print_rusage();
    return 0;
}