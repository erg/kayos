#include "buffer.h"

#include <stdlib.h>
#include <string.h>

// Binary protocol: opcode, 8byte length
// Ascii protocol: set, get,
// HTTP protocol
// Long poll

char *buffer_skip_tabspace(char *buffer, uint64_t buffer_length) {
	return buffer_take_while(buffer, buffer_length, " \t", 2);
}

char *buffer_skip_whitespace(char *buffer, uint64_t buffer_length) {
	return buffer_take_while(buffer, buffer_length, " \t\r\n\0", 5);
}

char *buffer_find_eol(char *buffer, uint64_t buffer_length) {
	char *end = buffer + buffer_length;
	char *ptr = buffer_skip_until(buffer, buffer_length, "\r\n", 2);
	return buffer_take_while(ptr, end - ptr, "\r\n", 2);
}

char *buffer_take_while(char *buffer, uint64_t buffer_length, char *take, uint64_t take_length) {
	if(!buffer)
		return 0;
	uint64_t i = 0, j = 0;

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

char *buffer_skip_until(char *buffer, uint64_t buffer_length, char *until, uint64_t until_length) {
	if(!buffer)
		return 0;
	uint64_t i = 0, j = 0;

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

uint64_t compact_buffer(char *buffer, uint64_t len, char *ptr) {
    uint64_t diff = ptr - buffer;
    uint64_t filled = len - diff;
    memmove(buffer, ptr, filled);
	memset(buffer + filled, 0, diff);
	//printf("memsetting: buffer %p, ptr %p, filled %d, diff %d\n", buffer, ptr, filled, diff);
	return filled;
}
