#ifndef _H_GENERIC_HELPER_
#define _H_GENERIC_HELPER_

#include <string.h>

#define CHECK_RETCODE(retcode) if ( 0 != retcode ) goto _out;

typedef int BOOL;
typedef unsigned long  DWORD;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int  UINT;
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int  uint;
typedef void*         LPVOID;

#define TRUE      1
#define FALSE     0 
#define MAX_PATH  260

// 一块内存
class mem_t
{
public:
	mem_t()
	{
		_mem = NULL;
		_mem_len = 0;
		_capacity_len = 0;
	}

	mem_t(unsigned int sz)
	{
		_mem = NULL;
		_mem_len = 0;
		_capacity_len = 0;
		reserve(sz);
	}

	mem_t(const mem_t& m)
	{
		_mem = NULL;
		_mem_len = 0;
		_capacity_len = 0;
		reserve(m.get_capacity());
		cp(m.address(), m.get_capacity());
		_capacity_len = m._capacity_len;
		_mem_len = m._mem_len;
	}

	const mem_t& operator = (const mem_t& m)
	{
		delete _mem;
		_mem = NULL;
		_mem_len = 0;
		_capacity_len = 0;
		reserve(m.get_capacity());
		cp(m.address(), m.get_capacity());
		_capacity_len = m._capacity_len;
		_mem_len = m._mem_len;
		return *this;
	}

	~mem_t()
	{
		delete []_mem;
	}

	void reserve(unsigned int capacity)
	{
		if ( capacity > _capacity_len )  // 不能减少. 
		{
			unsigned char* save = _mem;
			_mem = new unsigned char[capacity];
			memset(_mem, 0, capacity);
			memcpy(_mem, save, _mem_len);
			_capacity_len = capacity;

			delete save;
		}
	}

	void resize(unsigned int size)
	{
		reserve(size);
		_mem_len = size;
	}

	// 把内存拷入.
	void cp(const unsigned char* src, int len = -1)
	{
		_mem_len = 0;
		append(src, len);
	}

	void append(const unsigned char* src, int len = -1)
	{
		if ( -1 == len )
		{
			len = strlen((char*)src);
		}

		int len_at_least = len + _mem_len;

		if ( len_at_least > _capacity_len )
		{
			reserve(len_at_least);
		}
		memcpy(_mem + _mem_len , src, len);
		_mem_len = len_at_least;
	}

	//unsigned char& operator[](unsigned int index)
	//{
	//	if ( index < _mem_len )
	//	{
	//		return _mem[index];
	//	}
	//	else
	//	{
	//		throw "index out of bounds in mem_t::operator[]";
	//	}
	//}

	unsigned char* address()const
	{
		return _mem;
	}

	int get_size()const
	{
		return _mem_len;
	}

	int get_capacity()const
	{
		return _capacity_len;
	}

	void clear()
	{
		delete _mem;
		_mem = NULL;
		_mem_len = 0;
		_capacity_len = 0;
	}

private:
	unsigned char * _mem;   // 首地址. 
	uint  _mem_len;         // 使用了多长. 
	uint  _capacity_len;    // 最大容量. 
};

class HoldTheMutex
{
public:
	explicit HoldTheMutex(HANDLE mutex)
//	HoldTheMutex(HANDLE mutex)
	{
		WaitForSingleObject(mutex, INFINITE);
		m_mutex = mutex;
		m_released = false;
	}

	~HoldTheMutex()
	{
		Release();
	}

	void Release()
	{
		if ( !m_released )  // 如果还没释放. 
		{
			ReleaseMutex(m_mutex);
			m_released = true;
		}
	}

private:
	HoldTheMutex(const HoldTheMutex&);

private:
	HANDLE m_mutex;
	BOOL   m_released;
};

#endif