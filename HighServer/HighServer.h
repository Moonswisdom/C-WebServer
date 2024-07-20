#pragma once

// c++��׼��
#include<iostream>
#include<cstring>
#include<iomanip>

// STLģ���
#include<vector>
#include<unordered_map>

// ���̣߳�����ԭ�Ӳ�����
#include<thread>
#include<mutex>
#include<atomic>


// ��Ϣͷ
#include "MsgHead.h"
// ��ʱ��
#include "EfficientTimer.h"
// ������
#include "CellTask.h"

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
	int getMsglen() const;
	// �޸���Ϣ����������
	void setMsglen(int mlen);
	// ��ȡ���ͻ�����
	char* getSendBuf();
	// ��ȡ��Ϣ����������
	int getSendlen() const;
	// �޸���Ϣ����������
	void setSendlen(int slen);
	// ���͸���Ӧ�ͻ�����Ϣ
	int SendData(DataHeader* header);
private:
	// �ͻ��� socket
	SOCKET _cSock;
	// ���տͻ������ݵ���Ϣ�����������ڴ���ճ�����ٰ�����
	char _MsgBuf[RECV_BUFF_SIZE];
	// ��Ϣ����������
	int _MsgBuf_len;
	// ���ͻ�����������������ݣ����ٷ��ʹ���
	char _SendBuf[SEND_BUFF_SIZE];
	// ���ͻ���������
	int _SendBuf_len;
	// ��ʱ���͵ļ�ʱ��
	EfficientTimer _ctimer;
};



class cellServer;
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
	virtual void JoinEvent(ClientData* pClient) = 0;
	// �ͻ����뿪�¼�
	virtual void LeaveEvent(ClientData* pClient) = 0;
	// ���տͻ�������Ĺ����¼�
	virtual void RecvEvent(ClientData* pClient) = 0;
	// ����ͻ�������Ĺ����¼�
	virtual void RequestEvent(cellServer* pServer, ClientData* pClient, DataHeader* header) = 0;
private:
};



/**
* ----- ���������� -----
*/
class SendTaskToClient : public CellTask
{
public:
	SendTaskToClient(ClientData* _pClient, DataHeader* _pHeader);
	~SendTaskToClient() {}
public:
	// ��������
	virtual void doTask();
private:
	ClientData* _pClient;
	DataHeader* _pHeader;
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
	// ��ӷ�������
	void addSendTask(ClientData* pClient, DataHeader* header);
private:
	// ������socket
	SOCKET _sockfd;
	// �����пͻ��˵Ĺ�ϣ���������
	std::unordered_map<SOCKET, ClientData*> _Clients;
	// �����¼���ͻ��˵Ķ�̬����
	std::vector<ClientData*> _ClientAddBuf;
	// ��֤�̰߳�ȫ����
	std::mutex _mutex;
	// �̶߳���
	std::thread _pthread;
	// �����¼�ʵ������
	TaskEvent* _pEvent;
	// �������������
	CellTaskServer _TaskServer;
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
	virtual void JoinEvent(ClientData* pClient);
	// �ͻ����뿪�¼�
	virtual void LeaveEvent(ClientData* pClient);
	// ���տͻ�������Ĺ����¼�
	virtual void RecvEvent(ClientData* pClient);
	// ����ͻ�������Ĺ����¼�
	virtual void RequestEvent(cellServer* pServer, ClientData* pClient, DataHeader* header);
private:
	// ������socket
	SOCKET _sockfd;
	// ���ӵĿͻ���������ԭ�����ԣ���֤�̰߳�ȫ
	std::atomic_int _clientNum;	
	// ���յ���Ϣ������ԭ�����ԣ���֤�̰߳�ȫ
	std::atomic_int _recvNum; 
	// �������Ϣ������ԭ�����ԣ���֤�̰߳�ȫ
	std::atomic_int _MsgNum;
	// �̷߳���������
	std::vector<cellServer*> _cellServers;
	// ��ʱ��
	EfficientTimer _timer;
};