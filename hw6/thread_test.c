#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

uint16_t refAngle = 0;
char refAngleString[64];

void *threadUserInput(void* vargp)
{
    scanf("%s", refAngleString);
    refAngle = (uint16_t) atoi(refAngleString);
    printf("Angle is: %d\n", refAngle);

    return NULL;
}


int main(void)
{
   pthread_t thread_id;

   while(1) {
       pthread_create(&thread_id, NULL, threadUserInput, NULL);
       printf("do smth\n");
       pthread_join(thread_id, NULL);

       // Other functions were called below ...
   }
}

