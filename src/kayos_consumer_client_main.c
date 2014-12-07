#include <forestdb.h>

#include "kayos_common.h"
#include "kayos_paths.h"
#include "consumer_client.h"
#include "io.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        return client_usage(argc, argv);
    }

	ensure_kayos_data_path();
	char *dbname = argv[1];
	client_loop(dbname, do_forestdb_consumer_command);
	close_stdout();
	return 0;
}
