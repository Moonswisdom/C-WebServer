#pragma once
#include<vector>
#include<unordered_map>

#include "Tcphead.h"
#include "ClientData.h"
#include "sendTask.h"
#include "AgentEvent.h"

/**
* ----- 线程服务类 -----
*/
class tServer
{
public:
	tServer(SOCKET sock);
	~tServer();
public:
	// 设置代理对象
	void setAgent(AgentEvent* agent);
	// 开启线程
	void start();
	// 添加客户端
	void addClient(ClientPtr pClient);
	// 添加任务
	void addTask(ClientPtr pClient, msg_header* header);
	// 是否运行
	bool isRun();
	// 运行主程序
	void mainRun();
	// 处理select数据
	void NetRequest(fd_set& fdRead, bool& c_change);
	// 检测客户端发送和心跳时间
	void DTcheck(time_t dt, bool& c_change);
	// 接收数据
	int RecvData(ClientPtr pClient);
	// 解析数据
	void ParseData(ClientPtr pClient, msg_header* header);
	// 关闭线程服务器
	void Close();
	// 返回客户端个数
	int getClientNum();
private:
	// 服务器socket
	SOCKET _tSock;
	// 客户端列表
	std::unordered_map<SOCKET, ClientPtr> _tClients;
	// 客户端缓存列表
	std::vector<ClientPtr> _tClientBuf;
	// 线程锁
	std::mutex _mutex;
	// 发送任务服务器
	sTaskServer* _sTaskSvr;
	// 主服务器代理对象
	AgentEvent* _Agent;
	// 主程序运行标识
	bool _run;
};