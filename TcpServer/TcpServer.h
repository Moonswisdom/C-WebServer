#pragma once

#include<iomanip>
#include<atomic>
#include<vector>
#include "Tcphead.h"
#include "ClientData.h"
#include "threadServer.h"
#include "LogMgr.h"

/**
* ----- 主服务器类 -----
*/
class TcpServer : public AgentEvent
{
public:
	TcpServer();
	~TcpServer();
public:
	// 创建socket
	SOCKET InitSock();
	// 绑定端口
	int Bind(const char* ip, unsigned short port);
	// 监听端口
	int Listen(unsigned int pnum);
	// 接收连接
	int Accept();
	// 是否运行
	bool isRun();
	// 主运行程序
	void mainRun();
	// 关闭连接
	int Close();
	// 启动线程服务器
	void start(int tnum);
	// 向最少的线程服务器添加客户端
	void addClient(ClientPtr pClient);
public:
	// 打印基本信息, 可继承修改
	virtual void printInfo();
	// 代理 客户端加入
	virtual void joinEvent(ClientPtr pClient);
	// 代理 客户端离开
	virtual void leaveEvent(ClientPtr pClient);
	// 代理 接收数据
	virtual void recvEvent(ClientPtr pClient);
	// 代理 解析数据
	virtual void parseEvent(tServer* tsvr, ClientPtr pClient, msg_header* header);
private:
	// 创建socket
	SOCKET _Sock;
	// 线程服务器队列
	std::vector<tServer*> _tServers;
	// 计时器
	HRTimer _timer;
	// 客户端总数， 原子属性，线程安全
	std::atomic_int _clientNum;
	// 接收数据总数
	std::atomic_int _recvNum;
	// 解析数据总数
	std::atomic_int _parseNum;
};