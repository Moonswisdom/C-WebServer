#include "Server.h"

bool g_sRun = true;
void cmdThread()
{
	char _sendBuf[512] = {};
	while (true)
	{
		std::cin >> _sendBuf;
		if (strcmp(_sendBuf, "exit") == 0)
		{
			g_sRun = false;
			std::cout << "server exit ..." << std::endl;
			return;
		}
		else
		{
			std::cout << "unknow command ..." << std::endl;
		}
	}
}

int main()
{
	std::thread t1(&cmdThread);
	t1.detach();

	TcpServer server;
	server.InitSock();
	server.Bind(nullptr, 9999);
	server.Listen(8);
	server.startThread(4);

	while (server.isRun() && g_sRun)
	{
		server.mainRun();
	}

	return 0;
}