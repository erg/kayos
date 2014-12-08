#ifndef KAYOS_SRC_CONSUMER_HTTP_H_
#define KAYOS_SRC_CONSUMER_HTTP_H_

#include <forestdb.h>

#include "both.h"

size_t handle_consumer_http(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler_t handler, char *command, char *ptr, size_t len);

#endif
