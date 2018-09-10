#pragma once
#include <iostream>
#include <cstdio>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>

#define HELLO_PORT_STR "80"
#define SERVER_IP "localhost"

#define MSGSIZE 512U

#define LF 0x0A
#define CR 0x0D

using namespace std;

typedef enum {NO_ERR,NO_SLASH}clientErrType;

typedef struct
{
	clientErrType type;
	string detail;
}clientError;

class client
{
public:
	client();
	bool startConnection(const char* host);
	bool receiveMessage();
	bool messageToServer();
	void messageToServer(const char msg[MSGSIZE]);
	~client();

private:
	boost::asio::io_service* IO_handler;
	boost::asio::ip::tcp::socket* socket_forClient;
	boost::asio::ip::tcp::resolver* client_resolver;
	boost::asio::ip::tcp::resolver::iterator endpoint;
	string host;
	string path;
	string serverMessage;
	string messageFromServer;
	clientError err;
	void info2ServerMessage();
	void getPathAndHost(char buf[]);
	void clearBuf(char buf[], unsigned int size);
};

