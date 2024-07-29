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
		// �Ƿ��ڳ���
		bool _inPool;
		// ��һ���ö���
		object* _nextobj;
	};
public:
	// ���ٶ���
	void* obj_alloc(size_t len);
	// �ͷŶ���
	void obj_free(void* pbuf);
	// ��ʼ����
	void initoPool();
public:
	objectPool();
	~objectPool();
private:
	// ����ش�С
	size_t _oNum;
	// ������׵�ַ
	char* _oBuf;
	// ������׽ڵ�
	object* _headobj;
	// �߳���
	std::mutex _omutex;
};


template<typename oType, size_t oNum>
class objMgr
{
private:
	// ��������ع���
	objMgr() {}
	~objMgr() {}
	objMgr(const objMgr& objM) = delete;
	objMgr operator=(const objMgr& objM) = delete;
public:
	// �ع�new delete����
	void* operator new(size_t nlen)
	{
		return objM()->obj_alloc(nlen);
	}

	void operator delete(void* pbuf)
	{
		objM()->obj_free(pbuf);
	}

public:
	// ���ٶ���
	template<typename ... Args>
	static oType* newobj(Args ... args)
	{
		return new oType(args...);
	}
	// �ͷŶ���
	static void delobj(oType* obj)
	{
		delete obj;
	}
private:
	// ��ʼ������ض���
	typedef objectPool<oType, oNum> classTypePool;
	static classTypePool& objM()
	{
		static classTypePool oTpool;
		return oTpool;
	}
};



/**
* ----- ģ�庯��ʵ�� -----
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
		// �����û�ռ� -> �¿���
		newobj = (oType*)malloc(sizeof(oType) + sizeof(object));
		newobj->initobj(false, nullptr);
	}
	else
	{
		// ������пռ� -> ����
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

	// ���������С
	size_t realSize = sizeof(object) + sizeof(oType);
	_oBuf = (char*)malloc(realSize * _oNum);
	// ��ʼ������ڵ�
	_headobj = (object*)_oBuf;
	_headobj->initobj(true, nullptr);
	for (int i = 1; i < _oNum; ++i)
	{
		object* newobj = (object*)(_oBuf + i * realSize);
		newobj->initobj(true, _headobj);
		_headobj = newobj;
	}
}