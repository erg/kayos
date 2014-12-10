#ifndef KAYOS_SRC_CONSUMER_JSON_H_
#define KAYOS_SRC_CONSUMER_JSON_H_

#include <forestdb.h>
#include <jansson.h>

#include "both.h"

void call_json_get(fdb_kvs_handle *db, json_t *json_errors, json_t *json);
void call_json_iterate(fdb_kvs_handle *db, json_t *json_errors, json_t *json);
void call_consumer_json(fdb_file_handle *dbfile, fdb_kvs_handle *db, json_t *json);

#endif
