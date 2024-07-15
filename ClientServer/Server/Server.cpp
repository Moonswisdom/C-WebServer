#include "Server.h"

ClientSock::ClientSock(SOCKET sock):_cSock(sock)
{
	memset(_MsgBuf, 0, RECV_BUFF_SIZE * 10);
	_lastPos = 0;
}

SOCKET ClientSock::getSocket() const
{
	return _cSock;
}

char* ClientSock::getMsgBuf()
{
	return _MsgBuf;
}

int ClientSock::getlastPos() const
{
	return _lastPos;
}

void ClientSock::setlastPos(int pos)
{
	_lastPos = pos;
}

TcpServer::TcpServer()
{
	_sSock = INVALID_SOCKET;
	_sClients.clear();
}

TcpServer::~TcpServer()
{
	Close();
}

// ����socket
int TcpServer::InitSocket()
{
	// windowsƽ̨��socket����
#ifdef _WIN32
	// ʹ��socket 2.x�汾
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat = {};
	int err = WSAStartup(wr, &dat);
	if (err != 0)
	{
		std::cout << "window start up socket environment failed, error num is " << err << std::endl;
		return -1;
	}
#endif
	// �ж��Ƿ��о�����
	if (_sSock != INVALID_SOCKET)
	{
		Close();
		std::cout << "off old connect ..." << std::endl;
	}
	// ����Socket
	_sSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: create new socket failed ..." << std::endl;
		return -1;
	}
	std::cout << "create new socket succeed ..." << std::endl;
	return 0;
}

// �󶨶˿�
int TcpServer::Bind(const char* ip, unsigned short port)
{
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "Invalid socket can not bind ..." << std::endl;
		return -1;
	}
	sockaddr_in saddr = {};
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	if (ip) 
	{
		inet_pton(AF_INET, ip, &saddr.sin_addr);
	}
	else
	{
#ifdef _WIN32
		saddr.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		saddr.sin_addr.s_addr = INADDR_ANY;
#endif
	}

	if (SOCKET_ERROR == bind(_sSock, (struct sockaddr*)&saddr, sizeof(saddr))) 
	{
		std::cout << "ERROR: bind port error ..." << std::endl;
		return -1;
	}
	std::cout << "bind port " << port << " succeed ..." << std::endl;
	return 0;
}

// �����˿�
int TcpServer::Listen(int num)
{
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "Invalid socket can not listen ..." << std::endl;
		return -1;
	}
	if (SOCKET_ERROR == listen(_sSock, num)) 
	{
		std::cout << "ERROR: listen port error ..." << std::endl;
		return -1;
	}
	std::cout << "listen port succeed ..." << std::endl;

	return 0;
}

// ���ܿͻ�������
int TcpServer::Accept()
{
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "Invalid socket can not accept ..." << std::endl;
		return -1;
	}
	sockaddr_in caddr = {};
#ifdef _WIN32
	int clen = sizeof(caddr);
#else
	socklen_t clen = sizeof(caddr);
#endif
	SOCKET cSock = INVALID_SOCKET;
	cSock = accept(_sSock, (struct sockaddr*)&caddr, &clen);
	if (cSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: accept error ..." << std::endl;
		return -1;
	}
	char ip[32];
	inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
	std::cout << "New client <socket=" << cSock << "> connect, ip: " << ip << std::endl;
	NewUser newuser(cSock);
	SendToAll(&newuser);
	_sClients.emplace_back(new ClientSock(cSock));
	return 0;
}

// ������Ϣ
int TcpServer::SendData(SOCKET cSock, DataHeader* header)
{
	if (isRun() && header)
	{
		send(cSock, (char*)header, header->_Length, 0);
		return 0;
	}
	return -1;
}

// Ⱥ����Ϣ
void TcpServer::SendToAll(DataHeader* header)
{
	if (isRun() && header) {
		for (auto& client : _sClients)
		{
			SendData(client->getSocket(), header);
		}
	}
}

