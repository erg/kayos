#include "kayos_paths.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io.h"
#include "utils.h"

int safe_mkdir(const char *path, mode_t mode) {
	int ret;
	ret = mkdir(path, mode);
	if(ret == -1 && errno != EEXIST)
		return -1;
	return 0;
}

// http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
int mkpath(char* path, mode_t mode) {
	if(!path)
		return 0;

	char *p;
	for(p = strchr(path + 1, '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';
		if(safe_mkdir(path, mode) == -1) {
			*p = '/';
			return -1;
		}
		*p = '/';
	}
	return safe_mkdir(path, mode);
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


void ensure_kayos_data_path() {
	int ret;
	char *path = get_kayos_data_path();
	fprintf(stderr, "ensure_kayos_data_path(), path: %s\n", path);
	ret = mkpath(path, 0777);
	if(ret == -1)
		libc_fatal_error("mkdir failed");
	free(path);
}
