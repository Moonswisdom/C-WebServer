#include "SendTask.h"

sTaskServer::sTaskServer()
{
	_sTasks.clear();
	_sTaskBuf.clear();
	_isRun = false;
	_isEnd = false;
}

sTaskServer::~sTaskServer()
{
	Close();
}

void sTaskServer::addTask(sTaskPtr task)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	_sTaskBuf.emplace_back(task);
}

void sTaskServer::mainRun()
{
	// ��������
	_isRun = true;
	while (!_isEnd)
	{
		if (!_sTaskBuf.empty())
		{
			// ������������������ʽ����
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& task : _sTaskBuf)
			{
				_sTasks.emplace_back(task);
			}
			_sTaskBuf.clear();
		}
		
		if (!_sTasks.empty())
		{
			// �õ�ǰ�߳�����
			std::chrono::milliseconds ts(10);
			std::this_thread::sleep_for(ts);
			// ��ӡ����Ϊ����ʾ
			//LogMgr::addTask(new LogInfo(LEVEL_WARN, "send task list empty."));
		}

		// ������ʽ�����е�����
		for (auto& task : _sTasks)
		{
			task->dotask();
			// ���ͻ�����װ��������ѭ���ȴ�
			//while (0 == task())
			//{
			//	std::chrono::milliseconds tim(5);
			//	std::this_thread::sleep_for(tim);
			//}
		}
		_sTasks.clear();
	}
	// ��������
	_isRun = false;
}

void sTaskServer::start()
{
	std::thread tT(&sTaskServer::mainRun, this);
	tT.detach();
}

void sTaskServer::Close()
{
	_isEnd = true;
	while (_isRun)
	{
		std::chrono::milliseconds tim(1);
		std::this_thread::sleep_for(tim);
	}
}

sTask::sTask(ClientPtr client, msg_header* header)
{
	_pClient = client;
	_header = header;
}

void sTask::dotask()
{
	_pClient->SendData(_header);
	delete _header;
}
