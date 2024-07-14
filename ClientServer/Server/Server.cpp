#include "Server.h"

TcpServer::TcpServer()
{
	_sSock = INVALID_SOCKET;
	_sClients.clear();
}

TcpServer::~TcpServer()
{
	Close();
}

int TcpServer::InitSocket()
{
	// windows平台打开socket环境
#ifdef _WIN32
	// 使用socket 2.x版本
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(wr, &dat);
#endif
	// 判断是否有旧连接
	if (_sSock != INVALID_SOCKET)
	{
		std::cout << "off old connect ..." << std::endl;
		_sSock = INVALID_SOCKET;
	}
	// 创建Socket
	_sSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: create new socket failed ..." << std::endl;
		return -1;
	}
	std::cout << "create new socket succeed ..." << std::endl;
	return 0;
}

int TcpServer::Bind(const char* ip, unsigned short port)
{
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "Invalid socket can not bind ..." << std::endl;
		return -1;
	}
	sockaddr_in saddr;
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

int TcpServer::Accept()
{
	if (_sSock == INVALID_SOCKET)
	{
		std::cout << "Invalid socket can not accept ..." << std::endl;
		return -1;
	}
	sockaddr_in caddr;
	int clen = sizeof(caddr);
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
	_sClients.emplace_back(cSock);
	return cSock;
}

int TcpServer::SendData(SOCKET cSock, DataHeader* header)
{
	if (isRun() && header)
	{
		send(cSock, (char*)header, header->_Length, 0);
		return 0;
	}
	return -1;
}

void TcpServer::SendToAll(DataHeader* header)
{
	if (isRun() && header) {
		for (SOCKET client : _sClients)
		{
			SendData(client, header);
		}
	}
}

int TcpServer::RecvData(SOCKET cSock)
{
	// 缓冲区
	char szRecv[4096];
	int len = recv(cSock, szRecv, sizeof(DataHeader), 0);
	if (len <= 0)
	{
		std::cout << "<socket=" << cSock << "> client out connect ..." << std::endl;
		return -1;
	}
	// 解析数据
	DataHeader* header = (DataHeader*)szRecv;
	std::cout << "<socket=" << cSock << "> info length is " << header->_Length << std::endl;
	recv(cSock, szRecv + sizeof(DataHeader), header->_Length - sizeof(DataHeader), 0);
	ParseData(cSock, header);
	return 0;
}

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

void TcpServer::Close()
{
	if (_sSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_sSock);
		WSACleanup();
#else
		close(_sSock);
#endif
		_sSock = INVALID_SOCKET;
	}
}

bool TcpServer::isRun()
{
	return _sSock != INVALID_SOCKET;
}

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

		for (SOCKET client : _sClients)
		{
			FD_SET(client, &fdRead);
		}

		if (0 > select(_sSock + 1, &fdRead, &fdWrite, &fdExp, &tval))
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
		for (int i = 0; i < fdRead.fd_count; ++i) {
			auto client = fdRead.fd_array[i];
			if (-1 == RecvData(client))
			{
				auto it = find(_sClients.begin(), _sClients.end(), client);
				if (it != _sClients.end())
				{
					_sClients.erase(it);
				}
			}
		}
		return true;
	}
	return false;
}
