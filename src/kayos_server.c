#include "kayos_server.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "io.h"
#include "utils.h"

const char SERVER_PORT[] = "9890";

// fills in stdin,stdout pair
void safe_pipe(int fd[2]) {
	if(pipe(fd) == -1)
		fatal_error("pipe failed");
}

// XXX: macosx says dup2 can return EINTR, linux maybe can
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

int init_network() {
	int ret;
	struct addrinfo hints, *res;
	int sockfd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, SERVER_PORT, &hints, &res);

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1)
		fatal_error("socket failed");

	int optval = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if(ret == -1)
		fatal_error("setsockopt SO_REUSEPORT failed");

	ret = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if(ret == -1)
		fatal_error("bind failed");

	ret = listen(sockfd, 16);
	if(ret == -1)
		fatal_error("listen failed");
	return sockfd;
}

int main(int argc, char *arg[]) {
	int ret = 0;

	// pipes for kayos-writer child
	int fd_p2w[2];
	int fd_w2p[2];
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

	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	int sockfd = init_network();
	do {
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if(new_fd == -1)
			fatal_error("accept failed");

		int optval = 1;
		ret = setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval));
		if(ret == -1)
			fatal_error("setsockopt SO_REUSEPORT failed");

		fprintf(stderr, "client connected! fd: %d\n", new_fd);
		pid_t clientpid;
		if((clientpid = fork()) == -1)
			fatal_error("fork failed");

		if(clientpid == 0) {
			// Runs in child process
			// new stdin is socket, stdout is kayos-writer pipe handle
			redirect_child_stdin_stdout(new_fd, fd_p2w[1]);
			execvp("bin/kayos-client", 0);
			fatal_error("execvp kayos-client failed");
			// XXX: control passes to kayos-client main()
		} else {
			// Runs in parent process
			close(new_fd);
		}
	} while(1);

	int status;
	ret = waitpid(writerpid, &status, 0);
	fprintf(stderr, "server: writer status: %d\n", WEXITSTATUS(status));
	if(ret == -1) {
		fatal_error("wait_pid failed");
	}

	//cleanup parent to writer(stdin) pipe
	close(fd_p2w[1]);
	//cleanup child(stdout) to parent pipe
	close(fd_w2p[0]);

	return 0;
}
