#include <iostream>
#include <string>

#include "kayos_client.h"
#include "utils.h"

int handle_buffer(std::string buffer) {
	int stop = 0;
	if(buffer == "exit") {
		stop = 1;
	}
	return stop;
}

void client_loop() {
	int stop = 0;
	do {
		std::cerr << "client: loop head" << std::endl;
		std::string buffer;
		std::getline(std::cin, buffer);
		std::cerr << "client: got buffer: " << buffer << std::endl;
		std::cout << "ok\n" << std::endl;
		stop = handle_buffer(buffer);
	} while(!stop);
}

int main(int argc, char *arg[]) {
	client_loop();
	return 0;
}
