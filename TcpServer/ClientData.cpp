#include "ClientData.h"

ClientData::ClientData(SOCKET sock)
{
	_cSock = sock;
	t_heart = 0;
	// 定时发送时间
	t_send = 0;
}

int ClientData::SendData(msg_header* header)
{
	return _sendBuf.send2buff(_cSock, (const char*)header, header->_Len);
}

int ClientData::send2sock()
{
	// 发送数据，重置心跳时间和发送时间
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
	// 判断客户端心跳时间是否超时
	if (t_heart >= HEART_DEAD_TIME)
	{
		// 客户端连接超时提示
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
	// 如果到达发送定时，则立即发送数据
	if (t_send >= SEND_BUFF_TIME)
	{
		send2sock();
		// 重置发送定时
		resetsend();
	}
}

