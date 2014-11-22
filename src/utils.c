#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void hexdump(const unsigned char *buf, const uint64_t len)
{
    int i;
    char asciidata[17];

    if (!buf)
        return;

    memset(asciidata, '.', 16);
    asciidata[16] = 0;
    for (i=0; i<len; i++)
    {
        printf("%02x", (int)(buf[i]));
        if ((isprint(buf[i])) && (!isspace(buf[i])))
            asciidata[i%16] = buf[i];
        if (!((i+1)%16))
        {
            memset(asciidata, '.', 16);
            asciidata[16] = 0;
        }
        else if (!((i+1)%8)) printf("-");
        else printf(" ");
    }
    for(i=i%16;i<16; i++)
    {
        printf("   ");
        asciidata[i] = 0;
    }
    printf("\t%s", asciidata);
}

void named_hexdump(const char *title, const unsigned char *buf, const uint64_t len) {
    printf("Hexdump: %s\n", title);
	hexdump(buf, len);
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

void fatal_error(const char* msg) {
    printf("fatal_error: %s\n", msg);
    printf("errno = %d, strerrno: %s\n", errno, strerror(errno));
	abort();
}

