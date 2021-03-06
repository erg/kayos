#ifndef KAYOS_SRC_PRODUCER_HTTP_H_
#define KAYOS_SRC_PRODUCER_HTTP_H_

#include <stdlib.h>
#include <forestdb.h>

#include "both.h"

size_t handle_producer_http(fdb_file_handle *db, fdb_kvs_handle *kvs,
	forestdb_handler_t handler,
	char *ptr, size_t len);

#endif
