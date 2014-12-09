#include "json_utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "errors.h"

void json_print_error(json_error_t error) {
	fprintf(stderr, "json_error_t value:\n");
	fprintf(stderr, " text: \"%s\"\n", error.text);
	fprintf(stderr, " source: \"%s\"\n", error.source);
	fprintf(stderr, " line: %d\n", error.line);
	fprintf(stderr, " column: %d\n", error.column);
	fprintf(stderr, " position: %d\n", error.position);
}

void json_fatal_error(const char *msg, json_error_t error) {
	fprintf(stderr, "json_fatal_error: %s\n", msg);
	json_print_error(error);
	abort();
}

json_t *new_json_errors() {
	json_t *json_errors = json_array();
	if(!json_errors) fatal_error("start_json_errors(): json_array() failed");
	return json_errors;
}

json_t *make_json_base_error(const char *key) {
	json_t *error = json_object();
	if(!error) fatal_error("make_json_base_error(): json_object() failed");
	json_object_set_new(error, "status", json_string("fail"));
	if(key)
		json_object_set_new(error, "key", json_string(key));
	return error;
}

void print_json_object(FILE *restrict stream, json_t *json) {
	char *string = json_dumps(json, 0);
	if(!string) fatal_error("json_dumps() failed");
	fprintf(stream, "%s\n", string);
	free(string);
}

void print_and_free_json_object(FILE *restrict stream, json_t *json) {
	char *string = json_dumps(json, 0);
	if(!string) fatal_error("json_dumps() failed");
	fprintf(stream, "%s\n", string);
	json_decref(json);
	free(string);
}

void add_expected_json_key_error(json_t *json_errors, const char *key) {
	json_t *error = make_json_base_error(key);
	json_object_set_new(error, "reason", json_string("EXPECTED_KEY"));
	json_array_append_new(json_errors, error);
}

void add_unexpected_json_key_error(json_t *json_errors, const char *key) {
	json_t *error = make_json_base_error(key);
	json_object_set_new(error, "reason", json_string("UNEXPECTED_KEY"));
	json_array_append_new(json_errors, error);
}

void add_expected_json_string_error(json_t *json_errors, const char *key) {
	json_t *error = make_json_base_error(key);
	json_object_set_new(error, "reason", json_string("EXPECTED_STRING"));
	json_array_append_new(json_errors, error);
}

void add_expected_json_integer_error(json_t *json_errors, const char *key) {
	json_t *error = make_json_base_error(key);
	json_object_set_new(error, "reason", json_string("EXPECTED_INTEGER"));
	json_array_append_new(json_errors, error);
}

void add_custom_json_error(json_t *json_errors, const char *key, const char *value,
	const char *key2, const char *value2) {
	json_t *error = make_json_base_error(NULL);
	json_object_set_new(error, key, json_string(value));
	json_object_set_new(error, key2, json_string(value2));
	json_array_append_new(json_errors, error);
}

json_key_type which_key(const char *required_keys[],
	const char *optional_keys[],
	const char *candidate) {

	if(!candidate)
		return KEY_NOT_FOUND;

	char **key_ptr = (char **)required_keys;
	if(key_ptr) {
		while(*key_ptr) {
			if(!strcmp(candidate, *key_ptr))
				return REQUIRED_KEY;
			key_ptr++;
		}
	}

	key_ptr = (char **)optional_keys;
	if(key_ptr) {
		while(*key_ptr) {
			if(!strcmp(candidate, *key_ptr))
				return OPTIONAL_KEY;
			key_ptr++;
		}
	}

	return KEY_NOT_FOUND;
}

// Return 0 on success
void ensure_json_keys(json_t *json_errors, json_t *json,
	const char *required_keys[],
	const char *optional_keys[]) {

	char **key_ptr = (char **)required_keys;

	if(key_ptr) {
		while(*key_ptr) {
			// side effect is key will get added to json_errors if not found
			get_json_string_required(json_errors, json, *key_ptr);
			key_ptr++;
		}
	}

	void *iter = json_object_iter(json);
	if(!iter) fatal_error("ensure_json_keys(): json_object_iter() failed");

	do {
		const char *key = json_object_iter_key(iter);
		json_key_type type = which_key(required_keys, optional_keys, key);

		if(type == KEY_NOT_FOUND)
			add_unexpected_json_key_error(json_errors, key);

		iter = json_object_iter_next(json, iter);
	} while(iter);
}

int json_errors_p(json_t *json_errors) {
	return json_array_size(json_errors) > 0;
}

json_t *get_json_string_required(json_t *json_errors, json_t *json, const char *key) {
	json_t *value = json_object_get(json, key);
	if(!value) {
		add_expected_json_key_error(json_errors, key);
		return NULL;
	} else if(!json_is_string(value)) {
		add_expected_json_string_error(json_errors, key);
		return NULL;
	} else {
		return value;
	}
}

json_t *get_json_integer_required(json_t *json_errors, json_t *json, const char *key) {
	json_t *value = json_object_get(json, key);
	if(!value) {
		add_expected_json_key_error(json_errors, key);
		return NULL;
	} else if(!json_is_integer(value)) {
		add_expected_json_integer_error(json_errors, key);
		return NULL;
	} else {
		return value;
	}
}

json_t *get_json_string_optional(json_t *json_errors, json_t *json, const char *key) {
	json_t *value = json_object_get(json, key);
	if(!value) {
		return NULL;
	} else if(!json_is_string(value)) {
		add_expected_json_string_error(json_errors, key);
		return NULL;
	} else {
		return value;
	}
}

json_t *get_json_integer_optional(json_t *json_errors, json_t *json, const char *key) {
	json_t *value = json_object_get(json, key);
	if(!value) {
		return NULL;
	} else if(!json_is_integer(value)) {
		add_expected_json_integer_error(json_errors, key);
		return NULL;
	} else {
		return value;
	}
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

