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

	do
	{
		len = socket_forClient->read_some(boost::asio::buffer(buf), error);

		if (!error)
			buf[len] = '\0';

	} while (error.value() == WSAEWOULDBLOCK);

	if (!error)
	{
		if (buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't')
		{
			ret = true;
		}
		else
		{
			cout << endl << "Server says: " << buf << endl;
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
messageToServer()
{
	boost::system::error_code error;
	char buf[MSGSIZE];
	size_t len = 0;
	cout << "Write your message:" << endl;
	cin >> buf;

	do
	{
		len = socket_forClient->write_some(boost::asio::buffer(buf, strlen(buf)), error);
	} while ((error.value() == WSAEWOULDBLOCK));
	if (error)
		cout << "Error while trying to connect to server " << error.message() << endl;
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
