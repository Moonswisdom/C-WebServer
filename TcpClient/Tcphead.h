#pragma once

/**
* ----- C++标准头文件 -----
*/
#include<iostream>
#include<string>
#include<memory>
#include<thread>
#include<mutex>


/**
* ----- 自定义文件 -----
*/
// socket 环境
#include "NetEnv.h"
// 消息结构
#include "MsgFmt.h"
// 计时器
#include "HRTimer.h"


/**
* ----- 宏定义 -----
*/
// 接收缓冲区
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif	// !RECV_BUFF_SIZE
// 发送缓冲区
#ifndef SEND_BUFF_SIZE
#define SEND_BUFF_SIZE 10240	// 发送定量
#define SEND_BUFF_TIME 200		// 发送定时
#endif // !SEND_BUFF_SIZE
// 心跳检测时长
#ifndef HEART_DEAD_TIME
#define HEART_DEAD_TIME 60000	// 60s
#endif	// !HEART_DEAD_TIME
// 日志地址长度
#ifndef LOG_PATH_LEN
#define LOG_PATH_LEN 64
#endif	// !LOG_PATH_LEN

/**
* ----- 类和智能指针声明 -----
*/
// 日志任务
class LogTask;
typedef std::shared_ptr<LogTask> LogTaskptr;
// 日志器
class Logger;
typedef std::shared_ptr<Logger> LoggerPtr;