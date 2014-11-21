#include <iostream>
#include <string>
#include <unistd.h>

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
		buffer = rtrim(buffer);
		if(std::cin.bad()) {
			std::cerr << "client: cin read error" << std::endl;
			break;
		} else if(std::cin.eof()) {
			std::cerr << "client: client dc!" << std::endl;
			break;
		}
		std::cerr << "client: got buffer: " << buffer << std::endl;
		named_hexdump("client got:", (unsigned char *)buffer.c_str(), buffer.size());
		std::cout << "ok\n" << std::endl;
		stop = handle_buffer(buffer);
	} while(!stop);
}

int main(int argc, char *arg[]) {
	client_loop();
	std::cerr << "client exiting" << std::endl;
	return 0;
}
