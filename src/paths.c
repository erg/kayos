#include "paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "io.h"

// Allow a-z first char, then a-z, 0-9, _$()+-/
int kayos_dbname_valid_p(const char *dbname) {
    if(!dbname)
        return 0;
    if(!(dbname[0] >= 'a' && dbname[0] <= 'z'))
        return 0;
    return strlen(dbname + 1) == strspn(dbname + 1, "abcdefghijklmnopqrstuvwxyz0123456789_$()+-/");
}

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
