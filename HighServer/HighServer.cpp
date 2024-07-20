#include "HighServer.h"

/**
*  ----- �û���Ϣ�� ����ʵ�� -----
*/
ClientData::ClientData(SOCKET sock)
{
	_cSock = sock;
	memset(_MsgBuf, 0, RECV_BUFF_SIZE);
	_MsgBuf_len = 0;
	memset(_SendBuf, 0, SEND_BUFF_SIZE);
	_SendBuf_len = 0;
}

SOCKET ClientData::getSocket() const
{
	return _cSock;
}

char* ClientData::getMsgBuf()
{
	return _MsgBuf;
}

int ClientData::getMsglen() const
{
	return _MsgBuf_len;
}

void ClientData::setMsglen(int mlen)
{
	_MsgBuf_len = mlen;
}

char* ClientData::getSendBuf()
{
	return _SendBuf;
}

int ClientData::getSendlen() const
{
	return _SendBuf_len;
}

void ClientData::setSendlen(int slen)
{
	_SendBuf_len = slen;
}

int ClientData::SendData(DataHeader* header)
{
	int ret = SOCKET_ERROR;
	if (header)
	{
		// Ҫ���͵ĳ���
		int slen = header->_Length;
		// Ҫ���͵�����
		const char* sData = (const char*)header;

		// ��������
		if (_SendBuf_len + slen >= SEND_BUFF_SIZE)
		{
			while (_SendBuf_len + slen >= SEND_BUFF_SIZE)
			{
				// ���㻹���Կ��������ݳ���
				int copylen = SEND_BUFF_SIZE - _SendBuf_len;
				// ��������
				memcpy(_SendBuf + _SendBuf_len, sData, copylen);
				// ����ʣ������
				slen -= copylen;
				sData += copylen;
				// ��������
				ret = send(_cSock, _SendBuf, SEND_BUFF_SIZE, 0);
				//std::cout << "��������" << std::endl;
				_SendBuf_len = 0;
				// ���͹����ݸ��¼�ʱ��
				_ctimer.updateTime();
			}
		}

		// ��ʣ�����ݱ��浽������
		memcpy(_SendBuf + _SendBuf_len, sData, slen);
		_SendBuf_len += slen;

		// ��ʱ����
		auto tim = _ctimer.getElapseSecond();
		if (tim >= 2.0)
		{
			ret = send(_cSock, _SendBuf, _SendBuf_len, 0);
			//std::cout << "��ʱ����" << std::endl;
			_SendBuf_len = 0;
			_ctimer.updateTime();
		}
		
	}
	return ret;
}



/**
* ----- ���������� -----
*/
SendTaskToClient::SendTaskToClient(ClientData* pClient, DataHeader* header)
{
	_pClient = pClient;
	_pHeader = header;
}

void SendTaskToClient::doTask()
{
	_pClient->SendData(_pHeader);
	delete _pHeader;
}



/**
* ----- �̷߳������� ����ʵ�� -----
*/
cellServer::cellServer(SOCKET sock)
{
	_sockfd = sock;
	_pEvent = nullptr;
}

cellServer::~cellServer()
{
	Close();
}

void cellServer::SendToAll(DataHeader* header)
{
	if (isRun() && header)
	{
		for (const auto& client : _Clients)
		{
			client.second->SendData(header);
		}
	}
}

