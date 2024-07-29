#pragma once
#include "MemoryPool.h"

/** 
* ----- �ع�new��delete�������ڴ�� -----
*/
void* operator new(size_t nlen)
{
	return MemoryMgr::MemMgr().Mgr_alloc(nlen);
}

void operator delete(void* pbuf)
{
	MemoryMgr::MemMgr().Mgr_free(pbuf);
}

void* operator new[](size_t nlen)
{
	return MemoryMgr::MemMgr().Mgr_alloc(nlen);
}

void operator delete[](void* pbuf)
{
	MemoryMgr::MemMgr().Mgr_free(pbuf);
}