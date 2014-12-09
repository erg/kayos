#include "both.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>
#include <jansson.h>

#include "buffer.h"
#include "errors.h"
#include "hexdump.h"
#include "io.h"
#include "json_utils.h"
#include "paths.h"

static char buffer[BUFFER_LENGTH];

int client_usage(int argc, char *argv[]) {
    fprintf(stderr, "usage: %s dbpath\n", argv[0]);
    return 0;
}

fdb_file_handle *init_fdb_file_handle(const char *dbname) {
    if(!kayos_dbname_valid_p(dbname))
        fatal_error("invalid dbname");
    fdb_status status;
    fdb_file_handle *dbfile;
    fdb_config fconfig = fdb_get_default_config();
    char *path = get_kayos_data_path_for(dbname);
    status = fdb_open(&dbfile, path, &fconfig);
    free(path);

    if(status != FDB_RESULT_SUCCESS)
        fatal_error("fdb_open failed");

    return dbfile;
}

fdb_kvs_handle *init_fdb_kvs_handle(fdb_file_handle *dbfile) {
    fdb_status status;
    fdb_kvs_handle *db;
    fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
    status = fdb_kvs_open_default(dbfile, &db, &kvs_config);
    if(status != FDB_RESULT_SUCCESS)
        fatal_error("fdb_kvs_open_default failed");

    return db;
}

// Allocates, call free on return value
char *doc_to_string(fdb_doc *rdoc) {
	json_t *dict = json_object();
	json_object_set_new(dict, "key", json_stringn(rdoc->key, rdoc->keylen));
	json_object_set_new(dict, "meta", json_stringn(rdoc->meta, rdoc->metalen));
	json_object_set_new(dict, "body", json_stringn(rdoc->body, rdoc->bodylen));
	json_object_set_new(dict, "seqnum", json_integer(rdoc->seqnum));
	char *result = json_dumps(dict, 0);
	if(!result) fatal_error("json_dumps() failed");
	json_decref(dict);
	return result;
}

void parse_key_value(const char *line, char **key, char **value) {
	unimplemented("parse_key_value()");
}

void do_topic_command(fdb_kvs_handle *db,
	const char *dbname) {
	// open topic or lookup, switch current topic

	unimplemented("do_topic_command()");
}


void client_loop(const char *dbname,
	forestdb_handler_t handler,
	http_handler_t handle_http,
	json_handler_t json_handler) {

	size_t remaining = 0;

	fdb_file_handle *dbfile = NULL;
	fdb_kvs_handle *db = NULL;

	do {

#ifdef DEBUG
		named_hexdump(stderr, "producer_client loop head", buffer, remaining);
#endif
		ssize_t nbytes = safe_read(0, buffer + remaining, sizeof(buffer) - 1 - remaining);

		dbfile = init_fdb_file_handle(dbname);
		db = init_fdb_kvs_handle(dbfile);

		if(nbytes == 0) {
#ifdef DEBUG
			fprintf(stderr, "producer_client: client disconnected\n");
#endif
			fdb_kvs_close(db);
			db = NULL;
			fdb_close(dbfile);
			dbfile = NULL;
			break;
		} else if(nbytes == -1) {
			libc_fatal_error("consumer_client: safe_read");
		} else {
			buffer[nbytes] = 0;
#ifdef DEBUG
			named_hexdump(stderr, "consumer_client got", buffer, nbytes);
#endif
			remaining = handle_buffer(dbfile, db, handler, handle_http, json_handler, buffer, nbytes);
		}

		// Used for closing HTTP commands.
		// We returned -1, so we should disconnect the client.
		if(remaining == -1) {
			fdb_kvs_close(db);
			db = NULL;
			fdb_close(dbfile);
			dbfile = NULL;
			break;
		}
		fdb_kvs_close(db);
		db = NULL;
		fdb_close(dbfile);
		dbfile = NULL;
	} while(1);

	fdb_kvs_close(db);
	fdb_close(dbfile);

#ifdef DEBUG
	if(remaining > 0) {
		named_hexdump(stderr, "consumer client dc, unprocessed commands", buffer, remaining);
	}
#endif
}

ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db,
		forestdb_handler_t handler,
		http_handler_t handle_http,
		json_handler_t json_handler,
		char *buffer, size_t len) {

	ssize_t remaining = len;
	char *ptr = buffer;
	char *end = buffer + len;
	size_t previous_remaining = remaining;
	do {
#ifdef DEBUG
		fprintf(stderr, "******loop start\n");
		named_hexdump(stderr, "handle_buffer", ptr, remaining);
#endif
		previous_remaining = remaining;
		ptr = buffer_skip_whitespace(ptr, end - ptr);

		if(!ptr || ptr >= end) {
			break;
		}

		if(*ptr == '{' || *ptr == '[')
			remaining = parse_json(dbfile, db, json_handler, ptr, end - ptr);
		else if(*ptr <= 127)
			remaining = parse_line(dbfile, db, handler, handle_http, ptr, end - ptr);
		else
			remaining = parse_binary(dbfile, db, handler, ptr, end - ptr);

		ptr = end - remaining;
#ifdef DEBUG
		fprintf(stderr, "previous %zu, remaining: %zu\n", previous_remaining, remaining);
#endif
	} while(remaining > 0 && (previous_remaining != remaining));

#ifdef DEBUG
	fprintf(stderr, "compacting, remaining: %zu!\n", remaining);
	named_hexdump(stderr, "before compaction", buffer, len);
#endif

	remaining = compact_buffer(buffer, len, ptr);

#ifdef DEBUG
	named_hexdump(stderr, "after compaction", buffer, remaining);
#endif
	return remaining;
}

