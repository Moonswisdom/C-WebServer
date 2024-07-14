#include "Server.h"

int main()
{
	TcpServer server;
	server.InitSocket();
	server.Bind(NULL, 9999);
	server.Listen(8);
	
	while (server.isRun())
	{
		server.MainRun();
	}
	
	return 0;
}