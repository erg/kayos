#include "producer_client.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "kayos_common.h"
#include "buffer.h"
#include "io.h"
#include "utils.h"

void do_forestdb_producer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length) {
    fdb_status status;
	if(!strcmp(command, "set")) {
		// at least need a key to set, value is optional
		if(key) {
			fprintf(stdout, "set %s %s\n", key, val);
			//fprintf(stderr, "set %s %s\n", key, val);
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
			//fprintf(stderr, "delete %s\n", key);
			fprintf(stdout, "delete %s\n", key);
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
