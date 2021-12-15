#include <stdio.h>
#include <pcap.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char **argv) {
    int arr[] = {1, 2, 3, 4, 5};
    int *p = arr;
    for(int i = 0; i < 6; ++i)
        printf("%d ", p[i]);
    printf("\n");
    
    return 0;
}