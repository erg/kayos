#include "producer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "both.h"
#include "buffer.h"
#include "errors.h"

fdb_status do_set_command(fdb_file_handle *db, fdb_kvs_handle *kvs,
	void *key, size_t key_length,
	void *value, size_t value_length) {

	fdb_status status = FDB_RESULT_INVALID_ARGS;

	if(key) {
		status = fdb_set_kv(kvs, key, key_length, value, value_length);

		if(status != FDB_RESULT_SUCCESS) {
			fatal_error("fdb_set_kv");
		}

		status = fdb_commit(db, FDB_COMMIT_NORMAL);
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("fdb_commit");
	}
	return status;
}

fdb_status do_delete_command(fdb_file_handle *db, fdb_kvs_handle *kvs,
	void *key, size_t key_length) {

	fdb_status status = FDB_RESULT_INVALID_ARGS;

	if(key) {
		status = fdb_del_kv(kvs, key, key_length);
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("fdb_del_kv");

		status = fdb_commit(db, FDB_COMMIT_NORMAL);
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("fdb_commit");
	}
	return status;
}


void do_forestdb_producer_command(fdb_file_handle *db, fdb_kvs_handle *kvs,
	char *kvs_name,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length) {

	fdb_status ret = 0 ;

	if(command) {
		if(!strcmp(command, "set")) {
			if(!key) {
				key_expected(command);
			} else if(!value) {
				value_expected(command);
			} else {
				ret = do_set_command(db, kvs, key, key_length, value, value_length);
				command_ok(ret);
			}
		}
		else if(!strcmp(command, "delete")) {
			if(!key) {
				key_expected(command);
			} else {
				ret = do_delete_command(db, kvs, key, key_length);
				command_ok(ret);
			}
		} else if(!strcmp(command, "topic")) {
			if(!key) {
				key_expected(command);
			} else {
				do_topic_command(kvs_name, key);
			}
		} else {
			fprintf(stderr, "unknown command: %s\n", command);
		}
		fflush(stdout);
	}
}
