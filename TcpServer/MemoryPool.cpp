#include "MemoryPool.h"

MemoryBlock::MemoryBlock()
{
    _mID = 0;
    _inPool = false;
    _MemPool = nullptr;
    _nextBlock = nullptr;
}

void MemoryBlock::initBlock(int id, bool inpool, MemoryPool* Mpool, MemoryBlock* next)
{
    _mID = id;
    _inPool = inpool;
    _MemPool = Mpool;
    _nextBlock = next;
}

MemoryPool::MemoryPool()
{
    _bNum = 0;
    _bSize = 0;
    _mpBuf = nullptr;
    _headBlock = nullptr;
    //initMpool();
}


MemoryPool::MemoryPool(size_t num, size_t size)
{
    _bNum = num;
    _bSize = size;
    _mpBuf = nullptr;
    _headBlock = nullptr;
    // 初始化内存池
    initMpool();
}


MemoryPool::~MemoryPool()
{
    // 仅析构没有在使用的内存块
    std::lock_guard<std::mutex> mplock(_MPmutex);
    while (_headBlock)
    {
        MemoryBlock* temp = _headBlock;
        _headBlock = _headBlock->_nextBlock;
        free(temp);
    }

    /*
    if (_mpBuf)
    {
        free(_mpBuf);
    }
    */
}

void* MemoryPool::mem_alloc(size_t nlen)
{
    std::lock_guard<std::mutex> mplock(_MPmutex);
    // 判断是否初始化
    if (_mpBuf == nullptr)
    {
        initMpool();
    }
    MemoryBlock* outBlock = nullptr;
    if (_headBlock == nullptr)
    {
        // 内存池没空间 -> 申请新空间
        outBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock) + _bSize);
        if (outBlock)
        {
            outBlock->initBlock(-1, false, nullptr, nullptr);
            //outBlock->_mID = -1;
            //outBlock->_inPool = false;
            //outBlock->_MemPool = nullptr;
            //outBlock->_nextBlock = nullptr;
        }
    }
    else
    {
        // 池内有空间 -> 分配空间
        outBlock = this->_headBlock;
        this->_headBlock = this->_headBlock->_nextBlock;
    }

    return (char*)outBlock + sizeof(MemoryBlock);
}

void MemoryPool::mem_free(MemoryBlock* pblock)
{
    //std::lock_guard<std::mutex> mplock(_MPmutex);
    if (pblock)
    {
        if (pblock->_inPool)
        {
            // 池内内存块 -> 返还给内存池
            pblock->_nextBlock = _headBlock;
            _headBlock = pblock;
        }
        else
        {
            // 池外内存块 -> 释放
            free(pblock);
        }
    }
}

void MemoryPool::initMpool()
{
    if (_mpBuf)
    {
        return;
    }
    /* 申请内存
    * 1、申请一块连续空间，方便统一释放，但不方便扩展
    * 2、按块申请多个空间并连接，方便扩展和删除部分内存块，无法统一释放全部空间
    *（方便扩展选择第二种方式，难以人为释放的部分空间在进程结束会自动释放）
    */
    /*
    // 第一种方式
    // 计算内存池所需内存大小
    size_t poolSize = (_bSize + sizeof(MemoryBlock)) * _bNum;
    // 申请内存, 
    _mpBuf = (char*)malloc(poolSize);
    // 创建尾内存块
    _headBlock = (MemoryBlock*)_mpBuf;
    _headBlock->initBlock(0, true, this, nullptr);
    // 循环创建其它内存块
    for (int n = 1; n < _bNum; ++n)
    {
        MemoryBlock* tempBlock = (MemoryBlock*)(_mpBuf + n * (_bSize + sizeof(MemoryBlock)));
        tempBlock->initBlock(n, true, this, _headBlock);
        _headBlock = tempBlock;
    }
    */
    
    // 第二种方式
    // 内存块实际大小 = 内存块头 + 内存块可用空间

    size_t realSize = sizeof(MemoryBlock) + _bSize;
    // 逐个申请对应数量的内存块
    _mpBuf = (char*)malloc(realSize);
    _headBlock = (MemoryBlock*)_mpBuf;
    if (_headBlock)
    {
        _headBlock->initBlock(-1, false, nullptr, nullptr);
        //_headBlock->_mID = 0;
        //_headBlock->_inPool = true;
        //_headBlock->_MemPool = this;
        //_headBlock->_nextBlock = nullptr;
    }
    //_headBlock->initBlock(0, true, this, nullptr);
    for (int n = 1; n < _bNum; ++n)
    {
        MemoryBlock* tempBlock = (MemoryBlock*)malloc(realSize);
        tempBlock->initBlock(n, true, this, nullptr);
        _headBlock = tempBlock;
    }
    
}

