#include "consumer_client.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>
#include <jansson.h>

#include "kayos_common.h"
#include "buffer.h"
#include "io.h"
#include "utils.h"

void do_iterate_command(fdb_file_handle *dbfile, fdb_kvs_handle *db) {
	fdb_status status;
	fdb_iterator *iterator;
	fdb_doc *rdoc;
	fdb_iterator_sequence_init(db, &iterator, 0, -1, FDB_ITR_NONE);
	while(1) {
		status = fdb_iterator_next(iterator, &rdoc);
		if (status == FDB_RESULT_ITERATOR_FAIL) break;
		//rdoc->keylen, rdoc->metalen rdoc->bodylen
		//fprintf(stderr, "seqnum: %llu, meta: %s, key: %s, body: %s\r\n", rdoc->seqnum, rdoc->meta, rdoc->key, rdoc->body);
		json_t *dict = json_object();
		json_object_set_new(dict, "key", json_string(rdoc->key));
		json_object_set_new(dict, "meta", json_string(rdoc->meta));
		json_object_set_new(dict, "body", json_string(rdoc->body));
		json_object_set_new(dict, "seqnum", json_integer(rdoc->seqnum));
		char *result = json_dumps(dict, 0);
		//fprintf(stderr, "%s\n", result);
		fprintf(stdout, "%s\n", result);
		free(result);
		fflush(stdout);

		fdb_doc_free(rdoc);
	}
	fdb_iterator_close(iterator);
}

void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length) {
	fprintf(stderr, "executing fdb consumer command: %s\n", command);
	if(!strncmp(command, "iterate", 7)) {
		do_iterate_command(dbfile, db);
	} else {
		fprintf(stderr, "unknown command\n");
	}
}
