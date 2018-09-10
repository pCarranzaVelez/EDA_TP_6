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
		if (conquering.messageToServer())	//si le mando un mensaje al server, espera respuesta
		{
			cout << endl;
			exit = conquering.receiveMessage();
			cout << endl;
		}
	} while (!(exit));

	return 0;
}