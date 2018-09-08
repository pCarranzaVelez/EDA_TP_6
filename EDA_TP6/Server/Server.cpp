#include "Server.h"

void server::
startConnection()
{

	//Uncomment if non-blocking mode is desired
	//
	//When non-blocking mode is chosen accept operation
	//will fail with boost::asio::error::would_block
	//if there's no client inmediatelly connected when accept operation is performed.
	//Must comment blocking mode.
	//
	server_acceptor->non_blocking(true);
	boost::system::error_code error;
	do
	{
		server_acceptor->accept(*socket_forServer, error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to listen to " << HELLO_PORT << "Port " << error.message() << endl;

	//server_acceptor->accept(*socket_forServer);
	socket_forServer->non_blocking(true);
}

void server::
sendMessage()
{
	char buf[MSGSIZE] = "Hello from server.";

	size_t len;
	boost::system::error_code error;

	do
	{
		len = socket_forServer->write_some(boost::asio::buffer(buf, strlen(buf)), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to connect to server " << error.message() << endl;
}

void server::
sendMessage(const char msg[MSGSIZE])
{
	boost::system::error_code error;
	size_t len = 0;

	do
	{
		len = socket_forServer->write_some(boost::asio::buffer(msg, strlen(msg)), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to connect to server " << error.message() << endl;
}

void server::
receiveMessage(bool * value)
{
	boost::system::error_code error;
	char buf[MSGSIZE];
	size_t len = 0;

	do
	{
		len = socket_forServer->read_some(boost::asio::buffer(buf), error);

		if (!error)
			buf[len] = '\0';

	} while (error.value() == WSAEWOULDBLOCK);

	if (!error)
	{
		*value = true;
		cout << "Got a message:" << endl;
		cout << endl << buf << endl;
		if (validMessage(buf))	//si tiene los 3 CRLF esperados sigue parseando
		{
			if (parseFirstLine())	//se fija que la primera linea este en el formato esperado
			{
				if (parse2ndLine())		//se fija que la primera linea este en el formato esperado
				{
					isFilePresent();	//si estaba en el formato correcto busca el archivo solicitado y le contesta al cliente
				}
				else	//si no vino en el formato esperado lo ignora
				{
					cout << err.detail;
					err.type = NO_SERVER_ERR;
				}
			}
			else	//si no vino en el formato esperado lo ignora
			{
				cout << err.detail;
				err.type = NO_SERVER_ERR;
			}
		}
		else	//si no vino en el formato esperado lo ignora
		{
			cout << err.detail;	
			err.type = NO_SERVER_ERR;
		}
	}
	else
	{
		*value = false;
		cout << "Error while trying to connect with client " << error.message() << std::endl;
	}
}

/*validMessage: se fija que la primera linea termine con CRLF y la segunda con doble CRLF
Si es asi, ignora todo lo que haya despues del tercer CRLF, devuelve true y queda almacenada
en firstLine la primera linea sin el CRLF y secondLine la segunda sin los CRLF
En caso de no cumplirse lo dicho arriba devuelve false*/
bool server::
validMessage(char buf[])
{
	int i = 0;
	bool ret = false;
	while ((buf[i] != CR) && (buf[i] != '\0'))	//busca carriage return o el terminador, en ese caso error
	{
		firstLine += buf[i++];
	}
	if ((buf[i] == CR) && (buf[i+1] == LF))	//si la linea termino con CRLF continua
	{
		i++;
		while ((buf[i] != CR) && (buf[i] != '\0'))	//busca carriage return, pero termina tambien si encuentra el terminador, con error
		{
			secondLine += buf[i++]; 
		}
		if( (buf[i] == CR) && (buf[i+1] == LF) && (buf[i+2] == CR) && (buf[i + 3] == LF))	//si enuentra dos CRLF seguidos termina con exito el mensaje, sino error
		{
			ret = true;
		}
		else
		{
			err.type = WRONG_CRLF_FORMAT;
			err.detail = "No se encontro el doble CRLF esperado al terminar la segunda linea";
		}
	}
	else
	{
		err.type = WRONG_CRLF_FORMAT;
		err.detail = "No se encontro la secuencia esperada de CRLF en la primera linea";
	}
	return ret;
}

bool server::
parseFirstLine()
{
	bool ret = false;
	if (validCommand() && validVersion())	//se fija que se hayan enviado tanto un comando como una version de HTTP validos
	{
		while (firstLine[++commandEnd] == ' ');		//saltea los espacios hasta el principio del path
		while (firstLine[--versionStart] == ' ');		//saltea los espacios entre el final del filename y la version
		if (firstLine[commandEnd] == '/')
		{
			ret = true;
			path = firstLine.substr(commandEnd, versionStart); //si arranca con '/' guarda el path, desde donde terminan los espacios despues del comando, hasta donde arrancan los espacios antes de la version
		}
		else
		{
			err.type = WRONG_PATH_FORMAT;
			err.detail = "El path no comienza con '/'";
		}
	}
	return ret;
}

/*Se fija que el principio de firstLine sea un comando esperado de HTTP*/
bool server::
validCommand()
{
	const char *commands[COMMAND_NUM] = { "GET" };	//la idea es que si mas adelante se agregan mas comandos sea mas facil
	bool ret = false;
	for (int i = 0; (i < COMMAND_NUM) && !ret; i++)
	{
		if (!firstLine.compare(0,strlen(commands[i]),commands[i]))	//si la primera parte de la linea coincide con alguno de los comandos
		{
			ret = true;
			commandEnd = strlen(commands[i]);	//guarda la posicion donde termina el comando para mas adelante
		}
	}
	if (!ret)	//si no se encontro ninguno de los comandos
	{
		err.type = INVALID_COMMAND;
		err.detail = "No se encontro ningun comando HTTP esperado, los esperados son:\n";
		for (int i = 0; i < COMMAND_NUM; i++)
		{
			err.detail += ' ';
			err.detail += commands[i];
		}
	}
	return ret;
}

/*Se fija que el final de firstLine tenga una version valida de HTTP*/
bool server::
validVersion()
{
	const char *versions[VERSIONS_NUM] = { "HTTP/1.1" };	//en caso que mas adelante se acepten mas versiones, solo hay que modificar esto
	bool ret = false;
	for (int i = 0; (i < VERSIONS_NUM) && !ret; i++)
	{
		if (!firstLine.compare(firstLine.length() - strlen(versions[i]), strlen(versions[i]), versions[i]))
		{
			ret = true;
			versionStart = firstLine.length() - strlen(versions[i]);	//guarda el lugar donde empieza la version, que servira para saber donde termina el filename
		}
	}
	if (!ret)
	{
		err.type = INVALID_VERSION;
		err.detail = "No se encontro ninguna version de HTTP valida, las esperadas son:\n";
		for (int i = 0; i < VERSIONS_NUM; i++)
		{
			err.detail += ' ';
			err.detail += versions[i];
		}
	}
	return ret;
}

bool server::
parse2ndLine()
{
	bool ret = false;
	if (!secondLine.compare(0, strlen("Host: "), "Host: "))	//solo sirve si mandan con espacio despues de host:
	{
		secondLine.erase(0, strlen("Host: "));	
		if (!secondLine.compare("127.0.0.1") || !secondLine.compare("localhost"))	//solo sirve si hay un espacio entre el path y HTTP...
		{
			if (firstLine[0] == '/')
			{
				ret = true;
			}
		}
	}
}

void server::
isFilePresent()
{
	FILE * htmlFile;
	htmlFile = fopen(path.c_str(), "rb");
	if(htmlFile != NULL)	//se encontró el archivo solicitado 
	{
		sendSuccessMessage(htmlFile);	//envió mensaje al client 
		fclose(htmlFile);
	}
	else
	{
		sendFailMessage();
	}
}

void server::
sendSuccessMessage(FILE * htmlFile)
{
	boost::system::error_code error;
	size_t len = 0;

	answerMessage = "HTTP/1.1 200 OK" ;

	do
	{
		len = socket_forServer->write_some(boost::asio::buffer(msg, strlen(msg)), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to send message. " << error.message() << endl;
}

void server::
sendFailMessage()
{

}

server::
server()
{
	IO_handler = new boost::asio::io_service();
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), HELLO_PORT);

	socket_forServer = new boost::asio::ip::tcp::socket(*IO_handler);
	server_acceptor = new boost::asio::ip::tcp::acceptor(*IO_handler, ep);

}

server::
~server()
{
	server_acceptor->close();
	socket_forServer->close();
	delete server_acceptor;
	delete socket_forServer;
	delete IO_handler;
}

