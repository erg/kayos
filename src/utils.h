#ifndef KAYOS_SRC_UTILS_H_
#define KAYOS_SRC_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <time.h>

void hexdump(FILE *stream, const char *buf, const uint64_t len);
void named_hexdump(FILE *stream, const char *title, const char *buf, const uint64_t len);

void nano_sleep_impl(struct timespec *rqtp);
void nano_sleep(unsigned long long nanos);
void micro_sleep(unsigned long long micros);
void millis_sleep(unsigned long long millis);
void seconds_sleep(unsigned long long seconds);

void close_stdout();

void fatal_error(const char* msg);

#endif
