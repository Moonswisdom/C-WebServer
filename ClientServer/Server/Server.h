#pragma once

#include<iostream>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>
#include<iomanip>
#include<cstring>

// ��Ϣͷ
#include "MsgHead.h"
// ��ʱ��
#include "EfficientTimer.h"

/**
* ----- �û���Ϣ�� -----
*/ 
class ClientData 
{
public:
	ClientData(SOCKET sock);
	~ClientData() {}
public:
	// ��ȡ�ͻ���socket
	SOCKET getSocket() const;
	// ��ȡ��Ϣ������
	char* getMsgBuf();
	// ��ȡ��Ϣ����������
	int getlastPos() const;
	// �޸���Ϣ����������
	void setlastPos(int pos);
	// ���͸���Ӧ�ͻ�����Ϣ
	void SendData(DataHeader* header);
private:
	// �ͻ��� socket
	SOCKET _cSock;
	// ���տͻ������ݵ���Ϣ�����������ڴ���ճ�����ٰ�����
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	// ��Ϣ����������
	int _lastPos;
};



/**
* ----- �����¼������� -----
*/
class TaskEvent
{
public:
	TaskEvent() {}
	~TaskEvent() {}
public:
	// �ͻ��˼����¼�
	virtual void JoinEvent() = 0;
	// �ͻ����뿪�¼�
	virtual void LeaveEvent() = 0;
	// ����ͻ�������Ĺ����¼�
	virtual void RequestEvent(ClientData* pClient, DataHeader* header) = 0;
private:
};



/**
* ----- �̷߳������� -----
*/
class cellServer 
{
public:
	cellServer(SOCKET sock);
	~cellServer();
public:
	// Ⱥ����Ϣ
	void SendToAll(DataHeader* header);
	// ������Ϣ
	int RecvData(ClientData* pClient);
	// �����Ƿ�����
	bool isRun() const;
	// ��������������
	void mainRun();
	// �ر�����
	void Close();
	// ��ӿͻ���
	void addClient(ClientData* pClient);
	// ��ȡ�ͻ�������
	int getClientNum() const;
	// �����߳�
	void start();
	// ���ò����¼�ʵ������
	void setEvent(TaskEvent* event);
private:
	// ������socket
	SOCKET _sockfd;
	// �����пͻ��˵Ķ�̬����
	std::vector <ClientData*> _Clients;
	// �����¼���ͻ��˵Ķ�̬����
	std::vector<ClientData*> _ClientAddBuf;
	// ��֤�̰߳�ȫ����
	std::mutex _mutex;
	// �̶߳���
	std::thread _pthread;
	// �����¼�ʵ������
	TaskEvent* _pEvent;
	// �������ݻ�����
	char _recvBuf[RECV_BUFF_SIZE];
};



/**
* ----- ���������� -----
*/
class TcpServer : public TaskEvent
{
public:
	TcpServer();
	~TcpServer();
public:
	// ��ʼ�� socket
	int InitSock();
	// �󶨶˿�
	int Bind(const char* ip, unsigned short port);
	// �����˿�
	int Listen(unsigned int pnum);
	// ��������
	int Accept();
	// �����Ƿ�����
	bool isRun() const;
	// ��������������
	void mainRun();
	// �ر�����
	void Close();
	// �����̷߳�����
	void startThread(const int NUM_thread);
	// ���ͻ��˼����̷߳�����
	void addClient(ClientData* pClient);
	// ����������ݵ��ٶ�
	void DataSpeed();
public:
	// �ͻ��˼����¼�
	virtual void JoinEvent();
	// �ͻ����뿪�¼�
	virtual void LeaveEvent();
	// ����ͻ�������Ĺ����¼�
	virtual void RequestEvent(ClientData* pClient, DataHeader* header);
private:
	// ������socket
	SOCKET _sockfd;
	// ���ӵĿͻ���������ԭ�����ԣ���֤�̰߳�ȫ
	std::atomic_int _clientNum;	
	// ���յ���Ϣ������ԭ�����ԣ���֤�̰߳�ȫ
	std::atomic_int _recvDataNum; 
	// �̷߳���������
	std::vector<cellServer*> _cellServers;
	// ��ʱ��
	EfficientTimer _timer;
	// �뿪�Ŀͻ�������
	std::atomic_int _exitNum;
};