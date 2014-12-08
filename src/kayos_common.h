#ifndef KAYOS_SRC_KAYOS_COMMON_H_
#define KAYOS_SRC_KAYOS_COMMON_H_

//#include <errno.h>
#include <stdio.h>
//#include <string.h>
#include <stdlib.h>

#include <forestdb.h>
#include <jansson.h>

#define BUFFER_LENGTH 1024

typedef void forestdb_handler(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length);

fdb_file_handle *init_fdb_file_handle(const char *path);
fdb_kvs_handle *init_fdb_kvs_handle(fdb_file_handle *dbfile);

void handle_json_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, json_t *obj);

ssize_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len);
size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len);

ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *buffer, size_t len);
void client_loop(fdb_file_handle *dbfile, forestdb_handler handler);

int client_usage(int argc, char *argv[]);
int kayos_dbname_valid_p(const char *dbname);

void ensure_kayosdb_path();

#endif
