#pragma once

// C++11 �߾��ȼ�ʱ��
#include<chrono>

class EfficientTimer 
{
public:
	EfficientTimer() 
	{
		// ��ʼ��ʱ��ڵ�
		updateTime();
	}
	~EfficientTimer(){}
public:
	// ����ʱ��ڵ�
	void updateTime() 
	{
		_begin = std::chrono::high_resolution_clock::now();
	}
	// ��ȡ��
	double getElapseSecond() 
	{
		return getElapseMicroSecond() * 0.000001;
	}
	// ��ȡ����
	double getElapseMilliSecond()
	{
		return getElapseMicroSecond() * 0.001;
	}
	// ��ȡ΢��
	long long getElapseMicroSecond()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
	}
private:
	// �洢ʱ����ʼʱ��ڵ�
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};