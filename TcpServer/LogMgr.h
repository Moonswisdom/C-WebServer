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

// ��־��Ϣ
class LogInfo
{
public:
	LogInfo(Level level, const char* data);
	~LogInfo() {}
	// ��־����
	Level _level;
	// ��־����
	char _data[256];
};

// ��־����
class LogTask
{
public:
	LogTask(tm tnow, const char* path, LogInfo* loginfo);
	~LogTask() {}
public:
	// ִ������
	void doTask();
private:
	// ʱ��ڵ�
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
	void InitPath(const char* pINFO = "../ServerLOG/LOG_INFO.txt", const char* pWARN = "../ServerLOG/LOG_WARN.txt",
		const char* pDEBUG = "../ServerLOG/LOG_DEBUG.txt", const char* pERROR = "../ServerLOG/LOG_ERROR.txt", const char* pFATAL = "../ServerLOG/LOG_FATAL.txt");
	// ��ն�Ӧ�������־
	void ClearLevel(Level level);
	// ���������־
	void ClearAllLevel();

	// ���������־
	void addTask(LogInfo* loginfo);
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
	std::list<LogTaskPtr> _Tasks;
	// ��־���񻺳���
	std::list<LogTaskPtr> _TaskBuf;
	// ��־�����б�ʶ
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
	~LogMgr();
public:
	static LogMgr& LogM()
	{
		static LogMgr Mgr;
		return Mgr;
	}
	// ������־��
	static void start();
	// ���������־
	static void addTask(LogInfo* loginfo);
	// �����־��
	static void ClearAll();
	// �������������־
	static void ClearLevel(Level level);
private:
	// ����־��
	Logger* _logger;
};