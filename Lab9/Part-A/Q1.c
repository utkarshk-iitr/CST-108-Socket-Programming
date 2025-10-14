#include <stdio.h>
#include <unistd.h>

int main(){
    printf("u");
    write(STDOUT_FILENO, "m", 1);
    printf("d\n");
}
