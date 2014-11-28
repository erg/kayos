#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "buffer.h"
#include "io.h"
#include "utils.h"

const size_t BUFFER_LENGTH = 1024;

void handle_line(char *line) {
}

// if buffer doesn't find EOL, then return -1 and throw away those bytes
ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *buffer, size_t len) {
	fdb_status status;
	uint64_t remaining = len;

	char *ptr = buffer;
	char *command, *key, *val, *rest;
	size_t eol = 0;
	do {
		eol = strcspn(ptr + eol, "\r\n");
		char *end = ptr + eol;
		ptr[eol] = 0;

		ptr = buffer_skip_whitespace(ptr, end - ptr);
		command = strsep(&ptr, " \t\r\n");
		ptr = buffer_skip_whitespace(ptr, end - ptr);
		key = strsep(&ptr, " \t\r\n");
		ptr = buffer_skip_whitespace(ptr, end - ptr);
		val = strsep(&ptr, " \t\r\n");
		ptr = buffer_skip_whitespace(ptr, end - ptr);
		rest = ptr;
		fprintf(stderr, "loop start\n");
		fprintf(stderr, "command: %s\n", command);
		fprintf(stderr, "key: %s\n", key);
		fprintf(stderr, "val: %s\n", val);
		fprintf(stderr, "rest: %s\n", rest);

		if(!strcmp(command, "set")) {
			// at least need a key to set, value is optional
			if(key) {
				fprintf(stderr, "set %s %s\n", key, val);
				status = fdb_set_kv(db, key, strlen(key), val, strlen(val));
				if(status != FDB_RESULT_SUCCESS) {
					fatal_error("fdb_set_kv");
				}
				status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
				if(status != FDB_RESULT_SUCCESS)
					fatal_error("fdb_commit");
			}
		} else if(!strcmp(command, "delete")) {
			if(key) {
				fprintf(stderr, "deleting key\n");
				status = fdb_del_kv(db, key, strlen(key));
				fprintf(stderr, "delete status: %d\n", status);
				if(status != FDB_RESULT_SUCCESS)
					fatal_error("fdb_del_kv");
				status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
				if(status != FDB_RESULT_SUCCESS)
					fatal_error("fdb_commit");
			}
		} else {
			fprintf(stderr, "unknown command\n");
		}
	} while(remaining > 0);

	fprintf(stderr, "compacting, remaining: %llu!\n", remaining);
	named_hexdump(stderr, "after compaction", buffer, len);
	//remaining = compact_buffer(buffer, eol, len - eol);
	return remaining;
}

void producer_client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	char buffer[BUFFER_LENGTH];
	do {
		fprintf(stderr, "producer_client: loop head\n");
		ssize_t nbytes = safe_read(0, buffer, sizeof(buffer) - 1);
		if(nbytes == 0) {
			fprintf(stderr, "producer_client: client disconnected\n");
			break;
		} else if(nbytes == -1) {
			fatal_error("producer_client: safe_read");
		}

		buffer[nbytes] = 0;
		named_hexdump(stderr, "producer_client got", buffer, nbytes);
		handle_buffer(dbfile, db, buffer, nbytes);

		//ssize_t ret = safe_write(1, "ok\n", 3);
		//if(ret != 3)
			//fatal_error("producer_client safe_write failed");
	} while(1);
}

int main(int argc, char *arg[]) {
	fdb_file_handle *dbfile;
	fdb_kvs_handle *db;
	fdb_status status;

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
