////////////////////////////////////////////////////////////////////
/// 为了移植到Linux平台.把一些windows下的一些API用Linux平台下支持的函数实现. 
//////////////////////////////////////////////////////////////////////////////////

#ifndef _H_WINAPISONUNIX_
#define _H_WINAPISONUNIX_

#ifdef __GNUG__

#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define APIENTRY  __attribute__((stdcall))
#define WINAPI    __attribute__((stdcall))
#define __stdcall __attribute__((stdcall))
#define HMODULE   void* 
#define strtok_s  strtok_r
#define SOCKET    int           // winsock 里的 SOCKET 在linux下只是个int作文件描述符. 
#define INVALID_SOCKET  (~0)
#define ioctlsocket  ioctl 
#define SOCKADDR_IN  sockaddr_in  // winsock 里的这个结构体全大写，在linux全小写. 
#define closesocket  close

// win经常用的句柄. 
struct HANDLE
{
	enum HandleType { HT_NULL = 0, HT_MUTEX, HT_EVENT, HT_THREAD };
	HandleType  _type;

	union
	{
		pthread_mutex_t*  _mutex;
		pthread_t* _thread;
		struct
		{
			bool*  _signaled;
			bool  _manual_reset;
			pthread_mutex_t*  _mutex;
			// 目前的实现实际上并没有使用这个叫条件变量的东东.于是SetEvent后被WaitForSingleObject唤醒的线程数是不确定的.
			// 但是暂且可用嘛.
			pthread_cond_t*   _cond;
		}_event;
	}_data;

	const HANDLE& operator =(void* ptr);
//	bool operator ==(void* ptr)const;
	operator const void*()const;
};

//win的CreateMutex函数的简单移植版本,忽略WINAPI调用方式.忽略安全属性参数和命名属性参数. 
//HANDLE WINAPI CreateMutex(__in_opt  LPSECURITY_ATTRIBUTES lpMutexAttributes, __in BOOL bInitialOwner, __in_opt  LPCTSTR lpName);
HANDLE CreateMutex(void*, bool initial_owner, void*);

//win的WaitForSingleObject函数的移植版本.忽略WINAPI的调用方式. 
//DWORD WINAPI WaitForSingleObject(__in  HANDLE hHandle, __in  DWORD dwMilliseconds );
#define  WAIT_ABANDONED 0x00000080L
#define  WAIT_OBJECT_0  0x00000000L
#define  WAIT_TIMEOUT   0x00000102L
#define  WAIT_FAILED    0xFFFFFFFF 
#define  INFINITE       0xFFFFFFFF
unsigned int WaitForSingleObject(HANDLE handle, unsigned int milliseconds);

//win的ReleaseMutex函数的移植版本.忽略WINAPI调用方式. 
//If the function succeeds, the return value is nonzero.
//BOOL WINAPI ReleaseMutex(HANDLE hMutex);
bool ReleaseMutex(HANDLE handle);

//win的CloseHandle函数的移植版本.忽略WINAPI调用方式. 
//BOOL WINAPI CloseHandle(__in  HANDLE hObject);
bool CloseHandle(HANDLE handle);  

//win的CreateEvent函数的移植版本，忽略WINAPI调用方式.忽略安全参数和名字. 
//HANDLE WINAPI CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset,BOOL bInitialState,LPCTSTR lpName);
HANDLE CreateEvent(void*, bool manual_reset, bool initial_state, void*);

//win的SetEvent函数和ResetEvent函数的移植版本，忽略WINAPI调用方式.
bool SetEvent(HANDLE handle);
bool ResetEvent(HANDLE handle);

//win的Sleep函数的移植版本.忽略WINAPI调用方式.(注意第一个字母是大写S.小写的sleep在linux下也有.但参数是微秒.)
//void Sleep(DWORD dwMilliseconds);
void Sleep(unsigned int milliseconds);

//win的CreateThread的移植版本.忽略WINAPI调用方式.
//忽略安全参数，栈大小参数，创建选项参数，线程ID参数. 
//HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
//LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef unsigned long (__attribute__((stdcall)) *win_thread_routine)(void* arg);
HANDLE CreateThread(void*, unsigned int, win_thread_routine rtn, void* arg, unsigned int, void*);

//win的GetTickCount(void)的移植版本.忽略WINAPI调用方式. 
//DWORD WINAPI GetTickCount(void);
unsigned long GetTickCount(void);

//winsocket用到的几个数据结构和宏. 
struct WSADATA
{  
	// 什么都木有
};
#define SOCKET_ERROR  (-1)
//win的WSAStartup和WSACleanup的移植版本.忽略WINAPI调用方式.这两个函数的linux版本实际上不做任何事情.永远返回成功.成功时返回0. 
//int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
#define WSAStartup(first, second)    0
int WSACleanup(void);

//win的WSAGetLastError的linux版本. 
int WSAGetLastError(void);

//win的GetLastError的移植版本.
unsigned long GetLastError(void);

//win版的CreateDirectory函数的移植版本，忽略WINAPI调用方式.忽略安全属性. 
// BOOL WINAPI CreateDirectory(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
bool CreateDirectory(const char* path, void*);

#endif

#endif