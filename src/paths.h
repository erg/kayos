#ifndef KAYOS_SRC_KAYOS_PATHS_H_
#define KAYOS_SRC_KAYOS_PATHS_H_

void ensure_kayos_data_path();
int kayos_dbname_valid_p(const char *dbname);

void *get_kayos_data_path();
void *get_kayos_data_path_for(const char *dbname);

#endif
