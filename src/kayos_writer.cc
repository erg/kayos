#include <iostream>
#include <string>

#include "kayos_writer.h"
#include "utils.h"

int handle_buffer(std::string buffer) {
	int stop = 0;

	if(buffer == "exit") {
		stop = 1;
	}

	return stop;
}

void writer_loop() {
	int stop = 0;
	do {
		std::cerr << "writer: loop head" << std::endl;
		std::string buffer;
		std::getline(std::cin, buffer);
		std::cerr << "writer: got buffer: " << buffer << std::endl;
		std::cout << "ok\n" << std::endl;
		stop = handle_buffer(buffer);
	} while(!stop);
}

int main(int argc, char *arg[]) {
	writer_loop();
	return 0;
}
