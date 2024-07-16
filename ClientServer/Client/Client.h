#pragma once

// c++��׼��
#include<iostream>
#include<cstring>
#include<thread>
#include<vector>

// ��Ϣ��ͷ�ļ�
#include "MsgHead.h"

class TcpClient {
public:
	TcpClient();
	~TcpClient();

	// ����socket
	int InitSocket();
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
	bool isRun() const;
	// �ͻ�������������
	bool MainRun();
	
private:
	// �ͻ���socket
	SOCKET _cSock;
	// ��Ϣ��������������ϺͲ�ֽ��յ�����Ϣ
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	// ��Ϣ������β������
	int _lastPos;
};