void MemoryPool::ExMpool(size_t num)
{
    std::lock_guard<std::mutex> mplock(_MPmutex);
    // 逐个扩展新的内存块到内存池
    size_t realSize = sizeof(MemoryBlock) + _bSize;
    for (int n = (int)_bNum; n < _bNum + num; ++n)
    {
        MemoryBlock* tempBlock = (MemoryBlock*)malloc(realSize);
        tempBlock->initBlock(n, true, this, nullptr);
        _headBlock = tempBlock;
    }
    // 更新内存块
    _bNum += num;
}

size_t MemoryPool::DeMpool(size_t num)
{
    std::lock_guard<std::mutex> mplock(_MPmutex);
    // 如果有对应数量的内存块，减少对应数量，没有则只减少目前可减少的
    for (size_t n = 0; n < num; ++n)
    {
        if (_headBlock)
        {
            free(_headBlock);
            _headBlock = _headBlock->_nextBlock;
        }
        else {
            // 更新内存块数量
            _bNum -= n;
            // 返回释放成功的内存块个数
            return n;
        }
    }
    return num;
}

MemoryMgr::MemoryMgr()
{
    // 默认开辟
    _Mpools[32] = (MemoryPool*)(&Mpool_32);
    _Mpools[64] = (MemoryPool*)(&Mpool_64);
    _Mpools[128] = (MemoryPool*)(&Mpool_128);
    _Mpools[256] = (MemoryPool*)(&Mpool_256);
    _Mpools[512] = (MemoryPool*)(&Mpool_512);
    _Mpools[1024] = (MemoryPool*)(&Mpool_1024);
}

MemoryMgr& MemoryMgr::MemMgr()
{
    static MemoryMgr MemMgrobj;
    return MemMgrobj;
}

void* MemoryMgr::Mgr_alloc(size_t nlen)
{
    // 判断申请的内存大小有没有
    size_t nSize = tranSize(nlen);
    if (nSize <= 1024 && _Mpools[nSize])
    {
        // 有对应大小的内存池，申请
        return _Mpools[nSize]->mem_alloc(nSize);
    }

    // 没有对应大小的内存池，开辟
    MemoryBlock* newblock = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
    newblock->initBlock(-1, false, nullptr, nullptr);

    return (char*)newblock + sizeof(MemoryBlock);
}

void MemoryMgr::Mgr_free(void* pbuf)
{
    if (pbuf)
    {
        // 通过位偏移找到内存块标头
        MemoryBlock* freeBlock = (MemoryBlock*)((char*)pbuf - sizeof(MemoryBlock));
        if (freeBlock->_inPool)
        {
            // 池内内存 -> 内存池管理
            freeBlock->_MemPool->mem_free(freeBlock);
        }
        else
        {
            // 池外内存 -> 直接释放
            free(freeBlock);
        }
    }
}

void MemoryMgr::addPool(size_t size, size_t num)
{
    
    if (MemoryMgr::MemMgr()._Mpools[size])
    {
        // 有对应大小的内存池 -> 内存池扩大
        MemoryMgr::MemMgr()._Mpools[size]->ExMpool(num);
    }
    else
    {
        // 没有对应大小的内存池 -> 新开辟内存池
        MemoryMgr::MemMgr()._Mpools[size] = new MemoryPool(size, num);
    }
}

void MemoryMgr::delPool(size_t size, size_t num)
{
    if (size <= 1024 && MemoryMgr::MemMgr()._Mpools[size])
    {
        // 有对应大小的内存池 -> 内存池扩大
        MemoryMgr::MemMgr()._Mpools[size]->ExMpool(num);
    }
}

size_t MemoryMgr::tranSize(size_t nlen)
{
    // sizeof(void*) 对齐字节大小
    // BLOCK_BASE_SIZE 8*sizeof(void*) 机器位长
    return ((nlen - 1) / BLOCK_BASE_SIZE + 1) * BLOCK_BASE_SIZE;
}



