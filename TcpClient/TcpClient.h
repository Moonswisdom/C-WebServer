#pragma once
#include "Tcphead.h"
#include "DataBuff.h"
#include "LogMgr.h"

/**
* ----- 客户端类 -----
*/
class TcpClient
{
public:
	TcpClient();
	~TcpClient();
public:
	// 创建socket
	SOCKET InitSock();
	// 请求连接
	int Connect(const char* ip, unsigned short port);
	// 是否运行
	bool isRun();
	// 主运行函数
	void mainRun();
	// 发送消息
	int SendData(msg_header* header, int nSize);
	// 接收消息
	int RecvData();
	// 解析消息
	int ParseData();
	// 关闭客户端
	void Close();
private:
	// 客户端socket
	SOCKET _cSock;
	// 计时器
	HRTimer _timer;
	// 接收缓冲区
	DataBuff _recvBuf;
	// 接收缓冲区长度
	int _rlen;
	// 是否连接标识
	bool _isConnect;
};