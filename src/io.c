#include "io.h"

#include <unistd.h>
#include <errno.h>
#include "utils.h"

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

ssize_t safe_write_impl(int fd, const void* data, size_t size, size_t written) {
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
