#include "consumer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <forestdb.h>
#include <jansson.h>

#include "both.h"
#include "buffer.h"
#include "io.h"

static void telnet_print_doc(fdb_doc *doc);
static void telnet_print_key_doc(fdb_doc *doc);

fdb_status do_get_command(fdb_kvs_handle *db, const char *key, fdb_doc_print_t print_cb) {
    fdb_status status = FDB_RESULT_INVALID_ARGS;
	fdb_doc *doc;

	fdb_doc_create(&doc, (void*)key, strlen(key),
			NULL, 0, NULL, 0);
	status = fdb_get(db, doc);
	if(status == FDB_RESULT_SUCCESS) {
		//fprintf(stdout, "%s\n", doc_to_string(doc));
		print_cb(doc);
	}
	else if(status == FDB_RESULT_KEY_NOT_FOUND)
		fprintf(stdout, "{\"status\": \"fail\", \"reason\": \"NO_KEY\"}\n");
	fdb_doc_free(doc);
	return status;
}

fdb_status do_iterate_command(fdb_kvs_handle *db, fdb_seqnum_t start, fdb_doc_print_t print_cb) {
    fdb_status status = FDB_RESULT_INVALID_ARGS;
	fdb_iterator *iterator;
	fdb_doc *doc;

	status = fdb_iterator_sequence_init(db, &iterator, start, -1, FDB_ITR_NONE);
	fprintf(stderr, "fdb_iterator_sequence_init status: %d, start: %" PRIu64 "\n", status, start);
	if(status != FDB_RESULT_SUCCESS) {
		fprintf(stderr, "fdb_iterator_sequence_init failed");
		return status;
	}

	while(1) {
		fdb_iterator_get(iterator, &doc);
		if (status == FDB_RESULT_ITERATOR_FAIL) break;

		print_cb(doc);
		fdb_doc_free(doc);
		status = fdb_iterator_next(iterator);
		if (status == FDB_RESULT_ITERATOR_FAIL) break;
	}

	fflush(stdout);
	fdb_iterator_close(iterator);
	return status;
}

static void telnet_print_doc(fdb_doc *doc) {
	fprintf(stdout, "%" PRIu64 ", %.*s\n",
		doc->seqnum, (int)doc->bodylen, (char*)doc->body);
}

static void telnet_print_key_doc(fdb_doc *doc) {
	fprintf(stdout, "%" PRIu64 ", %.*s, %.*s\n",
		doc->seqnum, (int)doc->keylen, (char*)doc->key, (int)doc->bodylen, (char*)doc->body);
}

void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length) {

	fdb_status ret;

	if(command) {
		if(!strcmp(command, "topic")) {
			ret = do_topic_command(db, key);
		} else if(!strcmp(command, "get")) {
			ret = do_get_command(db, key, telnet_print_doc);
		} else if(!strcmp(command, "iterate")) {
			fdb_seqnum_t start = 0;
			string_to_seqnum(key, &start);
			ret = do_iterate_command(db, start, telnet_print_key_doc);
		} else {
			//fprintf(stderr, "fail: unknown command\n");
			fprintf(stdout, "fail: unknown command\n");
		}
		fflush(stdout);
	}
}

