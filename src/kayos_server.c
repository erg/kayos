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
#include "utils.h"

const char SERVER_PRODUCERS_PORT[] = "9890";
const char SERVER_CONSUMERS_PORT[] = "9891";

// fills in stdin,stdout pair
void safe_pipe(int fd[2]) {
	if(pipe(fd) == -1)
		fatal_error("pipe failed");
}

int safe_mkdir(const char *path, mode_t mode) {
	int ret;
	ret = mkdir(path, mode);
	if(ret == -1 && errno != EEXIST)
		return -1;
	return 0;
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
		fatal_error("child: failed stdin setup");
	if(new_stdout != -1 && safe_dup2(new_stdout, STDOUT_FILENO) != STDOUT_FILENO)
		fatal_error("child: failed stdout setup");
}

void init_ipc() {
	int ret;
	ret = safe_mkdir("./ipc", 0777);
	if(ret == -1)
		fatal_error("mkdir ./ipc failed");
}

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
		fatal_error("socket failed");

	int optval = 1;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if(ret == -1)
		fatal_error("setsockopt SO_REUSEPORT failed");

	ret = bind(sock_fd, res->ai_addr, res->ai_addrlen);
	if(ret == -1)
		fatal_error("bind failed");

	ret = listen(sock_fd, 16);
	if(ret == -1)
		fatal_error("listen failed");
	return sock_fd;
}

int accept_client(int sockfd, struct sockaddr_storage* their_addr, socklen_t *addr_size) {
	int new_fd = accept(sockfd, (struct sockaddr *)their_addr, addr_size);
	if(new_fd == -1)
		fatal_error("accept failed");

	int optval = 1;
	int ret = setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval));
	if(ret == -1)
		fatal_error("setsockopt SO_REUSEPORT failed");
	return new_fd;
}

void fork_socket_handler(int socket_fd, int parent_child_pipe, const char *binary_path) {
	pid_t clientpid;
	if((clientpid = fork()) == -1)
		fatal_error("fork failed");

	if(clientpid == 0) {
		// Runs in child process
		// new stdin is socket, stdout is kayos-writer pipe handle
		redirect_child_stdin_stdout(socket_fd, parent_child_pipe);
		execvp(binary_path, 0);
		fatal_error("execvp failed");
		// XXX: control passes to child main()
	} else {
		// Runs in parent process
		close(socket_fd);
	}
}

int main(int argc, char *arg[]) {
	/*
	int ret = 0;
	// pipes for kayos-writer child
	int fd_p2w[2];
	int fd_w2p[2];
	init_ipc();
	safe_pipe(fd_p2w);
	safe_pipe(fd_w2p);

	pid_t writerpid;
	if((writerpid = fork()) == -1)
		fatal_error("fork failed");

	if(writerpid == 0) {
		// Runs in child process
		redirect_child_stdin_stdout(fd_p2w[0], fd_w2p[1]);
		execvp("bin/kayos-writer", 0);
		fatal_error("execvp kayos-writer failed");
		// XXX: control passes to kayos-writer main()
	}
	else {
		// Runs in parent process
		// close stdin of parent to writer
		// and stdout of writer to parent
		close(fd_p2w[0]);
		close(fd_w2p[1]);
	}
	*/

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
				fatal_error("select");
		}

		if(FD_ISSET(producers_fd, &readfds)) {
			fprintf(stderr, "producer client connected! select ret: %d\n", ret);
			client_fd = accept_client(producers_fd, &their_addr, &addr_size);
			fork_socket_handler(client_fd, client_fd, "bin/kayos-producer-client");
		}
		if(FD_ISSET(consumers_fd, &readfds)) {
			fprintf(stderr, "consumer client connected! select ret: %d\n", ret);
			client_fd = accept_client(consumers_fd, &their_addr, &addr_size);
			fork_socket_handler(client_fd, client_fd, "bin/kayos-consumer-client");
		}
	} while(1);

	/*
	int status;
	ret = waitpid(writerpid, &status, 0);
	fprintf(stderr, "server: writer status: %d\n", WEXITSTATUS(status));
	if(ret == -1)
		fatal_error("wait_pid failed");

	//cleanup parent to writer(stdin) pipe
	close(fd_p2w[1]);
	//cleanup child(stdout) to parent pipe
	close(fd_w2p[0]);
	*/

	return 0;
}
