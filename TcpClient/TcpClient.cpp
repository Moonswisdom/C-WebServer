#include "TcpClient.h"

TcpClient::TcpClient()
{
	// ����socket����
	NetEnv::Init();
	// ������־��
	//LogMgr::start();
	
	// ��ʼ��
	_cSock = INVALID_SOCKET;
	_rlen = 0;
	_isConnect = false;
}

TcpClient::~TcpClient()
{
	Close();
}

SOCKET TcpClient::InitSock()
{
	// �ж��Ƿ��о�����
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		_cSock = INVALID_SOCKET;
	}

	// ������socket
	_cSock = socket(AF_INET, SOCK_STREAM, 0);

	return _cSock;
}

int TcpClient::Connect(const char* ip, unsigned short port)
{
	// �ж�socket�Ƿ񴴽��ɹ�
	if (InitSock() == INVALID_SOCKET)
	{
		LogMgr::addTask(LEVEL_ERROR, "create new socket error ...");
		return -1;
	}
	// ����socket�ɹ�
	//std::string str = "create new <socket = " + std::to_string((int)_cSock) + "> successful.";
	//LogMgr::addTask(LEVEL_INFO, str.c_str());

	// ������ַ�˿ڱ���
	sockaddr_in caddr = {};
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &caddr.sin_addr);
	// ��������
	if (SOCKET_ERROR == connect(_cSock, (struct sockaddr*)&caddr, sizeof(caddr))) 
	{
		// ����ʧ��
		std::string str = "<socket = " + std::to_string((int)_cSock) + "> connect server error.";
		LogMgr::addTask(LEVEL_ERROR, str.c_str());
		return -1;
	}
	// ���ӳɹ�
	std::string str = "<socket = " + std::to_string((int)_cSock) + "> connect server successful.";
	LogMgr::addTask(LEVEL_ERROR, str.c_str());
	_isConnect = true;
	return 0;
}

bool TcpClient::isRun()
{
	return _cSock != INVALID_SOCKET && _isConnect;
}

void TcpClient::mainRun()
{
	if (isRun())
	{
		fd_set fdRead = {};
		//fd_set fdWrite = {};
		//fd_set fdExp = {};
		FD_ZERO(&fdRead);
		//FD_ZERO(&fdWrite);
		//FD_ZERO(&fdExp);
		FD_SET(_cSock, &fdRead);
		//FD_SET(_cSock, &fdWrite);
		//FD_SET(_cSock, &fdExp);
		timeval tval = { 0, 50000 };
		
		if (0 > select((int)_cSock + 1, &fdRead, NULL, NULL, &tval))
		{
			std::string str = "<socket=" + std::to_string((int)_cSock) + "> select error.";
			LogMgr::addTask(LEVEL_ERROR, str.c_str());
			return;
		}

		if (FD_ISSET(_cSock, &fdRead))
		{
			FD_CLR(_cSock, &fdRead);
			{
				if (RecvData() == -1)
				{
					Close();
				}
			}
		}
	}
}

int TcpClient::SendData(msg_header* header, int nSize)
{
	return send(_cSock, (const char*)header, nSize, 0);
}

int TcpClient::RecvData()
{
	return _recvBuf.recv2buff(_cSock);
}

int TcpClient::ParseData()
{
	// �������ݣ��ɵ����߳�
	if (isRun())
	{
		if (_recvBuf.hasMsg())
		{
			// �����ݣ�����
			msg_header* header = _recvBuf.getMsg();
			// ��������
			switch (header->_Msg)
			{
				case MSG_LOGIN_RESULT:
				{
					msg_LoginRet* loginRet = (msg_LoginRet*)header;
					// �Զ��崦��ʽ��������־
					//std::string str = "<socket=" + std::to_string((int)_cSock) + " LoginResult> " + loginRet->_Ret;
					//LogMgr::addTask(LEVEL_INFO, str.c_str());
					break;
				}
				case MSG_LOGOUT_RESULT:
				{
					msg_LogoutRet* logoutRet = (msg_LogoutRet*)header;
					// �Զ��崦��ʽ��������־
					//std::string str = "<socket=" + std::to_string((int)_cSock) + " LogoutResult> " + logoutRet->_Ret;
					//LogMgr::addTask(LEVEL_INFO, str.c_str());
					break;
				}
				default:
				{
					break;
				}
			}
			// ������ϣ��ӻ�����ɾ��
			_recvBuf.delMsg(header->_Len);
		}
		else
		{
			// û���ݣ�����
			std::chrono::milliseconds tim(10);
			std::this_thread::sleep_for(tim);
		}
	}
	return 0;
}

void TcpClient::Close()
{
	_isConnect = false;
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif // _WIN32
		_cSock = INVALID_SOCKET;
	}
}
