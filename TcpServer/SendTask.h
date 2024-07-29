#pragma once

#include<list>
#include<functional>
#include "Tcphead.h"
#include "ClientData.h"
#include "LogMgr.h"

// �������� ���Է�װ�࣬ʹ������ָ�롣Ҳ����ʹ��function, lambda���ʽ����ص�
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
* ----- ���������� -----
*/
class sTaskServer
{
public:
	// function�������������ٺ��ͷſռ�
	//typedef std::function<int()> sTaskPtr;
	sTaskServer();
	~sTaskServer();
public:
	// �������
	void addTask(sTaskPtr task);
	// �����߳�
	void start();
	// �رշ�����
	void Close();
private:
	// ִ������
	void mainRun();
private:
	// ������ʽ����
	std::list<sTaskPtr> _sTasks;
	// ���񻺳����
	std::list<sTaskPtr> _sTaskBuf;
	// �߳���
	std::mutex _mutex;
	// ���б�ʶ�����ڿ����̵߳ȴ���ѭ���˳���������������
	bool _isRun;
	// ������ʶ�����ڿ�����ѭ���˳�
	bool _isEnd;
};