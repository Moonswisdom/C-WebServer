#pragma once

#include<iostream>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>
#include<iomanip>
#include<cstring>

// 消息头
#include "MsgHead.h"
// 计时器
#include "EfficientTimer.h"

/**
* ----- 用户信息类 -----
*/ 
class ClientData 
{
public:
	ClientData(SOCKET sock);
	~ClientData() {}
public:
	// 获取客户端socket
	SOCKET getSocket() const;
	// 获取消息缓冲区
	char* getMsgBuf();
	// 获取消息缓冲区长度
	int getlastPos() const;
	// 修改消息缓冲区长度
	void setlastPos(int pos);
	// 发送给对应客户端消息
	void SendData(DataHeader* header);
private:
	// 客户端 socket
	SOCKET _cSock;
	// 接收客户端数据的消息缓冲区，用于处理粘包，少包问题
	char _MsgBuf[RECV_BUFF_SIZE * 10];
	// 消息缓冲区长度
	int _lastPos;
};



/**
* ----- 操作事件抽象类 -----
*/
class TaskEvent
{
public:
	TaskEvent() {}
	~TaskEvent() {}
public:
	// 客户端加入事件
	virtual void JoinEvent() = 0;
	// 客户端离开事件
	virtual void LeaveEvent() = 0;
	// 处理客户端请求的工作事件
	virtual void RequestEvent(ClientData* pClient, DataHeader* header) = 0;
private:
};



/**
* ----- 线程服务器类 -----
*/
class cellServer 
{
public:
	cellServer(SOCKET sock);
	~cellServer();
public:
	// 群发消息
	void SendToAll(DataHeader* header);
	// 接收消息
	int RecvData(ClientData* pClient);
	// 程序是否运行
	bool isRun() const;
	// 程序运行主程序
	void mainRun();
	// 关闭连接
	void Close();
	// 添加客户端
	void addClient(ClientData* pClient);
	// 获取客户端数量
	int getClientNum() const;
	// 启动线程
	void start();
	// 设置操作事件实例对象
	void setEvent(TaskEvent* event);
private:
	// 服务器socket
	SOCKET _sockfd;
	// 运行中客户端的动态数组
	std::vector <ClientData*> _Clients;
	// 缓存新加入客户端的动态数组
	std::vector<ClientData*> _ClientAddBuf;
	// 保证线程安全的锁
	std::mutex _mutex;
	// 线程对象
	std::thread _pthread;
	// 操作事件实例对象
	TaskEvent* _pEvent;
	// 接收数据缓冲区
	char _recvBuf[RECV_BUFF_SIZE];
};



/**
* ----- 主服务器类 -----
*/
class TcpServer : public TaskEvent
{
public:
	TcpServer();
	~TcpServer();
public:
	// 初始化 socket
	int InitSock();
	// 绑定端口
	int Bind(const char* ip, unsigned short port);
	// 监听端口
	int Listen(unsigned int pnum);
	// 建立连接
	int Accept();
	// 程序是否运行
	bool isRun() const;
	// 程序运行主程序
	void mainRun();
	// 关闭连接
	void Close();
	// 启动线程服务器
	void startThread(const int NUM_thread);
	// 将客户端加入线程服务器
	void addClient(ClientData* pClient);
	// 计算接收数据的速度
	void DataSpeed();
public:
	// 客户端加入事件
	virtual void JoinEvent();
	// 客户端离开事件
	virtual void LeaveEvent();
	// 处理客户端请求的工作事件
	virtual void RequestEvent(ClientData* pClient, DataHeader* header);
private:
	// 服务器socket
	SOCKET _sockfd;
	// 连接的客户端总数，原子属性，保证线程安全
	std::atomic_int _clientNum;	
	// 接收的消息总数，原子属性，保证线程安全
	std::atomic_int _recvDataNum; 
	// 线程服务器数组
	std::vector<cellServer*> _cellServers;
	// 计时器
	EfficientTimer _timer;
	// 离开的客户端数量
	std::atomic_int _exitNum;
};