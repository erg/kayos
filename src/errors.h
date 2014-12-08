#ifndef KAYOS_SRC_ERRORS_H_
#define KAYOS_SRC_ERRORS_H_

#include <jansson.h>

void libc_fatal_error(const char *msg);
void fatal_error(const char *msg);
void json_print_error(json_error_t error);
void json_fatal_error(const char *msg, json_error_t error);

#endif
