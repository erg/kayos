#ifndef KAYOS_SRC_BOTH_H_
#define KAYOS_SRC_BOTH_H_

#include <stdlib.h>
#include <sys/types.h>
#include <forestdb.h>
#include <jansson.h>

#define BUFFER_LENGTH 1024

typedef void forestdb_handler_t(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length);
typedef size_t http_handler_t(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, char *command, char *ptr, size_t len);

int client_usage(int argc, char *argv[]);

fdb_file_handle *init_fdb_file_handle(const char *path);
fdb_kvs_handle *init_fdb_kvs_handle(fdb_file_handle *dbfile);

char *doc_to_string(fdb_doc *rdoc);
void parse_key_value(const char *line, char **key, char **value);
void do_topic_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, const char *dbname);

void client_loop(const char *dbname, forestdb_handler_t handler, http_handler_t handle_http);
ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, http_handler_t handle_http, char *buffer, size_t len);

void handle_json_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, json_t *obj);

ssize_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, http_handler_t http_handler, char *line, size_t len);
size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, char *line, size_t len);
ssize_t parse_json(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, char *buffer, size_t len);


#endif
