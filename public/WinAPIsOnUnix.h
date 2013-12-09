////////////////////////////////////////////////////////////////////
/// Ϊ����ֲ��Linuxƽ̨.��һЩwindows�µ�һЩAPI��Linuxƽ̨��֧�ֵĺ���ʵ��. 
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
#define SOCKET    int           // winsock ��� SOCKET ��linux��ֻ�Ǹ�int���ļ�������. 
#define INVALID_SOCKET  (~0)
#define ioctlsocket  ioctl 
#define SOCKADDR_IN  sockaddr_in  // winsock �������ṹ��ȫ��д����linuxȫСд. 
#define closesocket  close

// win�����õľ��. 
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
			// Ŀǰ��ʵ��ʵ���ϲ�û��ʹ����������������Ķ���.����SetEvent��WaitForSingleObject���ѵ��߳����ǲ�ȷ����.
			// �������ҿ�����.
			pthread_cond_t*   _cond;
		}_event;
	}_data;

	const HANDLE& operator =(void* ptr);
//	bool operator ==(void* ptr)const;
	operator const void*()const;
};

//win��CreateMutex�����ļ���ֲ�汾,����WINAPI���÷�ʽ.���԰�ȫ���Բ������������Բ���. 
//HANDLE WINAPI CreateMutex(__in_opt  LPSECURITY_ATTRIBUTES lpMutexAttributes, __in BOOL bInitialOwner, __in_opt  LPCTSTR lpName);
HANDLE CreateMutex(void*, bool initial_owner, void*);

//win��WaitForSingleObject��������ֲ�汾.����WINAPI�ĵ��÷�ʽ. 
//DWORD WINAPI WaitForSingleObject(__in  HANDLE hHandle, __in  DWORD dwMilliseconds );
#define  WAIT_ABANDONED 0x00000080L
#define  WAIT_OBJECT_0  0x00000000L
#define  WAIT_TIMEOUT   0x00000102L
#define  WAIT_FAILED    0xFFFFFFFF 
#define  INFINITE       0xFFFFFFFF
unsigned int WaitForSingleObject(HANDLE handle, unsigned int milliseconds);

//win��ReleaseMutex��������ֲ�汾.����WINAPI���÷�ʽ. 
//If the function succeeds, the return value is nonzero.
//BOOL WINAPI ReleaseMutex(HANDLE hMutex);
bool ReleaseMutex(HANDLE handle);

//win��CloseHandle��������ֲ�汾.����WINAPI���÷�ʽ. 
//BOOL WINAPI CloseHandle(__in  HANDLE hObject);
bool CloseHandle(HANDLE handle);  

//win��CreateEvent��������ֲ�汾������WINAPI���÷�ʽ.���԰�ȫ����������. 
//HANDLE WINAPI CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset,BOOL bInitialState,LPCTSTR lpName);
HANDLE CreateEvent(void*, bool manual_reset, bool initial_state, void*);

//win��SetEvent������ResetEvent��������ֲ�汾������WINAPI���÷�ʽ.
bool SetEvent(HANDLE handle);
bool ResetEvent(HANDLE handle);

//win��Sleep��������ֲ�汾.����WINAPI���÷�ʽ.(ע���һ����ĸ�Ǵ�дS.Сд��sleep��linux��Ҳ��.��������΢��.)
//void Sleep(DWORD dwMilliseconds);
void Sleep(unsigned int milliseconds);

//win��CreateThread����ֲ�汾.����WINAPI���÷�ʽ.
//���԰�ȫ������ջ��С����������ѡ��������߳�ID����. 
//HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
//LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef unsigned long (__attribute__((stdcall)) *win_thread_routine)(void* arg);
HANDLE CreateThread(void*, unsigned int, win_thread_routine rtn, void* arg, unsigned int, void*);

//win��GetTickCount(void)����ֲ�汾.����WINAPI���÷�ʽ. 
//DWORD WINAPI GetTickCount(void);
unsigned long GetTickCount(void);

//winsocket�õ��ļ������ݽṹ�ͺ�. 
struct WSADATA
{  
	// ʲô��ľ��
};
#define SOCKET_ERROR  (-1)
//win��WSAStartup��WSACleanup����ֲ�汾.����WINAPI���÷�ʽ.������������linux�汾ʵ���ϲ����κ�����.��Զ���سɹ�.�ɹ�ʱ����0. 
//int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
#define WSAStartup(first, second)    0
int WSACleanup(void);

//win��WSAGetLastError��linux�汾. 
int WSAGetLastError(void);

//win��GetLastError����ֲ�汾.
unsigned long GetLastError(void);

//win���CreateDirectory��������ֲ�汾������WINAPI���÷�ʽ.���԰�ȫ����. 
// BOOL WINAPI CreateDirectory(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
bool CreateDirectory(const char* path, void*);

#endif

#endif