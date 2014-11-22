#include "kayos_writer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <forestdb.h>

#include "io.h"
#include "utils.h"

//char * strtok_r(char *restrict str, const char *restrict sep, char **restrict lasts);

//<command name> <key> <flags> <exptime> <bytes> [noreply]\r\n
//cas <key> <flags> <exptime> <bytes> <cas unique> [noreply]\r\n

// cas, set, add, replace, append, prepend



int handle_buffer(char *buffer, ssize_t len) {
	int stop = 0;

	char *sep = " \t\r\n";
	char *word, *phrase, *brkt, *brkb;

	word = strtok_r(buffer, sep, &brkt);
	while(word) {
		fprintf(stderr, "word: %s\n", word);
		word = strtok_r(0, sep, &brkt);
	}
	//if(!strncmp(buffer, "exit", len))
		//stop = 1;
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
