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
#define COMMAND_NUM 1	//cantidad de comandos HTTP esperados, si se aumenta, agregar los comandos esperados en server::validCommand()
#define VERSIONS_NUM 1
#define LF 0x0A
#define CR 0x0D

using namespace std;

typedef enum{NO_SERVER_ERR,WRONG_CRLF_FORMAT,INVALID_COMMAND,INVALID_VERSION,WRONG_PATH_FORMAT}servErrType;

typedef struct
{
	servErrType type;
	string detail;
}serverError;

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
	string path;
	string answerMessage;
	unsigned int commandEnd;
	unsigned int versionStart;
	bool validMessage(char buf[]);
	bool parseFirstLine();
	bool validCommand();
	bool validVersion();
	void isFilePresent();
	bool parse2ndLine();
	void sendSuccessMessage(FILE * htmlFile);
	void sendFailMessage();
	serverError err;
};