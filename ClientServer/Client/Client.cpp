#include "Client.h"

TcpClient::TcpClient()
{
	_cSock = INVALID_SOCKET;
	memset(_MsgBuf, 0, RECV_BUFF_SIZE * 10);
	_lastPos = 0;
}

TcpClient::~TcpClient()
{
	Close();
}

int TcpClient::InitSocket()
{
	// windows平台打开socket环境
#ifdef _WIN32
	// 使用socket 2.x版本
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat;
	int err = WSAStartup(wr, &dat);
	if (err != 0)
	{
		std::cout << "window start up socket environment failed, error num is " << err << std::endl;
		return -1;
	}
#endif
	// 判断是否有旧连接
	if (_cSock != INVALID_SOCKET)
	{
		Close();
		std::cout << "off old connect ..." << std::endl;
	}
	// 创建Socket
	_cSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_cSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: create new socket failed ..." << std::endl;
		return -1;
	}
	//std::cout << "create new socket succeed ..." << std::endl;
	return 0;
}

int TcpClient::Connect(const char* ip, unsigned short port)
{
	if (_cSock == INVALID_SOCKET)
	{
		InitSocket();
	}
	// 连接服务器
	sockaddr_in caddr = {};
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &caddr.sin_addr);
	if (SOCKET_ERROR == connect(_cSock, (struct sockaddr*)&caddr, sizeof(caddr)))
	{
		std::cout << "ERROR: connect server failed ..." << std::endl;
		return -1;
	}
	std::cout << "socket<" << _cSock << "> connect server succeed ..." << std::endl;
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

// 处理粘包、少包问题，通过 接收缓冲区 -> 消息缓冲区 二级缓冲区来组合拆分数据
// 接收缓冲区：接收数据
char recvBuf[RECV_BUFF_SIZE] = {};
int TcpClient::RecvData()
{
	// 接收数据
	int recvlen = recv(_cSock, recvBuf, RECV_BUFF_SIZE, 0);
	if (recvlen <= 0)
	{
		std::cout << "server off connect ..." << std::endl << std::endl;
		return -1;
	}
	// 将接收的数据拼接到消息缓冲区
	memcpy(_MsgBuf + _lastPos, recvBuf, recvlen);
	_lastPos += recvlen;
	// 判断消息长度是否满足完整的数据头， while循环解析数据，数据量大时较慢，后续改为多线程
	while (_lastPos >= sizeof(DataHeader)) {
		// 消息转化获取数据体长度
		DataHeader* header = (DataHeader*)_MsgBuf;
		// 判断消息长度是否满足完整的数据体
		if (_lastPos >= header->_Length) {
			// 记录剩余长度
			int pos = _lastPos - header->_Length;
			// 解析数据
			ParseData(header);
			// 将消息缓冲区中内容前移，覆盖已处理数据
			memcpy(_MsgBuf, _MsgBuf + header->_Length, pos); 
			// 修改
			_lastPos = pos;
		}
		else {
			break;
		}
	}
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
			//std::cout << "Recv: cmd is LOGIN_RESULT, length is " << loginRet->_Length << std::endl;
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

bool TcpClient::isRun() const
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
		timeval tval = { 1, 0 };
		if (0 > select(_cSock + 1, &fdRead, 0, 0, &tval)) {
			std::cout << "ERROR: select error ..." << std::endl;
			return false;
		}
		if (FD_ISSET(_cSock, &fdRead))
		{
			FD_CLR(_cSock, &fdRead);
			if (RecvData() == -1)
			{
				_cSock = INVALID_SOCKET;
				return false;
			}
		}
		return true;
	}
	return false;
}

