#include "HighServer.h"

// 是否运行标识
bool g_sRun = true;
// 输入指令
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

// 继承服务器，用户可自定义操作事件
class Myserver : public TcpServer 
{
public:
	// 客户端加入事件
	virtual void JoinEvent(ClientData* pClient)
	{
		TcpServer::JoinEvent(pClient);
	}
	// 客户端离开事件
	virtual void LeaveEvent(ClientData* pClient)
	{
		TcpServer::LeaveEvent(pClient);
	}
	// 接收客户端请求的工作事件
	virtual void RecvEvent(ClientData* pClient)
	{
		TcpServer::RecvEvent(pClient);
	}
	// 处理客户端请求的工作事件
	virtual void RequestEvent(cellServer* pServer, ClientData* pClient, DataHeader* header)
	{
		TcpServer::RequestEvent(pServer, pClient, header);
		switch (header->_Cmd)
		{
			case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				//std::cout << "Recv <socet=" << pClient->getSocket() << "> command = LOGIN, username = " << login->_Username << ", password = " << login->_Password << std::endl;
				//LoginResult loginRet = {};
				//char result[] = "login succeed";
				//memcpy(loginRet._Result, result, sizeof(result));
				//pClient->SendData(&loginRet);
				LoginResult* ret = new LoginResult();
				pServer->addSendTask(pClient, ret);
				break;
			}
			case CMD_LOGOUT:
			{
				Logout* logout = (Logout*)header;
				//std::cout << "Recv <socet=" << pClient->getSocket() << "> command = LOGOUT, username = " << logout->_Username << std::endl;
				//LogoutResult logoutRet = {};
				//char result[] = "logout succeed";
				//memcpy(logoutRet._Result, result, sizeof(result));
				//pClient->SendData(&logoutRet);
				break;
			}
		}
	}
private:

};

// 服务器主函数
int main()
{
	// 启动输入指令线程
	std::thread t1(&cmdThread);
	t1.detach();

	// 启动主服务器
	Myserver server;
	server.InitSock();
	server.Bind(nullptr, 9999);
	server.Listen(8);
	server.startThread(4);

	// 主服务器持续运行
	while (server.isRun() && g_sRun)
	{
		server.mainRun();
	}

	return 0;
}