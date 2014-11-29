#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "kayos_common.h"
#include "buffer.h"
#include "io.h"
#include "utils.h"

void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length) {
	fprintf(stderr, "executing fdb consumer command: %s\n", command);
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
				//fprintf(stderr, "got key: %s, val: %s\n", key, rvalue);
				fprintf(stdout, "got key: %s, val: %s\n", key, rvalue);
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
			//fprintf(stderr, "seqnum: %llu, meta: %s, key: %s, body: %s\r\n", rdoc->seqnum, rdoc->meta, rdoc->key, rdoc->body);
			fprintf(stdout, "seqnum: %llu, meta: %s, key: %s, body: %s\r\n", rdoc->seqnum, rdoc->meta, rdoc->key, rdoc->body);
			fflush(stdout);

			fdb_doc_free(rdoc);
		}
		fdb_iterator_close(iterator);
	} else {
		fprintf(stderr, "unknown command\n");
	}
}

int main(int argc, char *arg[]) {
	struct fdb_handles handles = init_fdb("./test.fdb");
	client_loop(handles.dbfile, handles.db, do_forestdb_consumer_command);
	close_fdb_handles(handles);
	return 0;
}
