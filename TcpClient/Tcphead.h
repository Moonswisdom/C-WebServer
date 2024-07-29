#pragma once

/**
* ----- C++��׼ͷ�ļ� -----
*/
#include<iostream>
#include<string>
#include<memory>
#include<thread>
#include<mutex>


/**
* ----- �Զ����ļ� -----
*/
// socket ����
#include "NetEnv.h"
// ��Ϣ�ṹ
#include "MsgFmt.h"
// ��ʱ��
#include "HRTimer.h"


/**
* ----- �궨�� -----
*/
// ���ջ�����
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif	// !RECV_BUFF_SIZE
// ���ͻ�����
#ifndef SEND_BUFF_SIZE
#define SEND_BUFF_SIZE 10240	// ���Ͷ���
#define SEND_BUFF_TIME 200		// ���Ͷ�ʱ
#endif // !SEND_BUFF_SIZE
// �������ʱ��
#ifndef HEART_DEAD_TIME
#define HEART_DEAD_TIME 60000	// 60s
#endif	// !HEART_DEAD_TIME
// ��־��ַ����
#ifndef LOG_PATH_LEN
#define LOG_PATH_LEN 64
#endif	// !LOG_PATH_LEN

/**
* ----- �������ָ������ -----
*/
// ��־����
class LogTask;
typedef std::shared_ptr<LogTask> LogTaskptr;
// ��־��
class Logger;
typedef std::shared_ptr<Logger> LoggerPtr;