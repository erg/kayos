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
#include "debug.h"

static char buffer[BUFFER_LENGTH];

int client_usage(int argc, char *argv[]) {
	(void)argc;
	fprintf(stderr, "usage: %s dbpath\n", argv[0]);
	return 0;
}

void key_expected(const char *command) {
	fprintf(stdout, "key expected for command ``%s``\n", command);
}

void value_expected(const char *command) {
	fprintf(stdout, "value expected for command ``%s``\n", command);
}

fdb_file_handle *init_fdb_file_handle(const char *dbname) {
	if(!kayos_dbname_valid_p(dbname))
		fatal_error("invalid dbname");
	fdb_status status;
	fdb_file_handle *db;
	fdb_config db_config = fdb_get_default_config();
	char *path = get_kayos_data_path_for(dbname);
	status = fdb_open(&db, path, &db_config);
	free(path);

	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_open failed");

	return db;
}

fdb_kvs_handle *init_fdb_kvs_handle(fdb_file_handle *db, char *kvs_name) {
	fdb_status status;
	fdb_kvs_handle *kvs;
	fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
	status = fdb_kvs_open(db, &kvs, kvs_name, &kvs_config);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_kvs_open_default failed");

	return kvs;
}

void do_topic_command(char *kvs_name, char *new_name) {
	free(kvs_name);
	kvs_name = strdup(new_name);
}

// Allocates, call free on return value
char *doc_to_string(fdb_doc *doc) {
	json_t *dict = json_object();
	json_object_set_new(dict, "key", json_stringn(doc->key, doc->keylen));
	json_object_set_new(dict, "meta", json_stringn(doc->meta, doc->metalen));
	json_object_set_new(dict, "body", json_stringn(doc->body, doc->bodylen));
	json_object_set_new(dict, "seqnum", json_integer(doc->seqnum));
	char *result = json_dumps(dict, 0);
	if(!result) fatal_error("json_dumps() failed");
	json_decref(dict);
	return result;
}

int string_to_seqnum(char *str, fdb_seqnum_t *in_out) {
	int success = 0;
	if(str) {
		char *end;
		unsigned long long value = strtoull(str, &end, 10);
		if (end == str || *end != '\0' || errno == ERANGE) {
			return 0;
		} else {
			success = 1;
			*in_out = value;
		}
	}
	return success;
}

void parse_key_value(const char *line, char **key, char **value) {
	unimplemented("parse_key_value()");
}

void command_ok(fdb_status status) {
	if(status == FDB_RESULT_SUCCESS)
		fprintf(stdout, "OK\n");
	else
		fprintf(stdout, "FAIL: %s\n", fdb_error_msg(status));
}

void client_loop(const char *dbname,
	forestdb_handler_t handler,
	http_handler_t handle_http,
	json_handler_t json_handler) {

	ssize_t remaining = 0;

	fdb_file_handle *db = NULL;
	fdb_kvs_handle *kvs = NULL;
	char *kvs_name = strdup("default");

	do {

		debug_hexdump("producer_client loop head", buffer, remaining);
		ssize_t nbytes = safe_read(0, buffer + remaining, sizeof(buffer) - 1 - remaining);

		db = init_fdb_file_handle(dbname);
		kvs = init_fdb_kvs_handle(db, kvs_name);

		if(nbytes == 0) {
			debug_print("producer_client: client disconnected\n");
			fdb_kvs_close(kvs);
			kvs = NULL;
			fdb_close(db);
			db = NULL;
			break;
		} else if(nbytes == -1) {
			libc_fatal_error("consumer_client: safe_read");
		} else {
			buffer[nbytes] = 0;
			debug_hexdump("consumer_client got", buffer, nbytes);
			remaining = handle_buffer(db, kvs, kvs_name, handler, handle_http, json_handler, buffer, nbytes);
		}

		// Used for closing HTTP commands.
		// We returned -1, so we should disconnect the client.
		if(remaining == -1) {
			fdb_kvs_close(kvs);
			kvs = NULL;
			fdb_close(db);
			db = NULL;
			break;
		}
		fdb_kvs_close(kvs);
		kvs = NULL;

		fdb_close(db);
		db = NULL;
	} while(1);

	if(kvs_name) {
		free(kvs_name);
		kvs_name = NULL;
	}

	if(kvs) {
		fdb_kvs_close(kvs);
		kvs = NULL;
	}

	if(db) {
		fdb_close(db);
		db = NULL;
	}

#ifdef KAYOS_DEBUG
	if(remaining > 0) {
		debug_hexdump("consumer client dc, unprocessed commands", buffer, remaining);
	}
#endif
}

