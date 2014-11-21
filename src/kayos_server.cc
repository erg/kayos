#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include "kayos_server.h"
#include "io.h"
#include "utils.h"

// fills in stdin,stdout pair
void safe_pipe(int fd[2]) {
	int ret = pipe(fd);
	if(ret == -1) {
		fatal_error("pipe failed");
	}
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



int main(int argc, char *arg[]) {
	int ret = 0;
	int fd_p2c[2];
	int fd_c2p[2];
	pid_t childpid;

	safe_pipe(fd_p2c);
	safe_pipe(fd_c2p);

	if((childpid = fork()) == -1) {
		fatal_error("fork failed");
	}

	if(childpid == 0) {
		// Runs in child process
		setup_child_pipes(fd_p2c, fd_c2p);

		int ret = execvp("bin/kayos-writer", 0);
		if(ret == -1) {
			fatal_error("execvp kayos-writer failed");
		}
	}
	else {
		// Runs in parent process
		setup_parent_pipes(fd_p2c, fd_c2p);

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

		int status;
		//ret = waitpid(childpid, &status, WNOHANG); // don't wait
		ret = waitpid(childpid, &status, 0);
		std::cerr << "child status: " << WEXITSTATUS(status) << std::endl;
		if(ret == -1) {
			fatal_error("wait_pid failed");
		}

		// cleanup, but never gets here due to waitpid()
		close(fd_p2c[1]);
		close(fd_c2p[0]);
	}

	return 0;
}
