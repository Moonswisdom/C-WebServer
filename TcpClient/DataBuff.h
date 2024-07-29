#pragma once
#include "Tcphead.h"
#include "LogMgr.h"

/**
* -----  ���ݻ������� -----
*/
class DataBuff
{
public:
	DataBuff();
	~DataBuff() {}
public:
	// ���յ�������
	int recv2buff(SOCKET sock);
	// �жϻ������Ƿ���յ���������
	bool hasMsg();
	// ��ȡ��һ������
	msg_header* getMsg();
	// ɾ����һ������
	void delMsg(int len);
private:
	// ������
	char _Dbuf[RECV_BUFF_SIZE] = {};
	// �������ܳ���
	int _BuffSize;
	// �������������ݳ���
	int _Dlen;
	// ������������
	int _FullCount;
	// �߳���
	std::mutex _mutex;
};