#include "kayos_common.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "buffer.h"
#include "http.h"
#include "io.h"
#include "utils.h"

// Allow a-z first char, then a-z, 0-9, _$()+-/
int kayos_dbname_valid_p(const char *dbname) {
	if(!dbname)
		return 0;
	if(!(dbname[0] >= 'a' && dbname[0] <= 'z'))
		return 0;
	return strlen(dbname + 1) == strspn(dbname + 1, "abcdefghijklmnopqrstuvwxyz0123456789_$()+-/");
}

struct fdb_handles init_fdb(const char *path) {
	if(!kayos_dbname_valid_p(path))
		fatal_error("invalid dbname");
	fdb_status status;
	fdb_file_handle *dbfile;
	fdb_kvs_handle *db;
	fdb_config fconfig = fdb_get_default_config();
	fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
	status = fdb_open(&dbfile, path, &fconfig);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_open failed");
	status = fdb_kvs_open_default(dbfile, &db, &kvs_config);
	if(status != FDB_RESULT_SUCCESS)
		fatal_error("fdb_kvs_open_default failed");

	struct fdb_handles fdb_handles;
	fdb_handles.dbfile = dbfile;
	fdb_handles.db = db;
	return fdb_handles;
}

void close_fdb_handles(struct fdb_handles handles) {
	fdb_kvs_close(handles.db);
	fdb_close(handles.dbfile);
}

void handle_json_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, json_t *obj) {

	// This is just debug stuff
	void *iter = json_object_iter(obj);
	if(!iter) fatal_error("json_object_iter");

	int count = 0;
	do {
		const char *key = json_object_iter_key(iter);
		const json_t *value = json_object_iter_value(iter);
		fprintf(stderr, "got obj %d:\n", count);
		fprintf(stderr, " key: %s\n", key);
		fprintf(stderr, " value: %s\n", json_string_value(value));
		iter = json_object_iter_next(obj, iter);
		count++;
	} while(iter);



	// Real command handling
	json_t *command_object = json_object_get(obj, "command");
	json_t *key_object = json_object_get(obj, "key");
	json_t *value_object = json_object_get(obj, "value");
	char *command = (char *)json_string_value(command_object);
	char *key = (char *)json_string_value(key_object);
	char *value = (char *)json_string_value(value_object);

	fprintf(stderr, "command: %s\n", command);
	fprintf(stderr, "key: %s\n", key);
	fprintf(stderr, "value: %s\n", value);

	handler(dbfile, db, command,
						key,
						key ? strlen(key) : 0,
						value,
						value ? strlen(value) : 0);
}

ssize_t parse_json(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *buffer, size_t len) {
	named_hexdump(stderr, "parse_json", buffer, len);
	//buffer[len-1] = 0;

	json_error_t error;
	json_t *obj;

	obj = json_loads(buffer, JSON_REJECT_DUPLICATES, &error);
	if(!obj) {
		//json_fatal_error("json_loads messed up", error);
		fprintf(stderr, "json loads messed up!\n");
		json_print_error(error);
		return len - error.position;
	}

	fprintf(stderr, "json_loads() succeeded!\n");
	json_print_error(error);

	// [{},{},{}]
	if(json_is_array(obj)) {
		size_t index;
		json_t *value;
		json_array_foreach(obj, index, value) {
			handle_json_command(dbfile, db, handler, value);
		}
	} else {
	// {}
		handle_json_command(dbfile, db, handler, obj);
	}
/*
// TODO: figure out cleanup here
#ifdef DEBUG
	fprintf(stderr, "decref command_object\n");
#endif
	if(command_object) json_decref(command_object);
#ifdef DEBUG
	fprintf(stderr, "decref key_object\n");
#endif
	if(key_object) json_decref(key_object);
#ifdef DEBUG
	fprintf(stderr, "decref value_object\n");
#endif
	if(value_object) json_decref(value_object);
#ifdef DEBUG
	fprintf(stderr, "decref obj\n");
#endif
	if(obj) json_decref(obj);
#ifdef DEBUG
	fprintf(stderr, "returning error.position: %d\n", error.position);
#endif
*/

	return len - error.position;
}

size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len) {
	fatal_error("unimplemented parse_binary");
	return 0;
}

ssize_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len) {
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
		return handle_http(dbfile, db, handler, command, ptr, end - ptr);
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

ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *buffer, size_t len) {
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
			remaining = parse_json(dbfile, db, handler, ptr, end - ptr);
		else if(*ptr <= 127)
			remaining = parse_line(dbfile, db, handler, ptr, end - ptr);
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

void client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler) {
	char buffer[BUFFER_LENGTH];
	size_t remaining = 0;
	do {
#ifdef DEBUG
		named_hexdump(stderr, "producer_client loop head", buffer, remaining);
#endif
		ssize_t nbytes = safe_read(0, buffer + remaining, sizeof(buffer) - 1 - remaining);
		if(nbytes == 0) {
#ifdef DEBUG
			fprintf(stderr, "producer_client: client disconnected\n");
#endif
			break;
		} else if(nbytes == -1) {
			libc_fatal_error("consumer_client: safe_read");
		} else {
			buffer[nbytes] = 0;
#ifdef DEBUG
			named_hexdump(stderr, "consumer_client got", buffer, nbytes);
#endif
			remaining = handle_buffer(dbfile, db, handler, buffer, nbytes);
		}

		// Used for closing HTTP commands.
		// We returned -1, so we should disconnect the client.
		if(remaining == -1) {
			break;
		}
	} while(1);

#ifdef DEBUG
	if(remaining > 0) {
		named_hexdump(stderr, "consumer client dc, unprocessed commands", buffer, remaining);
	}
#endif
}

int client_usage(int argc, char *argv[]) {
	fprintf(stderr, "usage: %s dbpath\n", argv[0]);
	return 0;
}

