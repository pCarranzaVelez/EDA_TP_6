#pragma once

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>

#define HELLO_PORT 80

#define MSGSIZE 512
#define MAXTIME 20

#define LF 0x0A
#define CR 0x0D

using namespace std;

class server
{
public:
	server();
	void startConnection();
	void sendMessage();
	void sendMessage(const char msg[MSGSIZE]);
	void receiveMessage(bool * ret);
	~server();

private:
	boost::asio::io_service*  IO_handler;
	boost::asio::ip::tcp::socket* socket_forServer;
	boost::asio::ip::tcp::acceptor* server_acceptor;
	string firstLine;
	string secondLine;
	bool searchCrlf(char buf[]);
	void isFilePresent();
	bool parse2ndLine();
	bool parseFirstLine();
};