#pragma once
#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<cstring>
#include<iomanip>

#include "MsgHead.h"
#include "EfficientTimer.h"

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
	void SendData(DataHeader* header, int MsgNum);
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
	// ������Ϣ������
	char _MsgBuf[RECV_BUFF_SIZE];
	// ��Ϣ����������
	int _lastPos;
};