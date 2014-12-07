#ifndef KAYOS_SRC_UTILS_H_
#define KAYOS_SRC_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <jansson.h>

void hexdump(FILE *stream, const char *buf, const uint64_t len);
void named_hexdump(FILE *stream, const char *title, const char *buf, const uint64_t len);

void nano_sleep_impl(struct timespec *rqtp);
void nano_sleep(unsigned long long nanos);
void micro_sleep(unsigned long long micros);
void millis_sleep(unsigned long long millis);
void seconds_sleep(unsigned long long seconds);

void libc_fatal_error(const char* msg);
void fatal_error(const char* msg);
void json_print_error(json_error_t error);
void json_fatal_error(const char* msg, json_error_t error);

#endif
