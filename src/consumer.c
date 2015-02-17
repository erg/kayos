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
#include "debug.h"
#include "errors.h"
#include "io.h"

static void telnet_print_doc(fdb_doc *doc);
static void telnet_print_key_doc(fdb_doc *doc);

fdb_status do_get_command(fdb_kvs_handle *kvs, const char *key, fdb_doc_print_t print_cb) {
	fdb_status status = FDB_RESULT_INVALID_ARGS;
	fdb_doc *doc = NULL;

	if(!key)
		fatal_error("do_get_command() called with key = null");

	fdb_doc_create(&doc, (void*)key, strlen(key),
			NULL, 0, NULL, 0);
	status = fdb_get(kvs, doc);
	if(status == FDB_RESULT_SUCCESS) {
		//fprintf(stdout, "%s\n", doc_to_string(doc));
		print_cb(doc);
	}
	else if(status == FDB_RESULT_KEY_NOT_FOUND)
		fprintf(stdout, "{\"status\": \"fail\", \"reason\": \"NO_KEY\"}\n");
	fdb_doc_free(doc);
	return status;
}

fdb_status do_iterate_command(fdb_kvs_handle *kvs, fdb_seqnum_t start, fdb_doc_print_t print_cb) {
	fdb_status status = FDB_RESULT_INVALID_ARGS;
	fdb_iterator *iterator;
	fdb_doc *doc = NULL;

	status = fdb_iterator_sequence_init(kvs, &iterator, start, -1, FDB_ITR_NONE);
	debug_print("fdb_iterator_sequence_init status: %d, start: %" PRIu64 "\n", status, start);

	if(status != FDB_RESULT_SUCCESS) {
		debug_print("fdb_iterator_sequence_init failed\n");
		return status;
	}

	while(1) {
		debug_print("about to call fdb_iterator_get\n");
		fdb_iterator_get(iterator, &doc);
		debug_print("fdb_iterator_get got doc %p\n", doc);


		//if (status == FDB_RESULT_ITERATOR_FAIL) break;
		if(status != FDB_RESULT_SUCCESS)
			fatal_error("do_iterate_command() failed 0\n");

		if(!doc) {
			debug_print("iterator got no doc, stopping loop\n");
			break;
		}
		print_cb(doc);
		status = fdb_iterator_next(iterator);
		if (status == FDB_RESULT_ITERATOR_FAIL) break;
	}
	if(doc)
		fdb_doc_free(doc);

	fflush(stdout);
	fdb_iterator_close(iterator);
	return status;
}

static void telnet_print_doc(fdb_doc *doc) {
	fprintf(stdout, "%" PRIu64 ", %.*s\n",
			doc->seqnum, (int)doc->bodylen, (char*)doc->body);
}

static void telnet_print_key_doc(fdb_doc *doc) {
	//debug_print("%" PRIu64 ", %d, %d\n",
	//doc->seqnum, (int)doc->keylen, (int)doc->bodylen);

	fprintf(stdout, "%" PRIu64 ", %.*s, %.*s\n",
			doc->seqnum, (int)doc->keylen, (char*)doc->key, (int)doc->bodylen, (char*)doc->body);
}

void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *kvs,
		char *command,
		void *key, size_t key_length,
		void *value, size_t value_length) {

	fdb_status ret;

	if(command) {
		if(!strcmp(command, "topic")) {
			if(!key)
				key_expected(command);
			else
				ret = do_topic_command(kvs, key);
		} else if(!strcmp(command, "get")) {
			if(!key)
				key_expected(command);
			else
				ret = do_get_command(kvs, key, telnet_print_doc);
		} else if(!strcmp(command, "iterate")) {
			if(!key)
				key_expected(command);
			else {
				fdb_seqnum_t start = 0;
				string_to_seqnum(key, &start);
				ret = do_iterate_command(kvs, start, telnet_print_key_doc);
			}
		} else {
			//fprintf(stderr, "fail: unknown command\n");
			fprintf(stdout, "fail: unknown command\n");
		}
		fflush(stdout);
	}
}

