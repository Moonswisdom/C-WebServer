#include "LogMgr.h"

LogTask::LogTask(tm tnow, Level level, const char* path, const char* data)
{
	_tnow = tnow;
	_level = level;
	memcpy(_path, path, strlen(path) + 1);
	memcpy(_data, data, strlen(data) + 1);
}

void LogTask::doTask()
{
	// 打开文件
	std::ofstream ofs;
	ofs.open(_path, std::ios::app);
	if (ofs.is_open())
	{
		// 打印时间
		ofs << "[" << _tnow.tm_year + 1900 << "-" << _tnow.tm_mon << "-" << _tnow.tm_wday << " "
			<< _tnow.tm_hour << ":" << _tnow.tm_min << ":" << _tnow.tm_sec << "]";
		// 打印内容
		ofs << _data << std::endl;
		ofs.close();
	}
}

Logger::Logger()
{
	_Tasks.clear();
	InitPath();
	_isRun = false;
	_isEnd = false;
}

Logger::~Logger()
{
	_isEnd = true;
	while (_isRun)
	{
		std::chrono::milliseconds tim(1);
		std::this_thread::sleep_for(tim);
	}
}

void Logger::InitPath(const char* pINFO, const char* pWARN, const char* pDEBUG, const char* pERROR, const char* pFATAL)
{
	memcpy(_path_INFO, pINFO, strlen(pINFO) + 1);
	memcpy(_path_WARN, pWARN, strlen(pWARN) + 1);
	memcpy(_path_DEBUG, pDEBUG, strlen(pDEBUG) + 1);
	memcpy(_path_ERROR, pERROR, strlen(pERROR) + 1);
	memcpy(_path_FATAL, pFATAL, strlen(pFATAL) + 1);
}

void Logger::ClearLevel(Level level)
{
	if (!_ofs.is_open())
	{
		switch (level)
		{
		case LEVEL_INFO:
		{
			_ofs.open(_path_INFO, std::ios::out);
			if (_ofs.is_open())
			{
				_ofs.close();
			}
			break;
		}
		case LEVEL_WARN:
		{
			_ofs.open(_path_WARN, std::ios::out);
			if (_ofs.is_open())
			{
				_ofs.close();
			}
			break;
		}
		case LEVEL_DEBUG:
		{
			_ofs.open(_path_DEBUG, std::ios::out);
			if (_ofs.is_open())
			{
				_ofs.close();
			}
			break;
		}
		case LEVEL_ERROR:
		{
			_ofs.open(_path_ERROR, std::ios::out);
			if (_ofs.is_open())
			{
				_ofs.close();
			}
			break;
		}
		case LEVEL_FATAL:
		{
			_ofs.open(_path_FATAL, std::ios::out);
			if (_ofs.is_open())
			{
				_ofs.close();
			}
			break;
		}
		}
	}
}

void Logger::ClearAllLevel()
{
	if (!_ofs.is_open())
	{
		_ofs.open(_path_INFO, std::ios::out);
		if (_ofs.is_open())
		{
			_ofs.close();
		}
		_ofs.open(_path_WARN, std::ios::out);
		if (_ofs.is_open())
		{
			_ofs.close();
		}
		_ofs.open(_path_DEBUG, std::ios::out);
		if (_ofs.is_open())
		{
			_ofs.close();
		}
		_ofs.open(_path_ERROR, std::ios::out);
		if (_ofs.is_open())
		{
			_ofs.close();
		}
		_ofs.open(_path_FATAL, std::ios::out);
		if (_ofs.is_open())
		{
			_ofs.close();
		}
	}
}

void Logger::addTask(Level level, const char* pStr)
{
	tm tnow = getNowtm();
	std::lock_guard<std::mutex> autolock(_mutex);
	switch (level)
	{
	case LEVEL_INFO:
	{
		_TaskBuf.emplace_back(new LogTask(tnow, level, _path_INFO, pStr));
		break;
	}
	case LEVEL_WARN:
	{
		_TaskBuf.emplace_back(new LogTask(tnow, level, _path_WARN, pStr));
		break;
	}
	case LEVEL_DEBUG:
	{
		_TaskBuf.emplace_back(new LogTask(tnow, level, _path_DEBUG, pStr));
		break;
	}
	case LEVEL_ERROR:
	{
		_TaskBuf.emplace_back(new LogTask(tnow, level, _path_ERROR, pStr));
		break;
	}
	case LEVEL_FATAL:
	{
		_TaskBuf.emplace_back(new LogTask(tnow, level, _path_FATAL, pStr));
		break;
	}
	}
}

void Logger::mainRun()
{
	while (_isRun)
	{
		if (!_TaskBuf.empty())
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			for (auto& task : _TaskBuf)
			{
				_Tasks.emplace_back(task);
			}
			_TaskBuf.clear();
		}

		if (_Tasks.empty())
		{
			std::chrono::milliseconds t(5);
			std::this_thread::sleep_for(t);
			continue;
		}

		for (auto& task : _Tasks)
		{
			task->doTask();
		}
		_Tasks.clear();
	}
}

void Logger::start()
{
	std::thread th(&Logger::mainRun, this);
	th.detach();
}

tm Logger::getNowtm()
{
	time_t time_sec = time(0);
	struct tm tnow;
#ifdef _WIN32
	localtime_s(&tnow, &time_sec);
#else
	localtime_r(&time_sec, &tnow);
#endif
	return tnow;
}

LogMgr::LogMgr()
{
	LoggerPtr logger(new Logger());
	_logger = logger;
}

void LogMgr::start()
{
	LogM()._logger->start();
}

void LogMgr::addTask(Level level, const char* pStr)
{
	LogM()._logger->addTask(level, pStr);
}

void LogMgr::ClearAll()
{
	LogM()._logger->ClearAllLevel();
}

void LogMgr::ClearLevel(Level level)
{
	LogM()._logger->ClearLevel(level);
}
