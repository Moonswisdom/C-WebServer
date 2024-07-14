#pragma once

// c++��׼��
#include<iostream>
#include<cstring>
#include<vector>

// ��Ϣ��ͷ�ļ�
#include "MsgHead.h"

class TcpServer {
public:
	TcpServer();
	~TcpServer();

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
	int RecvData(SOCKET cSock);
	// ������Ϣ
	void ParseData(SOCKET cSock, DataHeader* header);
	// �ر�����
	void Close();
	// �жϿͻ����Ƿ�����
	bool isRun();
	// �ͻ�������������
	bool MainRun();
private:
	SOCKET _sSock;
	std::vector<SOCKET> _sClients;
};