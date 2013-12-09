#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <map>
#include <iostream>
#include "pentagon.h"
#include "WinAPIsOnUnix.h"
#include "EIBCommNEO.h"
#include "generic_helper.h"

using namespace std;

void lurker(const uchar* buff, int len, const void* arg)
{
	char buxx[1024];
	int  buxx_len = sizeof(buxx);
	bins2hexs(buff, len, buxx, buxx_len);

	static unsigned long lurk;

	if ( ! lurk++ )
	{
		printf("######## 发送脚本指定的帧.\n");
	}

	const char* ptr = (const char*)arg;
	if ( *ptr == 't' )
		printf("发出帧 %s \n", buxx);
	else if ( *ptr == 'r' )
		printf("收到帧 %s \n", buxx);
}

int main(int argc, char* argv[])
{
	int retcode;
	uchar host_addr[2] = { 0x00, 0x01 };
	
	SetINIPath("EIBCommNEO_svr.ini");
//	retcode = Init(3, 115200, host_addr);
	retcode = Init_udp("127.0.0.1", 1215, host_addr);
	assert(0==retcode);
	SetMonitor(lurker);
	Listen_Async();
	cin.get();
	Uninit();

	return 0;
}
