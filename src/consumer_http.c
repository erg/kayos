#include "consumer_http.h"

#include <stdio.h>
#include <string.h>

#include <http_parser.h>

#include "both.h"
#include "buffer.h"
#include "consumer.h"
#include "hexdump.h"

//static int consumer_cb(http_parser*, const char *at, size_t length);

/*
int consumer_cb(http_parser* parser, const char *at, size_t length) {
	return 0;
}
*/

size_t handle_consumer_http(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	forestdb_handler_t handler,
	char *command,
	char *ptr, size_t len) {

	// http_parser_settings settings;
	// settings.on_url = my_url_callback;
	// settings.on_header_field = my_header_field_callback;
	/* ... */

	// http_parser *parser = malloc(sizeof(http_parser));
	// http_parser_init(parser, HTTP_REQUEST);
	// parser->data = my_socket;

	/*
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
		if(!strncmp(url, "/_iterate", 9)) {
			do_iterate_command(db, 0);
		}
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

	*/

	return -1;
}
