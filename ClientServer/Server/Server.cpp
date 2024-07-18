#include "Server.h"

/**
*  ----- �û���Ϣ�� ����ʵ�� -----
*/
ClientData::ClientData(SOCKET sock)
{
	_cSock = sock;
	memset(_MsgBuf, 0, RECV_BUFF_SIZE * 10);
	_lastPos = 0;
}

SOCKET ClientData::getSocket() const
{
	return _cSock;
}

char* ClientData::getMsgBuf()
{
	return _MsgBuf;
}

int ClientData::getlastPos() const
{
	return _lastPos;
}

void ClientData::setlastPos(int pos)
{
	_lastPos = pos;
}

void ClientData::SendData(DataHeader* header)
{
	if (header)
	{
		send(_cSock, (char*)header, header->_Length, 0);
	}
}



/**
* ----- �̷߳������� ����ʵ�� -----
*/
cellServer::cellServer(SOCKET sock)
{
	_sockfd = sock;
	_pEvent = nullptr;
	memset(_recvBuf, 0, RECV_BUFF_SIZE);
}

cellServer::~cellServer()
{
	Close();
}

void cellServer::SendToAll(DataHeader* header)
{
	if (isRun() && header)
	{
		for (int n = (int)_Clients.size() - 1; n >= 0; --n)
		{
			_Clients[n]->SendData(header);
		}
	}
}

int cellServer::RecvData(ClientData* pClient)
{
	// �������ݵ�������
	int recvlen = recv(pClient->getSocket(), _recvBuf, RECV_BUFF_SIZE, 0);
	if (recvlen <= 0)
	{
		//std::cout << "client off connect ..." << std::endl;
		return -1;
	}
	// �����������ݿ������ͻ�����Ϣ������
	memcpy(pClient->getMsgBuf() + pClient->getlastPos(), _recvBuf, recvlen);
	pClient->setlastPos(pClient->getlastPos() + recvlen);
	// �жϻ����������Ƿ���������Ϣͷ
	while (pClient->getlastPos() >= sizeof(DataHeader))
	{
		// ��ȡ��Ϣ�峤��
		DataHeader* header = (DataHeader*)pClient->getMsgBuf();
		if (pClient->getlastPos() >= header->_Length)
		{
			// ����ʣ����Ϣ����
			int pos = pClient->getlastPos() - header->_Length;
			// ���ò����¼�
			_pEvent->RequestEvent(pClient, header);
			// ɾ�����������Ϣ
			memcpy(pClient->getMsgBuf(), pClient->getMsgBuf() + header->_Length, pos);
			// �޸Ŀͻ�����Ϣ�����������ݳ���
			pClient->setlastPos(pos);
		}
		else {
			break;
		}
	}
	return 0;
}

bool cellServer::isRun() const
{
	return _sockfd != INVALID_SOCKET;
}

void cellServer::mainRun()
{
	// �̷߳�����������������
	while (isRun())
	{
		// ������Ŀͻ�����ӵ�������
		if (!_ClientAddBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (int n = (int)_ClientAddBuf.size() - 1; n >= 0; --n)
			{
				_Clients.emplace_back(_ClientAddBuf[n]);
			}
			_ClientAddBuf.clear();
		}

		// Ϊ�վ��´�ѭ������Ȼselect�󶨿ջ᷵�ش���
		if (_Clients.empty())
		{
			continue;
		}

		// ֻ����ɶ�����
		fd_set fdRead;
		FD_ZERO(&fdRead);
		timeval tval = { 1, 0 };

		// ������еĿͻ���
		int maxfd = 0;
		for (int n = (int)_Clients.size() - 1; n >= 0; --n)
		{
			FD_SET(_Clients[n]->getSocket(), &fdRead);
			maxfd = max(maxfd, (int)_Clients[n]->getSocket());
		}

		// select ��ȡ����
		int ret = select(maxfd + 1, &fdRead, NULL, NULL, &tval);
		if (ret < 0)
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return;
		}
		
		for (int n = (int)_Clients.size() - 1; n >= 0; --n)
		{
			if (FD_ISSET(_Clients[n]->getSocket(), &fdRead))
			{
				if (RecvData(_Clients[n]) == -1)
				{
					auto it = _Clients.begin() + n;
					if (it != _Clients.end())
					{
						_pEvent->LeaveEvent();
						delete _Clients[n];
						_Clients.erase(it);
					}
				}
			}
		}
	}
}

