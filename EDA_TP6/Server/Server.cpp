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

bool server::
receiveMessage(bool * value)
{
	bool ret = false;
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
				if (parse2ndLine())		//se fija que la segunda linea este en el formato esperado
				{
					ret = true;
					cout << "Correct format, searching " << path << "..." << endl;
					isFilePresent();	//si estaba en el formato correcto busca el archivo solicitado y le contesta al cliente
				}
			}
		}
		if (err.type != NO_SERVER_ERR)	//si hubo algun error
		{
			cout << err.detail << endl;
			err.type = NO_SERVER_ERR;	
		}
	}
	else
	{
		*value = false;
		cout << "Error while trying to connect with client " << error.message() << std::endl;
		err.type = CLIENT_CONNECTION_ERR;
		err.detail = error.message();
	}
	return ret;
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
	firstLine = "";
	secondLine = "";
	while ((buf[i] != CR) && (buf[i] != '\0'))	//busca carriage return o el terminador, en ese caso error
	{
		firstLine += buf[i++];
	}
	if ((buf[i] == CR) && (buf[i+1] == LF))	//si la linea termino con CRLF continua
	{
		i+=2;
		while ((buf[i] != CR) && (buf[i] != '\0'))	//busca carriage return, pero termina tambien si encuentra el terminador, con error
		{
			secondLine += buf[i++]; 
		}
		if( (buf[i] == CR) && (buf[i+1] == LF) && (buf[i+2] == CR) && (buf[i + 3] == LF))	//si enuentra dos CRLF seguidos termina con exito el mensaje, sino error
		{
			ret = true;
			err.type = NO_SERVER_ERR;
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
	//if (validCommand() && validVersion())	
	if (validCommand())		//se fija que se hayan enviado un comando valido
	{
		while (firstLine[++cursor] == ' ');		//saltea los espacios hasta el principio del path
		if (firstLine[cursor] == '/')
		{
			path = '/';
			while( (firstLine[++cursor] != ' ') && (firstLine[cursor] != '\0'))	//hasta el siguiente espacio
			{
				if( (firstLine[cursor] == '%') && !firstLine.compare(cursor, strlen("%20"), "%20"))	//si viene un %20, lo toma como espacio
				{
					path += ' ';
					cursor += 2;
				}
				else
				{
					path += firstLine[cursor];
				}
			}
			if (firstLine[cursor] != '\0')	//si salio por un espacio
			{
				while (firstLine[++cursor] == ' ');
				if (validVersion())	//se fija que sea una version valida de HTTP
				{
					while (firstLine[++cursor] != '\0')	//se fija que hasta el final de la linea solo haya espacios
					{
						if (firstLine[cursor] != ' ')
						{
							err.type = WRONG_1ST_FORMAT;
							err.detail = "Solo se eperan espacios despues de la version HTTP";
						}
					}
					if (err.type != WRONG_1ST_FORMAT)
					{
						ret = true;
						err.type = NO_SERVER_ERR;
					}
				}
			}
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
			cursor = strlen(commands[i]);	//guarda la posicion donde termina el comando para mas adelante
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
		if (!firstLine.compare(cursor, strlen(versions[i]), versions[i]))
		{
			ret = true;
			err.type = NO_SERVER_ERR;
			cursor += strlen(versions[i]);
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

/*Se fija que la segunda linea tenga el formato correcto y que el host enviado sea uno de los esperados*/
bool server::
parse2ndLine()
{
	bool ret = false;
	const char *hosts[HOSTS_NUM] = { "localhost","127.0.0.1" };
	if (!secondLine.compare(0, strlen("Host:"), "Host:"))	
	{
		cursor = strlen("Host:");
		while (secondLine[cursor] == ' ')	//saltea espacios
		{
			cursor++;
		}
		for (int i = 0; (i < HOSTS_NUM) && !ret ; i++)
		{
			if (!secondLine.compare(cursor, strlen(hosts[i]), hosts[i]))	//se fija si el host enviado coincide con alguno de los esperados
			{
				unsigned int j = cursor;	
				while(secondLine[j + strlen(hosts[i])] != '\0')	//en lo que queda de la linea se fija que solo haya espacios
				{
					if (secondLine[j + strlen(hosts[i])] != ' ') 	//si no es un espacio
					{
						err.type = WRONG_2ND_FORMAT;
						err.detail = "Error de formato de la segunda linea, solo se espera 'Host: (host)', sin mas caracteres que espacios despues de host";
					}
					j++;
				}
				if (err.type != WRONG_2ND_FORMAT)	//si no hubo error de formato
				{
					ret = true;		//hubo exito
					host = secondLine.substr(cursor, strlen(hosts[i]));	//guarda el host enviado
				}
			}
		}
		if (!ret && (err.type == NO_SERVER_ERR))	//esta condicion para evitar pisar otros errores
		{
			err.type = INVALID_HOST;
			err.detail = "No se encontro un host valido, los esperados son:\n";
			for (int i = 0; i < HOSTS_NUM; i++)
			{
				err.detail += ' ';
				err.detail += hosts[i];
			}
		}

	}
	else
	{
		err.type = WRONG_HOST_FORMAT;
		err.detail = "La segunda linea no arranca con 'Host:'";
	}
	return ret;
}

void server::
isFilePresent()	//se fija si existe el archivo y envía mensaje al client dependiendo del resultado de búsqueda
{
	FILE * htmlFile;
	htmlFile = fopen(path.c_str(), "rb");
	if(htmlFile != NULL)	//se encontró el archivo solicitado 
	{
		sendSuccessMessage(htmlFile);	//envía mensaje de éxito al client
		fclose(htmlFile);
	}
	else
	{
		sendFailMessage();	//envía mensaje de error al client
	}
}

void server::
sendSuccessMessage(FILE * htmlFile)	//envía el mensaje de éxito al client
{
	infoSuccessClientMessage(htmlFile);	//crea el mensaje

	boost::system::error_code error;
	size_t len = 0;

	do
	{
		len = socket_forServer->write_some(boost::asio::buffer(answerMessage.c_str(), answerMessage.length()), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to send message to client. " << error.message() << endl;
}

void server::
sendFailMessage()	//envía el mensaje de error al client
{
	infoFailClientMessage();	//crea el mensaje

	boost::system::error_code error;
	size_t len = 0;

	do
	{
		len = socket_forServer->write_some(boost::asio::buffer(answerMessage.c_str(), answerMessage.length()), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to send message to client. " << error.message() << endl;
}

string server::
contentLength(FILE *htmlFile)	//devuelve el largo del archivo (bytes) en un string
{
	fseek(htmlFile, 0, SEEK_END);
	long int contentLength = ftell(htmlFile);
	return to_string(contentLength);
}

string server::
getCurrentDate(int state)	//devuelve la fecha y hora en el formato pedido (ejemplo: Tue, 04 Sep 2018 18:21:19 GMT)
{
	time_t time_;
	struct tm * timeinfo;
	char forDate[100];

	time(&time_);
	timeinfo = localtime(&time_);

	if (state == INIT)
	{
		strftime(forDate, 100, "%a, %d %h %Y %X %Z", timeinfo);
	}
	else
	{
		if (timeinfo->tm_sec + 30 > 60)
		{
			timeinfo->tm_sec = (timeinfo->tm_sec - 60) + 30;
			timeinfo->tm_min++;
			strftime(forDate, 100, "%a, %d %h %Y %X %Z", timeinfo);
		}
		else
		{
			timeinfo->tm_sec = timeinfo->tm_sec + 30;
			strftime(forDate, 100, "%a, %d %h %Y %X %Z", timeinfo);
		}
	}
	return forDate;
}

void server::
infoSuccessClientMessage(FILE *htmlFile)
{
	answerMessage = "HTTP/1.1 200 OK";
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Date: " + getCurrentDate(INIT);
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Location: " + host + path;
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Cache-Control: max-age = 30";
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Expires: " + getCurrentDate(MAX);// +date mas 30s
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Content-Length: " + contentLength(htmlFile);
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Content-Type: text/html; charset=iso-8859-1";
	answerMessage += CR;
	answerMessage += LF;

	//contenido del archivo
}

void server::
infoFailClientMessage()
{
	answerMessage = "HTTP/1.1 404 Not Found";
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Date: " + getCurrentDate(INIT);
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Cache-Control: max-age = 30";
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Expires: " + getCurrentDate(MAX);// +date mas 30s
	answerMessage += CR;
	answerMessage += LF;
	answerMessage += "Content-Length: 0";
	answerMessage += CR;
	answerMessage += LF;

}

server::
server()
{
	IO_handler = new boost::asio::io_service();
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), HELLO_PORT);

	socket_forServer = new boost::asio::ip::tcp::socket(*IO_handler);
	server_acceptor = new boost::asio::ip::tcp::acceptor(*IO_handler, ep);

}

serverError server::
getError()
{
	return err;
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

