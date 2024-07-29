#pragma once

#ifdef _WIN32
// windows 平台 socket 库
#define FD_SETSIZE 1024
#define _WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<windows.h>
#pragma comment(lib, "ws2_32.lib")
#include<ws2tcpip.h>
#else
// Linux 平台 socket 库
#include<unistd>
#include<arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET SOCKET(~0)
#define SOCKET_ERROR -1
#endif	// _WIN32

/**
* ----- socket 环境单例类-----
*/ 
class NetEnv
{
public:
	NetEnv() 
	{
		// 开启Windows平台socket环境
#ifdef _WIN32
		WORD wr = MAKEWORD(2, 2);
		WSADATA dat = {};
		if (WSAStartup(wr, &dat) == SOCKET_ERROR)
		{
			std::cout << "windows start up socket environment error." << std::endl;
		}
#endif // _WIN32

#ifndef _WIN32
		signal(SIGPIPE, SIG_IGN);
#endif // !_WIN32
	}

	~NetEnv()
	{
#ifdef _WIN32
		WSACleanup();
#endif // !_WIN32
	}
public:
	NetEnv(const NetEnv& Env) = delete;
	NetEnv& operator=(const NetEnv& Env) = delete;
	// 单例静态模式，保证环境的打开和关闭只进行一次
	static void Init()
	{
		static NetEnv Env;
	}
};