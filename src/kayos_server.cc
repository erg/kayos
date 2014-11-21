#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "kayos_server.h"
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

		/*
		do {
			// Write to child
			std::cerr << "server: loop start" << std::endl;
			std::string cmd = "hi client\n";
			safe_write(fd_p2c[1], (void*)cmd.c_str(), cmd.size());
			std::cerr << "server: wrote msg" << std::endl;
			char buffer[4096];
			ssize_t nbytes;
			nbytes = safe_read(fd_c2p[0], buffer, sizeof(buffer));
			std::cerr << "server: got " << nbytes << " bytes from writer: " << buffer << std::endl;
			millis_sleep(500);
			std::cerr << "server: loop end, looping again" << std::endl;
		} while(1);
		*/

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

		std::cerr << "client connected! fd: " << new_fd << std::endl;
		if((childpid = fork()) == -1)
			fatal_error("fork failed");

		int fd_p2w[2];
		int fd_w2p[2];
		safe_pipe(fd_p2w);
		safe_pipe(fd_w2p);

		if(childpid == 0) {
			setup_child_pipes(fd_p2w, fd_w2p);

			int ret = execvp("bin/kayos-client", 0);
			if(ret == -1)
				fatal_error("execvp kayos-client failed");
		} else {
			setup_parent_pipes(fd_p2w, fd_w2p);
		}
	} while(1);

	int status;
	//ret = waitpid(childpid, &status, WNOHANG); // don't wait
	ret = waitpid(childpid, &status, 0);
	std::cerr << "child status: " << WEXITSTATUS(status) << std::endl;
	if(ret == -1) {
		fatal_error("wait_pid failed");
	}

	return 0;
}
