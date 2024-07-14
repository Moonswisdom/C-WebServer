#pragma once

// c++标准库
#include<iostream>
#include<cstring>
#include<vector>

// 消息体头文件
#include "MsgHead.h"

class TcpServer {
public:
	TcpServer();
	~TcpServer();

	// 创建socket
	int InitSocket();
	// 绑定端口
	int Bind(const char* ip, unsigned short port);
	// 监听端口
	int Listen(int num);
	// 接受客户端连接
	int Accept();
	// 发送消息
	int SendData(SOCKET cSock, DataHeader* header);
	// 群发消息
	void SendToAll(DataHeader* header);
	// 接收消息
	int RecvData(SOCKET cSock);
	// 解析消息
	void ParseData(SOCKET cSock, DataHeader* header);
	// 关闭连接
	void Close();
	// 判断客户端是否运行
	bool isRun();
	// 客户端运行主程序
	bool MainRun();
private:
	SOCKET _sSock;
	std::vector<SOCKET> _sClients;
};