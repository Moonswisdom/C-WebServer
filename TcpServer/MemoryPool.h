#pragma once
#include<iostream>
#include<mutex>
#include<memory>

#define BLOCK_BASE_SIZE 32

class MemoryPool;
// 内存块
class MemoryBlock
{
public:
	MemoryBlock();
	~MemoryBlock() {}
	// 初始化内存块
	void initBlock(int id, bool inpool, MemoryPool* Mpool, MemoryBlock* next);
	// 内存编号, 可有可无，调试用
	int _mID;
	// 是否在池内
	bool _inPool;
	// 所在的内存池
	MemoryPool* _MemPool;
	// 下一可用内存块
	MemoryBlock* _nextBlock;
};

// 内存池
class MemoryPool
{	
	// 线程锁
	std::mutex _MPmutex;
public:
	MemoryPool();
	MemoryPool(size_t num, size_t size);
	~MemoryPool();

public:
	// 申请内存
	void* mem_alloc(size_t nlen);
	// 释放内存
	void mem_free(MemoryBlock* pblock);
	// 初始化内存池
	void initMpool();
	// 扩大内存池
	void ExMpool(size_t num);
	// 减小内存池
	size_t DeMpool(size_t num);
protected:
	// 内存块最大数量
	size_t _bNum;
	// 内存块大小
	size_t _bSize;
	// 内存池首地址, 连续空间开辟用
	char* _mpBuf;
	// 可用内存块头节点
	MemoryBlock* _headBlock;
};

// 内存池实例对象，使用模板初始化
template<size_t bSize, size_t bNum>
class MPobj : public MemoryPool
{
public:
	MPobj() 
	{
		_bSize = bSize;
		_bNum = bNum;
		initMpool();
	}
	~MPobj() {}
};

// 内存池管理
class MemoryMgr
{
	// 单例模式，隐藏构造，删除拷贝
private:
	MemoryMgr();
	~MemoryMgr() {}
	MemoryMgr(const MemoryMgr& MemMgr) = delete;
	MemoryMgr operator=(const MemoryMgr& MemMgr) = delete;
public:
	// 获取单例对象
	static MemoryMgr& MemMgr();
	// 开辟空间
	void* Mgr_alloc(size_t nlen);
	// 删除空间
	void Mgr_free(void* pbuf);

	// 添加内存池
	static void addPool(size_t size, size_t num);
	// 减少内存池
	static void delPool(size_t size, size_t num);
private:
	// 内存化整，减少内存碎片
	size_t tranSize(size_t nlen);
private:
	// 管理的内存池
	MPobj<32, 10000> Mpool_32;
	MPobj<64, 10000> Mpool_64;
	MPobj<128, 5000> Mpool_128;
	MPobj<256, 1000> Mpool_256;
	MPobj<512, 500> Mpool_512;
	MPobj<1024, 100> Mpool_1024;
	// 内存池索引
	MemoryPool* _Mpools[1024 + 1];
};