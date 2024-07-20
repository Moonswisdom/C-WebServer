#include "CellTask.h"

/**
* ----- ���ͷ������� -----
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
		// ������������ ������ ������������
		if (!_taskBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& task : _taskBuf)
			{
				_tasks.push_back(task);
			}
			_taskBuf.clear();
		}

		// ���û���񣬵ȴ�
		if (_tasks.empty())
		{
			continue;
		}

		// ��������񣬴���
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
