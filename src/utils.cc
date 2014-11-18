#include "utils.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <boost/crc.hpp>


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

    std::cout << resetiosflags(std::ios_base::showbase);
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


