#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void process(){
    char cmd[100];
    scanf("%s", cmd);
    if(cmd[0] == 'p' && cmd[1] == '-'){
        printf("priority command\n");
    }
    else if(cmd[0] == 'n' && cmd[1] == '-'){
        printf("normal command\n");
    }
    else{
        printf("command error\n");       
    }
}

int main()
{
    while(1){
        process();
    }
}
