#ifndef KAYOS_SRC_KAYOS_PATHS_H_
#define KAYOS_SRC_KAYOS_PATHS_H_
#include <unistd.h>
///#include <stdarg.h>

int safe_mkdir(const char *path, mode_t mode);
int mkpath(char* path, mode_t mode);
void *safe_malloc(size_t size);
void *malloc_vsnprintf(const char * restrict format, ...);

void *get_kayosdb_path();
void *get_kayosdb_path_for(char *dbname);

void ensure_kayosdb_path();

#endif
