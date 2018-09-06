#include "Client.h"

int
main(int argc, char* argv[])
{
	client conquering;
	cout << "Trying to connect to " << SERVER_IP << " on port " << HELLO_PORT_STR << std::endl;
	conquering.startConnection(SERVER_IP);
	conquering.receiveMessage();
	cout << "Press Enter to exit..." << std::endl;
	getchar();
	return 0;
}