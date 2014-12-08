#ifndef KAYOS_SRC_SLEEP_H_
#define KAYOS_SRC_SLEEP_H_

#include <time.h>

void nano_sleep_impl(struct timespec *rqtp);
void nano_sleep(unsigned long long nanos);
void micro_sleep(unsigned long long micros);
void millis_sleep(unsigned long long millis);
void seconds_sleep(unsigned long long seconds);

#endif
