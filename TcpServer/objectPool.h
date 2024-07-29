#pragma once
#include<iostream>
#include<mutex>

template<typename oType, size_t oNum>
class objectPool
{
public:
	struct object
	{
		void initobj(bool inpool, object* next)
		{
			_inPool = inpool;
			_nextobj = next;
		}
		// 是否在池种
		bool _inPool;
		// 下一可用对象
		object* _nextobj;
	};
public:
	// 开辟对象
	void* obj_alloc(size_t len);
	// 释放对象
	void obj_free(void* pbuf);
	// 初始化池
	void initoPool();
public:
	objectPool();
	~objectPool();
private:
	// 对象池大小
	size_t _oNum;
	// 对象池首地址
	char* _oBuf;
	// 对象池首节点
	object* _headobj;
	// 线程锁
	std::mutex _omutex;
};


template<typename oType, size_t oNum>
class objMgr
{
private:
	// 单例对象池管理
	objMgr() {}
	~objMgr() {}
	objMgr(const objMgr& objM) = delete;
	objMgr operator=(const objMgr& objM) = delete;
public:
	// 重构new delete操作
	void* operator new(size_t nlen)
	{
		return objM()->obj_alloc(nlen);
	}

	void operator delete(void* pbuf)
	{
		objM()->obj_free(pbuf);
	}

public:
	// 开辟对象
	template<typename ... Args>
	static oType* newobj(Args ... args)
	{
		return new oType(args...);
	}
	// 释放对象
	static void delobj(oType* obj)
	{
		delete obj;
	}
private:
	// 初始化对象池对象
	typedef objectPool<oType, oNum> classTypePool;
	static classTypePool& objM()
	{
		static classTypePool oTpool;
		return oTpool;
	}
};



/**
* ----- 模板函数实现 -----
*/
template<typename oType, size_t oNum>
inline objectPool<oType, oNum>::objectPool()
{
	_oNum = oNum;
	initoPool();
}

template<typename oType, size_t oNum>
inline objectPool<oType, oNum>::~objectPool()
{
	if (_oBuf)
	{
		free(_oBuf);
	}
}

template<typename oType, size_t _oNum>
inline void* objectPool<oType, _oNum>::obj_alloc(size_t len)
{
	std::lock_guard<std::mutex> olock(_omutex);
	if (_oBuf == nullptr)
	{
		initoPool();
	}

	object* newobj = nullptr;
	if (_headobj == nullptr)
	{
		// 对象池没空间 -> 新开辟
		newobj = (oType*)malloc(sizeof(oType) + sizeof(object));
		newobj->initobj(false, nullptr);
	}
	else
	{
		// 对象池有空间 -> 分配
		newobj = _headobj;
		_headobj = _headobj->_nextobj;
	}
	return (char*)newobj + sizeof(object);
}

template<typename oType, size_t _oNum>
inline void objectPool<oType, _oNum>::obj_free(void* pbuf)
{
	object* freeobj = (object*)(pbuf - sizeof(object));
	if (freeobj->_inPool)
	{
		std::lock_guard<std::mutex> olock(_omutex);
		freeobj->_nextobj = _headobj;
		_headobj = freeobj;
	}
	else
	{
		free(freeobj);
	}
}

template<typename oType, size_t oNum>
inline void objectPool<oType, oNum>::initoPool()
{
	if (_oBuf)
	{
		return;
	}

	// 计算对象块大小
	size_t realSize = sizeof(object) + sizeof(oType);
	_oBuf = (char*)malloc(realSize * _oNum);
	// 初始化对象节点
	_headobj = (object*)_oBuf;
	_headobj->initobj(true, nullptr);
	for (int i = 1; i < _oNum; ++i)
	{
		object* newobj = (object*)(_oBuf + i * realSize);
		newobj->initobj(true, _headobj);
		_headobj = newobj;
	}
}