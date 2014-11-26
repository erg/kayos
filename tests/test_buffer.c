#include <stdio.h>
#include <string.h>
#include "buffer.h"

int test_buffer_take_while() {
	int sum = 0;
	char *ptr;
	char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

	ptr = buffer_take_while(alphabet, strlen(alphabet), "z", 1);
	if(ptr - alphabet != 0) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_take_while(alphabet, strlen(alphabet), "abc", 3);
	if(ptr - alphabet != 3) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_take_while(alphabet, strlen(alphabet), "z", 1);
	if(ptr - alphabet != 0) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_take_while(alphabet, strlen(alphabet), " ", 1);
	if(ptr - alphabet != 0) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_take_while(alphabet, strlen(alphabet), alphabet, strlen(alphabet));
	if(ptr != 0) sum++;
	printf("ptr: %s\n", ptr);

	return sum;
}

int test_buffer_skip_until() {
	int sum = 0;
	char *ptr;
	char alphabet[] = "   abcdef   ghi  jkl  mnopqrstuvwxyz\n";

	ptr = buffer_skip_until(alphabet, strlen(alphabet), " ", 1);
	if(ptr - alphabet != 0) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_skip_until(alphabet, strlen(alphabet), "a", 1);
	if(ptr - alphabet != 3) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_skip_until(alphabet, strlen(alphabet), "z", 1);
	if(ptr - alphabet != strlen(alphabet) - 2) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_skip_until(alphabet, strlen(alphabet), alphabet, strlen(alphabet));
	if(ptr - alphabet != 0) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_skip_until(alphabet, strlen(alphabet), "5", 1);
	if(ptr != 0) sum++;
	printf("ptr: %s\n", ptr);

	return sum;
}


int main(int argc, char *argv[]) {
	int sum;
	sum = test_buffer_take_while();
	if(sum != 0)
		printf("buffer tests failed!\n");

	sum = test_buffer_skip_until();
	if(sum != 0)
		printf("buffer tests failed!\n");
	
	return 0;
}
