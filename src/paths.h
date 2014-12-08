#ifndef KAYOS_SRC_KAYOS_PATHS_H_
#define KAYOS_SRC_KAYOS_PATHS_H_
#include <unistd.h>
///#include <stdarg.h>

int safe_mkdir(const char *path, mode_t mode);
int mkpath(char* path, mode_t mode);

void *get_kayos_data_path();
void *get_kayos_data_path_for(const char *dbname);

void ensure_kayos_data_path();

int kayos_dbname_valid_p(const char *dbname);

#endif
