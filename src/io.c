#include "io.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "errors.h"

static ssize_t safe_write_impl(int fd, const void* data, size_t size, size_t written);

ssize_t safe_read(int fd, char* data, ssize_t size) {
	ssize_t nbytes = read(fd, data, size-1);
	if (nbytes < 0) {
		if (errno == EINTR)
			return safe_read(fd, data, size-1);
		else
			return -1;
	}
	data[nbytes] = 0;
	return nbytes;
}

static ssize_t safe_write_impl(int fd, const void* data, size_t size, size_t written) {
	ssize_t nbytes = write(fd, (char*)data + written, size - written);
	if(nbytes == -1 && errno != EINTR) {
		return written;
	} else if(nbytes == size) {
		written += nbytes;
		return written;
	} else {
		written += nbytes;
		return safe_write_impl(fd, (char*)data + written, size - written, written);
	}
}

ssize_t safe_write(int fd, const void* data, size_t size) {
	return safe_write_impl(fd, data, size, 0);
}

int safe_open(const char *path, int oflags, ...) {
	va_list args;
	int fd;
	do {
		va_start(args, oflags);
		fd = open(path, oflags, args);
		va_end(args);
	} while(fd == -1 && errno == EINTR);
	return fd;
}

// fills in stdin,stdout pair
void safe_pipe(int fd[2]) {
	if(pipe(fd) == -1)
		libc_fatal_error("pipe failed");
}

// XXX: macosx says dup2 can return EINTR, linux maybe does?
// retries on EINTR
// makes another handle from fildes with ordinal filedes2
// and closes existing fildes2 if open
int safe_dup2(int fildes, int fildes2) {
	int ret = dup2(fildes, fildes2);
	if(ret == -1) {
		if(errno == EINTR)
			return safe_dup2(fildes, fildes2);
		return -1;
	}
	return ret;
}

// dup2 closes/replaces the second fd with the first fd
// e.g. a pipe that the parent can write to might become the new stdin for the child
// don't replace stdin/stdout if value is -1
void redirect_child_stdin_stdout(int new_stdin, int new_stdout) {
	printf("redirect_child_stdin_stdout %d, %d\n", new_stdin, new_stdout);
	if(new_stdin != -1 && safe_dup2(new_stdin, STDIN_FILENO) != STDIN_FILENO)
		libc_fatal_error("child: failed stdin setup");
	if(new_stdout != -1 && safe_dup2(new_stdout, STDOUT_FILENO) != STDOUT_FILENO)
		libc_fatal_error("child: failed stdout setup");
}

// Needed to close sockets if connected.
void close_stdout() {
    fflush(stdout);
    close(1);
}

void *safe_malloc(size_t size) {
	void *ptr = malloc(size);
	if(!ptr)
		libc_fatal_error("malloc failed");
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
