#ifndef KAYOS_SRC_PRODUCER_H_
#define KAYOS_SRC_PRODUCER_H_

#include <stdlib.h>
#include <forestdb.h>

void do_set_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	void *key, size_t key_length,
	void *value, size_t value_length);

void do_delete_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	void *key, size_t key_length);

void do_forestdb_producer_command(fdb_file_handle *dbfile, fdb_kvs_handle *db,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length);

#endif
