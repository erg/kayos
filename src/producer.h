#ifndef KAYOS_SRC_PRODUCER_H_
#define KAYOS_SRC_PRODUCER_H_

#include <stdlib.h>
#include <forestdb.h>

fdb_status do_set_command(fdb_file_handle *db, fdb_kvs_handle *kvs,
	void *key, size_t key_length,
	void *value, size_t value_length);

fdb_status do_delete_command(fdb_file_handle *db, fdb_kvs_handle *kvs,
	void *key, size_t key_length);

void do_forestdb_producer_command(fdb_file_handle *db, fdb_kvs_handle *kvs,
	char *kvs_name,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length);

#endif
