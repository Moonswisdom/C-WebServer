#include "TcpServer.h"

TcpServer::TcpServer()
{
	// 开启 socket 环境
	NetEnv::Init();
	// 启动日志器
	LogMgr::start();
	// 清空日志
	LogMgr::ClearAll();

	// 初始化
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
	// 判断是否有旧连接
	if (_Sock != INVALID_SOCKET)
	{
		// 关闭旧连接
#ifdef _WIN32
		closesocket(_Sock);
#else
		close(_Sock);
#endif
		_Sock = INVALID_SOCKET;
	}
	// 创建 socket 文件描述符
	_Sock = socket(AF_INET, SOCK_STREAM, 0);
	// 判断创建是否成功
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
	// 判断 socket 是否有效
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "invalid socket can not bind port."));
		return -1;
	}
	// 创建地址和端口结构体
	sockaddr_in saddr = {};
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	// 是否使用默认ip
	if (ip)
	{
		// 设置IP地址
		inet_pton(AF_INET, ip, &saddr.sin_addr);
	}
	else
	{
		// 默认IP地址
#ifdef _WIN32
		saddr.sin_addr.S_un.S_addr = INADDR_ANY;
#else 
		saddr.sin_addr.s_addr = INADDR_ANY;
#endif
	}
	// 绑定端口和地址
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
	// 判断 socket 是否有效
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "invalid socket can not listen port."));
		return -1;
	}
	// 设置监听端口
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
	// 判断 socket 是否有效
	if (_Sock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_ERROR, "invalid socket can not accept connect."));
		return -1;
	}
	// 创建接收用户信息的结构体
	sockaddr_in caddr = {};
#ifdef _WIN32
	int clen = sizeof(caddr);
#else 
	addrlen_t clen = sizeof(caddr);
#endif
	// 建立连接
	SOCKET cSock = INVALID_SOCKET;
	cSock = accept(_Sock, (struct sockaddr*)&caddr, &clen);
	if (cSock == INVALID_SOCKET)
	{
		LogMgr::addTask(new LogInfo(LEVEL_DEBUG, "connect client error."));
		return -1;
	}
	// 解析用户消息
	char ip[32];
	inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
	// 连接信息打印到日志
	std::string str = "New client <socket=" + std::to_string(cSock) + "> connect succeed, ip = " + ip + ", port = " + std::to_string(caddr.sin_port) + ".";
	LogMgr::addTask(new LogInfo(LEVEL_INFO, str.c_str()));

	// 在堆上创建新的客户端消息
	ClientPtr pClient(new ClientData(cSock));
	// 将新用户添加到连接最少的线程服务器
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
		// 创建select变量
		fd_set fdRead = {};
		fd_set fdWrite = {};
		fd_set fdExp = {};
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		timeval tval = { 0, 50000 };

		// 绑定主服务器socket
		FD_SET(_Sock, &fdRead);
		FD_SET(_Sock, &fdWrite);
		FD_SET(_Sock, &fdExp);

		// select模型监听
		if (0 > select((int)_Sock, &fdRead, &fdWrite, &fdExp, &tval))
		{
			LogMgr::addTask(new LogInfo(LEVEL_FATAL, "main server select error."));
			return;
		}
		// 判断是否有新连接
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

		// 基本信息打印
		printInfo();
	}
}

int TcpServer::Close()
{
	// 关闭线程服务器
	for (auto& tSvr : _tServers)
	{
		delete tSvr;
	}
	// 关闭socket;
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
		// 创建线程服务器
		_tServers.emplace_back(new tServer(_Sock));
		// 启动线程服务器
		_tServers[i]->start();
		// 设置代理对象
		_tServers[i]->setAgent(this);
	}
}

void TcpServer::addClient(ClientPtr pClient)
{
	// 寻找客户端最少的服务器
	tServer* msvr = _tServers.front();
	for (auto& svr : _tServers)
	{
		if (msvr->getClientNum() > svr->getClientNum())
		{
			msvr = svr;
		}
	}
	// 将新客户端加入最少的服务器
	msvr->addClient(pClient);
}

void TcpServer::printInfo()
{
	auto tim = _timer.getSecond();
	if (tim >= 1.0)
	{
		std::cout << "Server<sockfd=" << _Sock << ">, threadServer<num=" << _tServers.size() << ">，time<interval="
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
