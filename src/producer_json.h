#ifndef KAYOS_SRC_PRODUCER_JSON_H_
#define KAYOS_SRC_PRODUCER_JSON_H_

#include <forestdb.h>
#include <jansson.h>

void call_json_set(fdb_file_handle *dbfile, fdb_kvs_handle *db, json_t *json_errors, json_t *json);
void call_json_delete(fdb_file_handle *dbfile, fdb_kvs_handle *db, json_t *json_errors, json_t *json);
void call_producer_json(fdb_file_handle *dbfile, fdb_kvs_handle *db, json_t *json);

#endif
