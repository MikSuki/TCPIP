#include <stdio.h>

void test(char* mac){
    int a[6];
	sscanf(mac, "%x:%x:%x:%x:%x:%x", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
	printf("%02X\n", a[0]);
	printf("%02X\n", a[1]);
	printf("%02X\n", a[2]);
	printf("%02X\n", a[3]);
	printf("%02X\n", a[4]);
	printf("%02X\n", a[5]);
}

int main(int argc, char* argv[]) {
	// char mac[] = "00:13:a9:1f:b0:88";
    test(argv[1]);
	
	return 0;
}
