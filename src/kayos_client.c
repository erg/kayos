#include "kayos_client.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "io.h"
#include "utils.h"

int handle_buffer(const char *buffer, ssize_t len) {
	int stop = 0;

	if(!strncmp(buffer, "exit", len))
		stop = 1;

	return stop;
}

void client_loop() {
	int stop = 0;
	do {
		char buffer[4096];
		fprintf(stderr, "client: loop head\n");
		ssize_t nbytes = safe_read(0, buffer, sizeof(buffer));
		if(nbytes == -1) {
			fatal_error("client safe_read");
		} else if(nbytes == 0) {
			fprintf(stderr, "client: client dc!\n");
			break;
		}
		fprintf(stderr, "client: got %zd bytes, buffer: %s\n", nbytes, buffer);
		named_hexdump(stderr, "client got", buffer, nbytes);

		fprintf(stdout, "hello writer from client\r\n");
		fflush(stdout);

		stop = handle_buffer(buffer, sizeof(buffer));

		ssize_t written = 0;
		ssize_t ret = safe_write(0, "ok\n", 3, &written);
		(void) ret;
	} while(!stop);
}

int main(int argc, char *arg[]) {
	client_loop();
	fprintf(stderr, "client exiting\n");
	return 0;
}
