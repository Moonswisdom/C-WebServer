#pragma once
#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<cstring>

#include "MsgHead.h"
class TcpClient {
public:
	TcpClient();
	~TcpClient();
public:
	// 创建socket
	int InitSock();
	// 请求连接
	int Connect(const char* ip, unsigned int port);
	// 发送消息
	void SendData(DataHeader* header);
	// 接收消息
	int RecvData();
	// 解析消息
	void ParseData(DataHeader* header);
	// 关闭连接
	void Close();
	// 是否运行
	bool isRun() const;
	// 运行主程序
	void mainRun();
private:
	// 客户端socket
	SOCKET _cSock;
	// 是否连接标志
	bool _isConnect;
	// 一级接收缓冲区
	char _recvBuf[RECV_BUFF_SIZE];
	// 二级消息缓冲区
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	// 消息缓冲区长度
	int _lastPos;
};