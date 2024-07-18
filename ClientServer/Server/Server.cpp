#include "Server.h"

/**
*  ----- 用户信息类 函数实现 -----
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
* ----- 线程服务器类 函数实现 -----
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
	// 接收数据到缓冲区
	int recvlen = recv(pClient->getSocket(), _recvBuf, RECV_BUFF_SIZE, 0);
	if (recvlen <= 0)
	{
		//std::cout << "client off connect ..." << std::endl;
		return -1;
	}
	// 将缓冲区数据拷贝到客户端消息缓冲区
	memcpy(pClient->getMsgBuf() + pClient->getlastPos(), _recvBuf, recvlen);
	pClient->setlastPos(pClient->getlastPos() + recvlen);
	// 判断缓冲区数据是否是完整消息头
	while (pClient->getlastPos() >= sizeof(DataHeader))
	{
		// 获取消息体长度
		DataHeader* header = (DataHeader*)pClient->getMsgBuf();
		if (pClient->getlastPos() >= header->_Length)
		{
			// 保存剩余消息长度
			int pos = pClient->getlastPos() - header->_Length;
			// 调用操作事件
			_pEvent->RequestEvent(pClient, header);
			// 删除处理完的消息
			memcpy(pClient->getMsgBuf(), pClient->getMsgBuf() + header->_Length, pos);
			// 修改客户端消息缓冲区中数据长度
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
	// 线程服务器持续处理任务
	while (isRun())
	{
		// 将缓存的客户端添加到运行中
		if (!_ClientAddBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (int n = (int)_ClientAddBuf.size() - 1; n >= 0; --n)
			{
				_Clients.emplace_back(_ClientAddBuf[n]);
			}
			_ClientAddBuf.clear();
		}

		// 为空就下次循环，不然select绑定空会返回错误
		if (_Clients.empty())
		{
			continue;
		}

		// 只处理可读请求
		fd_set fdRead;
		FD_ZERO(&fdRead);
		timeval tval = { 1, 0 };

		// 监测运行的客户端
		int maxfd = 0;
		for (int n = (int)_Clients.size() - 1; n >= 0; --n)
		{
			FD_SET(_Clients[n]->getSocket(), &fdRead);
			maxfd = max(maxfd, (int)_Clients[n]->getSocket());
		}

		// select 读取请求
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
	// 清空客户端队列
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
*  ----- 主服务器类 函数实现 -----
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
	// 开启Windows平台socket环境
#ifdef _WIN32
	WORD wr = MAKEWORD(2, 2);
	WSADATA dat = {};
	if (WSAStartup(wr, &dat) == SOCKET_ERROR)
	{
		return -1;
	}
#endif
	// 判断是否有旧连接
	if (_sockfd != INVALID_SOCKET)
	{
		// 关闭旧连接
#ifdef _WIN32
		closesocket(_sockfd);
#else
		close(_sockfd);
#endif
		_sockfd = INVALID_SOCKET;
	}
	// 创建 socket 文件描述符
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// 判断创建是否成功
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
	// 判断 socket 是否有效
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: invalid socket can not bind port ..." << std::endl;
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
	// 判断 socket 是否有效
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: invalid socket can not listen port ..." << std::endl;
		return -1;
	}
	// 设置监听端口
	if (SOCKET_ERROR == listen(_sockfd, pnum)) {
		std::cout << "ERROR: listen port error ..." << std::endl;
		return -1;
	}
	std::cout << "listen port succeed ,,," << std::endl;

	return 0;
}

int TcpServer::Accept()
{
	// 判断 socket 是否有效
	if (_sockfd == INVALID_SOCKET)
	{
		std::cout << "ERROR: invalid socket can not accept connect ..." << std::endl;
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
	cSock = accept(_sockfd, (struct sockaddr*)&caddr, &clen);
	if (cSock == INVALID_SOCKET)
	{
		std::cout << "ERROR: connect client error ..." << std::endl;
		return -1;
	}
	// 解析用户消息
	char ip[32];
	inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
	//std::cout << "New client <socket=" << cSock << "> connect succeed, ip = " << ip << ", port = " << caddr.sin_port << std::endl;
	
	// 将新用户添加到连接最少的线程服务器
	addClient(new ClientData(cSock));
	// 触发新用户加入事件
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
		// 创建select变量
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		// 清空读，写，异常缓冲区
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		// 绑定监听服务器socket
		FD_SET(_sockfd, &fdRead);
		FD_SET(_sockfd, &fdWrite);
		FD_SET(_sockfd, &fdExp);
		// select最大等待时间(秒，微秒)； 缓冲区有事件则立刻向下执行
		timeval tval = { 1, 0 };

		// select 模型 检测是否有事件
		int ret = select((int)_sockfd + 1, &fdRead, &fdWrite, &fdExp, &tval);
		if (ret < 0)
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return;
		}

		// 判断是否有连接请求
		if (FD_ISSET(_sockfd, &fdRead))
		{
			// 处理请求
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
	// 清空线程服务器数组
	for (int n = (int)_cellServers.size(); n >= 0; --n)
	{
		delete _cellServers[n];
		_cellServers.pop_back();
	}
	// 关闭服务器socket
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


