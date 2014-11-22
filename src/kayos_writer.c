#include "kayos_writer.h"

#include <stdio.h>
#include <string.h>

#include "io.h"
#include "utils.h"

int handle_buffer(char *buffer, ssize_t len) {
	int stop = 0;

	if(!strncmp(buffer, "exit", len))
		stop = 1;

	return stop;
}

void writer_loop() {
	int stop = 0;
	do {
        char buffer[4096];
        fprintf(stderr, "writer: loop head\n");
        safe_read(0, buffer, sizeof(buffer));
        fprintf(stderr, "writer: got buffer: %s\n", buffer);
        printf("ok\n");
		stop = handle_buffer(buffer, sizeof(buffer));
	} while(!stop);
}

int main(int argc, char *arg[]) {
	writer_loop();
	return 0;
}
