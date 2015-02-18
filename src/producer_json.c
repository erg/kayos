#include "producer_json.h"

#include <string.h>

#include <forestdb.h>
#include <jansson.h>

#include "both.h"
#include "producer.h"
#include "json_utils.h"

void call_json_set(fdb_file_handle *db, fdb_kvs_handle *kvs,
	json_t *json_errors,
	json_t *json) {

	const char *key = json_string_value(
		get_json_string_required(json_errors, json, "key"));

	const char *value = json_string_value(
		get_json_string_required(json_errors, json, "value"));

	if(!json_errors_p(json_errors))
		do_set_command(db, kvs,
			(void *)key, strlen(key),
			(void *)value, strlen(value));
}

void call_json_delete(fdb_file_handle *db, fdb_kvs_handle *kvs,
	json_t *json_errors,
	json_t *json) {

	const char *key = json_string_value(
		get_json_string_required(json_errors, json, "key"));

	if(!json_errors_p(json_errors))
		do_delete_command(db, kvs, (void *)key, strlen(key));
}


void call_producer_json(fdb_file_handle *db,
	fdb_kvs_handle *kvs,
	json_t *json) {

	json_t *json_errors = new_json_errors();
	const char *command = json_string_value(
		get_json_string_required(json_errors, json, "command"));

	if(command) {
		if(!strcmp(command, "set"))
			call_json_set(db, kvs, json_errors, json);
		else if(!strcmp(command, "delete"))
			call_json_delete(db, kvs, json_errors, json);
		else
			add_custom_json_error(json_errors, "key", "command", "error", "NOT_A_PRODUCER_COMMAND");
	}

	if(json_errors_p(json_errors))
		print_json_object(stdout, json_errors);

	json_decref(json_errors);
}
