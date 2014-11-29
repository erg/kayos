#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void hexdump(FILE *stream, const char *buf, const uint64_t len) {
    if (!buf)
        return;
	int mod = 0;
    uint64_t i = 0;
    char asciidata[17];
    memset(asciidata, '.', 16);
    asciidata[16] = 0;
    for (i = 0; i < len; i++) {
		mod = i % 16;
		if(mod == 0)
			fprintf(stream, "%08llx: ", i);
		else if(mod == 8)
			fprintf(stream, "-");
		else
        	fprintf(stream, " ");

        fprintf(stream, "%02x", (unsigned char)(buf[i]));
        if ((isprint(buf[i])) && (!isspace(buf[i])))
            asciidata[i%16] = buf[i];
        if (mod == 15) {
			fprintf(stream, "\t%s\n", asciidata);
            memset(asciidata, '.', 16);
        }
    }
	// don't print two newlines if we ended perfectly
	if(mod != 15) {
		for(i=i%16;i<16; i++) {
			fprintf(stream, "   ");
			asciidata[i] = 0;
		}
		fprintf(stream, "\t%s\n", asciidata);
	}
}

void named_hexdump(FILE *stream, const char *title, const char *buf, const uint64_t len) {
    fprintf(stream, "Hexdump: %s, %lld bytes\n", title, len);
	hexdump(stream, buf, len);
}

void nano_sleep_impl(struct timespec *rqtp) {
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

// Needed to close sockets if connected.
void close_stdout() {
	fflush(stdout);
	close(1);
}

void fatal_error(const char* msg) {
    fprintf(stderr, "fatal_error: %s\n", msg);
    fprintf(stderr, "errno = %d, strerrno: %s\n", errno, strerror(errno));
	abort();
}

