#include "TcpServer.h"

TcpServer::TcpServer()
{
	// ���� socket ����
	NetEnv::Init();
	// ������־��
	LogMgr::start();
	// �����־
	LogMgr::ClearAll();

	// ��ʼ��
	_Sock = INVALID_SOCKET;
	_tServers.clear();
	_clientNum = 0;
	_recvNum = 0;
	_parseNum = 0;
}

TcpServer::~TcpServer()
{
	Close();
}

SOCKET TcpServer::InitSock()
{
	// �ж��Ƿ��о�����
	if (_Sock != INVALID_SOCKET)
	{
		// �رվ�����
#ifdef _WIN32
		closesocket(_Sock);
#else
		close(_Sock);
#endif
		_Sock = INVALID_SOCKET;
	}
	// ���� socket �ļ�������
	_Sock = socket(AF_INET, SOCK_STREAM, 0);
	// �жϴ����Ƿ�ɹ�
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "create new socket error."));
		return INVALID_SOCKET;
	}
	LogMgr::addTask(new LogInfo(LEVEL_INFO, "create new socket succeed."));
	return _Sock;
}

int TcpServer::Bind(const char* ip, unsigned short port)
{
	// �ж� socket �Ƿ���Ч
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "invalid socket can not bind port."));
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
	if (SOCKET_ERROR == bind(_Sock, (struct sockaddr*)&saddr, sizeof(saddr)))
	{
		std::string str = "bind port " + std::to_string(port) + " error.";
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, str.c_str()));
		return -1;
	}
	std::string str = "bind port " + std::to_string(port) + " successful.";
	LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));

	return 0;
}

int TcpServer::Listen(unsigned int pnum)
{
	// �ж� socket �Ƿ���Ч
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "invalid socket can not listen port."));
		return -1;
	}
	// ���ü����˿�
	if (SOCKET_ERROR == listen(_Sock, pnum)) 
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "listen port error."));
		return -1;
	}
	LogMgr::addTask(new LogInfo(LEVEL_INFO, "listen port successful."));

	return 0;
}

int TcpServer::Accept()
{
	// �ж� socket �Ƿ���Ч
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "invalid socket can not accept connect."));
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
	cSock = accept(_Sock, (struct sockaddr*)&caddr, &clen);
	if (cSock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_DEBUG, "connect client error."));
		return -1;
	}
	// �����û���Ϣ
	char ip[32];
	inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
	// ������Ϣ��ӡ����־
	std::string str = "New client <socket=" + std::to_string(cSock) + "> connect succeed, ip = " + ip + ", port = " + std::to_string(caddr.sin_port) + ".";
	LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));

	// �ڶ��ϴ����µĿͻ�����Ϣ
	ClientPtr pClient(new ClientData(cSock));
	// �����û���ӵ��������ٵ��̷߳�����
	addClient(pClient);

	return 0;
}

bool TcpServer::isRun()
{
	return _Sock != INVALID_SOCKET;
}

void TcpServer::mainRun()
{
	if (isRun())
	{
		// ����select����
		fd_set fdRead = {};
		fd_set fdWrite = {};
		fd_set fdExp = {};
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		timeval tval = { 0, 50000 };

		// ����������socket
		FD_SET(_Sock, &fdRead);
		FD_SET(_Sock, &fdWrite);
		FD_SET(_Sock, &fdExp);

		// selectģ�ͼ���
		if (0 > select((int)_Sock, &fdRead, &fdWrite, &fdExp, &tval))
		{
			LogMgr::addTask(new LogInfo(LEVEL_FATAL, "main server select error."));
			return;
		}
		// �ж��Ƿ���������
		if (FD_ISSET(_Sock, &fdRead))
		{
			FD_CLR(_Sock, &fdRead);
			if (Accept() == -1)
			{
				LogMgr::addTask(new LogInfo(LEVEL_DEBUG, "client connect error."));
				return;
			}
		}
		if (FD_ISSET(_Sock, &fdExp))
		{
			FD_CLR(_Sock, &fdExp);
			LogMgr::addTask(new LogInfo(LEVEL_FATAL, "exceptfd error."));
			return;
		}

		// ������Ϣ��ӡ
		printInfo();
	}
}

int TcpServer::Close()
{
	// �ر��̷߳�����
	for (auto& tSvr : _tServers)
	{
		delete tSvr;
	}
	// �ر�socket;
	if (_Sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_Sock);
#else
		close(_Sock);
#endif
		_Sock = INVALID_SOCKET;
	}
	return 0;
}

void TcpServer::start(int tnum)
{
	for (int i = 0; i < tnum; ++i)
	{
		// �����̷߳�����
		_tServers.emplace_back(new tServer(_Sock));
		// �����̷߳�����
		_tServers[i]->start();
		// ���ô������
		_tServers[i]->setAgent(this);
	}
}

void TcpServer::addClient(ClientPtr pClient)
{
	// Ѱ�ҿͻ������ٵķ�����
	tServer* msvr = _tServers.front();
	for (auto& svr : _tServers)
	{
		if (msvr->getClientNum() > svr->getClientNum())
		{
			msvr = svr;
		}
	}
	// ���¿ͻ��˼������ٵķ�����
	msvr->addClient(pClient);
}

void TcpServer::printInfo()
{
	auto tim = _timer.getSecond();
	if (tim >= 1.0)
	{
		std::cout << "Server<sockfd=" << _Sock << ">, threadServer<num=" << _tServers.size() << ">��time<interval="
			<< std::setiosflags(std::ios::fixed) << std::setprecision(6) << tim << ">, Client<num=" << _clientNum << 
			">, Recv<num=" << int(_recvNum / tim) << ">, Parse<Num=" << int(_parseNum / tim) << ">" << std::endl;
		_timer.updateTime();
		_recvNum = 0;
		_parseNum = 0;
	}
}

void TcpServer::joinEvent(ClientPtr pClient)
{
	++_clientNum;
	//std::cout << "client <socket=" << pClient->getSocket() << "> join ..." << std::endl;
}

void TcpServer::leaveEvent(ClientPtr pClient)
{
	--_clientNum;
	//std::cout << "client <socket=" << pClient->getSocket() << "> leave ..." << std::endl;
}

void TcpServer::recvEvent(ClientPtr pClient)
{
	++_recvNum;
	//std::cout << "recv <socket=" << pClient->getSocket() << "> message ..." << std::endl;
}

void TcpServer::parseEvent(tServer* tSvr, ClientPtr pClient, msg_header* header)
{
	++_parseNum;
	//std::cout << "parse <socket=" << pClient->getSocket() << "> message ..." << std::endl;
}
