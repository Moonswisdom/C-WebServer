#include "HighServer.h"

/**
*  ----- 用户信息类 函数实现 -----
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
		// 要发送的长度
		int slen = header->_Length;
		// 要发送的数据
		const char* sData = (const char*)header;

		// 定量发送
		if (_SendBuf_len + slen >= SEND_BUFF_SIZE)
		{
			while (_SendBuf_len + slen >= SEND_BUFF_SIZE)
			{
				// 计算还可以拷贝的数据长度
				int copylen = SEND_BUFF_SIZE - _SendBuf_len;
				// 拷贝数据
				memcpy(_SendBuf + _SendBuf_len, sData, copylen);
				// 更新剩余数据
				slen -= copylen;
				sData += copylen;
				// 发送数据
				ret = send(_cSock, _SendBuf, SEND_BUFF_SIZE, 0);
				//std::cout << "定量发送" << std::endl;
				_SendBuf_len = 0;
				// 发送过数据更新计时器
				_ctimer.updateTime();
			}
		}

		// 将剩余数据保存到缓冲区
		memcpy(_SendBuf + _SendBuf_len, sData, slen);
		_SendBuf_len += slen;

		// 定时发送
		auto tim = _ctimer.getElapseSecond();
		if (tim >= 2.0)
		{
			ret = send(_cSock, _SendBuf, _SendBuf_len, 0);
			//std::cout << "定时发送" << std::endl;
			_SendBuf_len = 0;
			_ctimer.updateTime();
		}
		
	}
	return ret;
}



/**
* ----- 发送任务类 -----
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
* ----- 线程服务器类 函数实现 -----
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
	// 接收数据到消息缓冲区
	char* _recvBuf = pClient->getMsgBuf() + pClient->getMsglen();
	int recvlen = recv(pClient->getSocket(), _recvBuf, RECV_BUFF_SIZE - pClient->getMsglen(), 0);
	if (recvlen <= 0)
	{
		//std::cout << "client off connect ..." << std::endl;
		return -1;
	}
	// 更新缓冲区长度
	pClient->setMsglen(pClient->getMsglen() + recvlen);
	// 判断缓冲区数据是否是完整消息头
	while (pClient->getMsglen() >= sizeof(DataHeader))
	{
		// 获取消息体长度
		DataHeader* header = (DataHeader*)pClient->getMsgBuf();
		if (pClient->getMsglen() >= header->_Length)
		{
			// 保存剩余消息长度
			int pos = pClient->getMsglen() - header->_Length;
			// 调用操作事件
			_pEvent->RequestEvent(this, pClient, header);
			// 删除处理完的消息
			memcpy(pClient->getMsgBuf(), pClient->getMsgBuf() + header->_Length, pos);
			// 修改客户端消息缓冲区中数据长度
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
	// fdRead缓存 和 改变标志，用于减少循环FD_SET的消耗
	fd_set _fdRead_buf = {};
	bool _client_change = false;
	int maxfd = 0;
	// 线程服务器持续处理任务
	while (isRun())
	{
		// 将缓存的客户端添加到运行中, 提示有客户端修改
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

		// 为空就下次循环，不然select绑定空会返回错误
		if (_Clients.empty())
		{
			continue;
		}

		// 只处理可读请求
		fd_set fdRead = {};
		FD_ZERO(&fdRead);
		timeval tval = { 1, 0 };

		// 监测运行的客户端，如果有客户端更新，则重新绑定，否则直接拷贝
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

		// select 读取请求
		if (0 > select(maxfd + 1, &fdRead, NULL, NULL, &tval))
		{
			std::cout << "ERROR: select error ..." << std::endl;
			return;
		}

		// 分平台处理，window平台有fd_count 和 fd_array，能减少查询速率
#ifdef _WIN32
		for (int n = fdRead.fd_count - 1; n >= 0; --n)
		{
			auto it = _Clients.find(fdRead.fd_array[n]);
			if (it != _Clients.end())
			{
				if (RecvData(it->second) == -1)
				{
					// client改动标识
					_client_change = true;
					// client离开触发事件
					_pEvent->LeaveEvent(it->second);
					// 在线程服务器中删除对应的客户端
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
					// client改动标识
					_client_change = true;
					// client离开触发事件
					_pEvent->leaveEvent(it->second);
					// 在线程服务器中删除对应的客户端
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
	// 清空客户端缓存队列
	for (int n = (int)_ClientAddBuf.size() - 1; n >= 0; --n)
	{
		delete _ClientAddBuf[n];
		_ClientAddBuf.pop_back();
	}
	// 清空客户端队列
	for (auto it = _Clients.begin(); it != _Clients.end();)
	{
		delete it->second;
		_Clients.erase(it);
	}
	// 重置当前线程服务器的socket, 但不关闭，由主服务器来关闭
	_sockfd = INVALID_SOCKET;
}

void cellServer::addClient(ClientData* pClient)
{
	// 将新加入的客户端放入缓存中，避免频繁对运行客户端数组进行修改，导致线程不安全
	std::lock_guard<std::mutex> autolock(_mutex);
	_ClientAddBuf.emplace_back(pClient);
}

int cellServer::getClientNum() const
{
	return int(_Clients.size() + _ClientAddBuf.size());
}

void cellServer::start()
{
	// 启动线程服务器，需要this指针作为线程函数的参数来指定是哪一个类的成员函数
	_pthread = std::thread(&cellServer::mainRun, this);
	_pthread.detach();
	// 启动发送任务服务器
	_TaskServer.start();
}

void cellServer::setEvent(TaskEvent* event)
{
	// 设置操作事件的实例化对象
	_pEvent = event;
}

void cellServer::addSendTask(ClientData* pClient, DataHeader* header)
{
	// 为发送任务服务器添加任务
	SendTaskToClient* task = new SendTaskToClient(pClient, header);
	_TaskServer.addTask(task);
}



/**
*  ----- 主服务器类 函数实现 -----
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
	
	// 在堆上创建新的客户端消息
	ClientData* pClient = new ClientData(cSock);
	// 将新用户添加到连接最少的线程服务器
	addClient(pClient);
	// 触发新用户加入事件
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
		// 创建select变量
		fd_set fdRead = {};
		fd_set fdWrite = {};
		fd_set fdExp = {};
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
		
		// 打印显示当前服务器的传输速率
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
		// 创建线程服务器
		auto server = new cellServer(_sockfd);
		_cellServers.emplace_back(server);
		// 将线程服务器的操作事件对象设置为主服务器
		server->setEvent(this);
		// 启动线程服务器
		server->start();
	}
}

void TcpServer::addClient(ClientData *pClient)
{
	// 查询客户端最少的服务器，为其添加客户端
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
	// 定时打印服务器的基本信息
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

// 客户端加入操作事件，可以通过继承主服务器来自定义
void TcpServer::JoinEvent(ClientData* pClient)
{
	_clientNum++;
	//std::cout << "client <socket=" << pClient->getSocket() << "> join ..." << std::endl;
}

// 客户端离开操作事件，可以通过继承主服务器来自定义
void TcpServer::LeaveEvent(ClientData* pClient)
{

	_clientNum--;
	//std::cout << "client <socket=" << pClient->getSocket() << "> leave ..." << std::endl;
}

// 接收客户端信息操作事件，可以通过继承主服务器来自定义
void TcpServer::RecvEvent(ClientData* pClient)
{
	_recvNum++;
	//std::cout << "recv <socket=" << pClient->getSocket() << "> message ..." << std::endl;
}

// 解析客户端消息操作事件，可以通过继承主服务器来自定义
void TcpServer::RequestEvent(cellServer* pServer, ClientData* pClient, DataHeader* header)
{
	_MsgNum++;
	//std::cout << "parse <socket=" << pClient->getSocket() << "> message ..." << std::endl;
}
