#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Binary protocol: opcode, 8byte length
// Long poll

char *buffer_skip_tabspace(char *buffer, size_t buffer_length) {
	return buffer_take_while(buffer, buffer_length, " \t", 2);
}

char *buffer_skip_whitespace(char *buffer, size_t buffer_length) {
	return buffer_take_while(buffer, buffer_length, " \t\r\n", 4);
}

char *buffer_find_eol(char *buffer, size_t buffer_length) {
	char *end = buffer + buffer_length;
	char *ptr = buffer_skip_until(buffer, buffer_length, "\r\n", 2);
	return buffer_take_while(ptr, end - ptr, "\r\n", 2);
}

char *buffer_token(char *buffer, size_t buffer_length, char **next) {
	if(!buffer && !*next) {
		return 0;
	}
	char *end = buffer + buffer_length;
	char *ptr = buffer_skip_whitespace(buffer, buffer_length);
	char *found = buffer_skip_until(ptr, end - ptr, " \t\r\n", 4);
#ifdef DEBUG
	fprintf(stderr, "end: %p\n", end);
	fprintf(stderr, "ptr: %p\n", ptr);
	fprintf(stderr, "found: %p\n", found);
#endif
	if(found)
		*found = '\0';
	if(found < end)
		*next = found + 1;
	else
		*next = 0;
	return ptr;
}

// Returns 0 on out of bounds
char *buffer_take_while(char *buffer, size_t buffer_length, char *take, size_t take_length) {
	if(!buffer)
		return 0;
	size_t i = 0, j = 0;

	for(i = 0; i < buffer_length; i++) {
		for(j = 0; j < take_length; j++) {
			if(buffer[i] == take[j])
				goto buffer_next;
		}
		break;
buffer_next:
		continue;
	}
	if(i == buffer_length)
		return 0;
	return buffer + i;
}

// Returns 0 on out of bounds
char *buffer_skip_until(char *buffer, size_t buffer_length, char *until, size_t until_length) {
	if(!buffer)
		return 0;
	size_t i = 0, j = 0;

	for(i = 0; i < buffer_length; i++) {
		for(j = 0; j < until_length; j++) {
			if(buffer[i] == until[j])
				goto end;
		}
	}
	if(i == buffer_length)
		return 0;
end:
	return buffer + i;
}

// pointer to buffer head, length of entire buffer, pointer to last processed char
size_t compact_buffer(char *buffer, size_t len, char *ptr) {
	size_t diff = ptr - buffer;
	size_t filled = len - diff;
	if(!ptr) {
		memset(buffer, 0, len);
		return 0;
	}
#ifdef DEBUG
	fprintf(stderr, "memsetting: buffer %p, ptr %p, filled %zu, diff %zu\n", buffer, ptr, filled, diff);
#endif
	memmove(buffer, ptr, filled);
	memset(buffer + filled, 0, diff);
	return filled;
}
