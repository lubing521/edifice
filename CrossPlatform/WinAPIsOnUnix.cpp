#ifdef __GNUG__

#include "WinAPIsOnUnix.h"
#include <assert.h>
#include <stdio.h>

//bool HANDLE::operator ==(void* ptr)const
//{
//	void* pointer = NULL;
//	if ( _type == HANDLE::HT_NULL )
//	{
//		return ptr == NULL;
//	}
//	if ( _type == HANDLE::HT_MUTEX )
//	{
//		pointer = _data._mutex; 
//	}
//	if ( 更多type ... )
//		...
//
//	return pointer == ptr;
//}

const HANDLE& HANDLE::operator =(void* ptr)
{
	if ( ptr != NULL )
	{
		throw "HANDLE::operator = is not expected to be assigned to a non-zero value";
	}

	memset(this, 0, sizeof(HANDLE));
	return *this;
}

HANDLE::operator const void*()const
{
	return &_data;
}

HANDLE CreateMutex(void*, bool initial_owner, void*)
{
	HANDLE handle;
	memset(&handle, 0, sizeof(HANDLE));

	pthread_mutex_t* mutex = new pthread_mutex_t;
	int retcode = pthread_mutex_init(mutex, NULL);
	if ( 0 != retcode )
	{
		delete mutex;
	}
	else
	{
		handle._type = HANDLE::HT_MUTEX;
		handle._data._mutex = mutex;
		if ( initial_owner )
		{
			pthread_mutex_lock(mutex);
		}
	}

	return handle;
}

unsigned int WaitForSingleObject(HANDLE handle, unsigned int milliseconds)
{
	unsigned int time_elapsed = 0;
	while ( time_elapsed < milliseconds || milliseconds == INFINITE )
	{
		if ( handle._type == HANDLE::HT_NULL )
		{
			return WAIT_FAILED;
		}
		else if ( handle._type == HANDLE::HT_MUTEX )
		{
			// 拿到锁了. 
			if ( 0 == pthread_mutex_trylock(handle._data._mutex) )
			{
				return WAIT_OBJECT_0;
			}
		}
		else if ( handle._type == HANDLE::HT_EVENT )
		{
			// 拿到锁. 
			if ( 0 == pthread_mutex_trylock(handle._data._event._mutex) )
			{
				// 已经被 signaled 啦.于是WaitForSingleObject就可以返回嘞. 
				if ( *handle._data._event._signaled )
				{
					if ( ! handle._data._event._manual_reset ) // 若需要自动Reset回到无信号状态. 
					{
						*handle._data._event._signaled = false;
					}
					pthread_mutex_unlock(handle._data._event._mutex);
					return WAIT_OBJECT_0;
				}
				else
				{
					pthread_mutex_unlock(handle._data._event._mutex);
				}
			}
		}
		else if ( handle._type == HANDLE::HT_THREAD )
		{
			void* rval_ptr;
			int retcode;
			retcode = pthread_join(*handle._data._thread, &rval_ptr);  // 一直阻塞直到线程不存在. 
			if ( retcode == 0 )
				return WAIT_OBJECT_0;
			else
				return WAIT_FAILED;
		}

		usleep(1000);  // usleep用微秒计数.所以乘以1000. 
		time_elapsed++;
	}

	// 至此已超时. 
	return WAIT_TIMEOUT;
}

bool ReleaseMutex(HANDLE handle)
{
	if ( handle._type != HANDLE::HT_MUTEX )
	{
		return -1;
	}

	int retcode = pthread_mutex_unlock(handle._data._mutex);
	if ( 0 == retcode )
	{
		return 1;  // windows的ReleaseMutex定义成成功时返回非0. 
	}
	else
	{
		return 0;
	}
}

