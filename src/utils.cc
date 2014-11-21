#include "utils.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <boost/crc.hpp>
#include <stdlib.h>

be_uint32_t crc32(const void *buf, uint64_t len) {
    boost::crc_32_type result;
    result.process_bytes(buf, len);
    return result.checksum();
}

be_uint64_t crc64(const void *buf, uint64_t len) {
    boost::crc_optimal<64, 0x04C11DB7, 0, 0, false, false> crc;
    crc.process_bytes(buf, len);
    return crc.checksum();
}

void hexdump(const unsigned char *buf, const uint64_t len)
{
    std::ios_base::fmtflags saved = std::cout.flags();
    int i;
    char asciidata[17];

    if (!buf)
        return;

    std::cout << std::resetiosflags(std::ios_base::showbase);
    std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << "0000: ";
    memset(asciidata, '.', 16);
    asciidata[16] = 0;
    for (i=0; i<len; i++)
    {
        std::cout << std::hex << std::setw(2) << (int)(buf[i]);
        if ((isprint(buf[i])) && (!isspace(buf[i])))
            asciidata[i%16] = buf[i];
        if (!((i+1)%16))
        {
            std::cout << "\t" << asciidata << std::endl << std::setw(4) << i+1 << std::setw(2) << ": ";
            memset(asciidata, '.', 16);
            asciidata[16] = 0;
        }
        else if (!((i+1)%8)) std::cout << "-";
        else std::cout << " ";
    }
    for(i=i%16;i<16; i++)
    {
        std::cout << "   ";
        asciidata[i] = 0;
    }
    std::cout << "\t" << asciidata << std::dec << std::endl;

    std::cout.flags(saved); // restore format flags to previous settings;
}

void named_hexdump(const char *title, const unsigned char *buf, const uint64_t len) {
	std::cout << "Hexdump: " << title << std::endl;
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
	std::cout << "fatal_error: " << msg << std::endl;
	std::cout << "errno = " << errno << ", msg: " << strerror(errno) << std::endl;
	std::cout << std::endl;
	abort();
}

