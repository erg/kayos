#ifndef KAYOS_SRC_UTILS_H_
#define KAYOS_SRC_UTILS_H_

#include <cstdint>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include "endian_types.h"

// http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

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
