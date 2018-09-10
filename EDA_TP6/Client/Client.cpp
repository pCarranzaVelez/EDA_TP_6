#include "Client.h"

client::
client()
{
	IO_handler = new boost::asio::io_service();
	socket_forClient = new boost::asio::ip::tcp::socket(*IO_handler);	//INFO DE LA IP Y EL PUERTO, IDENTIFICA LA CONEXION
	client_resolver = new boost::asio::ip::tcp::resolver(*IO_handler);
}

client::
~client()
{
	socket_forClient->close();
	delete client_resolver;
	delete socket_forClient;
	delete IO_handler;
}

bool client::
startConnection(const char* host)
{
	bool ret = false;
	endpoint = client_resolver->resolve(boost::asio::ip::tcp::resolver::query(host, HELLO_PORT_STR));
	boost::system::error_code error;
	boost::asio::connect(*socket_forClient, endpoint, error);
	if (error)
	{
		cout << "Error connecting to " << host << " Error Message: " << error.message() << endl;
		if (error.value() == boost::asio::error::connection_refused)
			cout << "Host " << host << " is not listening on the other side" << endl;
		ret = true;
	}
	socket_forClient->non_blocking(true);
	return ret;
}

bool client::
receiveMessage()
{
	boost::system::error_code error;
	char buf[MSGSIZE];
	char ret = false;
	size_t len = 0;
	cout << "Receiving Message" << endl;
	boost::timer::cpu_timer t;
	t.start();
	boost::timer::cpu_times pastTime = t.elapsed();
	double elapsedSeconds = 0.0;
	messageFromServer = "";
	do
	{
		len = socket_forClient->read_some(boost::asio::buffer(buf), error);
		if (!error)	//si no hubo error guarda la parte recibida del mensaje y vacia el buffer
		{
			if (len < MSGSIZE)
			{
				buf[len] = '\0';
				messageFromServer += buf;
			}
			else if (len == MSGSIZE)
			{
				char charBuf = buf[len-1];	//cuando len vale MSGSIZE no se puede escribir en buf[len]
				buf[len - 1] = '\0';
				messageFromServer += buf;
				messageFromServer += charBuf; 
			}
			clearBuf(buf, MSGSIZE);		//vacia el buffer
		}
	} while (error.value() == WSAEWOULDBLOCK);

	if (!error)
	{
		if (buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't')
		{
			ret = true;
		}
		else
		{
			cout << endl << "Server says: " << messageFromServer.c_str() << endl;
			//cout << endl << "Server says: " << buf << endl;
		}
	}
	else
	{
		cout << "Error while trying to connect to server " << error.message() << endl;
		ret = true;
	}
	return ret;
}

void client::
clearBuf(char buf[],unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
		buf[i] = '\0';
}

/*Devuelve true si se comunico con el server, para saber si esperar una respuesta o no*/
bool client::
messageToServer()
{
	bool ret = false;
	boost::system::error_code error;
	char buf[MSGSIZE];
	size_t len = 0;
	cout << "Write your message:" << endl;
	cin.getline(buf,sizeof(buf));

	getPathAndHost(buf);	//lo que haya en buf lo parsea y lo mete en host y path
	if (err.type != NO_ERR)
	{
		cout << err.detail << endl;
		err.type = NO_ERR;	//ya se leyo el error
	}
	else
	{
		ret = true;
		info2ServerMessage();	//mete lo recibido del usuario en messageToServer con el formato que espera el server
		do
		{
			len = socket_forClient->write_some(boost::asio::buffer(serverMessage.c_str(), serverMessage.length()), error);
		} while ((error.value() == WSAEWOULDBLOCK));
		if (error)
			cout << "Error while trying to connect to server " << error.message() << endl;
		else
			cout << "Message succesfully sent:" << endl << serverMessage.c_str();
	}
	return ret;
}

void client::
messageToServer(const char msg[MSGSIZE])
{
	boost::system::error_code error;
	size_t len = 0;

	do
	{
		len = socket_forClient->write_some(boost::asio::buffer(msg, strlen(msg)), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to connect to server " << error.message() << endl;
}


void client::
getPathAndHost(char buf[])
{
	int i = 0;
	host = "";
	path = "";
	while ((buf[i] != '\\') && (i < MSGSIZE))	//hasta la primera barra es el host
	{
		host += buf[i++];
	}
	if (i == MSGSIZE)	//si no encontro '\' es error
	{
		err.type = NO_SLASH;
		err.detail = "No se recibio ningun caracter \\ , ingrese localhost\\path\\filename\n";
	}
	else
	{
		while ((buf[i] != '\0') && (i < MSGSIZE))	//todo lo que venga despues se considera como path 
		{
			if (buf[i] != ' ')
			{
				path += buf[i];
			}
			else		//traduce los espacios a %20
			{
				path += "%20";
			}
			i++;
		}
	}
}

void client::
info2ServerMessage()
{
	serverMessage = "GET " + path + " HTTP/1.1 "; //mete lo recibido del usuario en messageToServer con el formato que espera el server
	serverMessage += CR;
	serverMessage += LF;
	serverMessage += "Host: " + host;
	serverMessage += CR;
	serverMessage += LF;
	serverMessage += CR;
	serverMessage += LF;
}