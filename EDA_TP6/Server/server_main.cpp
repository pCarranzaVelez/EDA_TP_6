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
		if(!conquering.receiveMessage(&value))		
			//conquering.sendMessage("Message received");
		//else
			conquering.sendMessage((conquering.getError()).detail.c_str());
	} while (value);
	conquering.sendMessage("exit");

	Sleep(50); // Le damos 50ms para que llegue el mensaje antes de cerrar el socket.

	return 0;
}