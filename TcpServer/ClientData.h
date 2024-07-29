#pragma once
#include "Tcphead.h"
#include "DataBuff.h"
#include "LogMgr.h"

/**
* ----- 客户端信息类 -----
*/
class ClientData
{
public:
	ClientData(SOCKET sock);
	~ClientData() {}
public:
	// 发送数据到缓冲区
	int SendData(msg_header* header);
	// 从发送缓冲区发送数据
	int send2sock();

	// 接收数据到缓冲区
	int RecvData();
	// 缓冲区是否有数据
	bool hasMsg();
	// 从缓冲区取一条数据
	msg_header* getMsg();
	// 从缓冲取删除一条消息
	void delMsg();
public:
	// 获取客户端socket
	SOCKET getSock();
	// 重置心跳检测时间
	void resetheart();
	// 判断是否到达约定最大心跳时间
	bool timeheart(time_t dt);
	// 重置发送时间
	void resetsend();
	// 判断是否到达约定最大发送时间
	void timesend(time_t dt);
private:
	// 客户端 socket
	SOCKET _cSock;
	// 发送缓冲区
	DataBuff _sendBuf;
	// 接收缓冲区
	DataBuff _recvBuf;
	// 心跳检测时间
	time_t t_heart;
	// 定时发送时间
	time_t t_send;
};
