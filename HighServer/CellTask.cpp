#include "CellTask.h"

/**
* ----- 发送服务器类 -----
*/
void CellTaskServer::addTask(CellTask* task)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	_taskBuf.push_back(task);
}

void CellTaskServer::start()
{
	std::thread t(&CellTaskServer::mainRun, this);
	t.detach();
}

void CellTaskServer::mainRun()
{
	while (true)
	{
		// 将缓冲区数据 拷贝到 待处理数据中
		if (!_taskBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& task : _taskBuf)
			{
				_tasks.push_back(task);
			}
			_taskBuf.clear();
		}

		// 如果没任务，等待
		if (_tasks.empty())
		{
			continue;
		}

		// 如果有任务，处理
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& task : _tasks)
			{
				task->doTask();
				delete task;
			}
			_tasks.clear();
		}
	}
}
