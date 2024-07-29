#include "DataBuff.h"

DataBuff::DataBuff()
{
	_Dlen = 0;
	_BuffSize = RECV_BUFF_SIZE;
	_FullCount = 0;
}

int DataBuff::recv2buff(SOCKET sock)
{
	int ret = 0;
	if (sock != INVALID_SOCKET)
	{
		if (_BuffSize - _Dlen > 0)
		{
			// �ɽ���ʣ�໺�������ȴ�С������
			char* szRecv = _Dbuf + _Dlen;
			ret = recv(sock, szRecv, _BuffSize - _Dlen, 0);
			if (ret == 0)
			{
				std::string str = "<socket=" + std::to_string((int)sock) + "> server off connect.";
				LogMgr::addTask(LEVEL_DEBUG, str.c_str());
			}
			else if (ret < 0)
			{
				std::string str = "<socket=" + std::to_string((int)sock) + "> recv error.";
				LogMgr::addTask(LEVEL_ERROR, str.c_str());
				return -1;
			}
			// ���»���������
			_Dlen += ret;
		}
		if (_Dlen == _BuffSize)
		{
			++_FullCount;
			// ������������ʾ
			std::string str = "<socket=" + std::to_string((int)sock) + "> recv DataBuff full, FullCount is " + std::to_string(_FullCount) + ".";
			LogMgr::addTask(LEVEL_WARN, str.c_str());
		}
	}
	return ret;
}

bool DataBuff::hasMsg()
{
	// �ж��Ƿ�����������
	if (_Dlen >= sizeof(msg_header))
	{
		msg_header* header = (msg_header*)_Dbuf;
		return _Dlen >= header->_Len;
	}
	return false;
}

msg_header* DataBuff::getMsg()
{
	// ȡ����һ������
	msg_header* header = (msg_header*)_Dbuf;
	return header;
}

void DataBuff::delMsg(int len)
{
	// ɾ����һ������
	std::lock_guard<std::mutex> autolock(_mutex);
	if (_Dlen - len > 0)
	{
		memcpy(_Dbuf, _Dbuf + len, _Dlen - len);
	}
	_Dlen -= len;
	if (_FullCount > 0)
	{
		--_FullCount;
	}
}
