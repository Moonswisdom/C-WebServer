#pragma once

// c++��׼��
#include<iostream>
#include<cstring>
#include<thread>

// ��Ϣ��ͷ�ļ�
#include "MsgHead.h"

class TcpClient {
public:
	TcpClient();
	~TcpClient();

	// ����socket
	void InitSocket();
	// ���ӷ�����
	int Connect(const char* ip, unsigned short port);
	// ������Ϣ
	int SendData(DataHeader* header);
	// ������Ϣ
	int RecvData();
	// ������Ϣ
	void ParseData(DataHeader* header);
	// �ر�����
	void Close();
	// �жϿͻ����Ƿ�����
	bool isRun();
	// �ͻ�������������
	bool MainRun();
private:
	SOCKET _cSock;
};