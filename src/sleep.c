#include "sleep.h"

#include <errno.h>
#include <time.h>

#include "errors.h"

static void nano_sleep_impl(struct timespec *rqtp);

static void nano_sleep_impl(struct timespec *rqtp) {
	struct timespec rmtp;
	int ret = nanosleep(rqtp, &rmtp);
	if(ret == -1) {
		if(errno == EINTR) {
			nano_sleep_impl(&rmtp);
		} else {
			fatal_error("nano_sleep_impl failed");
		}
	}
}

void nano_sleep(unsigned long long nanos) {
	struct timespec rqtp;
	rqtp.tv_sec = nanos / 1000000000;
	rqtp.tv_nsec = nanos % 1000000000;
	nano_sleep_impl(&rqtp);
}

void micro_sleep(unsigned long long micros) {
	nano_sleep(micros * 1000);
}

void millis_sleep(unsigned long long millis) {
	nano_sleep(millis * 1000000);
}

void seconds_sleep(unsigned long long seconds) {
	nano_sleep(seconds * 1000000000);
}
