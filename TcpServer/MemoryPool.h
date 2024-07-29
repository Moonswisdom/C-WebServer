#pragma once
#include<iostream>
#include<mutex>
#include<memory>

#define BLOCK_BASE_SIZE 32

class MemoryPool;
// �ڴ��
class MemoryBlock
{
public:
	MemoryBlock();
	~MemoryBlock() {}
	// ��ʼ���ڴ��
	void initBlock(int id, bool inpool, MemoryPool* Mpool, MemoryBlock* next);
	// �ڴ���, ���п��ޣ�������
	int _mID;
	// �Ƿ��ڳ���
	bool _inPool;
	// ���ڵ��ڴ��
	MemoryPool* _MemPool;
	// ��һ�����ڴ��
	MemoryBlock* _nextBlock;
};

// �ڴ��
class MemoryPool
{	
	// �߳���
	std::mutex _MPmutex;
public:
	MemoryPool();
	MemoryPool(size_t num, size_t size);
	~MemoryPool();

public:
	// �����ڴ�
	void* mem_alloc(size_t nlen);
	// �ͷ��ڴ�
	void mem_free(MemoryBlock* pblock);
	// ��ʼ���ڴ��
	void initMpool();
	// �����ڴ��
	void ExMpool(size_t num);
	// ��С�ڴ��
	size_t DeMpool(size_t num);
protected:
	// �ڴ���������
	size_t _bNum;
	// �ڴ���С
	size_t _bSize;
	// �ڴ���׵�ַ, �����ռ俪����
	char* _mpBuf;
	// �����ڴ��ͷ�ڵ�
	MemoryBlock* _headBlock;
};

// �ڴ��ʵ������ʹ��ģ���ʼ��
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

// �ڴ�ع���
class MemoryMgr
{
	// ����ģʽ�����ع��죬ɾ������
private:
	MemoryMgr();
	~MemoryMgr() {}
	MemoryMgr(const MemoryMgr& MemMgr) = delete;
	MemoryMgr operator=(const MemoryMgr& MemMgr) = delete;
public:
	// ��ȡ��������
	static MemoryMgr& MemMgr();
	// ���ٿռ�
	void* Mgr_alloc(size_t nlen);
	// ɾ���ռ�
	void Mgr_free(void* pbuf);

	// ����ڴ��
	static void addPool(size_t size, size_t num);
	// �����ڴ��
	static void delPool(size_t size, size_t num);
private:
	// �ڴ滯���������ڴ���Ƭ
	size_t tranSize(size_t nlen);
private:
	// ������ڴ��
	MPobj<32, 10000> Mpool_32;
	MPobj<64, 10000> Mpool_64;
	MPobj<128, 5000> Mpool_128;
	MPobj<256, 1000> Mpool_256;
	MPobj<512, 500> Mpool_512;
	MPobj<1024, 100> Mpool_1024;
	// �ڴ������
	MemoryPool* _Mpools[1024 + 1];
};