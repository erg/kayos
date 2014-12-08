#include "errors.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

void fatal_error(const char *msg) {
	fprintf(stderr, "fatal_error: %s\n", msg);
	abort();
}

void libc_fatal_error(const char *msg) {
	fprintf(stderr, "libc fatal_error: %s\n", msg);
	fprintf(stderr, "errno = %d, strerrno: %s\n", errno, strerror(errno));
	abort();
}

void json_print_error(json_error_t error) {
	fprintf(stderr, "json_error_t value:\n");
	fprintf(stderr, " text: \"%s\"\n", error.text);
	fprintf(stderr, " source: \"%s\"\n", error.source);
	fprintf(stderr, " line: %d\n", error.line);
	fprintf(stderr, " column: %d\n", error.column);
	fprintf(stderr, " position: %d\n", error.position);
}

void json_fatal_error(const char *msg, json_error_t error) {
	fprintf(stderr, "json_fatal_error: %s\n", msg);
	json_print_error(error);
	abort();
}
