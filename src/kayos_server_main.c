#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "io.h"
#include "kayos_paths.h"
#include "utils.h"

const char SERVER_PRODUCERS_PORT[] = "9890";
const char SERVER_CONSUMERS_PORT[] = "9891";

int init_socket(const char *servname) {
	int ret;
	struct addrinfo addrinfo, *res;
	int sock_fd;

	memset(&addrinfo, 0, sizeof(addrinfo));
	addrinfo.ai_family = AF_UNSPEC;
	addrinfo.ai_socktype = SOCK_STREAM;
	addrinfo.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, servname, &addrinfo, &res);

	sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sock_fd == -1)
		libc_fatal_error("socket failed");

	int optval = 1;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if(ret == -1)
		libc_fatal_error("setsockopt SO_REUSEPORT failed");

	ret = bind(sock_fd, res->ai_addr, res->ai_addrlen);
	if(ret == -1)
		libc_fatal_error("bind failed");

	ret = listen(sock_fd, 16);
	if(ret == -1)
		libc_fatal_error("listen failed");
	return sock_fd;
}

int accept_client(int sockfd, struct sockaddr_storage* their_addr, socklen_t *addr_size) {
	int new_fd = accept(sockfd, (struct sockaddr *)their_addr, addr_size);
	if(new_fd == -1)
		libc_fatal_error("accept failed");

	int optval = 1;
	int ret = setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval));
	if(ret == -1)
		libc_fatal_error("setsockopt SO_REUSEPORT failed");

	ret = setsockopt(new_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	if(ret == -1)
		libc_fatal_error("setsockopt SO_KEEPALIVE failed");

	return new_fd;
}

void fork_socket_handler(int new_stdin, int new_stdout, char *binary_path, char *dbpath) {
	pid_t clientpid;
	if((clientpid = fork()) == -1)
		libc_fatal_error("fork failed");

	if(clientpid == 0) {
		// Runs in child process
		redirect_child_stdin_stdout(new_stdin, new_stdout);
		char *args[] = {binary_path, dbpath, 0};
		execvp(binary_path, args);
		libc_fatal_error("execvp failed");
		// XXX: control passes to child main()
	} else {
		// Runs in parent process
		close(new_stdin);
	}
}

int main(int argc, char *argv[]) {
	ensure_kayos_data_path();

	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	int producers_fd = init_socket(SERVER_PRODUCERS_PORT);
	int consumers_fd = init_socket(SERVER_CONSUMERS_PORT);
	int max_fd = consumers_fd;
	int client_fd = 0;
	if(producers_fd > consumers_fd)
		max_fd = producers_fd;
	do {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(producers_fd, &readfds);
		FD_SET(consumers_fd, &readfds);

		int ret = select(max_fd + 1, &readfds, 0, 0, 0);
		if(ret == -1) {
			if(errno == EINTR)
				continue;
			else
				libc_fatal_error("select");
		}

		if(FD_ISSET(producers_fd, &readfds)) {
			fprintf(stderr, "producer client connected! select ret: %d\n", ret);
			client_fd = accept_client(producers_fd, &their_addr, &addr_size);
			fork_socket_handler(client_fd, client_fd, "bin/kayos-producer-client", "kayosdbs/test");
		}
		if(FD_ISSET(consumers_fd, &readfds)) {
			fprintf(stderr, "consumer client connected! select ret: %d\n", ret);
			client_fd = accept_client(consumers_fd, &their_addr, &addr_size);
			fork_socket_handler(client_fd, client_fd, "bin/kayos-consumer-client", "kayosdbs/test");
		}
	} while(1);

	return 0;
}
