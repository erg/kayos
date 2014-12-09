#ifndef KAYOS_SRC_BUFFER_H_
#define KAYOS_SRC_BUFFER_H_

#include <stdlib.h>

char *buffer_token(char *buffer, size_t buffer_length, char **next);
char *buffer_skip_tabspace(char *buffer, size_t buffer_length);
char *buffer_skip_whitespace(char *buffer, size_t buffer_length);
char *buffer_find_eol(char *buffer, size_t buffer_length);

char *buffer_take_while(char *buffer, size_t buffer_length,
	char *take, size_t take_length);

char *buffer_skip_until(char *buffer, size_t buffer_length,
	char *until, size_t until_length);

size_t compact_buffer(char *buffer, size_t n_to_copy, char *ptr);
#endif

