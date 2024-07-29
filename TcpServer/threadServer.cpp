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
	// ������������
	_sTaskSvr = new sTaskServer();
	_sTaskSvr->start();
}

void tServer::addClient(ClientPtr pClient)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	// ����ͻ��˻�����
	_tClientBuf.emplace_back(pClient);
	// ���ÿͻ��˼����¼�
	_Agent->joinEvent(pClient);
}

void tServer::addTask(ClientPtr pClient, msg_header* header)
{
	// ʹ��function����lambda�������Զ�������ͷ�
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
	// ��ʱ��
	HRTimer timer;
	// select �ɶ�����������
	fd_set fdRead_buf = {};
	fd_set fdWrite_buf = {};
	// client�Ƿ�ı��ʶ
	bool c_change = false;
	// ���sockfd
	int maxfd = 0;
	// ������ѭ������
	_run = true;
	while (isRun())
	{
		// �����ͻ��˻�����е���ʽ����
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

		// �ͻ��˶���û����ѭ���ȴ��������޿ͻ��˰�select�ᱨ��
		if (_tClients.empty())
		{
			std::chrono::milliseconds tim(5);
			std::this_thread::sleep_for(tim);
			continue;
		}

		// ����select����
		fd_set fdRead = {};
		//fd_set fdWrite = {};
		// fd_set fdExp = {};
		FD_ZERO(&fdRead);
		//FD_ZERO(&fdWrite);
		//FD_ZERO(&fdExp);
		timeval tval = { 0, 50000 };

		// �����пͻ��˰󶨵�select
		if (c_change)
		{
			// �пͻ��˱䶯�����°�
			for (auto& client : _tClients)
			{
				FD_SET(client.first, &fdRead);
				//FD_SET(client.first, &fdWrite);
				maxfd = max(maxfd, (int)client.first);
			}
			// �����µĿͻ��˿�����������
			memcpy(&fdRead_buf, &fdRead, sizeof(fd_set));
			//memcpy(&fdWrite_buf, &fdWrite, sizeof(fd_set));
			c_change = false;
		}
		else
		{
			// �޿ͻ��˱䶯��ֱ�ӿ���
			memcpy(&fdRead, &fdRead_buf, sizeof(fd_set));
			//memcpy(&fdWrite, &fdWrite_buf, sizeof(fd_set));
		}
		
		// select ģ�ͼ��
		if (0 > select(maxfd + 1, &fdRead, NULL, NULL, &tval))
		{
			LogMgr::addTask(new LogInfo(LEVEL_ERROR, "tServer select error."));
			return;
		}
		
		// �����ɶ��Ŀͻ�����Ϣ
		NetRequest(fdRead, c_change);
		// ��������ͷ���ʱ��
		time_t dt = (time_t)timer.getMilliSecond();
		DTcheck(dt, c_change);
		timer.updateTime();
	}
	// �˳�������
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
			// ����ͻ��˽�����Ϣ�쳣���Ƴ��ͻ���
			if (-1 == RecvData(it->second))
			{
				// ���ô����뿪�¼�
				_Agent->leaveEvent(it->second);
				// �пͻ��˱仯
				c_change = true;
				// �Ƴ���Ч�ͻ���
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
			// ����ͻ��˽�����Ϣ�쳣���Ƴ��ͻ���
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
		// ������������Լ��ʱ�䣬ɾ���ͻ���
		if (it->second->timeheart(dt))
		{
			_Agent->leaveEvent(it->second);
			c_change = true;
			_tClients.erase(it);
		}
		else
		{
			// ������������ⶨʱ����
			it->second->timesend(dt);
			++it;
		}
	}
}

int tServer::RecvData(ClientPtr pClient)
{
	// �������ݵ���Ӧ�ͻ���
	int ret = pClient->RecvData();
	if(ret < 0)
	{
		return -1;
	}
	// ���ô�������¼�
	_Agent->recvEvent(pClient);
	// ������������ݣ���������
	if (pClient->hasMsg())
	{
		ParseData(pClient, pClient->getMsg());
		pClient->delMsg();
	}
	return ret;
}

void tServer::ParseData(ClientPtr pClient, msg_header* header)
{
	// ʹ������ָ������thisָ�룺�̳�enable_shared_from_this<class>, Ȼ��shared_from_this()����
	// ���ô�������¼�
	_Agent->parseEvent(this, pClient, header);
}

void tServer::Close()
{
	// �ر���������
	delete _sTaskSvr;
	// �ر��̷߳�����
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
