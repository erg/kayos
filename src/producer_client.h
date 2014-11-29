#ifndef KAYOS_SRC_PRODUCER_CLIENT_H_
#define KAYOS_SRC_PRODUCER_CLIENT_H_

#include <stdlib.h>
#include <forestdb.h>

void do_forestdb_producer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db, char *command, char *key, size_t key_length, char *val, size_t val_length);

#endif
