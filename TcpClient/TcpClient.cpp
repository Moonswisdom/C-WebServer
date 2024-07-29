#include "TcpClient.h"

TcpClient::TcpClient()
{
	// 启动socket环境
	NetEnv::Init();
	// 启动日志器
	//LogMgr::start();
	
	// 初始化
	_cSock = INVALID_SOCKET;
	_rlen = 0;
	_isConnect = false;
}

TcpClient::~TcpClient()
{
	Close();
}

SOCKET TcpClient::InitSock()
{
	// 判断是否有旧连接
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		_cSock = INVALID_SOCKET;
	}

	// 创建新socket
	_cSock = socket(AF_INET, SOCK_STREAM, 0);

	return _cSock;
}

int TcpClient::Connect(const char* ip, unsigned short port)
{
	// 判断socket是否创建成功
	if (InitSock() == INVALID_SOCKET)
	{
		LogMgr::addTask(LEVEL_ERROR, "create new socket error ...");
		return -1;
	}
	// 创建socket成功
	//std::string str = "create new <socket = " + std::to_string((int)_cSock) + "> successful.";
	//LogMgr::addTask(LEVEL_INFO, str.c_str());

	// 创建地址端口变量
	sockaddr_in caddr = {};
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &caddr.sin_addr);
	// 请求连接
	if (SOCKET_ERROR == connect(_cSock, (struct sockaddr*)&caddr, sizeof(caddr))) 
	{
		// 连接失败
		std::string str = "<socket = " + std::to_string((int)_cSock) + "> connect server error.";
		LogMgr::addTask(LEVEL_ERROR, str.c_str());
		return -1;
	}
	// 连接成功
	std::string str = "<socket = " + std::to_string((int)_cSock) + "> connect server successful.";
	LogMgr::addTask(LEVEL_ERROR, str.c_str());
	_isConnect = true;
	return 0;
}

bool TcpClient::isRun()
{
	return _cSock != INVALID_SOCKET && _isConnect;
}

void TcpClient::mainRun()
{
	if (isRun())
	{
		fd_set fdRead = {};
		//fd_set fdWrite = {};
		//fd_set fdExp = {};
		FD_ZERO(&fdRead);
		//FD_ZERO(&fdWrite);
		//FD_ZERO(&fdExp);
		FD_SET(_cSock, &fdRead);
		//FD_SET(_cSock, &fdWrite);
		//FD_SET(_cSock, &fdExp);
		timeval tval = { 0, 50000 };
		
		if (0 > select((int)_cSock + 1, &fdRead, NULL, NULL, &tval))
		{
			std::string str = "<socket=" + std::to_string((int)_cSock) + "> select error.";
			LogMgr::addTask(LEVEL_ERROR, str.c_str());
			return;
		}

		if (FD_ISSET(_cSock, &fdRead))
		{
			FD_CLR(_cSock, &fdRead);
			{
				if (RecvData() == -1)
				{
					Close();
				}
			}
		}
	}
}

int TcpClient::SendData(msg_header* header, int nSize)
{
	return send(_cSock, (const char*)header, nSize, 0);
}

int TcpClient::RecvData()
{
	return _recvBuf.recv2buff(_cSock);
}

int TcpClient::ParseData()
{
	// 解析数据，可单开线程
	if (isRun())
	{
		if (_recvBuf.hasMsg())
		{
			// 有数据，处理
			msg_header* header = _recvBuf.getMsg();
			// 解析数据
			switch (header->_Msg)
			{
				case MSG_LOGIN_RESULT:
				{
					msg_LoginRet* loginRet = (msg_LoginRet*)header;
					// 自定义处理方式，存入日志
					//std::string str = "<socket=" + std::to_string((int)_cSock) + " LoginResult> " + loginRet->_Ret;
					//LogMgr::addTask(LEVEL_INFO, str.c_str());
					break;
				}
				case MSG_LOGOUT_RESULT:
				{
					msg_LogoutRet* logoutRet = (msg_LogoutRet*)header;
					// 自定义处理方式，存入日志
					//std::string str = "<socket=" + std::to_string((int)_cSock) + " LogoutResult> " + logoutRet->_Ret;
					//LogMgr::addTask(LEVEL_INFO, str.c_str());
					break;
				}
				default:
				{
					break;
				}
			}
			// 解析完毕，从缓冲区删除
			_recvBuf.delMsg(header->_Len);
		}
		else
		{
			// 没数据，休眠
			std::chrono::milliseconds tim(10);
			std::this_thread::sleep_for(tim);
		}
	}
	return 0;
}

void TcpClient::Close()
{
	_isConnect = false;
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif // _WIN32
		_cSock = INVALID_SOCKET;
	}
}
