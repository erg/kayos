#ifndef KAYOS_SRC_ERRORS_JSON_H_
#define KAYOS_SRC_ERRORS_JSON_H_

#include <forestdb.h>
#include <jansson.h>

typedef enum  {
	REQUIRED_KEY = 0,
	OPTIONAL_KEY = 1,
	KEY_NOT_FOUND = 2,
} json_key_type;

#define FOREACH_JSON_ERROR(ERROR) \
        ERROR(COMMAND_MISSING)   \
        ERROR(INVALID_COMMAND)  \
        ERROR(KEY_REQUIRED)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum KAYOS_JSON_ERROR_ENUM {
    FOREACH_JSON_ERROR(GENERATE_ENUM)
};

static const char *KAYOS_JSON_ERROR_STRING[] = {
    FOREACH_JSON_ERROR(GENERATE_STRING)
};

void json_print_error(json_error_t error);
void json_fatal_error(const char *msg, json_error_t error);

json_t *make_json_base_error(const char *key);
json_t *new_json_errors();
void print_json_object(FILE *restrict stream, json_t *json);
void print_and_free_json_object(FILE *restrict stream, json_t *json);
void add_expected_json_key_error(json_t *json_errors, const char *key);
void add_unexpected_json_key_error(json_t *json_errors, const char *key);
void add_expected_json_string_error(json_t *json_errors, const char *key);
void add_expected_json_integer_error(json_t *json_errors, const char *key);
void add_custom_json_error(json_t *json_errors, const char *key, const char *value, const char *key2, const char *value2);

json_key_type which_key(const char *required_keys[], const char *optional_keys[], const char *candidate);
void ensure_json_keys(json_t *json_errors, json_t *json, const char *required_keys[], const char *optional_keys[]);
int json_errors_p(json_t *json_errors);
json_t *get_json_string_required(json_t *json_errors, json_t *json, const char *key);
json_t *get_json_integer_required(json_t *json_errors, json_t *json, const char *key);
json_t *get_json_string_optional(json_t *json_errors, json_t *json, const char *key);
json_t *get_json_integer_optional(json_t *json_errors, json_t *json, const char *key);

#endif
