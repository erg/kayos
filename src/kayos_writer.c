#include "kayos_writer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <forestdb.h>

#include "io.h"
#include "utils.h"

int handle_buffer(char *buffer, ssize_t len) {
	int stop = 0;

	if(!strncmp(buffer, "exit", len))
		stop = 1;

	return stop;
}

void writer_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	int stop = 0;
	do {
		char buffer[4096];
		fprintf(stderr, "writer: loop head\n");
		ssize_t nbytes = safe_read(0, buffer, sizeof(buffer));
		if(nbytes == 0)
			fprintf(stderr, "writer: client disconnected\n");
		else if(nbytes == -1) {
			fatal_error("writer: safe_read");
		}
        named_hexdump(stderr, "writer got", buffer, nbytes);
		stop = handle_buffer(buffer, sizeof(buffer));

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
	//if (comp_type == DAEMON_COMPACTION) {
		//fconfig.compaction_mode = FDB_COMPACTION_AUTO;
		//fconfig.compaction_threshold = 10;
		//fconfig.compactor_sleep_duration = 5;
	//}
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
