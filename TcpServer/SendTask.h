#pragma once

#include<list>
#include<functional>
#include "Tcphead.h"
#include "ClientData.h"
#include "LogMgr.h"

// 发送任务 可以封装类，使用智能指针。也可以使用function, lambda表达式管理回调
class sTask
{
public:
	sTask(ClientPtr client, msg_header* header);
	~sTask() {}
public:
	void dotask();
private:
	ClientPtr _pClient;
	msg_header* _header;
};


/**
* ----- 发送任务类 -----
*/
class sTaskServer
{
public:
	// function管理任务函数开辟和释放空间
	//typedef std::function<int()> sTaskPtr;
	sTaskServer();
	~sTaskServer();
public:
	// 添加任务
	void addTask(sTaskPtr task);
	// 开启线程
	void start();
	// 关闭服务器
	void Close();
private:
	// 执行任务
	void mainRun();
private:
	// 任务正式队列
	std::list<sTaskPtr> _sTasks;
	// 任务缓冲队列
	std::list<sTaskPtr> _sTaskBuf;
	// 线程锁
	std::mutex _mutex;
	// 运行标识，用于控制线程等待主循环退出，避免析构错误
	bool _isRun;
	// 结束标识，用于控制主循环退出
	bool _isEnd;
};