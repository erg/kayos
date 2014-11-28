#include "kayos_common.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "buffer.h"
#include "io.h"
#include "utils.h"

struct fdb_handles init_fdb(const char *path) {
	fdb_status status;
	fdb_file_handle *dbfile;
	fdb_kvs_handle *db;
	fdb_config fconfig = fdb_get_default_config();
	fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
	status = fdb_open(&dbfile, path, &fconfig);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_open failed");
	status = fdb_kvs_open_default(dbfile, &db, &kvs_config);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_kvs_open_default failed");

	struct fdb_handles fdb_handles;
	fdb_handles.dbfile = dbfile;
	fdb_handles.db = db;
	return fdb_handles;
}

void close_fdb_handles(struct fdb_handles handles) {
	fdb_kvs_close(handles.db);
	fdb_close(handles.dbfile);
}

size_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len) {
    char *ptr = line;
    char *command = 0, *key = 0, *val = 0, *rest = 0;
    char *end = line + len;
    char *eol = line;

	// look for entire line, if no entire line, process it next time around
	size_t first_eol = strcspn(ptr, "\r\n");
#ifdef DEBUG
	fprintf(stderr, "first_eol %zu\n", first_eol);
#endif
	// found trailing NULL, no newline?
	if(ptr + first_eol == end)
		return first_eol;
	eol = ptr + first_eol;
	while(eol != end && (*eol == '\r' || *eol == '\n')) {
		*eol = 0;
		eol++;
	}

	ptr = buffer_skip_whitespace(ptr, end - ptr);
	command = strsep(&ptr, " \t\r\n");
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	key = strsep(&ptr, " \t\r\n");
	size_t key_length = key ? strlen(key) : 0;
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	val = strsep(&ptr, " \t\r\n");
	size_t val_length = val ? strlen(val) : 0;
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	rest = ptr;
#ifdef DEBUG
	fprintf(stderr, "command: %s\n", command);
	fprintf(stderr, "key: %s\n", key);
	fprintf(stderr, "val: %s\n", val);
	fprintf(stderr, "rest: %s\n\n", rest);
#endif
	handler(dbfile, db, command, key, key_length, val, val_length);
	return end - eol;
}

size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len) {
	fatal_error("unimplemented parse_binary");
	return 0;
}

size_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *buffer, size_t len) {
    size_t remaining = len;
    char *ptr = buffer;
    char *end = buffer + len;
	size_t previous_remaining = remaining;
	do {
#ifdef DEBUG
		fprintf(stderr, "******loop start\n");
		named_hexdump(stderr, "handle_buffer", ptr, remaining);
#endif
		previous_remaining = remaining;
		if(ptr >= end) break;

		if(*ptr <= 127)
			remaining = parse_line(dbfile, db, handler, ptr, end - ptr);
		else
			remaining = parse_binary(dbfile, db, handler, ptr, end - ptr);

		ptr = end - remaining;
#ifdef DEBUG
        fprintf(stderr, "previous %zu, remaining: %zu\n", previous_remaining, remaining);
#endif
	} while(remaining > 0 && (previous_remaining != remaining));

#ifdef DEBUG
	fprintf(stderr, "compacting, remaining: %zu!\n", remaining);
	named_hexdump(stderr, "before compaction", buffer, len);
#endif

	remaining = compact_buffer(buffer, len, ptr);

#ifdef DEBUG
	named_hexdump(stderr, "after compaction", buffer, remaining);
#endif
	return remaining;
}

void client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler) {
	char buffer[BUFFER_LENGTH];
	size_t remaining = 0;
	do {
#ifdef DEBUG
		named_hexdump(stderr, "producer_client loop head", buffer, remaining);
#endif
		ssize_t nbytes = safe_read(0, buffer + remaining, sizeof(buffer) - 1 - remaining);
		if(nbytes == 0) {
#ifdef DEBUG
			fprintf(stderr, "producer_client: client disconnected\n");
#endif
			break;
		} else if(nbytes == -1) {
			fatal_error("consumer_client: safe_read");
		} else {
			buffer[nbytes] = 0;
#ifdef DEBUG
			named_hexdump(stderr, "consumer_client got", buffer, nbytes);
#endif
			remaining = handle_buffer(dbfile, db, handler, buffer, nbytes);
		}
	} while(1);

#ifdef DEBUG
	if(remaining > 0) {
		named_hexdump(stderr, "consumer client dc, unprocessed commands", buffer, remaining);
	}
#endif
}
