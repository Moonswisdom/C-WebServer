#pragma once
#include<fstream>
#include<list>
#include<time.h>
#include "Tcphead.h"

/**
* ----- ��־������ -----
*/
// ��־����
enum Level {
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_DEBUG,
	LEVEL_ERROR,
	LEVEL_FATAL,
};

// ��־����
class LogTask
{
public:
	LogTask(tm tnow, Level level, const char* path, const char* data);
	~LogTask() {}
public:
	// ִ������
	void doTask();
private:
	// ��־ʱ��
	tm _tnow;
	// ��־����
	Level _level;
	// ��־��ַ
	char _path[LOG_PATH_LEN];
	// ��־����
	char _data[256];
};

// ��־��
class Logger
{
public:
	Logger();
	~Logger();
public:
	// ��ʼ����־���λ��
	void InitPath(const char* pINFO = "../ClientLOG/LOG_INFO.txt", const char* pWARN = "../ClientLOG/LOG_WARN.txt",
		const char* pDEBUG = "../ClientLOG/LOG_DEBUG.txt", const char* pERROR = "../ClientLOG/LOG_ERROR.txt", const char* pFATAL = "../ClientLOG/LOG_FATAL.txt");
	// ��ն�Ӧ�������־
	void ClearLevel(Level level);
	// ���������־
	void ClearAllLevel();

	// ���������־
	void addTask(Level level, const char* pStr);
	// ������־����
	void mainRun();
	// ���������߳�
	void start();

private:
	// ��ȡtm��ʽ�ĵ�ǰʱ��
	tm getNowtm();

private:
	// �����
	std::ofstream _ofs;
	// ������־�洢��ַ
	char _path_INFO[LOG_PATH_LEN];
	char _path_WARN[LOG_PATH_LEN];
	char _path_DEBUG[LOG_PATH_LEN];
	char _path_ERROR[LOG_PATH_LEN];
	char _path_FATAL[LOG_PATH_LEN];
	// ��־����
	std::list<LogTaskptr> _Tasks;
	// ��־���񻺳���
	std::list<LogTaskptr> _TaskBuf;
	// ��־������
	bool _isRun;
	// ��־��������ʶ
	bool _isEnd;
	// �̰߳�ȫ��
	std::mutex _mutex;
};


// ��־������������չ�����־��
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
	// ������־��
	static void start();
	// ���������־
	static void addTask(Level level, const char* pStr);
	// �����־��
	static void ClearAll();
	// �������������־
	static void ClearLevel(Level level);
private:
	// ����־��
	LoggerPtr _logger;
};