void cellServer::Close()
{
	// ��տͻ��˶���
	for (int n = (int)_ClientAddBuf.size() - 1; n >= 0; --n)
	{
		delete _ClientAddBuf[n];
		_ClientAddBuf.pop_back();
	}
	for (int n = (int)_Clients.size() - 1; n >= 0; --n)
	{
		delete _Clients[n];
		_Clients.pop_back();
	}
	_sockfd = INVALID_SOCKET;
}

void cellServer::addClient(ClientData* pClient)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	_ClientAddBuf.emplace_back(pClient);
}

int cellServer::getClientNum() const
{
	return int(_Clients.size() + _ClientAddBuf.size());
}

void cellServer::start()
{
	_pthread = std::thread(&cellServer::mainRun, this);
	_pthread.detach();
}

void cellServer::setEvent(TaskEvent* event)
{
	_pEvent = event;
}



/**
*  ----- ���������� ����ʵ�� -----
*/
TcpServer::TcpServer()
{
	_sockfd = INVALID_SOCKET;
}

TcpServer::~TcpServer()
{
	_sockfd = INVALID_SOCKET;
}

int TcpServer::InitSock()
{
	// ����Windowsƽ̨socket����
#ifdef _WIN32
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat = {};
	if (WSAStartup(wr, &dat) == SOCKET_ERROR)
	{
		return -1;
	}
#endif
	// �ж��Ƿ��о�����
	if (_sockfd != INVALID_SOCKET)
	{
		// �رվ�����
#ifdef _WIN32
		closesocket(_sockfd);
#else
		close(_sockfd);
#endif
		_sockfd = INVALID_SOCKET;
	}
	// ���� socket �ļ�������
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// �жϴ����Ƿ�ɹ�
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: create new socket error ..." << std::endl;
		return -1;
	}
	std::cout << "create new socket succeed ..." << std::endl;

	return 0;
}

int TcpServer::Bind(const char *ip, unsigned short port)
{
	// �ж� socket �Ƿ���Ч
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: invalid socket can not bind port ..." << std::endl;
		return -1;
	}
	// ������ַ�Ͷ˿ڽṹ��
	sockaddr_in saddr = {};
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	// �Ƿ�ʹ��Ĭ��ip
	if (ip) 
	{
		// ����IP��ַ
		inet_pton(AF_INET, ip, &saddr.sin_addr);
	}
	else
	{
		// Ĭ��IP��ַ
#ifdef _WIN32
		saddr.sin_addr.S_un.S_addr = INADDR_ANY;
#else 
		saddr.sin_addr.s_addr = INADDR_ANY;
#endif
	}
	// �󶨶˿ں͵�ַ
	if (SOCKET_ERROR == bind(_sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
	{
		std::cout << "ERROR: bind port " << port << " error ..." << std::endl;
		return -1;
	}
	std::cout << "bind port " << port << " succeed ,,," << std::endl;

	return 0;
}

int TcpServer::Listen(unsigned int pnum)
{
	// �ж� socket �Ƿ���Ч
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: invalid socket can not listen port ..." << std::endl;
		return -1;
	}
	// ���ü����˿�
	if (SOCKET_ERROR == listen(_sockfd, pnum)) {
		std::cout << "ERROR: listen port error ..." << std::endl;
		return -1;
	}
	std::cout << "listen port succeed ,,," << std::endl;

	return 0;
}

int TcpServer::Accept()
{
	// �ж� socket �Ƿ���Ч
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: invalid socket can not accept connect ..." << std::endl;
		return -1;
	}
	// ���������û���Ϣ�Ľṹ��
	sockaddr_in caddr = {};
#ifdef _WIN32
	int clen = sizeof(caddr);
#else 
	addrlen_t clen = sizeof(caddr);
