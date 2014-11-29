#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <forestdb.h>

#include "kayos_common.h"
#include "producer_client.h"
#include "buffer.h"
#include "io.h"
#include "utils.h"

int main(int argc, char *arg[]) {
	struct fdb_handles handles = init_fdb("./test.fdb");
	client_loop(handles.dbfile, handles.db, do_forestdb_producer_command);
	close_stdout();
	close_fdb_handles(handles);
	return 0;
}
