#include "TcpServer.h"

/**
* ----- �ڴ�غͶ����ʹ�� -----
*/
//#include "Alloctor.h"
// �ڴ��ֱ�Ӵ�ͷ�ļ�����

//#include "objectPool.h"
// �����ʹ����Ҫ�̳� objMgr
// �����ʹ�þ���: class nameA : public objMgr<nameA, 100> ����100��С��nameA������


/**
* ----- �Զ�������� -----
*/
// �̳з�������������չ
class MyServer : public TcpServer
{
public:
	MyServer() {}
	~MyServer() {}
public:
	// ��д��ӡ����
	void printInfo() override
	{
		TcpServer::printInfo();
	}
	// ��д�ͻ��˼���
	void joinEvent(ClientPtr pClient) override
	{
		TcpServer::joinEvent(pClient);
	}
	// ��д�ͻ����뿪
	void leaveEvent(ClientPtr pClient) override
	{
		TcpServer::leaveEvent(pClient);
	}
	// ��д��������
	void recvEvent(ClientPtr pClient) override
	{
		// ���û�������, �������������
		TcpServer::recvEvent(pClient);
	}
	// ��д��������
	void parseEvent(tServer* tSvr, ClientPtr pClient, msg_header* header) override
	{
		// ���û�������, �����������
		TcpServer::parseEvent(tSvr, pClient, header);
		// ��Ӿ����������
		switch (header->_Msg)
		{
			case MSG_LOGIN:
			{
				msg_Login* login = (msg_Login*)header;
				// �������д�뵽��־
				//std::string str = "recv message LOGIN, username<" + static_cast<std::string>(login->_Name) + ">, password<" + static_cast<std::string>(login->_pwd) + ">.";
				//LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));
				// ��������
				msg_LoginRet* loginRet = new msg_LoginRet("Login successful, you are welcome.");
				tSvr->addTask(pClient, loginRet);
				break;
			}
			case MSG_LOGOUT:
			{
				msg_Logout* logout = (msg_Logout*)header;
				// �������д�뵽��־
				std::string str = "recv message LOGOUT, username<" + static_cast<std::string>(logout->_Name) + ">.";
				LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));
				// ��������
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

// �����߳�
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
	// ���������߳�
	std::thread t(cmdThread);
	t.detach();

	// ����������
	MyServer server;
	server.InitSock();
	server.Bind(nullptr, 4567);
	server.Listen(8);

	// �����̷߳�����
	server.start(4);

	// ��������ѭ��
	while (server.isRun() && g_Run)
	{
		server.mainRun();
	}

	return 0;
}