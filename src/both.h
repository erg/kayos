#ifndef KAYOS_SRC_BOTH_H_
#define KAYOS_SRC_BOTH_H_

#include <stdlib.h>
#include <sys/types.h>
#include <forestdb.h>
#include <jansson.h>

#define BUFFER_LENGTH 1024

typedef void forestdb_handler_t(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length);

typedef size_t http_handler_t(fdb_file_handle *dbfile,fdb_kvs_handle *db,
	forestdb_handler_t handler,
	char *command,
	char *ptr, size_t len);

typedef void json_handler_t(fdb_file_handle *dbfile, fdb_kvs_handle *db, json_t *json);

typedef void fdb_doc_print_t(fdb_doc *doc);

int client_usage(int argc, char *argv[]);
fdb_file_handle *init_fdb_file_handle(const char *path);
fdb_kvs_handle *init_fdb_kvs_handle(fdb_file_handle *dbfile);

// Forestdb

char *doc_to_string(fdb_doc *doc);

// returns 1 if success
int string_to_seqnum(char *str, fdb_seqnum_t *in_out);

// Commands
fdb_status do_topic_command(fdb_kvs_handle *db, const char *dbname);

void command_ok(fdb_status status);

// Parser
void parse_key_value(const char *line, char **key, char **value);

void client_loop(const char *dbname,
		forestdb_handler_t handler,
		http_handler_t handle_http,
		json_handler_t json_handler);

ssize_t parse_line(fdb_file_handle *dbfile,
	fdb_kvs_handle *db,
	forestdb_handler_t handler,
	http_handler_t http_handler,
	char *line, size_t len);

size_t parse_binary(fdb_file_handle *dbfile,
	fdb_kvs_handle *db,
	forestdb_handler_t handler,
	char *line, size_t len);

ssize_t parse_json(fdb_file_handle *dbfile,
	fdb_kvs_handle *db,
	json_handler_t handler,
	char *buffer, size_t len);

ssize_t handle_buffer(fdb_file_handle *dbfile,
	fdb_kvs_handle *db,
	forestdb_handler_t handler,
	http_handler_t handle_http,
	json_handler_t json_handler,
	char *buffer, size_t len);



#endif
