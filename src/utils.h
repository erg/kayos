#ifndef KAYOS_SRC_UTILS_H_
#define KAYOS_SRC_UTILS_H_

#include <cstdint>

#include "endian_types.h"


be_uint32_t crc32(const void *buf, uint64_t len);
be_uint64_t crc64(const void *buf, uint64_t len);
void hexdump(const unsigned char *buf, const uint64_t len);
void named_hexdump(const char *title, const unsigned char *buf, const uint64_t len);

void nano_sleep_impl(struct timespec *rqtp);
void nano_sleep(unsigned long long nanos);
void micro_sleep(unsigned long long micros);
void millis_sleep(unsigned long long millis);
void seconds_sleep(unsigned long long seconds);

void fatal_error(const char* msg);

#endif
