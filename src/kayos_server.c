#include "kayos_server.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "io.h"
#include "utils.h"

const char SERVER_PORT[] = "9890";

// fills in stdin,stdout pair
void safe_pipe(int fd[2]) {
	if(pipe(fd) == -1)
		fatal_error("pipe failed");
}

void setup_child_pipes(int fd_p2c[2], int fd_c2p[2]) {
	if (dup2(fd_p2c[0], 0) != 0
		|| close(fd_p2c[0]) != 0
		|| close(fd_p2c[1]) != 0) {
		fatal_error("child: failed stdin setup");
	}
	if (dup2(fd_c2p[1], 1) != 1
		|| close(fd_c2p[1]) != 0
		|| close(fd_c2p[0]) != 0) {
		fatal_error("child: failed stdout setup");
	}
}

void setup_parent_pipes(int fd_p2c[2], int fd_c2p[2]) {
	close(fd_p2c[0]);
	close(fd_c2p[1]);
}

int init_network() {
	struct addrinfo hints, *res;
	int sockfd;
	int ret;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, SERVER_PORT, &hints, &res);

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1)
		fatal_error("socket failed");

	ret = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if(ret == -1)
		fatal_error("bind failed");

	int optval = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if(ret == -1)
		fatal_error("setsockopt SO_REUSEPORT failed");


	ret = listen(sockfd, 16);
	if(ret == -1)
		fatal_error("listen failed");
	return sockfd;
}

int main(int argc, char *arg[]) {
	int ret = 0;
	int fd_p2c[2];
	int fd_c2p[2];

	pid_t childpid;

	safe_pipe(fd_p2c);
	safe_pipe(fd_c2p);

	if((childpid = fork()) == -1)
		fatal_error("fork failed");

	if(childpid == 0) {
		// Runs in child process
		setup_child_pipes(fd_p2c, fd_c2p);

		int ret = execvp("bin/kayos-writer", 0);
		if(ret == -1)
			fatal_error("execvp kayos-writer failed");
	}
	else {
		// Runs in parent process
		setup_parent_pipes(fd_p2c, fd_c2p);

		// cleanup
		close(fd_p2c[1]);
		close(fd_c2p[0]);
	}

	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);

	int sockfd = init_network();
	do {
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if(new_fd == -1)
			fatal_error("accept failed");

		fprintf(stderr, "client connected! fd: %d\n", new_fd);
		if((childpid = fork()) == -1)
			fatal_error("fork failed");

		//int fd_p2w[2];
		//int fd_w2p[2];
		//safe_pipe(fd_p2w);
		//safe_pipe(fd_w2p);

		if(childpid == 0) {
			//setup_child_pipes(fd_p2w, fd_w2p);
			if (dup2(new_fd, 0) != 0)
				fatal_error("client child: failed stdin setup");
			//if (dup2(fd_c2p[1], 1) != 1)
				//fatal_error("client child: failed stdout setup");

			int ret = execvp("bin/kayos-client", 0);
			if(ret == -1)
				fatal_error("execvp kayos-client failed");
		} else {
			close(new_fd);
			//setup_parent_pipes(fd_p2w, fd_w2p);
		}
	} while(1);

	int status;
	//ret = waitpid(childpid, &status, WNOHANG); // don't wait
	ret = waitpid(childpid, &status, 0);
	fprintf(stderr, "child status: %d\n", WEXITSTATUS(status));
	if(ret == -1) {
		fatal_error("wait_pid failed");
	}

	return 0;
}
