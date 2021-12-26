#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
// #include <string>

char *input;

// The function to be executed by all threads
void *threadFun(int v)
{

    // Print the argument, static and global variables
    while (1)
    {
        printf("%d\n", v);
    }
}

// https://stackoverflow.com/questions/55640416/get-user-input-without-blocking-while-loop-using-threads-in-c

void *threadInput()
{
    int i;
    // Print the argument, static and global variables
    scanf("%d", &i);
    printf("your input: %d\n", i);
}

int main()
{
    int i;
    pthread_t tid;

    // Let us create three threads
    pthread_create(&tid, NULL, threadFun, 1);
    pthread_create(&tid, NULL, threadFun, 2);
    pthread_create(&tid, NULL, threadInput, NULL);

    pthread_exit(NULL);
    while (1)
        printf("333\n");
    return 0;
}
