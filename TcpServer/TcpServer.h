#pragma once

#include<iomanip>
#include<atomic>
#include<vector>
#include "Tcphead.h"
#include "ClientData.h"
#include "threadServer.h"
#include "LogMgr.h"

/**
* ----- ���������� -----
*/
class TcpServer : public AgentEvent
{
public:
	TcpServer();
	~TcpServer();
public:
	// ����socket
	SOCKET InitSock();
	// �󶨶˿�
	int Bind(const char* ip, unsigned short port);
	// �����˿�
	int Listen(unsigned int pnum);
	// ��������
	int Accept();
	// �Ƿ�����
	bool isRun();
	// �����г���
	void mainRun();
	// �ر�����
	int Close();
	// �����̷߳�����
	void start(int tnum);
	// �����ٵ��̷߳�������ӿͻ���
	void addClient(ClientPtr pClient);
public:
	// ��ӡ������Ϣ, �ɼ̳��޸�
	virtual void printInfo();
	// ���� �ͻ��˼���
	virtual void joinEvent(ClientPtr pClient);
	// ���� �ͻ����뿪
	virtual void leaveEvent(ClientPtr pClient);
	// ���� ��������
	virtual void recvEvent(ClientPtr pClient);
	// ���� ��������
	virtual void parseEvent(tServer* tsvr, ClientPtr pClient, msg_header* header);
private:
	// ����socket
	SOCKET _Sock;
	// �̷߳���������
	std::vector<tServer*> _tServers;
	// ��ʱ��
	HRTimer _timer;
	// �ͻ��������� ԭ�����ԣ��̰߳�ȫ
	std::atomic_int _clientNum;
	// ������������
	std::atomic_int _recvNum;
	// ������������
	std::atomic_int _parseNum;
};