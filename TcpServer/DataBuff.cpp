#include "DataBuff.h"

DataBuff::DataBuff()
{
	_Dlen = 0;
	_BuffSize = RECV_BUFF_SIZE;
	_FullCount = 0;
}

int DataBuff::send2buff(SOCKET sock, const char* pData, int len)
{
	/* ������д��֮��Ĵ���
	* 1��д�뵽���ݿ� �� ����Ӳ��
	* 2�����ӻ�������С
	* 3��ʹ��������ʽ�Ļ�������һ��д���˾Ϳ�����һ��
	if (_Dlen + Len > _BuffSize) // BuffFullCount > 10 �ɶ������ж����
	{
		// �鿴����Ҫ�Ŀռ���
		int diffnum = (_Dlen + Len) - _BuffSize;
		// ������չ�ռ�
		if (diffnum < 8192)
		{
			std::lock_guard<std::muetx> autolock(_mutex);	// ��Ҫ����
			diffnum = 8192;
			char* buff = new char[__BuffSize + diffnum];
			memcpy(buff, _Dbuf, _Dlen);
			delete[] _Dbuf;
			_Dbuf = buff;
		}
	}
	*/
	int ret = 0;
	// �жϻ������Ƿ�ŵ���
	if (_Dlen + len <= _BuffSize)
	{
		// �����ݿ�����������
		memcpy(_Dbuf + _Dlen, pData, len);
		_Dlen += len;
		ret = len;
	}
	
	// ���������˼���
	if (_Dlen + len >= _BuffSize)
	{
		// ���������˼������ṩ������Ϣ���ڵ���
		++_FullCount;
		// ��������10�ν���һ������
		if (_FullCount > 10)
		{
			LogMgr::addTask(new LogInfo(LEVEL_WARN, "Send DataBuff full."));
			_FullCount = 0;
		}
		
		// ������д�����˾���������
		send2sock(sock);
	}

	// ���ش�ŵ����ݳ���
	return ret;
}

int DataBuff::send2sock(SOCKET sock)
{
	int ret = 0;
	if (_Dlen > 0 && _Dbuf)
	{
		// ������������������ȫ�����ͳ�ȥ
		ret = send(sock, _Dbuf, _Dlen, 0);
		_Dlen = 0;
	}
	return ret;
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
			ret = (int)recv(sock, szRecv, _BuffSize - _Dlen, 0);
			if (ret == 0)
			{
				LogMgr::addTask(new LogInfo(LEVEL_WARN, "client off connect."));
			}
			else if (ret < 0)
			{
				LogMgr::addTask(new LogInfo(LEVEL_ERROR, "recv to buff error."));
				return -1;
			}
			// ���»���������
			_Dlen += ret;
		}
		if (_Dlen == _BuffSize)
		{
			++_FullCount;
			// ��������10�ν���һ������
			if (_FullCount > 10){
				// ������������ʾ
				LogMgr::addTask(new LogInfo(LEVEL_WARN, "recv DataBuff full."));
				_FullCount = 0;
			}
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
