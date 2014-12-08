#ifndef KAYOS_SRC_CONSUMER_CLIENT_H_
#define KAYOS_SRC_CONSUMER_CLIENT_H_

#include <stdlib.h>
#include <forestdb.h>

char *doc_to_string(fdb_doc *rdoc);

void do_get_queue(fdb_file_handle *dbfile, fdb_kvs_handle *db, const char *key);
void do_get_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, const char *key);
void do_iterate_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, const char *key);
void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, const char *command, const char *key, size_t key_length, const char *val, size_t val_length);

#endif
