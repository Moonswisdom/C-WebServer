#pragma once
#include "Tcphead.h"
#include "DataBuff.h"
#include "LogMgr.h"

/**
* ----- �ͻ�����Ϣ�� -----
*/
class ClientData
{
public:
	ClientData(SOCKET sock);
	~ClientData() {}
public:
	// �������ݵ�������
	int SendData(msg_header* header);
	// �ӷ��ͻ�������������
	int send2sock();

	// �������ݵ�������
	int RecvData();
	// �������Ƿ�������
	bool hasMsg();
	// �ӻ�����ȡһ������
	msg_header* getMsg();
	// �ӻ���ȡɾ��һ����Ϣ
	void delMsg();
public:
	// ��ȡ�ͻ���socket
	SOCKET getSock();
	// �����������ʱ��
	void resetheart();
	// �ж��Ƿ񵽴�Լ���������ʱ��
	bool timeheart(time_t dt);
	// ���÷���ʱ��
	void resetsend();
	// �ж��Ƿ񵽴�Լ�������ʱ��
	void timesend(time_t dt);
private:
	// �ͻ��� socket
	SOCKET _cSock;
	// ���ͻ�����
	DataBuff _sendBuf;
	// ���ջ�����
	DataBuff _recvBuf;
	// �������ʱ��
	time_t t_heart;
	// ��ʱ����ʱ��
	time_t t_send;
};
