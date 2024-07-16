#pragma once

// c++标准库
#include<iostream>
#include<cstring>
#include<thread>
#include<vector>

// 消息体头文件
#include "MsgHead.h"

class TcpClient {
public:
	TcpClient();
	~TcpClient();

	// 创建socket
	int InitSocket();
	// 连接服务器
	int Connect(const char* ip, unsigned short port);
	// 发送消息
	int SendData(DataHeader* header);
	// 接收消息
	int RecvData();
	// 解析消息
	void ParseData(DataHeader* header);
	// 关闭连接
	void Close();
	// 判断客户端是否运行
	bool isRun() const;
	// 客户端运行主程序
	bool MainRun();
	
private:
	// 客户端socket
	SOCKET _cSock;
	// 消息缓冲区：用于组合和拆分接收到的消息
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	// 消息缓冲区尾部索引
	int _lastPos;
};