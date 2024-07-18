#include "Client.h"

bool g_cRun = true;
void cmdThread()
{
	char sendBuf[512];
	while (true)
	{
		std::cin >> sendBuf;
		if (strcmp(sendBuf, "exit") == 0)
		{
			g_cRun = false;
			std::cout << "client exit ..." << std::endl;
			return;
		}
		else
		{
			std::cout << "unknow command ..." << std::endl;
		}
	}
}

const int tCount = 4;
const int cCount = 1000;
std::vector<TcpClient*> _Clients(cCount);
void startThread(int n)
{
	// 分4个线程创建
	int c = cCount / tCount;
	int begin = c * (n - 1);
	int end = c * n;
	for (int i = begin; i < end; ++i)
	{
		if (!g_cRun) break;
		_Clients[i] = new TcpClient();
#ifdef _WIN32
		_Clients[i]->Connect("127.0.0.1", 9999);
#else
		_Clients[i]->Connect("192.168.200.1", 9999);
#endif
	}

	std::cout << "create new client " << begin << " - " << end << " succeed ... " << std::endl;
	// 稍微等待一会儿连接
	auto tim = std::chrono::microseconds(50);
	std::this_thread::sleep_for(tim);

	// 客户端线程工作
	char name[] = "xiaohua";
	char pwd[] = "123456";
	Login login;
	memcpy(login._Username, name, sizeof(name));
	memcpy(login._Password, pwd, sizeof(pwd));
	while (g_cRun) {
		for (int i = begin; i < end; ++i)
		{
			for (int j = 0; j < 10; ++j) {
				_Clients[i]->SendData(&login);
			}
			_Clients[i]->mainRun();
		}
	}

	for (int i = begin; i < end; ++i)
	{
		delete _Clients[i];
	}
	
}

int main()
{
	std::thread t1(cmdThread);
	t1.detach();

	std::vector<std::thread> tClients;
	for (int i = 0; i < tCount; ++i)
	{
		tClients.emplace_back(std::thread(startThread, i + 1));
	}
	for (auto& tClient : tClients)
	{
		tClient.join();
	}

#if _WIN32
	system("pause");
#endif

	return 0;
}