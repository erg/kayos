#include <forestdb.h>

#include "both.h"
#include "io.h"
#include "paths.h"
#include "producer.h"
#include "producer_http.h"
#include "producer_json.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		return client_usage(argc, argv);
	}

	ensure_kayos_data_path();
	char *dbname = argv[1];
	client_loop(dbname, do_forestdb_producer_command, handle_producer_http, call_producer_json);
	close_stdout();
	return 0;
}
