#include "ClientData.h"

ClientData::ClientData(SOCKET sock)
{
	_cSock = sock;
	t_heart = 0;
	// ��ʱ����ʱ��
	t_send = 0;
}

int ClientData::SendData(msg_header* header)
{
	return _sendBuf.send2buff(_cSock, (const char*)header, header->_Len);
}

int ClientData::send2sock()
{
	// �������ݣ���������ʱ��ͷ���ʱ��
	int ret = _sendBuf.send2sock(_cSock);
	if (ret > 0)
	{
		resetheart();
		resetsend();
	}
	return ret;
}

int ClientData::RecvData()
{
	return _recvBuf.recv2buff(_cSock);
}

bool ClientData::hasMsg()
{
	return _recvBuf.hasMsg();
}

msg_header* ClientData::getMsg()
{
	return _recvBuf.getMsg();
}

void ClientData::delMsg()
{
	if (hasMsg())
	{
		_recvBuf.delMsg(getMsg()->_Len);
	}
}

SOCKET ClientData::getSock()
{
	return _cSock;
}

void ClientData::resetheart()
{
	t_heart = 0;
}

bool ClientData::timeheart(time_t dt)
{
	t_heart += dt;
	// �жϿͻ�������ʱ���Ƿ�ʱ
	if (t_heart >= HEART_DEAD_TIME)
	{
		// �ͻ������ӳ�ʱ��ʾ
		std::string str = "<socket=" + std::to_string(_cSock) + "> connect timeout.";
		LogMgr::addTask(new LogInfo(LEVEL_WARN, str.c_str()));
		return true;
	}
	return false;
}

void ClientData::resetsend()
{
	t_send = 0;
}

void ClientData::timesend(time_t dt)
{
	t_send += dt;
	// ������﷢�Ͷ�ʱ����������������
	if (t_send >= SEND_BUFF_TIME)
	{
		send2sock();
		// ���÷��Ͷ�ʱ
		resetsend();
	}
}

