#include "kayos_paths.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

void *safe_malloc(size_t size) {
	void *ptr = malloc(size);
	if(!ptr)
		fatal_error("malloc failed");
	return ptr;
}

// vsnprintf returns number of characters that would have been printed, need to +1 for null
// ptr is null terminated
void *malloc_vsnprintf(const char * restrict format, ...) {
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args);
	va_end(args);
	va_start(args, format);
	void *ptr = safe_malloc(len + 1);
	vsnprintf(ptr, len + 1, format, args);
	va_end(args);
	return ptr;
}

void *get_kayos_data_path() {
	char *data_path = getenv("KAYOS_DATA_PATH");
	if(!data_path)
		return malloc_vsnprintf("%s", "/usr/local/var/lib/kayos");
	else
		return malloc_vsnprintf("%s", data_path);
}

void *get_kayos_data_path_for(char *dbname) {
	return 0;
}


void ensure_kayos_data_path() {
	int ret;
	char *path = get_kayos_data_path();
	fprintf(stderr, "ensure_kayos_data_path(), path: %s\n", path);
	ret = mkpath(path, 0777);
	if(ret == -1)
		fatal_error("mkdir failed");
	free(path);
}
