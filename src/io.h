#ifndef KAYOS_SRC_IO_H_
#define KAYOS_SRC_IO_H_

#include <sys/types.h>

ssize_t safe_read(int fd, char* data, ssize_t size);
int check_write(int fd, void* data, ssize_t size);
void safe_write(int fd, void* data, ssize_t size);

#endif
