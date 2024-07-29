#include "threadServer.h"

tServer::tServer(SOCKET sock)
{
	_tSock = sock;
	_tClients.clear();
	_tClients.clear();
	_Agent = nullptr;
	_run = false;
}

tServer::~tServer()
{
	Close();
}

void tServer::setAgent(AgentEvent* agent)
{
	_Agent = agent;
}

void tServer::start()
{
	std::thread t(&tServer::mainRun, this);
	t.detach();
	// 启动任务处理器
	_sTaskSvr = new sTaskServer();
	_sTaskSvr->start();
}

void tServer::addClient(ClientPtr pClient)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	// 加入客户端缓冲区
	_tClientBuf.emplace_back(pClient);
	// 调用客户端加入事件
	_Agent->joinEvent(pClient);
}

void tServer::addTask(ClientPtr pClient, msg_header* header)
{
	// 使用function保存lambda函数，自动管理和释放
	// typedef std::shared_ptr<sTask> sTaskPtr;
	sTaskPtr task(new sTask(pClient, header));
	//delete header;
	_sTaskSvr->addTask(task);
}

bool tServer::isRun()
{
	return _tSock != INVALID_SOCKET;
}

void tServer::mainRun()
{
	// 计时器
	HRTimer timer;
	// select 可读缓冲区备份
	fd_set fdRead_buf = {};
	fd_set fdWrite_buf = {};
	// client是否改变标识
	bool c_change = false;
	// 最大sockfd
	int maxfd = 0;
	// 进入主循环程序
	_run = true;
	while (isRun())
	{
		// 拷贝客户端缓冲队列到正式队列
		if (!_tClientBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& client : _tClientBuf)
			{
				_tClients.emplace(client->getSock(), client);
			}
			_tClientBuf.clear();
			c_change = true;
		}

		// 客户端队列没有则循环等待，否则无客户端绑定select会报错
		if (_tClients.empty())
		{
			std::chrono::milliseconds tim(5);
			std::this_thread::sleep_for(tim);
			continue;
		}

		// 创建select变量
		fd_set fdRead = {};
		//fd_set fdWrite = {};
		// fd_set fdExp = {};
		FD_ZERO(&fdRead);
		//FD_ZERO(&fdWrite);
		//FD_ZERO(&fdExp);
		timeval tval = { 0, 50000 };

		// 将所有客户端绑定到select
		if (c_change)
		{
			// 有客户端变动，重新绑定
			for (auto& client : _tClients)
			{
				FD_SET(client.first, &fdRead);
				//FD_SET(client.first, &fdWrite);
				maxfd = max(maxfd, (int)client.first);
			}
			// 将最新的客户端拷贝到缓冲区
			memcpy(&fdRead_buf, &fdRead, sizeof(fd_set));
			//memcpy(&fdWrite_buf, &fdWrite, sizeof(fd_set));
			c_change = false;
		}
		else
		{
			// 无客户端变动，直接拷贝
			memcpy(&fdRead, &fdRead_buf, sizeof(fd_set));
			//memcpy(&fdWrite, &fdWrite_buf, sizeof(fd_set));
		}
		
		// select 模型检测
		if (0 > select(maxfd + 1, &fdRead, NULL, NULL, &tval))
		{
			LogMgr::addTask(new LogInfo(LEVEL_ERROR, "tServer select error."));
			return;
		}
		
		// 解析可读的客户端信息
		NetRequest(fdRead, c_change);
		// 检测心跳和发送时间
		time_t dt = (time_t)timer.getMilliSecond();
		DTcheck(dt, c_change);
		timer.updateTime();
	}
	// 退出主程序
	_run = false;
}

void tServer::NetRequest(fd_set& fdRead, bool& c_change)
{
#ifdef _WIN32
	for (int n = fdRead.fd_count - 1; n >= 0; --n)
	{
		auto it = _tClients.find(fdRead.fd_array[n]);
		if (it != _tClients.end())
		{
			// 如果客户端接收消息异常，移除客户端
			if (-1 == RecvData(it->second))
			{
				// 调用代理离开事件
				_Agent->leaveEvent(it->second);
				// 有客户端变化
				c_change = true;
				// 移除无效客户端
				_tClients.erase(it);
			}
		}
	}
#else
	for (auto it = _tClients.begin(); it != _tClients.end();)
	{
		if (FD_ISSET(it->first, &fdRead))
		{
			FD_CLR(it->first, &fdRead);
			// 如果客户端接收消息异常，移除客户端
			if (-1 == RecvData(it->second))
			{
				_Agent->leaveEvent(it->second);
				c_change = true;
				_tClients.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
#endif // _WIN32
}

void tServer::DTcheck(time_t dt, bool& c_change)
{
	for (auto it = _tClients.begin(); it != _tClients.end();)
	{
		// 如果到心跳检测约定时间，删除客户端
		if (it->second->timeheart(dt))
		{
			_Agent->leaveEvent(it->second);
			c_change = true;
			_tClients.erase(it);
		}
		else
		{
			// 正常心跳，检测定时发送
			it->second->timesend(dt);
			++it;
		}
	}
}

int tServer::RecvData(ClientPtr pClient)
{
	// 接收数据到对应客户端
	int ret = pClient->RecvData();
	if(ret < 0)
	{
		return -1;
	}
	// 调用代理接收事件
	_Agent->recvEvent(pClient);
	// 如果有完整数据，解析数据
	if (pClient->hasMsg())
	{
		ParseData(pClient, pClient->getMsg());
		pClient->delMsg();
	}
	return ret;
}

void tServer::ParseData(ClientPtr pClient, msg_header* header)
{
	// 使用智能指针对象的this指针：继承enable_shared_from_this<class>, 然后shared_from_this()调用
	// 调用代理解析事件
	_Agent->parseEvent(this, pClient, header);
}

void tServer::Close()
{
	// 关闭任务处理器
	delete _sTaskSvr;
	// 关闭线程服务器
	_tSock = INVALID_SOCKET;
	while (_run)
	{
		std::chrono::milliseconds tim(1);
		std::this_thread::sleep_for(tim);
	}
}

int tServer::getClientNum()
{
	return (int)(_tClients.size() + _tClientBuf.size());
}
