#include "paths.h"

#include <stdio.h>

#include "errors.h"
#include "io.h"

void ensure_kayos_data_path() {
	int ret;
	char *path = get_kayos_data_path();
	fprintf(stderr, "ensure_kayos_data_path(), path: %s\n", path);
	ret = mkpath(path, 0777);
	if(ret == -1)
		libc_fatal_error("mkdir failed");
	free(path);
}

void *get_kayos_data_path() {
	char *data_path = getenv("KAYOS_DATA_PATH");
	if(!data_path)
		return malloc_vsnprintf("%s", "/usr/local/var/lib/kayos");
	else
		return malloc_vsnprintf("%s", data_path);
}

void *get_kayos_data_path_for(const char *dbname) {
	char *data_path = get_kayos_data_path();
	char *dbpath = malloc_vsnprintf("%s/%s", data_path, dbname);
	free(data_path);
	return dbpath;
}
