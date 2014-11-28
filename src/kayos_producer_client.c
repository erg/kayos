#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "buffer.h"
#include "io.h"
#include "utils.h"

const size_t BUFFER_LENGTH = 1024;

void do_forestdb_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, char *val) {
    fdb_status status;
	if(!strcmp(command, "set")) {
		// at least need a key to set, value is optional
		if(key) {
			fprintf(stderr, "set %s %s\n", key, val);
			status = fdb_set_kv(db, key, strlen(key), val, strlen(val));
#ifdef DEBUG
			fprintf(stderr, "set status: %d\n", status);
#endif
			if(status != FDB_RESULT_SUCCESS) {
				fatal_error("fdb_set_kv");
			}
			status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_commit");
		}
	} else if(!strcmp(command, "delete")) {
		if(key) {
			fprintf(stderr, "delete %s\n", key);
			status = fdb_del_kv(db, key, strlen(key));
#ifdef DEBUG
			fprintf(stderr, "delete status: %d\n", status);
#endif
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_del_kv");
			status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_commit");
		}
	} else {
		fprintf(stderr, "unknown command: %s\n", command);
	}
}

size_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *line, size_t len) {
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
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	val = strsep(&ptr, " \t\r\n");
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	rest = ptr;
#ifdef DEBUG
	fprintf(stderr, "command: %s\n", command);
	fprintf(stderr, "key: %s\n", key);
	fprintf(stderr, "val: %s\n", val);
	fprintf(stderr, "rest: %s\n\n", rest);
#endif
	do_forestdb_command(dbfile, db, command, key, val);
	return end - eol;
}

size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *line, size_t len) {
	fatal_error("unimplemented parse_binary");
	return 0;
}

ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *buffer, size_t len) {
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
        // Set ptr to head of next command
		if(ptr >= end) break;

		if(*ptr <= 127)
			remaining = parse_line(dbfile, db, ptr, end - ptr);
		else
			remaining = parse_binary(dbfile, db, ptr, end - ptr);

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

void producer_client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
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
			fatal_error("producer_client: safe_read");
		} else {
			buffer[nbytes] = 0;
#ifdef DEBUG
			named_hexdump(stderr, "producer_client got", buffer, nbytes);
#endif
			remaining = handle_buffer(dbfile, db, buffer, nbytes);
		}
	} while(1);

#ifdef DEBUG
	if(remaining > 0) {
		named_hexdump(stderr, "producer client dc, unprocessed commands", buffer, remaining);
	}
#endif
}

int main(int argc, char *arg[]) {
	fdb_status status;
	fdb_file_handle *dbfile;
	fdb_kvs_handle *db;
	fdb_config fconfig = fdb_get_default_config();
	fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
	status = fdb_open(&dbfile, "./test.fdb", &fconfig);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_open failed");
	status = fdb_kvs_open_default(dbfile, &db, &kvs_config);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_kvs_open_default failed");

	producer_client_loop(dbfile, db);

	// cleanup forestdb
	fdb_kvs_close(db);
	fdb_close(dbfile);
	return 0;
}
