#include "kayos_server.h"
#include "utils.h"

#include <zmq.hpp>
//#include <zmq.h>


int main(int argc, char *arg[]) {
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	socket.bind("tcp://*:9890");
	
	while(true) {
		zmq::message_t request;
		socket.recv(&request);
		named_hexdump("received request", (unsigned char*)request.data(), request.size());

		zmq::message_t reply(5);
		memcpy((void*)reply.data(), "World", 5);
		socket.send(reply);
	}

	return 0;
}
