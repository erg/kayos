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

// Allocates, call free on return value
char *doc_to_string(fdb_doc *rdoc) {
	json_t *dict = json_object();
	json_object_set_new(dict, "key", json_stringn(rdoc->key, rdoc->keylen));
	json_object_set_new(dict, "meta", json_stringn(rdoc->meta, rdoc->metalen));
	json_object_set_new(dict, "body", json_stringn(rdoc->body, rdoc->bodylen));
	json_object_set_new(dict, "seqnum", json_integer(rdoc->seqnum));
	char *result = json_dumps(dict, 0);
	return result;
}

void do_get_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	const char *key) {

	fdb_status status;
	fdb_doc *rdoc;

	fdb_doc_create(&rdoc, (void*)key, strlen(key),
			NULL, 0, NULL, 0);
	status = fdb_get(db, rdoc);
	if(status == FDB_RESULT_SUCCESS) {
		fprintf(stdout, "%s\n", doc_to_string(rdoc));
		fdb_doc_free(rdoc);
	} else if(status == FDB_RESULT_KEY_NOT_FOUND) {
		fprintf(stdout, "{\"result\": \"fail\", \"reason\": \"NO_KEY\"}\n");
	}
}

void do_queue_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	const char *key) {
	// open queue or lookup, switch current queue
}

void do_iterate_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	const char *key) {

	int start = 0;

	if(key) {
		start = atoi(key);
	}

	fdb_status status;
	fdb_iterator *iterator;
	fdb_doc *rdoc;
	status = fdb_iterator_sequence_init(db, &iterator, start, -1, FDB_ITR_NONE);
	fprintf(stderr, "fdb_iterator_sequence_init status: %d\n", status);
	if(status != FDB_RESULT_SUCCESS) {
		fprintf(stderr, "fdb_iterator_sequence_init failed");
		return;
	}

	while(1) {
		status = fdb_iterator_next(iterator, &rdoc);
		//fprintf(stderr, "fdb_iterator_next status: %d\n", status);
		if (status == FDB_RESULT_ITERATOR_FAIL) break;

		char *result = doc_to_string(rdoc);
		//fprintf(stderr, "%s\n", result);
		fprintf(stdout, "%s\n", result);
		fflush(stdout);
		free(result);
		fdb_doc_free(rdoc);
	}
	fdb_iterator_close(iterator);
}

void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	const char *command,
	const char *key, size_t key_length,
	const char *val, size_t val_length) {

	fprintf(stderr, "executing fdb consumer command: %s\n", command);
	if(!strncmp(command, "queue", 5)) {
		fprintf(stderr, "QUEUE\n");
		do_queue_command(dbfile, db, key);
	} else if(!strncmp(command, "get", 3)) {
		fprintf(stderr, "GETTING\n");
		do_get_command(dbfile, db, key);
	} else if(!strncmp(command, "iterate", 7)) {
		fprintf(stderr, "ITERATING\n");
		do_iterate_command(dbfile, db, key);
	} else {
		fprintf(stderr, "fail: unknown command\n");
		fprintf(stdout, "fail: unknown command\n");
	}
}

