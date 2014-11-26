#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "io.h"
#include "utils.h"

const size_t BUFFER_LENGTH = 1024;

// if buffer doesn't find EOL, then return -1 and throw away those bytes
ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *buffer, size_t len) {
	fdb_status status;
	char *ptr = buffer;
	size_t remaining = len;
	do {
		size_t count = strcspn(ptr, "\r\n");
		// no newline, try again after we get more chars
		if(count == len)
			break;

		// skip \r\n
		if(count == 0 && remaining > 0) {
			ptr++;
			continue;
		}
			
		char *sep = " \t";
		char *brkt, *command, *key, *val;

		command = strtok_r(ptr, sep, &brkt);
		if(!command)
			continue;
		key = strtok_r(0, sep, &brkt);
		if(!key)
			continue;
		if(!strcmp(command, "set")) {
			val = strtok_r(0, sep, &brkt);
			if(!val)
				continue;

			fprintf(stderr, "setting key: %s, val: %s\n", key, val);
			status = fdb_set_kv(db, key, strlen(key), val, strlen(val));
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_set_kv");
			status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
			if(status != FDB_RESULT_SUCCESS)
				fatal_error("fdb_commit");
		} else if(!strcmp(command, "get")) {
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
		} else if(!strcmp(command, "delete")) {
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
	} while(1);

	size_t diff = ptr - (char*)buffer;

	memmove(buffer, (char*)buffer + diff, diff);

	return 0;
}

void writer_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	char buffer[BUFFER_LENGTH];
	do {
		fprintf(stderr, "writer: loop head\n");
		ssize_t nbytes = safe_read(0, buffer, sizeof(buffer));
		if(nbytes == 0) {
			fprintf(stderr, "writer: client disconnected\n");
			break;
		} else if(nbytes == -1) {
			fatal_error("writer: safe_read");
		}
		named_hexdump(stderr, "writer got", buffer, nbytes);
		handle_buffer(dbfile, db, buffer, sizeof(buffer));

		ssize_t ret = safe_write(0, "ok\n", 3);
		(void) ret;
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

	writer_loop(dbfile, db);

	// cleanup forestdb
	fdb_kvs_close(db);
	fdb_close(dbfile);
	return 0;
}
