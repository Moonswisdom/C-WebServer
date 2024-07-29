#include "TcpClient.h"
#include<vector>
#include<iomanip>
#include<unordered_set>

// 程序运行标识
bool g_Run = true;

// 输入函数
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

// 模拟多个客户端
const int tCount = 2; // 线程个数
const int cCount = 1000; // 客户端个数
const int nCount = cCount / tCount;
std::atomic_int readyCount = 0; // 连接成功的线程数
std::atomic_int sendCount = 0; // 发送的数据数量
//std::unordered_set<int> leaveClient;	// 统计断开连接的客户端
typedef std::shared_ptr<TcpClient> ClientPtr;	// 管理客户端开辟和释放
std::vector<ClientPtr> vClient(cCount);
std::mutex g_mutex;


// 处理数据
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

// 发送数据
const int msgNum = 10; // 每次发送的数量
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

		// 10000个客户端循环发送速度太快了，稍微休眠一点模拟现实
		std::chrono::milliseconds tim(500);
		std::this_thread::sleep_for(tim);
	}
}
// 主循环
void mainRun(int n)
{
	int begin = n * nCount;
	int end = (n + 1) * nCount;
	for (int i = begin; i < end; ++i)
	{
		ClientPtr client(new TcpClient());
		// 建立连接
#ifdef _WIN32
		client->Connect("127.0.0.1", 4567);
#else
		client->Connect("192.168.200.1", 9999);
#endif
		vClient[i] = client;
	}

	// 等待所有客户端连接完成再进行后续
	++readyCount;
	while (readyCount < tCount && g_Run)
	{
		std::chrono::milliseconds tim(5);
		std::this_thread::sleep_for(tim);
	}

	// 启动数据处理
	std::thread t1(&workRun, begin, end);
	t1.detach();
	// 启动数据发送
	std::thread t2(&sendRun, begin, end);
	t2.detach();
	const int msgNum = 1; // 每次发送的数量
	msg_Login login[msgNum];
	int nlen = sizeof(msg_Login) * msgNum;

	// 主循环
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
	// 开启日志器
	LogMgr::start();
	// 输入线程
	std::thread tcin(cmdThread);
	tcin.detach();
	//leaveClient.clear();

	// 创建线程
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

	// 打印基本信息
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