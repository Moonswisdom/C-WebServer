#pragma once
#include<chrono>

/**
* ----- ��ʱ���� -----
*/
class HRTimer
{
public:
	HRTimer()
	{
		// ��ʼ��ʱ���
		updateTime();
	}
	~HRTimer() {}
public:
	// ��ȡ��ǰʱ���
	static time_t getNowTime()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}
	// ��ȡ��
	double getSecond()
	{
		return getMicroSecond() * 0.000001;
	}
	// ��ȡ����
	double getMilliSecond()
	{
		return getMicroSecond() * 0.001;
	}
	// ��ȡ΢��
	long long getMicroSecond()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
	}
	// ���¼�ʱ��
	void updateTime()
	{
		_begin = std::chrono::high_resolution_clock::now();
	}
private:
	// ��ʱ��
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};