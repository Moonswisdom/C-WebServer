#pragma once
#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<cstring>

#include "MsgHead.h"
class TcpClient {
public:
	TcpClient();
	~TcpClient();
public:
	// ����socket
	int InitSock();
	// ��������
	int Connect(const char* ip, unsigned int port);
	// ������Ϣ
	void SendData(DataHeader* header);
	// ������Ϣ
	int RecvData();
	// ������Ϣ
	void ParseData(DataHeader* header);
	// �ر�����
	void Close();
	// �Ƿ�����
	bool isRun() const;
	// ����������
	void mainRun();
private:
	// �ͻ���socket
	SOCKET _cSock;
	// �Ƿ����ӱ�־
	bool _isConnect;
	// һ�����ջ�����
	char _recvBuf[RECV_BUFF_SIZE];
	// ������Ϣ������
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	// ��Ϣ����������
	int _lastPos;
};