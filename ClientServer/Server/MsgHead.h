#pragma once
#define _WIN32_LEAN_AND_MEAN

#ifdef _WIN32
// windows 平台 soocket 库
#include<WinSock2.h>
#include<windows.h>
#pragma comment(lib, "ws2_32.lib")
#include<ws2tcpip.h>
#else
// Linux 平台 socket 库
#include<unistd.h>
#include<arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET SOCKET(~0)
#define SOCKET_ERROR (-1)
#endif

enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER,
	CMD_ERROR
};
struct DataHeader {
	short _Cmd;
	short _Length;
};
struct Login : public DataHeader {
	Login() {
		_Cmd = CMD_LOGIN;
		_Length = sizeof(Login);
	}
	char _Username[32];
	char _Password[32];
};
struct LoginResult : public DataHeader {
	LoginResult() {
		_Cmd = CMD_LOGIN_RESULT;
		_Length = sizeof(LoginResult);
		_Result = 0;
	}
	int _Result;
};
struct Logout : public DataHeader {
	Logout() {
		_Cmd = CMD_LOGOUT;
		_Length = sizeof(Logout);
	}
	char _Username[32];
};
struct LogoutResult : public DataHeader {
	LogoutResult() {
		_Cmd = CMD_LOGOUT_RESULT;
		_Length = sizeof(LogoutResult);
		_Result = 0;
	}
	int _Result;
};
struct NewUser : public DataHeader {
	NewUser(SOCKET sock):_Sock(sock) {
		_Cmd = CMD_NEW_USER;
		_Length = sizeof(NewUser);
	}
	SOCKET _Sock;
};