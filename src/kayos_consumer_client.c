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

void handle_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, char *val) {
	fdb_status status;
	if(!strncmp(command, "get", 3)) {
		if(key) {
			fprintf(stderr, "getting key\n");
			void *rvalue;
			size_t rvalue_len;
			fprintf(stderr, "in get\n");
			status = fdb_get_kv(db, key, strlen(key), &rvalue, &rvalue_len);
			if(status == FDB_RESULT_KEY_NOT_FOUND)
				fprintf(stderr, "FDB_RESULT_KEY_NOT_FOUND, status: %d\n", status);
			else if(status == FDB_RESULT_SUCCESS) {
				fprintf(stderr, "FDB_RESULT_SUCCESS, status: %d\n", status);
				fprintf(stderr, "got key: %s, val: %s\n", key, rvalue);
				if(rvalue)
					free(rvalue);
			}
		}
	}
	else if(!strncmp(command, "iterate", 7)) {
		fdb_iterator *iterator;
		fdb_doc *rdoc;
		fdb_iterator_sequence_init(db, &iterator, 0, -1, FDB_ITR_NONE);
		while(1) {
			status = fdb_iterator_next(iterator, &rdoc);
			if (status == FDB_RESULT_ITERATOR_FAIL) break;
			//rdoc->keylen, rdoc->metalen rdoc->bodylen
			fprintf(stderr, "seqnum: %llu, meta: %s, key: %s, body: %s\n", rdoc->seqnum, rdoc->meta, rdoc->key, rdoc->body);

			fdb_doc_free(rdoc);
		}
		fdb_iterator_close(iterator);
	} else {
		fprintf(stderr, "unknown command\n");
	}
}

// buffer is null terminated
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
		while(eol != end && (*eol == '\r' || *eol == '\n'))
			eol++;
		*eol = 0;

		ptr = buffer_skip_whitespace(ptr, end - ptr);
		command = strsep(&ptr, " \t\r\n");
		if(!command)
			goto loop_end;
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
		//fprintf(stderr, "end: %p, eol: %p, ptr: %p\n", end, eol, ptr);
		remaining = end - eol;
		fprintf(stderr, "remaining: %zu\n", remaining);
	} while(remaining > 0);

	fprintf(stderr, "compacting, remaining: %zu!\n", remaining);
	fprintf(stderr, "end: %p, eol: %p, ptr: %p\n", end, eol, ptr);
	remaining = compact_buffer(buffer, len, eol);
	fprintf(stderr, "returning %zu\n", remaining);

	return remaining;
}

void producer_client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	char buffer[BUFFER_LENGTH];
	ssize_t offset = 0;
	ssize_t nbytes = 0;
	do {
		fprintf(stderr, "consumer_client: loop head\n");

		nbytes = safe_read(0, buffer + offset, sizeof(buffer) - offset - 1);
		if(nbytes == 0) {
			fprintf(stderr, "consumer_client: client disconnected\n");
			break;
		} else if(nbytes == -1) {
			fatal_error("consumer_client: safe_read");
		}

		buffer[nbytes] = 0;
		named_hexdump(stderr, "consumer_client got", buffer, nbytes);
		offset = handle_buffer(dbfile, db, buffer, nbytes + offset);
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
