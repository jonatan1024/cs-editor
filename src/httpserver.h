#ifndef __EDITOR_HTTPSERVER__
#define __EDITOR_HTTPSERVER__

#include <WinSock2.h>
#include <stdio.h>
#include <ws2tcpip.h>

#include "defs.h"

void urlDecode(char * source, char * destination, int length);

class httpServer{
private:
	SOCKET ssocket;
	HANDLE cthread;
	int forward;
public:
	httpServer();
	void setForward(int forward);
	void tick();
	bool start();
	~httpServer();

	//these two are public just for that thread fun
	char* craftResponse(char* req, char* version, int* len, bool cached);
	SOCKET getSSocket();
};
#endif