#include "Client.h"

bool g_cRun = true;
//void CTask(TcpClient* client) {
void CTask(){
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
		else 
		{
			std::cout << "nuknown command ..." << std::endl;
		}
		/*
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
		*/
	}
}

int main()
{
	std::thread t1(CTask);
	t1.detach();

	const int num = 150;
	std::vector<TcpClient*> Clients;
	for (int i = 0; i < num; ++i)
	{
		if (!g_cRun)
		{
			break;
		}
		TcpClient* client = new TcpClient();
		Clients.emplace_back(client);
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
	}

	Login login = {};
	const char name[] = "xiaohua";
	const char pwd[] = "123456";
	memcpy(login._Username, name, sizeof(name));
	memcpy(login._Password, pwd, sizeof(pwd));

	while (g_cRun)
	{
		for (auto& client : Clients)
		{
			client->SendData(&login);
			client->MainRun();
		}
	}
	
	for (auto& client : Clients)
	{
		delete client;
	}

#ifdef _WIN32
	system("pause");
#endif

	return 0;
}