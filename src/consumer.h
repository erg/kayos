#ifndef KAYOS_SRC_CONSUMER_H_
#define KAYOS_SRC_CONSUMER_H_

#include <stdlib.h>
#include <forestdb.h>

#include "both.h"

fdb_status do_get_command(fdb_kvs_handle *kvs, const char *key, fdb_doc_print_t cb);
fdb_status do_iterate_command(fdb_kvs_handle *kvs, fdb_seqnum_t start, fdb_doc_print_t cb);
void do_forestdb_consumer_command(fdb_file_handle *dbfile, fdb_kvs_handle *kvs,
	char *command,
	void *key, size_t key_length,
	void *value, size_t value_length);

#endif
