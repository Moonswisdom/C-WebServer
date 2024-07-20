#pragma once

#include<thread>
#include<mutex>
#include<atomic>
#include<list>	// Ƶ����ɾ

/**
* ----- ���������� -----
*/
class CellTask
{
public:
	CellTask() {}
	// ������
	virtual ~CellTask() {}
public:
	// ִ������
	virtual void doTask() = 0;
private:

};



/**
* ----- ���ͷ������� -----
*/
class CellTaskServer
{
public:
	CellTaskServer() {}
	~CellTaskServer() {}
public:
	// ��������
	void addTask(CellTask* task);
	// ��������
	void start();
protected:
	// ����������
	void mainRun();
private:
	// ��������
	std::list<CellTask*> _tasks;
	// �������ݻ�����
	std::list<CellTask*> _taskBuf;
	// ������
	std::mutex _mutex;
};