#include "hexdump.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hexdump(FILE *stream, const char *buf, const size_t len) {
	if (!buf)
		return;
	int mod = 0;
	size_t i = 0;
	char asciidata[17];
	memset(asciidata, '.', 16);
	asciidata[16] = 0;
	for (i = 0; i < len; i++) {
		mod = i % 16;
		if(mod == 0)
			fprintf(stream, "%08lx: ", i);
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

void named_hexdump(FILE *stream, const char *title,
	const char *buf, const size_t len) {

	fprintf(stream, "Hexdump: %lu bytes\n%s\n", len, title);
	hexdump(stream, buf, len);
}
