#pragma once

// c++��׼��
#include<iostream> 
#include<cstring>
#include<thread>
#include<vector>
#include<iomanip>

// ��Ϣ��ͷ�ļ�
#include "MsgHead.h"
// �߾��ȼ�ʱ��
#include "EfficientTimer.h"

class ClientSock {
public:
	ClientSock(SOCKET sock);
	~ClientSock() {}
public:
	// ��ȡ�ͻ���socket�ӿ�
	SOCKET getSocket() const;
	// ��ȡ�ͻ�����Ϣ�������ӿ�
	char* getMsgBuf();
	// ��ȡ��Ϣ���������Ⱥ͸��»���������
	int getlastPos() const;
	void setlastPos(int pos);
private:
	const SOCKET _cSock;
	// �ͻ�����Ϣ�����������ڷֱ���ÿһ���ͻ��˵���Ϣ
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	int _lastPos;
};

class TcpServer {
public:
	TcpServer();
	~TcpServer();
public:
	// ����socket
	int InitSocket();
	// �󶨶˿�
	int Bind(const char* ip, unsigned short port);
	// �����˿�
	int Listen(int num);
	// ���ܿͻ�������
	int Accept();
	// ������Ϣ
	int SendData(SOCKET cSock, DataHeader* header);
	// Ⱥ����Ϣ
	void SendToAll(DataHeader* header);
	// ������Ϣ
	int RecvData(ClientSock* Client);
	// ������Ϣ
	void ParseData(SOCKET cSock, DataHeader* header);
	// �ر�����
	void Close();
	// �жϿͻ����Ƿ�����
	bool isRun() const;
	// �ͻ�������������
	bool MainRun();
private:
	SOCKET _sSock;
	// ����ʹ�ÿͻ���ָ�����飬ջ�ռ䲻��������ô����Ϣ����������Ҫ�ڶ������ٿռ�
	std::vector<ClientSock*> _sClients;
	// ��ʱ��
	EfficientTimer _ETimer;
	// ������
	int _recvCount;
};