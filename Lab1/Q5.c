#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main() {
    sigset_t set,pending;
    int cnt = 0;

    sigemptyset(&set);
    sigaddset(&set,SIGALRM);
    sigaddset(&set,SIGINT);
    sigprocmask(SIG_BLOCK,&set,NULL);

    printf("Signals blocked, setting alarm and sleeping\n");
    alarm(10);
    sleep(300);

    sigpending(&pending);
    printf("\nPending signals:\n");

    if(sigismember(&pending,SIGALRM)){
        printf("  SIGALRM is pending\n");
        cnt++;
    }
    if(sigismember(&pending,SIGINT)){
        printf("  SIGINT is pending\n");
        cnt++;
    }
    printf("Total: %d signals pending\n",cnt);

    return 0;
}
