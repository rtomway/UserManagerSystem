﻿#ifndef SERVER_H_
#define SERVER_H_

#include<QHttpServer>

class Server
{

public:
	Server();
public:
	void userRoute();
	void privilegeRoute();
private:
	QHttpServer  m_server;
};

#endif // !SERVER_H_
