#pragma once

#include "Tcphead.h"

/**
* ----- �����¼��� -----
*/
class tServer;

class AgentEvent
{
public:
	AgentEvent() {}
	virtual ~AgentEvent() {}
public:
	// �ͻ��˼����¼�
	virtual void joinEvent(ClientPtr pClient) = 0;
	// �ͻ����뿪�¼�
	virtual void leaveEvent(ClientPtr pClient) = 0;
	// ���������¼�
	virtual void recvEvent(ClientPtr pClient) = 0;
	// ���������¼�
	virtual void parseEvent(tServer* svr, ClientPtr pClient, msg_header* header) = 0;
};