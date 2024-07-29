#pragma once
#include<iostream>
/**
* ----- 消息类别 -----
*/
enum MSG_CMD
{
	MSG_ERROR,
	MSG_LOGIN,
	MSG_LOGIN_RESULT,
	MSG_LOGOUT,
	MSG_LOGOUT_RESULT,
	MSG_NEW_USER
};

/**
* ----- 消息头 -----
*/
struct msg_header
{
	msg_header()
	{
		_Msg = MSG_ERROR;
		_Len = sizeof(msg_header);
	}
	short _Msg;
	unsigned short _Len;
};

/**
* ----- 心跳信号 -----

struct msg_s2c_heart : public msg_header
{
	msg_s2c_heart()
	{
		_Msg = MSG_S2C_HEART;
		_Len = sizeof(msg_s2c_heart);
	}
};
struct msg_c2s_heart : public msg_header
{
	msg_c2s_heart()
	{
		_Msg = MSG_C2S_HEART;
		_Len = sizeof(msg_c2s_heart);
	}
};
*/

/**
* ----- 登录登出消息 -----
*/
struct msg_Login : public msg_header
{
	msg_Login(const char* name = "root", const char* pwd = "123456")
	{
		_Msg = MSG_LOGIN;
		_Len = sizeof(msg_Login);
		memcpy(_Name, name, strlen(name) + 1);
		memcpy(_pwd, pwd, strlen(pwd) + 1);
	}
	char _Name[32];
	char _pwd[32];
};
struct msg_LoginRet : public msg_header
{
	msg_LoginRet(const char* ret = "Login successful, you are welcome ...")
	{
		_Msg = MSG_LOGIN_RESULT;
		_Len = sizeof(msg_LoginRet);
		memcpy(_Ret, ret, strlen(ret) + 1);
	}
	char _Ret[60];
};
struct msg_Logout : public msg_header
{
	msg_Logout(const char* name = "root")
	{
		_Msg = MSG_LOGOUT;
		_Len = sizeof(msg_Logout);
		memcpy(_Name, name, strlen(name) + 1);
	}
	char _Name[32];
};
struct msg_LogoutRet : public msg_header
{
	msg_LogoutRet(const char* ret = "Logout successful, good bye ...")
	{
		_Msg = MSG_LOGOUT_RESULT;
		_Len = sizeof(msg_LogoutRet);
		memcpy(_Ret, ret, strlen(ret) + 1);
	}
	char _Ret[60];
};

// 新用户提醒消息
struct msg_Newsuer : public msg_header
{
	msg_Newsuer(int sock)
	{
		_Msg = MSG_NEW_USER;
		_Len = sizeof(msg_Newsuer);
		_uSock = sock;
	}
	int _uSock;
};

