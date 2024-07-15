#include "Client.h"

bool g_cRun = true;
void CTask(TcpClient* client) {
	char sendBuf[512];
	while (true)
	{
		std::cin >> sendBuf;
		if (strcmp(sendBuf, "exit") == 0)
		{
			g_cRun = false;
			std::cout << "client exit ..." << std::endl << std::endl;
			return;
		}
		else if (strcmp(sendBuf, "login") == 0)
		{
			Login login = {};
			const char* name = "xiaohua";
			const char* pwd = "123456";
			memcpy(login._Username, name, sizeof(name));
			memcpy(login._Password, pwd, sizeof(pwd));
			client->SendData(&login);
		}
		else if (strcmp(sendBuf, "logout") == 0)
		{
			Logout logout = {};
			const char* name = "xiaohua";
			memcpy(logout._Username, name, sizeof(name));
			client->SendData(&logout);
		}
	}
}

int main()
{
	TcpClient* client = new TcpClient();
	client->InitSocket();
	
	// 服务器在windows运行
#ifdef _WIN32
	client->Connect("127.0.0.1", 9999);
#else
	client->Connect("192.168.200.1", 9999);	 // windows主机 相对于 linux虚拟机 的地址
#endif
	
	// 服务器在linux运行
/*
#ifdef _WIN32
	client->Connect("192.168.200.128", 9999);
#else
	client->Connect("127.0.0.1", 9999);		// linux主机 相对于 windows主机 的地址
#endif
*/

	std::thread t1(CTask, client);
	t1.detach();

	Login login = {};
	const char name[] = "xiaohua";
	const char pwd[] = "123456";
	memcpy(login._Username, name, sizeof(name));
	memcpy(login._Password, pwd, sizeof(pwd));

	while (g_cRun && client->isRun())
	{
		//client->SendData(&login);
		client->MainRun();
	}

	delete client;

#ifdef _WIN32
	system("pause");
#endif

	return 0;
}