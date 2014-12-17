#include "consumer_http.h"

#include <stdio.h>
#include <string.h>

#include <http_parser.h>

#include "both.h"
#include "buffer.h"
#include "consumer.h"
#include "hexdump.h"

int cb_count = 0;

// http_cb

int on_message_begin_cb(http_parser *p) {
	fprintf(stderr, "message_begin_cb called\n");
	//fprintf(stderr, "  method: %d, status_code: %d\n", p->method, p->status_code);

	fflush(stderr);
	cb_count++;
	return 0;
}

int on_headers_complete_cb(http_parser *p) {
	fprintf(stderr, "on_headers_complete_cb called\n");
	fprintf(stderr, "  method: %d, status_code: %d\n", p->method, p->status_code);
	fflush(stderr);
	cb_count++;
	return 0;
}

int on_message_complete_cb(http_parser *p) {
	fprintf(stderr, "on_message_complete_cb called\n");
	fprintf(stderr, "  method: %d, status_code: %d\n", p->method, p->status_code);
	fflush(stderr);
	cb_count++;
	return 0;
}


int on_header_field_cb(http_parser *p, const char *at, size_t length) {
	fprintf(stderr, "on_header_field_cb called\n");
	fprintf(stderr, "  ");
	fwrite(at, 1, length, stderr);
	fprintf(stderr, "\n");
	fflush(stderr);

	cb_count++;
	return 0;
}

int on_header_value_cb(http_parser *p, const char *at, size_t length) {
	fprintf(stderr, "on_header_value_cb called\n");
	fprintf(stderr, "  ");
	fwrite(at, 1, length, stderr);
	fprintf(stderr, "\n");
	fflush(stderr);

	cb_count++;
	return 0;
}

int on_url_cb(http_parser *p, const char *at, size_t length) {
	fprintf(stderr, "on_url_cb called\n");
	fprintf(stderr, "  ");
	fwrite(at, 1, length, stderr);
	fprintf(stderr, "\n");
	fflush(stderr);

	cb_count++;
	return 0;
}

int on_status_cb(http_parser *p, const char *at, size_t length) {
	fprintf(stderr, "on_status_cb called\n");
	fprintf(stderr, "  ");
	fwrite(at, 1, length, stderr);
	fprintf(stderr, "\n");
	fflush(stderr);

	cb_count++;
	return 0;
}

int on_body_cb(http_parser *p, const char *at, size_t length) {
	fprintf(stderr, "on_body_cb called\n");
	fprintf(stderr, "  ");
	fwrite(at, 1, length, stderr);
	fprintf(stderr, "\n");
	fflush(stderr);

	cb_count++;
	return 0;
}

// https://github.com/bodokaiser/libuv-webserver/blob/master/src/webserver.c
size_t handle_consumer_http(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	forestdb_handler_t handler,
	char *ptr, size_t len) {

	named_hexdump(stderr, "handle_consumer_http", ptr, len);
	http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);

	size_t nread;
	http_parser_settings settings;

	settings.on_message_begin = on_message_begin_cb;
	settings.on_header_field = on_header_field_cb;
	settings.on_header_value = on_header_value_cb;
	settings.on_url = on_url_cb;
	settings.on_status = on_status_cb;
	settings.on_body = on_body_cb;
	settings.on_headers_complete = on_headers_complete_cb;
	settings.on_message_complete = on_message_complete_cb;

	nread = http_parser_execute(&parser, &settings, ptr, len);
	fprintf(stderr, "0: http_parser_execute nread: %zu\n", nread);
	fprintf(stderr, "count: %d\n", cb_count);

	if(parser.upgrade) {
		// do something cool here for web sockets
	} else {

		if(nread != len) {
			return -1;
		}
	}

	/*
			if(!strncmp(url, "/_iterate", 9)) {
			do_iterate_command(db, 0);
	*/
	return nread;
}
