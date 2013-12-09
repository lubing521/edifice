#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <WinAPIsOnUnix.h>
#include "EIBCommNEO.h"
#include "pentagon.h"

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
	
	unsigned char host_addr[2] = { '\x0a', '\x0b' };
	unsigned char peer_addr[2] = { '\x12', '\xab' };
//	unsigned char peer_addr[2] = { '\x00', '\xFF' };

	int loop_count;
	if ( argc < 2 )
		loop_count = 1;
	else
		loop_count = atoi(argv[1]);
	//int loop_count = 0xff;
	PropertyFields fields = { 0 };
	uchar buff[512];
	memset(buff, '1', sizeof(buff));
	int buff_len = sizeof(buff);
	uchar count = 3;

	//retcode = Init(4, 115200, host_addr);
	retcode = Init_udp("127.0.0.1", 1216, host_addr);
	assert(0==retcode);
	SetMonitor(lurker);
	for ( int i = 0; i < loop_count; i++)
	{
		cout << "number of times: " << i << endl;
		retcode = GroupValue_Read(peer_addr, 0, 0, 0, buff, buff_len);
		retcode = PropertyValue_Write(peer_addr, 0, 0, 0, 1, 1, count, 1, buff, 3);
		retcode = PropertyValue_Read(peer_addr, 0, 0, 0, 1, 1, count, 1, buff, buff_len);
		retcode = Property_Read(peer_addr, 0, 0, 0, 1, 1, fields);
		fields._property_id = 1;
		fields._object_index = 1;
		fields._elem_count = 0x22;
		fields._elem_type = 0x21;
		retcode = Property_Write(peer_addr, 0, 0, 0, 1, 1, fields);
		buff_len = sizeof(buff);
		retcode = IndividualAddress_Read(host_addr, 0, 0, 0, buff, buff_len);
	}

	//DevMapInfo map_info;    // 映射信息. 
	//PropertyFields prop_fields;  // 接口对象的描述
	//uchar addresses[12] = { 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06 };     // 存6个组地址

	//retcode = get_map_info(-1, map_info);     // 得到映射信息. 
	//map_info._does_group_addrs_mapped = true; // 确保有组地址的映射. 
	//map_info._group_addrs_map[0] = 0xff;      // 映射到的接口对象index
	//map_info._group_addrs_map[1] = 0xfd;      // 属性id. 
	//retcode = set_map_info(-1, map_info);     // 把更改后的映射信息写回. 
	//retcode = get_prop_fields(-1, 0xff, 0xfd, prop_fields);        // 拿到映射到的接口对象的描述. 
	//prop_fields._elem_count = 6;        // 设成要存放六个组地址. 
	//retcode = set_prop_fields(-1, 0xff, 0xfd, prop_fields);     // 把更改后的接口对象描述写回. 
	//retcode = set_elem_values(-1, 0xff, 0xfd, 1, 6, addresses, 12);  // 把6个组地址的内容写入. 

	cout << "program is about to exit." << endl;
	cin.get();

	Uninit();

	return 0;
}
