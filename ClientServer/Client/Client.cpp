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
	// windowsƽ̨��socket����
#ifdef _WIN32
	// ʹ��socket 2.x�汾
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat;
	int err = WSAStartup(wr, &dat);
	if (err != 0)
	{
		std::cout << "window start up socket environment failed, error num is " << err << std::endl;
		return -1;
	}
#endif
	// �ж��Ƿ��о�����
	if (_cSock != INVALID_SOCKET)
	{
		Close();
		std::cout << "off old connect ..." << std::endl;
	}
	// ����Socket
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
	// ���ӷ�����
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

// ����ճ�����ٰ����⣬ͨ�� ���ջ����� -> ��Ϣ������ ��������������ϲ������
// ���ջ���������������
char recvBuf[RECV_BUFF_SIZE] = {};
int TcpClient::RecvData()
{
	// ��������
	int recvlen = recv(_cSock, recvBuf, RECV_BUFF_SIZE, 0);
	if (recvlen <= 0)
	{
		std::cout << "server off connect ..." << std::endl << std::endl;
		return -1;
	}
	// �����յ�����ƴ�ӵ���Ϣ������
	memcpy(_MsgBuf + _lastPos, recvBuf, recvlen);
	_lastPos += recvlen;
	// �ж���Ϣ�����Ƿ���������������ͷ�� whileѭ���������ݣ���������ʱ������������Ϊ���߳�
	while (_lastPos >= sizeof(DataHeader)) {
		// ��Ϣת����ȡ�����峤��
		DataHeader* header = (DataHeader*)_MsgBuf;
		// �ж���Ϣ�����Ƿ�����������������
		if (_lastPos >= header->_Length) {
			// ��¼ʣ�೤��
			int pos = _lastPos - header->_Length;
			// ��������
			ParseData(header);
			// ����Ϣ������������ǰ�ƣ������Ѵ�������
			memcpy(_MsgBuf, _MsgBuf + header->_Length, pos); 
			// �޸�
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
	// ��������
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
		// ����selectģ��
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

