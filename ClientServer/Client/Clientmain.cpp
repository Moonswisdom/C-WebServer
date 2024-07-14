#include "Client.h"

void CTask(TcpClient* client) {
	char sendBuf[512];
	while (true)
	{
		std::cin >> sendBuf;
		if (strcmp(sendBuf, "exit") == 0)
		{
			client->Close();
			std::cout << "client exit ..." << std::endl;
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
	TcpClient client;
	client.InitSocket();
#ifdef _WIN32
	client.Connect("127.0.0.1", 9999);
#else
	client.Connect("192.168.200.1", 9999);
#endif
	
	std::thread t1(CTask, &client);
	t1.detach();

	while (client.isRun())
	{
		client.MainRun();
	}
	client.Close();

	return 0;
}