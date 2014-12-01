#ifndef KAYOS_SRC_KAYOS_COMMON_H_
#define KAYOS_SRC_KAYOS_COMMON_H_

//#include <errno.h>
#include <stdio.h>
//#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#define BUFFER_LENGTH 1024

typedef void forestdb_handler(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length);

struct fdb_handles {
	fdb_file_handle *dbfile;
	fdb_kvs_handle *db;
};

struct fdb_handles init_fdb(const char *path);
void close_fdb_handles(struct fdb_handles handles);

ssize_t parse_line(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len);
size_t parse_binary(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *line, size_t len);

ssize_t handle_buffer(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *buffer, size_t len);
void client_loop(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler);

int client_usage(int argc, char *argv[]);

#endif
