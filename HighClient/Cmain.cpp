#include "HighClient.h"

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

// �̸߳���
const int tCount = 4;
// �ͻ��˸��� �� �洢����
const int cCount = 4000;
std::vector<TcpClient*> _Clients(cCount);
// ���ӽ�����ɵ��̸߳���
std::atomic_int readyCount = 0;
// ������������
std::atomic_int sendNum = 0;

void startThread(int n)
{
	// ��4���̴߳���
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

	// �ȴ�ȫ����������ٷ�������
	++readyCount;
	while (readyCount < tCount)
	{
		std::chrono::microseconds tim(5);
		std::this_thread::sleep_for(tim);
	}
	//std::cout << "create new client num = " << cCount << " succeed ... " << std::endl;

	// ������Ϣ��
	const int MsgNum = 10;
	Login login[MsgNum];
	char name[] = "xiaohua";
	char pwd[] = "123456";
	for (int i = 0; i < MsgNum; ++i)
	{
		memcpy(login[i]._Username, name, sizeof(name));
		memcpy(login[i]._Password, pwd, sizeof(pwd));
	}
	const int nlen = sizeof(login);
	// �ͻ����̹߳���
	while (g_cRun) {
		for (int i = begin; i < end; ++i)
		{
			_Clients[i]->SendData(login, nlen);
			sendNum += MsgNum;
			_Clients[i]->mainRun();
			//std::chrono::milliseconds tim(100);
			//std::this_thread::sleep_for(tim);
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
		tClients[i].detach();
	}

	EfficientTimer timer;
	while (g_cRun)
	{
		if (readyCount < tCount)
		{
			continue;
		}
		auto tim = timer.getElapseSecond();
		if (tim >= 1.0)
		{
			std::cout << "threadNum<" << tCount << ">, time<" << std::setiosflags(std::ios::fixed) << std::setprecision(6) << tim \
				<< ">, clientNum<" << cCount << ">, sendSpeed<" << int(sendNum / tim) << ">" << std::endl;
			
			timer.updateTime();
			sendNum = 0;
 		}
		
	}

#if _WIN32
	system("pause");
#endif

	return 0;
}