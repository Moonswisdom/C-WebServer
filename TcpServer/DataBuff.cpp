#include "DataBuff.h"

DataBuff::DataBuff()
{
	_Dlen = 0;
	_BuffSize = RECV_BUFF_SIZE;
	_FullCount = 0;
}

int DataBuff::send2buff(SOCKET sock, const char* pData, int len)
{
	/* 缓冲区写满之后的处理
	* 1、写入到数据库 或 其它硬盘
	* 2、增加缓冲区大小
	* 3、使用链表形式的缓冲区，一个写满了就开辟下一个
	if (_Dlen + Len > _BuffSize) // BuffFullCount > 10 可多条件判断添加
	{
		// 查看所需要的空间数
		int diffnum = (_Dlen + Len) - _BuffSize;
		// 进行扩展空间
		if (diffnum < 8192)
		{
			std::lock_guard<std::muetx> autolock(_mutex);	// 需要加锁
			diffnum = 8192;
			char* buff = new char[__BuffSize + diffnum];
			memcpy(buff, _Dbuf, _Dlen);
			delete[] _Dbuf;
			_Dbuf = buff;
		}
	}
	*/
	int ret = 0;
	// 判断缓冲区是否放的下
	if (_Dlen + len <= _BuffSize)
	{
		// 将数据拷贝到缓冲区
		memcpy(_Dbuf + _Dlen, pData, len);
		_Dlen += len;
		ret = len;
	}
	
	// 缓冲区满了计数
	if (_Dlen + len >= _BuffSize)
	{
		// 缓冲区满了计数，提供反馈消息用于调整
		++_FullCount;
		// 缓冲区满10次进行一次提醒
		if (_FullCount > 10)
		{
			LogMgr::addTask(new LogInfo(LEVEL_WARN, "Send DataBuff full."));
			_FullCount = 0;
		}
		
		// 缓冲区写不下了就立即发送
		send2sock(sock);
	}

	// 返回存放的数据长度
	return ret;
}

int DataBuff::send2sock(SOCKET sock)
{
	int ret = 0;
	if (_Dlen > 0 && _Dbuf)
	{
		// 立即将缓冲区的数据全部发送出去
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
			// 可接收剩余缓冲区长度大小的数据
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
			// 更新缓冲区长度
			_Dlen += ret;
		}
		if (_Dlen == _BuffSize)
		{
			++_FullCount;
			// 缓冲区满10次进行一次提醒
			if (_FullCount > 10){
				// 缓存区满了提示
				LogMgr::addTask(new LogInfo(LEVEL_WARN, "recv DataBuff full."));
				_FullCount = 0;
			}
		}
	}
	return ret;
}

bool DataBuff::hasMsg()
{
	// 判断是否有完整数据
	if (_Dlen >= sizeof(msg_header))
	{
		msg_header* header = (msg_header*)_Dbuf;
		return _Dlen >= header->_Len;
	}
	return false;
}

msg_header* DataBuff::getMsg()
{
	// 取出第一条数据
	msg_header* header = (msg_header*)_Dbuf;
	return header;
}

void DataBuff::delMsg(int len)
{
	// 删除第一条数据
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
