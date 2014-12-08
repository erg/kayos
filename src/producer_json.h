#ifndef KAYOS_SRC_PRODUCER_JSON_H_
#define KAYOS_SRC_PRODUCER_JSON_H_

#include <forestdb.h>
#include <jansson.h>

void call_producer_json(fdb_kvs_handle *db, json_t *json);

#endif
