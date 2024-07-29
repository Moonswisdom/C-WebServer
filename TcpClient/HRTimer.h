#pragma once
#include<chrono>

/**
* ----- 计时器类 -----
*/
class HRTimer
{
public:
	HRTimer()
	{
		// 初始化时间点
		updateTime();
	}
	~HRTimer() {}
public:
	// 获取当前时间戳
	static time_t getNowTime()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}
	// 获取秒
	double getSecond()
	{
		return getMicroSecond() * 0.000001;
	}
	// 获取毫秒
	double getMilliSecond()
	{
		return getMicroSecond() * 0.001;
	}
	// 获取微秒
	long long getMicroSecond()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
	}
	// 更新计时器
	void updateTime()
	{
		_begin = std::chrono::high_resolution_clock::now();
	}
private:
	// 计时器
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};