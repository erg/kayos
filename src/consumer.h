#ifndef KAYOS_SRC_CONSUMER_H_
#define KAYOS_SRC_CONSUMER_H_

#include <stdlib.h>
#include <forestdb.h>

void do_get_command(fdb_kvs_handle *db, const char *key);
void do_iterate_command(fdb_kvs_handle *db, fdb_seqnum_t start);
void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length);

#endif
