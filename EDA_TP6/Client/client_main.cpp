#include "Client.h"

int
main(int argc, char* argv[])
{
	client conquering;
	cout << "Trying to connect to " << SERVER_IP << " on port " << HELLO_PORT_STR << std::endl;
	conquering.startConnection(SERVER_IP);
	bool exit = conquering.receiveMessage();

	do
	{
		conquering.messageToServer();
		cout << endl;
		exit = conquering.receiveMessage();
		exit = conquering.receiveMessage();
		cout << endl;
	} while (!(exit));

	return 0;
}