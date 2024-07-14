#include "Client.h"

TcpClient::TcpClient()
{
	_cSock = INVALID_SOCKET;
}

TcpClient::~TcpClient()
{
	Close();
}

void TcpClient::InitSocket()
{
	// 判断是否有旧连接
	if (_cSock != INVALID_SOCKET)
	{
		std::cout << "off old connect ..." << std::endl;
		_cSock = INVALID_SOCKET;
	}
	// windows平台打开socket环境
#ifdef _WIN32
	// 使用socket 2.x版本
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(wr, &dat);
#endif
	// 创建Socket
	_cSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_cSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: create new socket failed ..." << std::endl;
		return;
	}
	std::cout << "create new socket succeed ..." << std::endl;

}

int TcpClient::Connect(const char* ip, unsigned short port)
{
	if (_cSock == INVALID_SOCKET)
	{
		InitSocket();
	}
	// 连接服务器
	sockaddr_in caddr;
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &caddr.sin_addr);
	if (SOCKET_ERROR == connect(_cSock, (struct sockaddr*)&caddr, sizeof(caddr)))
	{
		std::cout << "ERROR: connect server failed ..." << std::endl;
		return -1;
	}
	std::cout << "connect server succeed ..." << std::endl;
	return 0;
}

int TcpClient::SendData(DataHeader* header)
{
	if (isRun() && header)
	{
		send(_cSock, (char*)header, header->_Length, 0);
		return 0;
	}
	return -1;
}

int TcpClient::RecvData()
{
	// 接收缓冲区
	char szRecv[4096] = {};
	int len = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	if (len < 0)
	{
		std::cout << "ERROR: recv server error ..." << std::endl;
		return -1;
	}
	DataHeader* header = (DataHeader*)szRecv;
	std::cout << "Message length is " << header->_Length << std::endl;
	recv(_cSock, szRecv + sizeof(DataHeader), header->_Length - sizeof(DataHeader), 0);
	ParseData(header);
	return 0;
}

void TcpClient::ParseData(DataHeader* header)
{
	// 解析数据
	switch (header->_Cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginRet = (LoginResult*)header;
			std::cout << "Recv: cmd is LOGIN_RESULT, length is " << loginRet->_Length << std::endl;
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutRet = (LogoutResult*)header;
			std::cout << "Recv: cmd is LOGOUT_RESULT, length is " << logoutRet->_Length << std::endl;
			break;
		}
		case CMD_NEW_USER:
		{
			NewUser* newuser = (NewUser*)header;
			std::cout << "Recv: New user join, socket is " << newuser->_Sock << std::endl;
			break;
		}
	}
}

void TcpClient::Close()
{
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
		WSACleanup();
#else
		close(_cSock);
#endif
	}
	_cSock = INVALID_SOCKET;
}

bool TcpClient::isRun()
{
	return _cSock != INVALID_SOCKET;
}

bool TcpClient::MainRun()
{
	if (isRun())
	{
		// 创建select模型
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_cSock, &fdRead);
		//timeval tval = { 1, 0 };
		if (0 > select(_cSock + 1, &fdRead, 0, 0, NULL)) {
			std::cout << "ERROR: select error ..." << std::endl;
			return false;
		}
		if (FD_ISSET(_cSock, &fdRead))
		{
			FD_CLR(_cSock, &fdRead);
			if (RecvData() == -1)
			{
				Close();
				return false;
			}
			
		}
		return true;
	}
	return false;
}

