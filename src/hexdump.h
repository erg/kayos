#ifndef KAYOS_SRC_HEXDUMP_H_
#define KAYOS_SRC_HEXDUMP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void hexdump(FILE *stream, const char *buf, const uint64_t len);

void named_hexdump(FILE *stream, const char *title,
	const char *buf, const uint64_t len);

#endif
