#ifndef KAYOS_SRC_IO_H_
#define KAYOS_SRC_IO_H_

#include <sys/types.h>

// Unix wrappers
ssize_t safe_read(int fd, char *data, ssize_t size);
ssize_t safe_write(int fd, const void *data, size_t size);
int safe_open(const char *path, int oflag, ...);

void safe_pipe(int fd[2]);
int safe_dup2(int fildes, int fildes2);

// Utility I/O
void redirect_child_stdin_stdout(int new_stdin, int new_stdout);
void close_stdout();

void *safe_malloc(size_t size);
void *malloc_vsnprintf(const char *restrict format, ...);

int safe_mkdir(const char *path, mode_t mode);
int mkpath(char *path, mode_t mode);

#endif
