#pragma once
#include "Tcphead.h"
#include "LogMgr.h"

/**
* -----  数据缓冲区类 -----
*/
class DataBuff
{
public:
	DataBuff();
	~DataBuff() {}
public:
	// 接收到缓冲区
	int recv2buff(SOCKET sock);
	// 判断缓冲区是否接收到完整数据
	bool hasMsg();
	// 获取第一条数据
	msg_header* getMsg();
	// 删除第一条数据
	void delMsg(int len);
private:
	// 缓冲区
	char _Dbuf[RECV_BUFF_SIZE] = {};
	// 缓冲区总长度
	int _BuffSize;
	// 缓冲区已有数据长度
	int _Dlen;
	// 缓冲区满计数
	int _FullCount;
	// 线程锁
	std::mutex _mutex;
};