#endif
	// ��������
	SOCKET cSock = INVALID_SOCKET;
	cSock = accept(_sockfd, (struct sockaddr*)&caddr, &clen);
	if (cSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: connect client error ..." << std::endl;
		return -1;
	}
	// �����û���Ϣ
	char ip[32];
	inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
	//std::cout << "New client <socket=" << cSock << "> connect succeed, ip = " << ip << ", port = " << caddr.sin_port << std::endl;
	
	// �����û���ӵ��������ٵ��̷߳�����
	addClient(new ClientData(cSock));
	// �������û������¼�
	this->JoinEvent();

	return 0;
}

bool TcpServer::isRun() const
{
	return _sockfd != INVALID_SOCKET;
}

void TcpServer::mainRun()
{
	if (isRun())
	{
		// ����select����
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		// ��ն���д���쳣������
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		// �󶨼���������socket
		FD_SET(_sockfd, &fdRead);
		FD_SET(_sockfd, &fdWrite);
		FD_SET(_sockfd, &fdExp);
		// select���ȴ�ʱ��(�룬΢��)�� ���������¼�����������ִ��
		timeval tval = { 1, 0 };

		// select ģ�� ����Ƿ����¼�
		int ret = select((int)_sockfd + 1, &fdRead, &fdWrite, &fdExp, &tval);
		if (ret < 0)
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return;
		}

		// �ж��Ƿ�����������
		if (FD_ISSET(_sockfd, &fdRead))
		{
			// ��������
			FD_CLR(_sockfd, &fdRead);
			if (Accept() == -1)
			{
				return;
			}
		}
		
		DataSpeed();
	}
}

void TcpServer::Close()
{
	// ����̷߳���������
	for (int n = (int)_cellServers.size(); n >= 0; --n)
	{
		delete _cellServers[n];
		_cellServers.pop_back();
	}
	// �رշ�����socket
	if (_sockfd != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_sockfd);
		WSACleanup();
#else
		close(_sockfd);
#endif
		_sockfd = INVALID_SOCKET;
	}
}

void TcpServer::startThread(const int NUM_thread)
{
	for (int i = 0; i < NUM_thread; ++i)
	{
		auto server = new cellServer(_sockfd);
		_cellServers.emplace_back(server);
		server->setEvent(this);
		server->start();
	}
}

void TcpServer::addClient(ClientData *pClient)
{
	auto minServer = _cellServers[0];
	for (auto& server : _cellServers)
	{
		if (server->getClientNum() < minServer->getClientNum())
		{
			minServer = server;
		}
	}
	minServer->addClient(pClient);
}

void TcpServer::DataSpeed()
{
	auto tim = _timer.getElapseSecond();
	if (tim >= 1.0)
	{
		std::cout << "serverNum<" << _cellServers.size() << ">, clockTime<" << std::setiosflags(std::ios::fixed) << std::setprecision(6) << tim \
			<< ">, clientNum<" << _clientNum << ">, exitClientNum<" << _exitNum << ">, DataSpeed<" << int(_recvDataNum / tim) << " bit/s>" << std::endl;

		_timer.updateTime();
		_recvDataNum = 0;
		_exitNum = 0;
	}
}

void TcpServer::JoinEvent()
{
	_clientNum++;
}

void TcpServer::LeaveEvent()
{
	_clientNum--;
	_exitNum++;
}

void TcpServer::RequestEvent(ClientData* pClient, DataHeader* header)
{
	_recvDataNum++;
	switch (header->_Cmd)
	{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//std::cout << "Recv <socet=" << pClient->getSocket() << "> command = LOGIN, username = " << login->_Username << ", password = " << login->_Password << std::endl;
			LoginResult loginRet = {};
			char result[] = "login succeed";
			memcpy(loginRet._Result, result, sizeof(result));
			pClient->SendData(&loginRet);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			std::cout << "Recv <socet=" << pClient->getSocket() << "> command = LOGOUT, username = " << logout->_Username << std::endl;
			LogoutResult logoutRet = {};
			char result[] = "logout succeed";
			memcpy(logoutRet._Result, result, sizeof(result));
			pClient->SendData(&logoutRet);
			break;
		}
	}
	
}


