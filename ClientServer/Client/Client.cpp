#include "Client.h"

TcpClient::TcpClient()
{
	_cSock = INVALID_SOCKET;
	_isConnect = false;
	memset(_recvBuf, 0, RECV_BUFF_SIZE);
	memset(_MsgBuf, 0, RECV_BUFF_SIZE * 10);
	_lastPos = 0;
}

TcpClient::~TcpClient()
{
	Close();
}

int TcpClient::InitSock()
{
	// 开启windows平台socket环境
#ifdef _WIN32
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat = {};
	int wret = WSAStartup(wr, &dat);
	if (wret != 0) {
		std::cout << "ERROR: WSAStartup error ..." << std::endl;
		return -1;
	}
#endif
	// 判断是否有旧连接
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		_cSock = INVALID_SOCKET;
	}

	// 创建新socket
	_cSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_cSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: create new socket error ..." << std::endl;
		return -1;
	}
	//std::cout << "create new <socket=" << _cSock << "> succeed ..." << std::endl;

	return 0;
}

int TcpClient::Connect(const char* ip, unsigned int port)
{
	// 判断socket是否创建成功
	if (_cSock == INVALID_SOCKET)
	{
		InitSock();
	}
	// 创建地址端口变量
	sockaddr_in caddr = {};
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &caddr.sin_addr);
	// 请求连接
	if (SOCKET_ERROR == connect(_cSock, (struct sockaddr*)&caddr, sizeof(caddr))) {
		std::cout << "ERROR: <socket=" << _cSock << "> connect server error ..." << std::endl;
		return -1;
	}
	//std::cout << "<socket=" << _cSock << "> connect server succeed ..." << std::endl;
	_isConnect = true;
	return 0;
}

void TcpClient::SendData(DataHeader* header)
{
	if (isRun() && header) 
	{
		send(_cSock, (char*)header, header->_Length, 0);
	}
}

int TcpClient::RecvData()
{
	// 接收数据
	int recvlen = recv(_cSock, _recvBuf, RECV_BUFF_SIZE, 0);
	if (recvlen <= 0) 
	{
		std::cout << "server off connect ..." << std::endl;
		return -1;
	}
	// 组合拆分数据
	memcpy(_MsgBuf + _lastPos, _recvBuf, recvlen);
	_lastPos += recvlen;
	// 判断消息缓冲区长度是否够一条消息
	while (_lastPos >= sizeof(DataHeader)) 
	{
		DataHeader* header = (DataHeader*)_MsgBuf;
		if (_lastPos >= header->_Length)
		{
			// 对数据拆分并解析
			int pos = _lastPos - header->_Length;
			ParseData(header);
			memcpy(_MsgBuf, _MsgBuf + header->_Length, pos);
			_lastPos = pos;
		}
		else 
		{
			break;
		}
	}

	return 0;
}

void TcpClient::ParseData(DataHeader* header)
{
	switch (header->_Cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginRet = (LoginResult*)header;
			//std::cout << "Recv command = LOGIN_RESULT, result = " << loginRet->_Result << std::endl;
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutRet = (LogoutResult*)header;
			std::cout << "Recv command = LOGIN_RESULT, result = " << logoutRet->_Result << std::endl;
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
		_cSock = INVALID_SOCKET;
	}
	_isConnect = false;
}

bool TcpClient::isRun() const
{
	return _cSock != INVALID_SOCKET && _isConnect;
}

void TcpClient::mainRun()
{
	if (isRun())
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_cSock, &fdRead);
		timeval tval = { 1, 0 };

		int ret = select((int)_cSock + 1, &fdRead, NULL, NULL, &tval);
		if (ret < 0)
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return;
		}

		if (FD_ISSET(_cSock, &fdRead))
		{
			FD_CLR(_cSock, &fdRead);
			if (RecvData() == -1)
			{
				Close();
				return;
			}
		}
	}
}
