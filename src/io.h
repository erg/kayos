#ifndef KAYOS_SRC_IO_H_
#define KAYOS_SRC_IO_H_

#include <sys/types.h>

ssize_t safe_read(int fd, char* data, ssize_t size);
ssize_t safe_write_impl(int fd, const void* data, ssize_t size, ssize_t *written);
ssize_t safe_write(int fd, const void* data, ssize_t size, ssize_t *written);

#endif
