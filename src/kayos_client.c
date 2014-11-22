#include "kayos_client.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "io.h"
#include "utils.h"

int handle_buffer(char *buffer, ssize_t len) {
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
            fprintf(stderr, "client: client dc!\n");
        }
        
        fprintf(stderr, "client: got %zd bytes, buffer: %s\n", nbytes, buffer);
		named_hexdump("client got:", (unsigned char *)buffer, nbytes);
        printf("ok\n");
		stop = handle_buffer(buffer, sizeof(buffer));
	} while(!stop);
}

int main(int argc, char *arg[]) {
	client_loop();
	fprintf(stderr, "client exiting\n");
	return 0;
}
