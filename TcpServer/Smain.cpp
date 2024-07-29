#include "TcpServer.h"

/**
* ----- 内存池和对象池使用 -----
*/
//#include "Alloctor.h"
// 内存池直接打开头文件即可

//#include "objectPool.h"
// 对象池使用需要继承 objMgr
// 对象池使用举例: class nameA : public objMgr<nameA, 100> 开辟100大小的nameA类对象池


/**
* ----- 自定义服务器 -----
*/
// 继承服务器并进行扩展
class MyServer : public TcpServer
{
public:
	MyServer() {}
	~MyServer() {}
public:
	// 重写打印函数
	void printInfo() override
	{
		TcpServer::printInfo();
	}
	// 重写客户端加入
	void joinEvent(ClientPtr pClient) override
	{
		TcpServer::joinEvent(pClient);
	}
	// 重写客户端离开
	void leaveEvent(ClientPtr pClient) override
	{
		TcpServer::leaveEvent(pClient);
	}
	// 重写接收数据
	void recvEvent(ClientPtr pClient) override
	{
		// 调用基本处理, 后续可自由添加
		TcpServer::recvEvent(pClient);
	}
	// 重写解析数据
	void parseEvent(tServer* tSvr, ClientPtr pClient, msg_header* header) override
	{
		// 调用基本处理, 后续自由添加
		TcpServer::parseEvent(tSvr, pClient, header);
		// 添加具体解析内容
		switch (header->_Msg)
		{
			case MSG_LOGIN:
			{
				msg_Login* login = (msg_Login*)header;
				// 解析结果写入到日志
				//std::string str = "recv message LOGIN, username<" + static_cast<std::string>(login->_Name) + ">, password<" + static_cast<std::string>(login->_pwd) + ">.";
				//LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));
				// 返回数据
				msg_LoginRet* loginRet = new msg_LoginRet("Login successful, you are welcome.");
				tSvr->addTask(pClient, loginRet);
				break;
			}
			case MSG_LOGOUT:
			{
				msg_Logout* logout = (msg_Logout*)header;
				// 解析结果写入到日志
				std::string str = "recv message LOGOUT, username<" + static_cast<std::string>(logout->_Name) + ">.";
				LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));
				// 返回数据
				msg_LogoutRet logoutRet = { "Logout successful, good bye." };
				tSvr->addTask(pClient, &logoutRet);
				break;
			}
			default:
			{
				LogMgr::addTask(new LogInfo(LEVEL_DEBUG, "parse an invalid information."));
				break;
			}
		}
	}
};

// 输入线程
bool g_Run = true;
void cmdThread()
{
	char cinBuf[512] = {};
	while (g_Run)
	{
		std::cin >> cinBuf;
		if (0 == strcmp(cinBuf, "exit"))
		{
			std::cout << "server exit ..." << std::endl;
			g_Run = false;
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
	// 开启输入线程
	std::thread t(cmdThread);
	t.detach();

	// 创建服务器
	MyServer server;
	server.InitSock();
	server.Bind(nullptr, 4567);
	server.Listen(8);

	// 开启线程服务器
	server.start(4);

	// 主服务器循环
	while (server.isRun() && g_Run)
	{
		server.mainRun();
	}

	return 0;
}