ssize_t parse_json(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	json_handler_t json_handler,
	char *buffer, size_t len) {

#ifdef DEBUG
	named_hexdump(stderr, "parse_json", buffer, len);
#endif

	json_error_t error;
	json_t *json;

	json = json_loads(buffer, JSON_REJECT_DUPLICATES, &error);
	if(!json) {
		//json_fatal_error("json_loads messed up", error);
		fprintf(stderr, "json loads messed up!\n");
		json_print_error(error);
		return len - error.position;
	}
	// error is really parsing results, need not have an error condition
	// json_print_error(error);

	// [{},{},{}]
	if(json_is_array(json)) {
		size_t index;
		json_t *value;
		json_array_foreach(json, index, value) {
			json_handler(dbfile, db, json);
		}
	// {}
	} else
		json_handler(dbfile, db, json);

	json_decref(json);
	return len - error.position;
}

size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	forestdb_handler_t handler,
	char *line, size_t len) {

	fatal_error("unimplemented parse_binary");
	return 0;
}

ssize_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	forestdb_handler_t handler,
	http_handler_t http_handler,
	char *line, size_t len) {

#ifdef DEBUG
	fprintf(stderr, "parse_line called\n");
#endif
	char *ptr = line;
	char *command = 0, *key = 0, *val = 0, *rest = 0;
	char *end = line + len;
	char *eol = line;

	// look for entire line, if no entire line, process it next time around
	size_t first_eol = strcspn(ptr, "\r\n");
#ifdef DEBUG
	fprintf(stderr, "first_eol %zu\n", first_eol);
#endif
	// found trailing NULL, no newline?
	if(ptr + first_eol == end)
		return first_eol;
	eol = ptr + first_eol;
	while(eol != end && (*eol == '\r' || *eol == '\n')) {
		*eol = 0;
		eol++;
	}

	ptr = buffer_skip_whitespace(ptr, end - ptr);
	command = strsep(&ptr, " \t\r\n");

	// HTTP commands are uppercase
	if(isupper(command[0])) {
		return http_handler(dbfile, db, handler, command, ptr, end - ptr);
	}

	ptr = buffer_skip_whitespace(ptr, end - ptr);
	key = strsep(&ptr, " \t\r\n");
	size_t key_length = key ? strlen(key) : 0;
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	val = strsep(&ptr, " \t\r\n");
	size_t val_length = val ? strlen(val) : 0;
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	rest = ptr;
#ifdef DEBUG
	fprintf(stderr, "command: %s\n", command);
	fprintf(stderr, "key: %s\n", key);
	fprintf(stderr, "val: %s\n", val);
	fprintf(stderr, "rest: %s\n\n", rest);
#endif
	handler(dbfile, db, command, key, key_length, val, val_length);
	return end - eol;
}
