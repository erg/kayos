#include "consumer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>
#include <jansson.h>

#include "both.h"
#include "buffer.h"
#include "io.h"

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

void do_iterate_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, const char *key) {
	fdb_status status;
	fdb_iterator *iterator;
	fdb_doc *rdoc;

	fdb_seqnum_t start = 0;
	if(key) {
		char *end;
		long long value = strtoll(key, &end, 10); 
		if (end == key || *end != '\0' || errno == ERANGE) {
			fprintf(stderr, "strtol failed: key = %s\n", key);
			return;
		} else {
			start = value;
		}
	}

	status = fdb_iterator_sequence_init(db, &iterator, start, -1, FDB_ITR_NONE);
	fprintf(stderr, "fdb_iterator_sequence_init status: %d, start: %llu\n", status, start);
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

void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length) {

	fprintf(stderr, "executing fdb consumer command: %s\n", command);
	if(!strncmp(command, "topic", 5)) {
		fprintf(stderr, "TOPIC\n");
		do_topic_command(dbfile, db, key);
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