// ������Ϣ
// ���ջ����� -> ��Ϣ�������� ��ϺͲ�����������ճ�����ٰ�����
char recvBuf[RECV_BUFF_SIZE] = {};
int TcpServer::RecvData(ClientSock* Client)
{
	int recvlen = recv(Client->getSocket(), recvBuf, RECV_BUFF_SIZE, 0);
	if (recvlen < 0)
	{
		std::cout << "recv error ..." << std::endl;
		return -1;
	}
	else if (recvlen == 0)
	{
		std::cout << "<socket=" << Client->getSocket() << "> client out connect ..." << std::endl;
		return -1;
	}
	// ���յ�����Ϣ -> ��Ϣ������������������ϺͲ��
	memcpy(Client->getMsgBuf(), recvBuf, recvlen);
	Client->setlastPos(Client->getlastPos() + recvlen);
	// ����ͷ����  // while ѭ�������������ݣ�̫����������Ҫʹ�ö��߳�
	while (Client->getlastPos() >= sizeof(DataHeader))
	{
		DataHeader* header = (DataHeader*)Client->getMsgBuf();
		// ���������
		if (Client->getlastPos() >= header->_Length)
		{
			// ����Ϣ������������Ϣ��������Ϣ������
			int pos = Client->getlastPos() - header->_Length;
			ParseData(Client->getSocket(), header);
			memcpy(Client->getMsgBuf(), Client->getMsgBuf() + header->_Length, pos);
			Client->setlastPos(pos);
		}
		else {
			break;
		}
	}
	return 0;
}

// ������Ϣ
void TcpServer::ParseData(SOCKET cSock, DataHeader* header)
{
	switch (header->_Cmd)
	{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			std::cout << "<socket=" << cSock << "> Command is LOGIN, username is " << login->_Username << ", password is " << login->_Password << std::endl;
			LoginResult loginRet = {};
			SendData(cSock, &loginRet);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			std::cout << "<socket=" << cSock << "> Command is LOGOUT, username is " << logout->_Username << std::endl;
			LogoutResult logoutRet = {};
			SendData(cSock, &logoutRet);
			break;
		}
	}
}

// �ر�����
void TcpServer::Close()
{
	if (_sSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		for (auto& client : _sClients)
		{
			closesocket(client->getSocket());
			delete client;
		}
		closesocket(_sSock);
		WSACleanup();
#else
		for (auto& client : _sClients)
		{
			close(client->getSocket());
			delete client;
		}
		close(_sSock);
#endif
		_sSock = INVALID_SOCKET;
	}
}

// �жϿͻ����Ƿ�����
bool TcpServer::isRun() const
{
	return _sSock != INVALID_SOCKET;
}

// �ͻ�������������
bool TcpServer::MainRun()
{
	if (isRun())
	{
		fd_set fdRead, fdWrite, fdExp;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		FD_SET(_sSock, &fdRead);
		FD_SET(_sSock, &fdWrite);
		FD_SET(_sSock, &fdExp);
		timeval tval = { 1, 0 };

		SOCKET maxSock = _sSock;
		for (auto& client : _sClients)
		{
			FD_SET(client->getSocket(), &fdRead);
			maxSock = max(maxSock, client->getSocket());
		}

		if (0 > select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &tval))
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return false;
		}

		if (FD_ISSET(_sSock, &fdRead))
		{
			FD_CLR(_sSock, &fdRead);
			if (Accept() == -1)
			{
				return false;
			}
		}
		for (int i = 0; i < _sClients.size(); ++i) {
			if (FD_ISSET(_sClients[i]->getSocket(), &fdRead)) {
				if (-1 == RecvData(_sClients[i]))
				{
					auto it = _sClients.begin() + i;
					if (it != _sClients.end())
					{
						_sClients.erase(it);
					}
				}
			}
			
		}
		return true;
	}
	return false;
}

