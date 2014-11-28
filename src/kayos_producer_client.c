#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "buffer.h"
#include "io.h"
#include "utils.h"

const size_t BUFFER_LENGTH = 1024;

void handle_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, char *val) {
    fdb_status status;
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
}

size_t handle_line(char *line) {
}

size_t handle_binary(char *bytes) {
}

ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *buffer, size_t len) {
    size_t remaining = len;
    char *ptr = buffer;
    char *command = 0, *key = 0, *val = 0, *rest = 0;
    char *end = buffer + len;
    char *eol = buffer;
	do {
		fprintf(stderr, "loop start\n");
        // Set ptr to head of next command
        ptr = eol;

        // On second time through, advance past previous null byte
        if(ptr != buffer) ptr += 1;
        if(ptr >= end) break;

        // look for entire line, if no entire line, process it next time around
        size_t first_eol = strcspn(ptr, "\r\n");
        if(ptr + first_eol == end)
            break;
        //fprintf(stderr, "first_eol %zu\n", first_eol);
        eol = ptr + first_eol;
        if(*eol == 0) break;
        while(eol + 1 != end && (*(eol + 1) == '\r' || *(eol + 1) == '\n'))
            eol++;
        *eol = 0;

        ptr = buffer_skip_whitespace(ptr, end - ptr);
        command = strsep(&ptr, " \t\r\n");
        if(!command)
            goto loop_end;

		ptr = buffer_skip_whitespace(ptr, end - ptr);
		command = strsep(&ptr, " \t\r\n");
		ptr = buffer_skip_whitespace(ptr, end - ptr);
		key = strsep(&ptr, " \t\r\n");
		ptr = buffer_skip_whitespace(ptr, end - ptr);
		val = strsep(&ptr, " \t\r\n");
		ptr = buffer_skip_whitespace(ptr, end - ptr);
		rest = ptr;
		fprintf(stderr, "command: %s\n", command);
		fprintf(stderr, "key: %s\n", key);
		fprintf(stderr, "val: %s\n", val);
		fprintf(stderr, "rest: %s\n", rest);
		handle_command(dbfile, db, command, key, val);
loop_end:
        remaining = end - eol;
        fprintf(stderr, "remaining: %zu\n", remaining);
	} while(remaining > 0);

	fprintf(stderr, "compacting, remaining: %zu!\n", remaining);
	named_hexdump(stderr, "before compaction", buffer, len);
	remaining = compact_buffer(buffer, len, eol);
	named_hexdump(stderr, "after compaction", buffer, remaining);
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
