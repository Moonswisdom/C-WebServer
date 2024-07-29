#include "TcpClient.h"
#include<vector>
#include<iomanip>
#include<unordered_set>

// �������б�ʶ
bool g_Run = true;

// ���뺯��
void cmdThread()
{
	char cinBuf[512] = {};
	while (g_Run)
	{
		std::cin >> cinBuf;
		if (0 == strcmp(cinBuf, "exit"))
		{
			std::cout << "exit ..." << std::endl;
			g_Run = false;
			return;
		}
		else
		{
			std::cout << "unknow command ..." << std::endl;
		}
	}
}

// ģ�����ͻ���
const int tCount = 2; // �̸߳���
const int cCount = 1000; // �ͻ��˸���
const int nCount = cCount / tCount;
std::atomic_int readyCount = 0; // ���ӳɹ����߳���
std::atomic_int sendCount = 0; // ���͵���������
//std::unordered_set<int> leaveClient;	// ͳ�ƶϿ����ӵĿͻ���
typedef std::shared_ptr<TcpClient> ClientPtr;	// ����ͻ��˿��ٺ��ͷ�
std::vector<ClientPtr> vClient(cCount);
std::mutex g_mutex;


// ��������
void workRun(int begin, int end)
{
	while (g_Run)
	{
		for (int i = begin; i < end; ++i)
		{
			vClient[i]->ParseData();
		}
	}
}

// ��������
const int msgNum = 10; // ÿ�η��͵�����
void sendRun(int begin, int end)
{
	while (g_Run)
	{
		msg_Login login[msgNum];
		int nlen = sizeof(msg_Login) * msgNum;
		for (int i = begin; i < end; ++i)
		{
			vClient[i]->SendData(login, nlen);
			sendCount += msgNum;
		}

		// 10000���ͻ���ѭ�������ٶ�̫���ˣ���΢����һ��ģ����ʵ
		std::chrono::milliseconds tim(500);
		std::this_thread::sleep_for(tim);
	}
}
// ��ѭ��
void mainRun(int n)
{
	int begin = n * nCount;
	int end = (n + 1) * nCount;
	for (int i = begin; i < end; ++i)
	{
		ClientPtr client(new TcpClient());
		// ��������
#ifdef _WIN32
		client->Connect("127.0.0.1", 4567);
#else
		client->Connect("192.168.200.1", 9999);
#endif
		vClient[i] = client;
	}

	// �ȴ����пͻ�����������ٽ��к���
	++readyCount;
	while (readyCount < tCount && g_Run)
	{
		std::chrono::milliseconds tim(5);
		std::this_thread::sleep_for(tim);
	}

	// �������ݴ���
	std::thread t1(&workRun, begin, end);
	t1.detach();
	// �������ݷ���
	std::thread t2(&sendRun, begin, end);
	t2.detach();
	const int msgNum = 1; // ÿ�η��͵�����
	msg_Login login[msgNum];
	int nlen = sizeof(msg_Login) * msgNum;

	// ��ѭ��
	while (g_Run)
	{
		for (int i = begin; i < end; ++i)
		{
			if (vClient[i]->isRun())
			{
				vClient[i]->mainRun();
			}
		}
	}
}

int main()
{
	// ������־��
	LogMgr::start();
	// �����߳�
	std::thread tcin(cmdThread);
	tcin.detach();
	//leaveClient.clear();

	// �����߳�
	std::vector<std::thread> vthread(tCount);
	for(int i = 0; i < tCount; ++i)
	{
		vthread[i] = std::thread(&mainRun, i);
		vthread[i].detach();
	}
	
	while (readyCount < tCount && g_Run)
	{
		std::chrono::milliseconds tim(5);
		std::this_thread::sleep_for(tim);
	}

	// ��ӡ������Ϣ
	HRTimer timer;
	while (g_Run)
	{
		auto tim = timer.getSecond();
		if (tim >= 1.0)
		{
			//auto clientnum = leaveClient.size();
			std::cout << "thread<num=" << tCount << ">, Client<num=" << cCount << ">, time<interval="
				<< std::setiosflags(std::ios::fixed) << std::setprecision(6) << tim << ">, sendnum<speed="
				<< (int)(sendCount / tim) << ">" << std::endl;
			timer.updateTime();
			sendCount = 0;
		}
	}

	return 0;
}