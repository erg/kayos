#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "buffer.h"
#include "io.h"
#include "utils.h"

const size_t BUFFER_LENGTH = 1024;

// if buffer doesn't find EOL, then return -1 and throw away those bytes
ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *buffer, size_t len) {
	fdb_status status;
	char *ptr = buffer;
	char *end = buffer + len;
	char *command, *command_end, *key, *key_end, *val, *val_end;
	uint64_t remaining = len;
	do {
		ptr = command = buffer_skip_whitespace(ptr, end - ptr);
		if(!ptr) break;
		ptr = command_end = buffer_skip_until(ptr, end - ptr, " \t\r\n", 4);
		*command_end = '\0';
		fprintf(stderr, "command: %s\n", command);
		ptr = key = buffer_skip_whitespace(ptr, end - ptr);
		if(!ptr) break;
		ptr = key_end = buffer_skip_until(ptr, end - ptr, " \t\r\n", 4);
		if(!ptr) break;
		*key_end = '\0';
		fprintf(stderr, "key: %s\n", key);

		if(!memcmp(command, "set", 3)) {
			ptr = val = buffer_skip_whitespace(ptr, end - ptr);
			if(!ptr) break;
			ptr = val_end = buffer_skip_until(ptr, end - ptr, " \t\r\n", 4);
			if(!ptr) break;
			*val_end = '\0';

			named_hexdump(stderr, "after tokenization", buffer, 32);

			fprintf(stderr, "setting key: %s, val: %s\n", key, val);
			status = fdb_set_kv(db, key, strlen(key), val, strlen(val));
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_set_kv");
			status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_commit");
		} else if(!memcmp(command, "get", 3)) {
			void *rvalue;
			size_t rvalue_len;
			status = fdb_get_kv(db, key, strlen(key), &rvalue, &rvalue_len);
			if(status == FDB_RESULT_KEY_NOT_FOUND)
				fprintf(stderr, "FDB_RESULT_KEY_NOT_FOUND, status: %d\n", status);
			else if(status == FDB_RESULT_SUCCESS) {
				fprintf(stderr, "FDB_RESULT_SUCCESS, status: %d\n", status);
				fprintf(stderr, "got key: %s, val: %s\n", key, rvalue);
				if(rvalue)
					free(rvalue);
			}
		} else if(!memcmp(command, "delete", 6)) {
			fprintf(stderr, "deleting key: %s\n", key);
			status = fdb_del_kv(db, key, strlen(key));
			fprintf(stderr, "delete status: %d\n", status);
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_del_kv");
			status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_commit");
		} else
			fprintf(stderr, "unknown command: %s\n", command);

		remaining = compact_buffer(buffer, len, ptr);
		fprintf(stderr, "remaining: %d\n", remaining);
	} while(remaining > 0);
	if(!ptr)
		return 0;
	return remaining;
}

void producer_client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	char buffer[BUFFER_LENGTH];
	do {
		fprintf(stderr, "producer_client: loop head\n");
		ssize_t nbytes = safe_read(0, buffer, sizeof(buffer));
		if(nbytes == 0) {
			fprintf(stderr, "producer_client: client disconnected\n");
			break;
		} else if(nbytes == -1) {
			fatal_error("producer_client: safe_read");
		}
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
