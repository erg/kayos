#include "buffer.h"

#include <stdlib.h>

// Binary protocol: opcode, 8byte length
// Ascii protocol: set, get,
// HTTP protocol
// Long poll

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

//void compact_buffer(char *buffer, char *ptr, uint64_t n_to_copy) {
    //memmove(buffer, ptr, n_to_copy);
//}
