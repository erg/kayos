#ifndef KAYOS_SRC_HTTP_H_
#define KAYOS_SRC_HTTP_H_

#include <stdlib.h>
#include <forestdb.h>

#include "kayos_common.h"

size_t handle_http(fdb_file_handle *dbfile, fdb_kvs_handle *db, forestdb_handler handler, char *command, char *ptr, size_t len);

#endif
