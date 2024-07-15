#include "Server.h"

bool g_sRun = true;
void cmdThread() {
	char sendBuf[512];
	while (true)
	{
		std::cin >> sendBuf;
		if (strcmp(sendBuf, "exit") == 0)
		{
			g_sRun = false;
			std::cout << "server exit ..." << std::endl << std::endl;
			return;
		}
		else
		{
			std::cout << "unknown command, please input again ..." << std::endl;
		}
	}
}

int main()
{
	TcpServer server;
	server.InitSocket();
	server.Bind(NULL, 9999);
	server.Listen(8);
	
	std::thread t1(cmdThread);
	t1.detach();

	while (g_sRun && server.isRun())
	{
		server.MainRun();
	}
	
	return 0;
}