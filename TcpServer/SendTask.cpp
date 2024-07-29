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
	// 启动运行
	_isRun = true;
	while (!_isEnd)
	{
		if (!_sTaskBuf.empty())
		{
			// 将缓冲队列任务加入正式队列
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& task : _sTaskBuf)
			{
				_sTasks.emplace_back(task);
			}
			_sTaskBuf.clear();
		}
		
		if (!_sTasks.empty())
		{
			// 让当前线程休眠
			std::chrono::milliseconds ts(10);
			std::this_thread::sleep_for(ts);
			// 打印任务为空提示
			//LogMgr::addTask(new LogInfo(LEVEL_WARN, "send task list empty."));
		}

		// 处理正式队列中的任务
		for (auto& task : _sTasks)
		{
			task->dotask();
			// 发送缓冲区装不下任务，循环等待
			//while (0 == task())
			//{
			//	std::chrono::milliseconds tim(5);
			//	std::this_thread::sleep_for(tim);
			//}
		}
		_sTasks.clear();
	}
	// 结束运行
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
