#pragma once
#include "Tcphead.h"
#include "DataBuff.h"
#include "LogMgr.h"

/**
* ----- �ͻ����� -----
*/
class TcpClient
{
public:
	TcpClient();
	~TcpClient();
public:
	// ����socket
	SOCKET InitSock();
	// ��������
	int Connect(const char* ip, unsigned short port);
	// �Ƿ�����
	bool isRun();
	// �����к���
	void mainRun();
	// ������Ϣ
	int SendData(msg_header* header, int nSize);
	// ������Ϣ
	int RecvData();
	// ������Ϣ
	int ParseData();
	// �رտͻ���
	void Close();
private:
	// �ͻ���socket
	SOCKET _cSock;
	// ��ʱ��
	HRTimer _timer;
	// ���ջ�����
	DataBuff _recvBuf;
	// ���ջ���������
	int _rlen;
	// �Ƿ����ӱ�ʶ
	bool _isConnect;
};