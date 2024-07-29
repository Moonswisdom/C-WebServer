#pragma once
#include<fstream>
#include<list>
#include<time.h>
#include "Tcphead.h"

/**
* ----- 日志管理类 -----
*/
// 日志级别
enum Level {
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_DEBUG,
	LEVEL_ERROR,
	LEVEL_FATAL,
};

// 日志任务
class LogTask
{
public:
	LogTask(tm tnow, Level level, const char* path, const char* data);
	~LogTask() {}
public:
	// 执行任务
	void doTask();
private:
	// 日志时间
	tm _tnow;
	// 日志级别
	Level _level;
	// 日志地址
	char _path[LOG_PATH_LEN];
	// 日志内容
	char _data[256];
};

// 日志器
class Logger
{
public:
	Logger();
	~Logger();
public:
	// 初始化日志输出位置
	void InitPath(const char* pINFO = "../ClientLOG/LOG_INFO.txt", const char* pWARN = "../ClientLOG/LOG_WARN.txt",
		const char* pDEBUG = "../ClientLOG/LOG_DEBUG.txt", const char* pERROR = "../ClientLOG/LOG_ERROR.txt", const char* pFATAL = "../ClientLOG/LOG_FATAL.txt");
	// 清空对应级别的日志
	void ClearLevel(Level level);
	// 清空所有日志
	void ClearAllLevel();

	// 添加任务日志
	void addTask(Level level, const char* pStr);
	// 处理日志任务
	void mainRun();
	// 开启处理线程
	void start();

private:
	// 获取tm格式的当前时间
	tm getNowtm();

private:
	// 输出流
	std::ofstream _ofs;
	// 各类日志存储地址
	char _path_INFO[LOG_PATH_LEN];
	char _path_WARN[LOG_PATH_LEN];
	char _path_DEBUG[LOG_PATH_LEN];
	char _path_ERROR[LOG_PATH_LEN];
	char _path_FATAL[LOG_PATH_LEN];
	// 日志任务
	std::list<LogTaskptr> _Tasks;
	// 日志任务缓冲区
	std::list<LogTaskptr> _TaskBuf;
	// 日志器运行
	bool _isRun;
	// 日志器结束标识
	bool _isEnd;
	// 线程安全锁
	std::mutex _mutex;
};


// 日志管理器，可扩展多个日志器
class LogMgr
{
private:
	LogMgr();
	~LogMgr() {}
public:
	static LogMgr& LogM()
	{
		static LogMgr Mgr;
		return Mgr;
	}
	// 启动日志器
	static void start();
	// 添加任务日志
	static void addTask(Level level, const char* pStr);
	// 清空日志器
	static void ClearAll();
	// 清除单个级别日志
	static void ClearLevel(Level level);
private:
	// 主日志器
	LoggerPtr _logger;
};