ssize_t handle_buffer(fdb_file_handle *db, fdb_kvs_handle *kvs, char *kvs_name,
		forestdb_handler_t handler,
		http_handler_t handle_http,
		json_handler_t json_handler,
		char *buffer, size_t len) {

	ssize_t remaining = len;
	char *ptr = buffer;
	char *end = buffer + len;
	ssize_t previous_remaining = remaining;
	do {
		debug_print("******loop start\n");
		debug_hexdump("handle_buffer", ptr, remaining);

		previous_remaining = remaining;
		ptr = buffer_skip_whitespace(ptr, end - ptr);

		if(!ptr || ptr >= end) {
			break;
		}

		if(*ptr == '{' || *ptr == '[')
			remaining = parse_json(db, kvs, kvs_name, json_handler, ptr, end - ptr);
		else if(*ptr <= 127)
			remaining = parse_line(db, kvs, kvs_name, handler, handle_http, ptr, end - ptr);
		else
			remaining = parse_binary(db, kvs, kvs_name, handler, ptr, end - ptr);

		ptr = end - remaining;
		debug_print("previous %zu, remaining: %zu\n", previous_remaining, remaining);
	} while(remaining > 0 && (previous_remaining != remaining));

	debug_print("compacting, remaining: %zu!\n", remaining);
	debug_hexdump("before compaction", buffer, len);

	remaining = compact_buffer(buffer, len, ptr);

	debug_hexdump("after compaction", buffer, remaining);
	return remaining;
}

ssize_t parse_json(fdb_file_handle *db, fdb_kvs_handle *kvs,
	char *kvs_name,
	json_handler_t json_handler,
	char *buffer, size_t len) {

	debug_hexdump("parse_json", buffer, len);

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
		json_t *json_elt;
		json_array_foreach(json, index, json_elt) {
			json_handler(db, kvs, json_elt);
		}
	// {}
	} else
		json_handler(db, kvs, json);

	json_decref(json);
	return len - error.position;
}

size_t parse_binary(fdb_file_handle *db, fdb_kvs_handle *kvs,
	char *kvs_name,
	forestdb_handler_t handler,
	char *line, size_t len) {

	fatal_error("unimplemented parse_binary");
	return 0;
}

ssize_t parse_line(fdb_file_handle *db, fdb_kvs_handle *kvs,
	char *kvs_name,
	forestdb_handler_t handler,
	http_handler_t http_handler,
	char *line, size_t len) {

	debug_print("parse_line called\n");
	char *ptr = line;
	char *command = 0, *key = 0, *val = 0, *rest = 0;
	char *end = line + len;
	char *eol = line;

	// HTTP commands are uppercase
	// send entire line
	if(len > 0 && isupper(line[0])) {
		return http_handler(db, kvs, handler, line, end - line);
	}

	// look for entire line, if no entire line, process it next time around
	size_t first_eol = strcspn(ptr, "\r\n");
	debug_print("first_eol %zu\n", first_eol);
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

	ptr = buffer_skip_whitespace(ptr, end - ptr);
	key = strsep(&ptr, " \t\r\n");
	size_t key_length = key ? strlen(key) : 0;
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	val = strsep(&ptr, " \t\r\n");
	size_t val_length = val ? strlen(val) : 0;
	ptr = buffer_skip_whitespace(ptr, end - ptr);
	rest = ptr;

	debug_print("command: %s\n", command);
	debug_print("key: %s\n", key);
	debug_print("val: %s\n", val);
	debug_print("rest: %s\n\n", rest);

	handler(db, kvs, kvs_name, command, key, key_length, val, val_length);
	return end - eol;
}
