#pragma once

#include "Tcphead.h"

/**
* ----- 代理事件类 -----
*/
class tServer;

class AgentEvent
{
public:
	AgentEvent() {}
	virtual ~AgentEvent() {}
public:
	// 客户端加入事件
	virtual void joinEvent(ClientPtr pClient) = 0;
	// 客户端离开事件
	virtual void leaveEvent(ClientPtr pClient) = 0;
	// 接收数据事件
	virtual void recvEvent(ClientPtr pClient) = 0;
	// 解析数据事件
	virtual void parseEvent(tServer* svr, ClientPtr pClient, msg_header* header) = 0;
};