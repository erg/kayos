#include "consumer_json.h"

#include <forestdb.h>
#include <jansson.h>

#include <string.h>

#include "both.h"
#include "consumer.h"
#include "errors_json.h"

void call_json_get(fdb_kvs_handle *db, json_t *json_errors, json_t *json) {
	const char *required_keys[] = {"command", "key", 0};
	ensure_json_keys(json, json_errors, required_keys, NULL);
	json_t *value = get_json_string(json_errors, json, "key");

	if(!json_errors_p(json_errors))
		do_get_command(db, json_string_value(value));

	json_decref(value);
}

void call_json_iterate(fdb_kvs_handle *db, json_t *json_errors, json_t *json) {
	const char *required_keys[] = {"command", 0};
	const char *optional_keys[] = {"start", 0};

	ensure_json_keys(json_errors, json, required_keys, optional_keys);

	json_t *value = get_json_integer(json_errors, json, "start");
	int start = value ? json_integer_value(value) : 0;

	if(!json_errors_p(json_errors))
		do_iterate_command(db, start);
}

void call_consumer_json(fdb_kvs_handle *db, json_t *json) {
	json_t *json_errors = new_json_errors();
	json_t *command_object = get_json_string(json_errors, json, "command");
	if(command_object) {
		const char *command = json_string_value(command_object);

		if(!strcmp(command, "get")) {
			call_json_get(db, json_errors, json);
		} else if(!strcmp(command, "iterate")) {
			call_json_iterate(db, json_errors, json);
		} else {
			add_custom_json_error(json_errors, "key", "command", "error", "NOT_A_COMMAND");
		}
	}

	if(json_errors_p(json_errors))
		print_json_object(stdout, json_errors);

	json_decref(json_errors);
}
