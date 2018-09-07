#pragma once
#include <iostream>
#include <cstdio>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>

#define HELLO_PORT_STR "80"
#define SERVER_IP "localhost"

#define MSGSIZE 512

#define LF 0x0A
#define CR 0x0D

using namespace std;

class client
{
public:
	client();
	bool startConnection(const char* host);
	bool receiveMessage();
	void messageToServer();
	void messageToServer(const char msg[MSGSIZE]);
	~client();

private:
	boost::asio::io_service* IO_handler;
	boost::asio::ip::tcp::socket* socket_forClient;
	boost::asio::ip::tcp::resolver* client_resolver;
	boost::asio::ip::tcp::resolver::iterator endpoint;
};

