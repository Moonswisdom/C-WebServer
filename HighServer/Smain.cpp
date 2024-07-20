#include "HighServer.h"

// �Ƿ����б�ʶ
bool g_sRun = true;
// ����ָ��
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

// �̳з��������û����Զ�������¼�
class Myserver : public TcpServer 
{
public:
	// �ͻ��˼����¼�
	virtual void JoinEvent(ClientData* pClient)
	{
		TcpServer::JoinEvent(pClient);
	}
	// �ͻ����뿪�¼�
	virtual void LeaveEvent(ClientData* pClient)
	{
		TcpServer::LeaveEvent(pClient);
	}
	// ���տͻ�������Ĺ����¼�
	virtual void RecvEvent(ClientData* pClient)
	{
		TcpServer::RecvEvent(pClient);
	}
	// ����ͻ�������Ĺ����¼�
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

// ������������
int main()
{
	// ��������ָ���߳�
	std::thread t1(&cmdThread);
	t1.detach();

	// ������������
	Myserver server;
	server.InitSock();
	server.Bind(nullptr, 9999);
	server.Listen(8);
	server.startThread(4);

	// ����������������
	while (server.isRun() && g_sRun)
	{
		server.mainRun();
	}

	return 0;
}