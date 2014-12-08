#include <forestdb.h>

#include "both.h"
#include "consumer.h"
#include "consumer_http.h"
#include "io.h"
#include "paths.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        return client_usage(argc, argv);
    }

	ensure_kayos_data_path();
	char *dbname = argv[1];
	client_loop(dbname, do_forestdb_consumer_command, handle_consumer_http);
	close_stdout();
	return 0;
}