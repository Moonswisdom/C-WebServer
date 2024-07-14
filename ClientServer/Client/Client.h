#pragma once

// c++标准库
#include<iostream>
#include<cstring>
#include<thread>

// 消息体头文件
#include "MsgHead.h"

class TcpClient {
public:
	TcpClient();
	~TcpClient();

	// 创建socket
	void InitSocket();
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
	bool isRun();
	// 客户端运行主程序
	bool MainRun();
private:
	SOCKET _cSock;
};