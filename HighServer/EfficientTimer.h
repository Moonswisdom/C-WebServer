#pragma once

// C++11 高精度计时器
#include<chrono>

class EfficientTimer 
{
public:
	EfficientTimer() 
	{
		// 初始化时间节点
		updateTime();
	}
	~EfficientTimer(){}
public:
	// 更新时间节点
	void updateTime() 
	{
		_begin = std::chrono::high_resolution_clock::now();
	}
	// 获取秒
	double getElapseSecond() 
	{
		return getElapseMicroSecond() * 0.000001;
	}
	// 获取毫秒
	double getElapseMilliSecond()
	{
		return getElapseMicroSecond() * 0.001;
	}
	// 获取微秒
	long long getElapseMicroSecond()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
	}
private:
	// 存储时钟起始时间节点
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};