int cellServer::RecvData(ClientData* pClient)
{
	_pEvent->RecvEvent(pClient);
	// �������ݵ���Ϣ������
	char* _recvBuf = pClient->getMsgBuf() + pClient->getMsglen();
	int recvlen = recv(pClient->getSocket(), _recvBuf, RECV_BUFF_SIZE - pClient->getMsglen(), 0);
	if (recvlen <= 0)
	{
		//std::cout << "client off connect ..." << std::endl;
		return -1;
	}
	// ���»���������
	pClient->setMsglen(pClient->getMsglen() + recvlen);
	// �жϻ����������Ƿ���������Ϣͷ
	while (pClient->getMsglen() >= sizeof(DataHeader))
	{
		// ��ȡ��Ϣ�峤��
		DataHeader* header = (DataHeader*)pClient->getMsgBuf();
		if (pClient->getMsglen() >= header->_Length)
		{
			// ����ʣ����Ϣ����
			int pos = pClient->getMsglen() - header->_Length;
			// ���ò����¼�
			_pEvent->RequestEvent(this, pClient, header);
			// ɾ�����������Ϣ
			memcpy(pClient->getMsgBuf(), pClient->getMsgBuf() + header->_Length, pos);
			// �޸Ŀͻ�����Ϣ�����������ݳ���
			pClient->setMsglen(pos);
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
	// fdRead���� �� �ı��־�����ڼ���ѭ��FD_SET������
	fd_set _fdRead_buf = {};
	bool _client_change = false;
	int maxfd = 0;
	// �̷߳�����������������
	while (isRun())
	{
		// ������Ŀͻ�����ӵ�������, ��ʾ�пͻ����޸�
		if (!_ClientAddBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (const auto& addclient : _ClientAddBuf)
			{
				_Clients.emplace(addclient->getSocket(), addclient);
			}
			_ClientAddBuf.clear();
			_client_change = true;
		}

		// Ϊ�վ��´�ѭ������Ȼselect�󶨿ջ᷵�ش���
		if (_Clients.empty())
		{
			continue;
		}

		// ֻ����ɶ�����
		fd_set fdRead = {};
		FD_ZERO(&fdRead);
		timeval tval = { 1, 0 };

		// ������еĿͻ��ˣ�����пͻ��˸��£������°󶨣�����ֱ�ӿ���
		if (_client_change) 
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (const auto& client : _Clients)
			{
				FD_SET(client.first, &fdRead);
				maxfd = max(maxfd, (int)client.first);
			}
			memcpy(&_fdRead_buf, &fdRead, sizeof(fd_set));
			_client_change = false;
		}
		else
		{
			memcpy(&fdRead, &_fdRead_buf, sizeof(fd_set));
		}

		// select ��ȡ����
		if (0 > select(maxfd + 1, &fdRead, NULL, NULL, &tval))
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return;
		}

		// ��ƽ̨����windowƽ̨��fd_count �� fd_array���ܼ��ٲ�ѯ����
#ifdef _WIN32
		for (int n = fdRead.fd_count - 1; n >= 0; --n)
		{
			auto it = _Clients.find(fdRead.fd_array[n]);
			if (it != _Clients.end())
			{
				if (RecvData(it->second) == -1)
				{
					// client�Ķ���ʶ
					_client_change = true;
					// client�뿪�����¼�
					_pEvent->LeaveEvent(it->second);
					// ���̷߳�������ɾ����Ӧ�Ŀͻ���
					delete it->second;
					_Clients.erase(it->first);
				}
			}
		}
#else
		for (auto it = _Clients.begin(); it != _Clients.end();)
		{
			if (FD_SET(it->first, &fdRead))
			{
				FD_CLR(it->first, &fdRead);
				if (RecvData(it->second) == -1)
				{
					// client�Ķ���ʶ
					_client_change = true;
					// client�뿪�����¼�
					_pEvent->leaveEvent(it->second);
					// ���̷߳�������ɾ����Ӧ�Ŀͻ���
					delete it->second;
					_Clients.erase(it);
				}
			}
			else
			{
				++it
			}
		}
#endif // _WIN32
	}
}

void cellServer::Close()
{
	// ��տͻ��˻������
	for (int n = (int)_ClientAddBuf.size() - 1; n >= 0; --n)
	{
		delete _ClientAddBuf[n];
		_ClientAddBuf.pop_back();
	}
	// ��տͻ��˶���
	for (auto it = _Clients.begin(); it != _Clients.end();)
	{
		delete it->second;
		_Clients.erase(it);
	}
	// ���õ�ǰ�̷߳�������socket, �����رգ��������������ر�
	_sockfd = INVALID_SOCKET;
}

