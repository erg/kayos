#include "io.h"

#include <unistd.h>
#include <errno.h>
#include "utils.h"

ssize_t safe_read(int fd, char* data, ssize_t size) {
	ssize_t nbytes = read(fd, data, size-1);
	if (nbytes < 0) {
		if (errno == EINTR) {
			return safe_read(fd, data, size-1);
		} else {
			fatal_error("error reading fd");
		}
	}
	data[nbytes] = 0;
	return nbytes;
}

int check_write(int fd, void* data, ssize_t size) {
	if (write(fd, data, size) == size)
		return 1;
	else {
		if (errno == EINTR)
			return check_write(fd, data, size);
		else
			return 0;
	}
}

void safe_write(int fd, void* data, ssize_t size) {
	if (!check_write(fd, data, size))
		fatal_error("error writing fd");
}
