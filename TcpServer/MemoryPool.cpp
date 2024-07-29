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
    // ��ʼ���ڴ��
    initMpool();
}


MemoryPool::~MemoryPool()
{
    // ������û����ʹ�õ��ڴ��
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
    // �ж��Ƿ��ʼ��
    if (_mpBuf == nullptr)
    {
        initMpool();
    }
    MemoryBlock* outBlock = nullptr;
    if (_headBlock == nullptr)
    {
        // �ڴ��û�ռ� -> �����¿ռ�
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
        // �����пռ� -> ����ռ�
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
            // �����ڴ�� -> �������ڴ��
            pblock->_nextBlock = _headBlock;
            _headBlock = pblock;
        }
        else
        {
            // �����ڴ�� -> �ͷ�
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
    /* �����ڴ�
    * 1������һ�������ռ䣬����ͳһ�ͷţ�����������չ
    * 2�������������ռ䲢���ӣ�������չ��ɾ�������ڴ�飬�޷�ͳһ�ͷ�ȫ���ռ�
    *��������չѡ��ڶ��ַ�ʽ��������Ϊ�ͷŵĲ��ֿռ��ڽ��̽������Զ��ͷţ�
    */
    /*
    // ��һ�ַ�ʽ
    // �����ڴ�������ڴ��С
    size_t poolSize = (_bSize + sizeof(MemoryBlock)) * _bNum;
    // �����ڴ�, 
    _mpBuf = (char*)malloc(poolSize);
    // ����β�ڴ��
    _headBlock = (MemoryBlock*)_mpBuf;
    _headBlock->initBlock(0, true, this, nullptr);
    // ѭ�����������ڴ��
    for (int n = 1; n < _bNum; ++n)
    {
        MemoryBlock* tempBlock = (MemoryBlock*)(_mpBuf + n * (_bSize + sizeof(MemoryBlock)));
        tempBlock->initBlock(n, true, this, _headBlock);
        _headBlock = tempBlock;
    }
    */
    
    // �ڶ��ַ�ʽ
    // �ڴ��ʵ�ʴ�С = �ڴ��ͷ + �ڴ����ÿռ�

    size_t realSize = sizeof(MemoryBlock) + _bSize;
    // ��������Ӧ�������ڴ��
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
    // �����չ�µ��ڴ�鵽�ڴ��
    size_t realSize = sizeof(MemoryBlock) + _bSize;
    for (int n = (int)_bNum; n < _bNum + num; ++n)
    {
        MemoryBlock* tempBlock = (MemoryBlock*)malloc(realSize);
        tempBlock->initBlock(n, true, this, nullptr);
        _headBlock = tempBlock;
    }
    // �����ڴ��
    _bNum += num;
}

size_t MemoryPool::DeMpool(size_t num)
{
    std::lock_guard<std::mutex> mplock(_MPmutex);
    // ����ж�Ӧ�������ڴ�飬���ٶ�Ӧ������û����ֻ����Ŀǰ�ɼ��ٵ�
    for (size_t n = 0; n < num; ++n)
    {
        if (_headBlock)
        {
            free(_headBlock);
            _headBlock = _headBlock->_nextBlock;
        }
        else {
            // �����ڴ������
            _bNum -= n;
            // �����ͷųɹ����ڴ�����
            return n;
        }
    }
    return num;
}

MemoryMgr::MemoryMgr()
{
    // Ĭ�Ͽ���
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
    // �ж�������ڴ��С��û��
    size_t nSize = tranSize(nlen);
    if (nSize <= 1024 && _Mpools[nSize])
    {
        // �ж�Ӧ��С���ڴ�أ�����
        return _Mpools[nSize]->mem_alloc(nSize);
    }

    // û�ж�Ӧ��С���ڴ�أ�����
    MemoryBlock* newblock = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
    newblock->initBlock(-1, false, nullptr, nullptr);

    return (char*)newblock + sizeof(MemoryBlock);
}

void MemoryMgr::Mgr_free(void* pbuf)
{
    if (pbuf)
    {
        // ͨ��λƫ���ҵ��ڴ���ͷ
        MemoryBlock* freeBlock = (MemoryBlock*)((char*)pbuf - sizeof(MemoryBlock));
        if (freeBlock->_inPool)
        {
            // �����ڴ� -> �ڴ�ع���
            freeBlock->_MemPool->mem_free(freeBlock);
        }
        else
        {
            // �����ڴ� -> ֱ���ͷ�
            free(freeBlock);
        }
    }
}

void MemoryMgr::addPool(size_t size, size_t num)
{
    
    if (MemoryMgr::MemMgr()._Mpools[size])
    {
        // �ж�Ӧ��С���ڴ�� -> �ڴ������
        MemoryMgr::MemMgr()._Mpools[size]->ExMpool(num);
    }
    else
    {
        // û�ж�Ӧ��С���ڴ�� -> �¿����ڴ��
        MemoryMgr::MemMgr()._Mpools[size] = new MemoryPool(size, num);
    }
}

void MemoryMgr::delPool(size_t size, size_t num)
{
    if (size <= 1024 && MemoryMgr::MemMgr()._Mpools[size])
    {
        // �ж�Ӧ��С���ڴ�� -> �ڴ������
        MemoryMgr::MemMgr()._Mpools[size]->ExMpool(num);
    }
}

size_t MemoryMgr::tranSize(size_t nlen)
{
    // sizeof(void*) �����ֽڴ�С
    // BLOCK_BASE_SIZE 8*sizeof(void*) ����λ��
    return ((nlen - 1) / BLOCK_BASE_SIZE + 1) * BLOCK_BASE_SIZE;
}



