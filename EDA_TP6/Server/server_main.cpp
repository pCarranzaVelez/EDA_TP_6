#include "Server.h"

int
main(int argc, char* argv[])
{
	server conquering;

	bool value = true;

	cout << endl << "Start Listening on port " << HELLO_PORT << endl;
	conquering.startConnection();
	cout << "Somebody connected to port " << HELLO_PORT << endl;
	conquering.sendMessage();
	do
	{
		conquering.receiveMessage(&value);
		conquering.sendMessage("Message received");
		cout << endl;
	} while (value);
	conquering.sendMessage("exit");
	conquering.sendMessage("Server timed out");

	Sleep(50); // Le damos 50ms para que llegue el mensaje antes de cerrar el socket.

	return 0;
}