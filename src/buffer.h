#ifndef KAYOS_SRC_BUFFER_H_
#define KAYOS_SRC_BUFFER_H_

#include <stdlib.h>

char *buffer_take_while(char *buffer, uint64_t buffer_length, char *take, uint64_t take_length);
char *buffer_skip_until(char *buffer, uint64_t buffer_length, char *until, uint64_t until_length);

#endif
