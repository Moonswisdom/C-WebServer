#pragma once
#include<vector>
#include<unordered_map>

#include "Tcphead.h"
#include "ClientData.h"
#include "sendTask.h"
#include "AgentEvent.h"

/**
* ----- �̷߳����� -----
*/
class tServer
{
public:
	tServer(SOCKET sock);
	~tServer();
public:
	// ���ô������
	void setAgent(AgentEvent* agent);
	// �����߳�
	void start();
	// ��ӿͻ���
	void addClient(ClientPtr pClient);
	// �������
	void addTask(ClientPtr pClient, msg_header* header);
	// �Ƿ�����
	bool isRun();
	// ����������
	void mainRun();
	// ����select����
	void NetRequest(fd_set& fdRead, bool& c_change);
	// ���ͻ��˷��ͺ�����ʱ��
	void DTcheck(time_t dt, bool& c_change);
	// ��������
	int RecvData(ClientPtr pClient);
	// ��������
	void ParseData(ClientPtr pClient, msg_header* header);
	// �ر��̷߳�����
	void Close();
	// ���ؿͻ��˸���
	int getClientNum();
private:
	// ������socket
	SOCKET _tSock;
	// �ͻ����б�
	std::unordered_map<SOCKET, ClientPtr> _tClients;
	// �ͻ��˻����б�
	std::vector<ClientPtr> _tClientBuf;
	// �߳���
	std::mutex _mutex;
	// �������������
	sTaskServer* _sTaskSvr;
	// ���������������
	AgentEvent* _Agent;
	// ���������б�ʶ
	bool _run;
};