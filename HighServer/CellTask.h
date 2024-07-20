#pragma once

#include<thread>
#include<mutex>
#include<atomic>
#include<list>	// 频繁增删

/**
* ----- 任务抽象基类 -----
*/
class CellTask
{
public:
	CellTask() {}
	// 虚析构
	virtual ~CellTask() {}
public:
	// 执行任务
	virtual void doTask() = 0;
private:

};



/**
* ----- 发送服务器类 -----
*/
class CellTaskServer
{
public:
	CellTaskServer() {}
	~CellTaskServer() {}
public:
	// 增加任务
	void addTask(CellTask* task);
	// 启动服务
	void start();
protected:
	// 运行主程序
	void mainRun();
private:
	// 任务数据
	std::list<CellTask*> _tasks;
	// 任务数据缓冲区
	std::list<CellTask*> _taskBuf;
	// 互斥锁
	std::mutex _mutex;
};