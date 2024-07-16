#pragma once

// c++标准库
#include<iostream> 
#include<cstring>
#include<thread>
#include<vector>
#include<iomanip>

// 消息体头文件
#include "MsgHead.h"
// 高精度计时器
#include "EfficientTimer.h"

class ClientSock {
public:
	ClientSock(SOCKET sock);
	~ClientSock() {}
public:
	// 获取客户端socket接口
	SOCKET getSocket() const;
	// 获取客户端消息缓冲区接口
	char* getMsgBuf();
	// 获取消息缓冲区长度和更新缓冲区长度
	int getlastPos() const;
	void setlastPos(int pos);
private:
	const SOCKET _cSock;
	// 客户端消息缓冲区：用于分别处理每一个客户端的消息
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	int _lastPos;
};

class TcpServer {
public:
	TcpServer();
	~TcpServer();
public:
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
	int RecvData(ClientSock* Client);
	// 解析消息
	void ParseData(SOCKET cSock, DataHeader* header);
	// 关闭连接
	void Close();
	// 判断客户端是否运行
	bool isRun() const;
	// 客户端运行主程序
	bool MainRun();
private:
	SOCKET _sSock;
	// 必须使用客户端指针数组，栈空间不够分配这么多消息缓冲区，需要在堆区开辟空间
	std::vector<ClientSock*> _sClients;
	// 计时器
	EfficientTimer _ETimer;
	// 计数器
	int _recvCount;
};