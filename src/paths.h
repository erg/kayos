#ifndef KAYOS_SRC_PATHS_H_
#define KAYOS_SRC_PATHS_H_

int kayos_dbname_valid_p(const char *dbname);
void ensure_kayos_data_path();

void *get_kayos_data_path();
void *get_kayos_data_path_for(const char *dbname);

#endif
