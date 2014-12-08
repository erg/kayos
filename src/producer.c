#include "producer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "both.h"
#include "buffer.h"
#include "errors.h"

void do_set_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	void *key, size_t key_length,
	void *value, size_t value_length) {

    fdb_status status;

	if(key) {
		status = fdb_set_kv(db, key, key_length, value, value_length);

		if(status != FDB_RESULT_SUCCESS) {
			fatal_error("fdb_set_kv");
		}

		status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("fdb_commit");
	}
}

void do_delete_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	void *key, size_t key_length) {

    fdb_status status;

	if(key) {
		status = fdb_del_kv(db, key, key_length);
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("fdb_del_kv");

		status = fdb_commit(dbfile, FDB_COMMIT_NORMAL);
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("fdb_commit");
	}
}


void do_forestdb_producer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length) {

	if(!strcmp(command, "set"))
		do_set_command(dbfile, db, key, key_length, value, value_length);
	else if(!strcmp(command, "delete"))
		do_delete_command(dbfile, db, key, key_length);
	else
		fprintf(stderr, "unknown command: %s\n", command);
}
