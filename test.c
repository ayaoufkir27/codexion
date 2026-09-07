#include <unistd.h>
#include <stdio.h>


int main(int ac, char **av) {
    int pid = fork();
    printf("pid %d\n", getpid());
}
