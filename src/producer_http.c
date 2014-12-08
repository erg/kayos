#include "producer_http.h"

#include <stdio.h>
#include <string.h>

#include "both.h"
#include "buffer.h"
#include "consumer.h"
#include "hexdump.h"

size_t handle_producer_http(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, char *command, char *ptr, size_t len) {
	char *end = ptr + len;
	fprintf(stderr, "http command: %s\n", command);
	named_hexdump(stderr, "handle_http", ptr, len);

	ptr = buffer_skip_whitespace(ptr, end - ptr);
	char *url = strsep(&ptr, " \t\r\n");
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	char *version = strsep(&ptr, " \t\r\n");
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	char *headers = strsep(&ptr, " \t\r\n");
	fprintf(stderr, "url: %s\n", url);
	fprintf(stderr, "version: %s\n", version);
	fprintf(stderr, "headers: %s\n", headers);

    if(!strncmp(command, "GET", 3)) {
	}
    else if(!strncmp(command, "PUT", 3)) {
	}
    else if(!strncmp(command, "POST", 4)) {
	}
    else if (!strncmp(command, "DELETE", 6)) {
	}
	else {
		fprintf(stderr, "unknown http command: %s\n", command);
	}

	return -1;
}