bool CloseHandle(HANDLE handle)
{
	int retcode = -1;
	if ( handle._type == HANDLE::HT_MUTEX )
	{
		retcode = pthread_mutex_destroy(handle._data._mutex);
		delete handle._data._mutex;
	}
	else if ( handle._type == HANDLE::HT_EVENT )
	{
		int okay0, okay1;
		pthread_mutex_unlock(handle._data._event._mutex);
		okay0 = pthread_mutex_destroy(handle._data._event._mutex);
		okay1 = pthread_cond_destroy(handle._data._event._cond);
		delete handle._data._event._mutex;
		delete handle._data._event._cond;
		delete handle._data._event._signaled;
		if ( 0 == okay0 && 0 == okay1 )
			retcode = 0;
		else
			retcode = -1;
	}
	else if ( handle._type == HANDLE::HT_THREAD )
	{
		void* rval_ptr;
		pthread_cancel(*handle._data._thread);
		pthread_join(*handle._data._thread, &rval_ptr);
		delete handle._data._thread;
	}

	return !retcode;  // windows的CloseHandle定义成成功时返回非0. 
}

HANDLE CreateEvent(void*, bool manual_reset, bool initial_state, void*)
{
	int retcode;
	HANDLE handle;
	memset(&handle, 0, sizeof(HANDLE));

	pthread_mutex_t* mutex = new pthread_mutex_t;
	pthread_cond_t* cond = new pthread_cond_t;

	retcode = pthread_mutex_init(mutex, NULL);
	if ( retcode != 0 ) goto _err;
	retcode = pthread_cond_init(cond, NULL);
	if ( retcode != 0 ) goto _err;

	handle._type = HANDLE::HT_EVENT;
	handle._data._event._signaled = new bool;
	*handle._data._event._signaled = initial_state;
	handle._data._event._manual_reset = manual_reset;
	handle._data._event._mutex = mutex;
	handle._data._event._cond = cond;

	return handle;

_err:
	delete mutex;
	delete cond;
	memset(&handle, 0, sizeof(HANDLE));
	
	return handle;
}

bool SetEvent(HANDLE handle)
{
	if ( handle._type != HANDLE::HT_EVENT )
		return false;

	pthread_mutex_lock(handle._data._event._mutex);
	*handle._data._event._signaled = true;
	pthread_mutex_unlock(handle._data._event._mutex);
	
	return true;
}

bool ResetEvent(HANDLE handle)
{
	if ( handle._type != HANDLE::HT_EVENT )
		return false;

	pthread_mutex_lock(handle._data._event._mutex);
	*handle._data._event._signaled = false;
	pthread_mutex_unlock(handle._data._event._mutex);

	return true;
}

void Sleep(unsigned int milliseconds)
{
	usleep(milliseconds * 1000);
}

void* start_rtn(void* arg)
{
	// arg指向地址的第一个双字是windows线程例程的地址. 
	win_thread_routine rtn = (win_thread_routine)(*(unsigned int*)arg);
	// arg指向地址的第二个双字是windows线程例程的参数.
	void* argument = (void*)*(((unsigned int*)arg) + 1);

	return (void*)rtn(argument);  // Posix线程返回一个void*一般是一个error code如果失败的话. 
}
HANDLE CreateThread(void*, unsigned int, win_thread_routine rtn, void* arg, unsigned int, void*)
{
	int retcode;
	void* array[2];
	HANDLE handle;
	memset(&handle, 0, sizeof(HANDLE));

	pthread_t* thread = new pthread_t;

	array[0] = (void*)rtn;
	array[1] = arg;
	retcode = pthread_create(thread, NULL, start_rtn, array);
	if ( retcode != 0 ) goto _err;

	handle._type = HANDLE::HT_THREAD;
	handle._data._thread = thread;

	return handle;

_err:
	delete thread;
	memset(&handle, 0, sizeof(HANDLE));

	return handle;
}

unsigned long GetTickCount(void)
{
	int retcode;
	timespec tspec;
	unsigned long retval;
	retcode = clock_gettime(CLOCK_MONOTONIC, &tspec);
	if ( retcode == 0 )
	{
		retval = tspec.tv_sec * 1000 + tspec.tv_nsec / 1000000;
	}
	else
	{
		throw("clock_gettime is not expected to fail.");
	}
	return retval;
}

int WSACleanup()
{
	return 0;
}

static int g_wsa_last_err = 0xddddd;
int WSAGetLastError(void)
{
	return g_wsa_last_err;
}

static unsigned long g_last_err = 0xeeeee;
unsigned long GetLastError(void)
{
	return g_last_err;
}

bool CreateDirectory(const char* path, void*)
{
	int retcode = mkdir(path, 0600);
	return !retcode;
}

#endif