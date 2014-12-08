#include "consumer_json.h"

#include <forestdb.h>
#include <jansson.h>

#include <string.h>

#include "both.h"
#include "consumer.h"
#include "errors_json.h"

// {"command": "iterate", "omgasdf": "lol"}


void call_json_get(fdb_kvs_handle *db, json_t *json_errors, json_t *json) {
	const char *required_keys[] = {"command", "key", 0};
	ensure_json_keys(json, json_errors, required_keys, NULL);

	if(send_json_errors_p(json_errors)) return;

	json_t *value = get_json_string(json_errors, json, "key");
	do_get_command(db, json_string_value(value));
	json_decref(value);
}

void call_json_iterate(fdb_kvs_handle *db, json_t *json_errors, json_t *json) {
	fprintf(stderr, "call_json_iterate() called\n");
	const char *required_keys[] = {"command", 0};
	const char *optional_keys[] = {"start", 0};

	ensure_json_keys(json_errors, json, required_keys, optional_keys);

	json_t *value = get_json_integer(json_errors, json, "start");
	int start = value ? json_integer_value(value) : 0;

	// Send errors and abort command
	if(send_json_errors_p(json_errors)) {
		safe_json_decref(value);
		return;
	}

	fprintf(stderr, "start: %d\n", start);
	do_iterate_command(db, start);
	safe_json_decref(value);
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
			add_custom_json_error(json_errors, "error", "NOT_A_COMMAND");
		}
		//safe_json_decref(command_object);
	} else {
		add_custom_json_error(json_errors, "error", "command key missing");
	}

	print_json_object(stdout, json_errors);

	size_t index;
    json_t *value;

    json_array_foreach(json_errors, index, value) {
    	json_decref(value);
    }
	safe_json_decref(json_errors);

	fprintf(stderr, "done call_consumer_json\n");
}
