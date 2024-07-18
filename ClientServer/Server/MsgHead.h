#pragma once
#define _WIN32_LEAN_AND_MEAN

#ifdef _WIN32
// Windows 平台 socket 库
#define FD_SETSIZE 1024
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
#endif

#define RECV_BUFF_SIZE 10240

/**
*  消息类别
*/
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER,
	CMD_ERROR
};
/**
*  消息头
*/
struct DataHeader
{
	DataHeader()
	{
		_Cmd = CMD_ERROR;
		_Length = sizeof(DataHeader);
	}
	short _Cmd;
	int _Length;
};
/**
*  消息体
*/
// 登录消息
struct Login : public DataHeader
{
	Login()
	{
		_Cmd = CMD_LOGIN;
		_Length = sizeof(Login);
	}
	char _Username[32] = {};
	char _Password[32] = {};
	char _data[30] = {}; // 补充让每个信息凑到100方便观察
};
// 登录结果消息
struct LoginResult : public DataHeader
{
	LoginResult()
	{
		_Cmd = CMD_LOGIN_RESULT;
		_Length = sizeof(LoginResult);
	}
	char _Result[32] = {};
	char _data[62] = {}; // 补充让每个信息凑到100方便观察
};
// 登出消息
struct Logout : public DataHeader
{
	Logout()
	{
		_Cmd = CMD_LOGOUT;
		_Length = sizeof(Logout);
	}
	char _Username[32] = {};
	char _data[62] = {}; // 补充让每个信息凑到100方便观察
};
// 登出结果消息
struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		_Cmd = CMD_LOGOUT_RESULT;
		_Length = sizeof(LogoutResult);
	}
	char _Result[32] = {};
	char _data[62] = {}; // 补充让每个信息凑到100方便观察
};
// 新用户提醒消息
struct Newuser : public DataHeader
{
	Newuser(SOCKET sock)
	{
		_Cmd = CMD_NEW_USER;
		_Length = sizeof(Newuser);
		_userSock = sock;
	}
	SOCKET _userSock;
	char _data[90] = {}; // 补充让每个信息凑到100方便观察
};
