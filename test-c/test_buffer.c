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

int test_buffer_find_eol() {
	int sum = 0;

	return sum;
}

int test_buffer_compact() {
	int sum = 0;
	char *ptr;
	
	char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
	int len = strlen(alphabet);

	ptr = buffer_take_while(alphabet, strlen(alphabet), "abc", 3);
	if(ptr - alphabet != 3) sum++;
	printf("ptr: %s\n", ptr);

	int filled  = compact_buffer(alphabet, len, ptr);

	if(filled != 23) sum++;
	printf("strlen: %ld, alphabet: %s\n", strlen(alphabet), alphabet);

	return sum;
}

int test_buffer_token() {
	int sum = 0;
	char *ptr;
	
	char alphabet[] = "  ab  cd  e  ";
	char *next = alphabet;
	char *end = alphabet + strlen(alphabet);
	//int len = strlen(alphabet);

	ptr = buffer_token(alphabet, end - alphabet, &next);
	if(next - alphabet != 5) sum++;
	printf("ptr: %s\n", ptr);

	ptr = buffer_token(next, end - next, &next);
	printf("ptr: %s\n", ptr);

	ptr = buffer_token(next, end - next, &next);
	printf("ptr: %s\n", ptr);
	ptr = buffer_token(next, end - next, &next);
	printf("ptr: %s\n", ptr);
	ptr = buffer_token(next, end - next, &next);
	printf("ptr: %s\n", ptr);
	return sum;
}

int main(int argc, char *argv[]) {
	int sum;
	sum = test_buffer_take_while();
	if(sum != 0)
		printf("test_buffer_take_while tests failed!\n");

	sum = test_buffer_skip_until();
	if(sum != 0)
		printf("test_buffer_skip_until tests failed!\n");

	sum = test_buffer_find_eol();
	if(sum != 0)
		printf("test_buffer_find_eol tests failed!\n");

	sum = test_buffer_compact();
	if(sum != 0)
		printf("test_buffer_compact tests failed!\n");

	printf("test_buffer_token starting!\n");
	sum = test_buffer_token();
	if(sum != 0)
		printf("test_buffer_token tests failed!\n");

	return 0;
}
