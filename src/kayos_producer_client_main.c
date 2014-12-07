#include <forestdb.h>

#include "kayos_common.h"
#include "kayos_paths.h"
#include "producer_client.h"
#include "io.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		return client_usage(argc, argv);
	}

	ensure_kayos_data_path();

	char *dbname = argv[1];
	struct fdb_handles handles = init_fdb(dbname);
	client_loop(handles.dbfile, handles.db, do_forestdb_producer_command);
	close_stdout();
	close_fdb_handles(handles);
	return 0;
}
