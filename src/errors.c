#include "errors.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatal_error(const char *msg) {
	fprintf(stderr, "fatal_error: %s\n", msg);
	abort();
}

void libc_fatal_error(const char *msg) {
	fprintf(stderr, "libc fatal_error: %s\n", msg);
	fprintf(stderr, "errno = %d, strerrno: %s\n", errno, strerror(errno));
	abort();
}

void unimplemented(const char *msg) {
	fprintf(stderr, "unimplemented: %s\n", msg);
	abort();
}
