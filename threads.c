#include <pthread.h>
#include <stdio.h>

void *hello(void *arg)
{
    printf("Hello from the thread!\n");
    return (NULL);
}

int main(int ac, char **av)
{
    pthread_t thread;

    pthread_create(&thread, NULL, hello, NULL);
    pthread_join(thread, NULL);
}