void cellServer::addClient(ClientData* pClient)
{
	// ���¼���Ŀͻ��˷��뻺���У�����Ƶ�������пͻ�����������޸ģ������̲߳���ȫ
	std::lock_guard<std::mutex> autolock(_mutex);
	_ClientAddBuf.emplace_back(pClient);
}

int cellServer::getClientNum() const
{
	return int(_Clients.size() + _ClientAddBuf.size());
}

void cellServer::start()
{
	// �����̷߳���������Ҫthisָ����Ϊ�̺߳����Ĳ�����ָ������һ����ĳ�Ա����
	_pthread = std::thread(&cellServer::mainRun, this);
	_pthread.detach();
	// �����������������
	_TaskServer.start();
}

void cellServer::setEvent(TaskEvent* event)
{
	// ���ò����¼���ʵ��������
	_pEvent = event;
}

void cellServer::addSendTask(ClientData* pClient, DataHeader* header)
{
	// Ϊ��������������������
	SendTaskToClient* task = new SendTaskToClient(pClient, header);
	_TaskServer.addTask(task);
}



/**
*  ----- ���������� ����ʵ�� -----
*/
TcpServer::TcpServer()
{
	_sockfd = INVALID_SOCKET;
	_clientNum = 0;
	_recvNum = 0;
	_MsgNum = 0;
}

TcpServer::~TcpServer()
{
	Close();
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
	
	// �ڶ��ϴ����µĿͻ�����Ϣ
	ClientData* pClient = new ClientData(cSock);
	// �����û���ӵ��������ٵ��̷߳�����
	addClient(pClient);
	// �������û������¼�
	this->JoinEvent(pClient);

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
		fd_set fdRead = {};
		fd_set fdWrite = {};
		fd_set fdExp = {};
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
		
		// ��ӡ��ʾ��ǰ�������Ĵ�������
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
		// �����̷߳�����
		auto server = new cellServer(_sockfd);
		_cellServers.emplace_back(server);
		// ���̷߳������Ĳ����¼���������Ϊ��������
		server->setEvent(this);
		// �����̷߳�����
		server->start();
	}
}

void TcpServer::addClient(ClientData *pClient)
{
	// ��ѯ�ͻ������ٵķ�������Ϊ����ӿͻ���
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
	// ��ʱ��ӡ�������Ļ�����Ϣ
	auto tim = _timer.getElapseSecond();
	if (tim >= 1.0)
	{
		std::cout << "serverNum<" << _cellServers.size() << ">, clockTime<" << std::setiosflags(std::ios::fixed) << std::setprecision(6) << tim \
			<< ">, clientNum<" << _clientNum << ">, RecvNum<" << int(_recvNum/tim) << ">, DataSpeed<" << int(_MsgNum / tim) << ">" << std::endl;

		_timer.updateTime();
		_recvNum = 0;
		_MsgNum = 0;
	}
}

// �ͻ��˼�������¼�������ͨ���̳������������Զ���
void TcpServer::JoinEvent(ClientData* pClient)
{
	_clientNum++;
	//std::cout << "client <socket=" << pClient->getSocket() << "> join ..." << std::endl;
}

// �ͻ����뿪�����¼�������ͨ���̳������������Զ���
void TcpServer::LeaveEvent(ClientData* pClient)
{

	_clientNum--;
	//std::cout << "client <socket=" << pClient->getSocket() << "> leave ..." << std::endl;
}

// ���տͻ�����Ϣ�����¼�������ͨ���̳������������Զ���
void TcpServer::RecvEvent(ClientData* pClient)
{
	_recvNum++;
	//std::cout << "recv <socket=" << pClient->getSocket() << "> message ..." << std::endl;
}

// �����ͻ�����Ϣ�����¼�������ͨ���̳������������Զ���
void TcpServer::RequestEvent(cellServer* pServer, ClientData* pClient, DataHeader* header)
{
	_MsgNum++;
	//std::cout << "parse <socket=" << pClient->getSocket() << "> message ..." << std::endl;
}
