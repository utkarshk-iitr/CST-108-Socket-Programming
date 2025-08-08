#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigint_handler(int signum){
    printf(" CTRL-C pressed\n");
}

int main(){
    signal(SIGINT, sigint_handler);
    while(1);
/*
    We are unable to terminate the program by pressing CTRL-C, no matter 
    how many times we press it the program will not terminate. Our signal 
    is catched by the handler and corresponding function is called. In the 
    defined function there is only print statement so we get the output in 
    the terminal and no exit happens. 

    In my OS (Linux Mint), I don't need to manually restart the handler after
    the interrupt has been called, so my OS support reliable signals.
*/
    return 0;
}
