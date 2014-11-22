#include "kayos_writer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "io.h"
#include "utils.h"

int handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *buffer, ssize_t len) {
	int stop = 0;
	fdb_status status;

	char *sep = " \t\r\n";
	char *brkt, *command, *key, *val;

	command = strtok_r(buffer, sep, &brkt);
	if(!command) return stop;
	key = strtok_r(0, sep, &brkt);
	if(!key) return stop;
	if(!strcmp(command, "set")) {
		val = strtok_r(0, sep, &brkt);
		if(!val) return stop;

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
	return stop;
}

void writer_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	int stop = 0;
	char buffer[1024];
	do {
		fprintf(stderr, "writer: loop head\n");
		ssize_t nbytes = safe_read(0, buffer, sizeof(buffer));
		if(nbytes == 0)
			fprintf(stderr, "writer: client disconnected\n");
		else if(nbytes == -1) {
			fatal_error("writer: safe_read");
		}
		named_hexdump(stderr, "writer got", buffer, nbytes);
		stop = handle_buffer(dbfile, db, buffer, sizeof(buffer));

		ssize_t written = 0;
		ssize_t ret = safe_write(0, "ok\n", 3, &written);
		(void) ret;
	} while(!stop);
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
