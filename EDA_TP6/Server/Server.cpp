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
		if (searchClrf(buf))	//si tiene los 3 CRLF esperados sigue parseando
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

bool server::
searchCrlf(char buf[])
{
	int i = 0;
	bool ret = false;
	while ((buf[i] != CR) && (buf[i] != '\0'))	//busca carriage return, pero termina tambien si encuentra el terminador, con error
	{
		firstLine += buf[i++];
	}
	if ((buf[i] == CR) && (buf[i+1] == LF))
	{
		i++;
		while ((buf[i] != CR) && (buf[i] != '\0'))	//busca carriage return, pero termina tambien si encuentra el terminador, con error
		{
			secondLine += buf[i++];
		}
		if( (buf[i] == CR) && (buf[i+1] == LF) && (buf[i+2] == CR) && (buf[i + 3] == LF))
		{
			ret = true;
		}
		else
		{
			err.type = WRONG_CRLF_FORMAT;
			err.detail = "No se encontro la secuencia esperada de CRLF en la segunda linea";
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
	if (!firstLine.compare(0, strlen("GET "), "GET "))
	{
		firstLine.erase(0, strlen("GET "));
		if (!firstLine.compare(firstLine.length() - strlen(" HTTP/1.1"), strlen(" HTTP/1.1"), " HTTP/1.1"))	//solo sirve si hay un espacio entre el path y HTTP...
		{
			firstLine.erase(firstLine.length() - strlen(" HTTP/1.1"), strlen(" HTTP/1.1"));	//limpia el string y le deja solo el path
			if (firstLine[0] == '/')
			{
				ret = true;
			}
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

