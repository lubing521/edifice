// EIBCommNEO.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "EIBCommNEO.h"
#include "CommBase.h"
#include "EIBEmulatorImpl.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

static std::auto_ptr<EIBEmulatorImpl> g_eib_emulator(new EIBEmulatorImpl);
static unsigned char g_host_addr[2];

Frame* get_a_sample_frame()
{
	static const char* hex_frame = "3070000000FF1202C902042110010008000100FFFF1810000051";
	static const int hex_frame_len = strlen(hex_frame);
	static unsigned char* frame_mem = new uchar[hex_frame_len/2];
	int frame_len = hex_frame_len /2;
	hexs2bins(hex_frame, frame_mem, &frame_len);

	Frame* frame = Frame::BuildFrameFromBuff(frame_mem, frame_len, FALSE);
	return frame;
}

void SetINIPath(const char* ini_path)
{
	g_myconfig.Init(ini_path);
}

int Init(int port, int baudrate, const unsigned char* hostaddr)
{
	return Init_sp(port, baudrate, hostaddr);
}

int Init_sp(int port, int baudrate, const unsigned char* hostaddr)
{
	int retcode = -1;

	if ( NULL != hostaddr )
	{
		InitData initdata;
		initdata._serial_comm_initdata._port = port;
		initdata._serial_comm_initdata._baudrate = baudrate;
		initdata._ct = CT_SERIAL_PORT;
		retcode = g_eib_emulator->Init(initdata);
		g_host_addr[0] = hostaddr[0];
		g_host_addr[1] = hostaddr[1];
	}

	return retcode;
}

int Init_udp(const char* ip, ushort port, const unsigned char* hostaddr)
{
	if ( ip == NULL || hostaddr == NULL )
	{
		return -1;
	}

	int retcode;
	InitData initdata;
	initdata._udp_comm_initdata._bound_ip = inet_addr(ip);
	initdata._udp_comm_initdata._port = htons(port);
	initdata._ct = CT_UDP;
	retcode = g_eib_emulator->Init(initdata);
	g_host_addr[0] = hostaddr[0];
	g_host_addr[1] = hostaddr[1];
	return retcode;
}
 
int Uninit()
{
	int retcode = -1;
	retcode = g_eib_emulator->Uninit();
	return retcode;
}

void SetHostAddr(const unsigned char* hostaddr)
{
	if ( NULL != hostaddr )
	{
		g_host_addr[0] = hostaddr[0];
		g_host_addr[1] = hostaddr[1];
	}
}

int QueryAPDUTypesInRxQueue(APDUType types[], int& sz)
{
	int retcode;
	retcode = g_eib_emulator->QueryAPDUTypesInRxQueue(types, sz);
	return retcode;
}

/// 检索接收列里所有帧.返回一个FramePeek结构体的数组.
/// filter 可以用 APDUType 的类型指定你需要哪种帧.如果filter是 AT_UNAVAILABLE则返回所有的帧.
/// 需要配合 CleanFramePeek 释放调用这次函数分配的内存. 
/// 成功时返回0.失败时返回错误码.
#define FRAME_PEEKER_BUSY -1      // 在一小段时间内没有竞争到互斥锁则认为BUSY.
int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count)
{
	int retcode;
	retcode = g_eib_emulator->FramePeeker(filter, frame_peeks, count);
	return retcode;
}

/// 把 FramePeeker 函数返回的 frames 的地址传入.释放资源. 
/// 成功时返回0.失败时返回非0. 
int ClearFramePeek(void* addr)
{
	int retcode;
	retcode = g_eib_emulator->ClearFramePeek(addr);
	return retcode;
}

int Parse_GroupValue_Write(const FramePeek& peek, UCHAR* buff, USHORT& data_len_in_bit)
{
	int retcode;
	APDU_GroupValue_Write* apdu_gv_w;
	APDU* apdu = APDU::BuildAPDUFromFrame(*(const Frame*)(peek._frame_handle));
	if ( NULL == apdu )     // 从Frame里不能拿到 apdu.
	{
		retcode = -1;
		goto _out;
	}

	apdu_gv_w = dynamic_cast<APDU_GroupValue_Write*>(apdu);
	if ( NULL == apdu_gv_w )  // 不是一个 APDU_GroupValue_Write 的 apdu.
	{
		retcode = -1;
		goto _out;
	}

	retcode = apdu_gv_w->GetParams(buff, data_len_in_bit);

_out:
	delete apdu;
	return retcode;
}

Reporter SetMonitor(Reporter r)
{
	return g_eib_emulator->SetMonitor(r);
}

int PropertyValue_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type,
						unsigned char object_index, unsigned char property_id, unsigned char& count, 
						unsigned short start, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->PropertyValue_Read(g_host_addr, peer, priority, object_index, property_id, 
		count, start, buff, buff_len);
	return retcode;
}

int PropertyValue_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
						 unsigned char object_index, unsigned char property_id, unsigned char count, 
						 unsigned short start, const unsigned char* buff, int buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->PropertyValue_Write(g_host_addr, peer, priority, object_index, property_id, 
		count, start, buff, buff_len);
	return retcode;
}

int PropertyValue_Read2(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type,
						 unsigned char object_index, unsigned char property_id, unsigned short start, 
						 unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->PropertyValue_Read2(g_host_addr, peer, priority, object_index, property_id, 
		start, buff, buff_len);
	return retcode;
}

int PropertyValue_Write2(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
						  unsigned char object_index, unsigned char property_id, unsigned short start, 
						  const unsigned char* buff, int buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->PropertyValue_Write2(g_host_addr, peer, priority, object_index, property_id, start, buff, buff_len);
	return retcode;
}

int Property_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
				   unsigned char object_index, unsigned char property_id, PropertyFields& prop_fields)
{
	int retcode = -1;
	retcode = g_eib_emulator->Property_Read(g_host_addr, peer, priority, object_index, property_id, prop_fields);
	return retcode;
}

int Property_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
					unsigned char object_index, unsigned char property_id, const PropertyFields& prop_fields)
{
	int retcode = -1;
	retcode = g_eib_emulator->Property_Write(g_host_addr, peer, priority, object_index, property_id, prop_fields);
	return retcode;
}

int Authorize_Query(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
					 const unsigned char* key, unsigned char* level)
{
	int retcode = -1;
	retcode = g_eib_emulator->Authorize_Query(g_host_addr, peer, priority, key, level);
	return retcode;
}

int GroupValue_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
					 unsigned char* buff, int& buff_len )
{
	int retcode = -1;
	retcode = g_eib_emulator->GroupValue_Read(g_host_addr, peer, priority, buff, buff_len);
	return retcode;
}

int GroupValue_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
					  const unsigned char* buff, int buff_len_in_bit)
{
	int retcode = -1;
	retcode = g_eib_emulator->GroupValue_Write(g_host_addr, peer, priority, buff, buff_len_in_bit);
	return retcode;
}

int Link_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
			   unsigned char group_obj_num, unsigned char start_index, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->Link_Read(g_host_addr, peer, priority, group_obj_num, start_index, buff, buff_len);
	return retcode;
}

int Link_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, unsigned char hop_count_type, 
				unsigned char group_obj_num, unsigned char flags, const unsigned char* group_addr)
{
	int retcode = -1;
	retcode = g_eib_emulator->Link_Write(g_host_addr, peer, priority, group_obj_num, flags, group_addr);
	return retcode;
}

int UserMemory_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority,
					 unsigned char hop_count_type, unsigned int address, unsigned char number, unsigned char* buff)
{
	int retcode = -1;
	retcode = g_eib_emulator->UserMemory_Read(g_host_addr, peer, priority, address, number, buff);
	return retcode;
}

int UserMemory_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority,
					  unsigned char hop_count_type, unsigned int address, unsigned char number, unsigned char* buff)
{
	int retcode = -1;
	retcode = g_eib_emulator->UserMemory_Write(g_host_addr, peer, priority, address, number, buff);
	return retcode;
}

int IndividualAddress_Read(const unsigned char* host, unsigned char ack_request, 
							unsigned char priority, unsigned char hop_count_type, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->IndividualAddress_Read(host, priority, buff, buff_len);
	return retcode;
}


int IndividualAddress_Write(const unsigned char* host, unsigned char ack_request, 
							 unsigned char priority, unsigned char hop_count_type, const unsigned char* new_address)
{
	int retcode = -1;
	retcode = g_eib_emulator->IndividualAddress_Write(host, priority, new_address);
	return retcode;
}

int IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char ack_request, 
										unsigned char priority, unsigned char hop_count_type,
										unsigned char* serial_num, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->IndividualAddressSerialNumber_Read(host, priority, serial_num, buff, buff_len);
	return retcode;
}


int IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char ack_request,
										 unsigned char priority, unsigned char hop_count_type, 
										 const unsigned char* serial_num, const unsigned char* new_addr)
{
	int retcode = -1;
	retcode = g_eib_emulator->IndividualAddressSerialNumber_Write(host, priority, serial_num, new_addr);
	return retcode;
}

int Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, int& recv_buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->Send_Rawdate(raw_data, raw_data_len, recv_buff, recv_buff_len);
	return retcode;
}

int Listen_Async()
{
	int retcode = -1;
	retcode = g_eib_emulator->Listen_Async();
	return retcode;
}

int Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len, uint waiting_time)
{
	int retcode = -1;
	retcode = g_eib_emulator->Listen_Sync(recv_buff, recv_buff_len, send_buff, send_buff_len, waiting_time);
	return retcode;
}

int WriteGrpVal_FrameParser(unsigned char* frame, int frame_len, unsigned char* grp_addr, unsigned char* buff, unsigned short& buff_len)
{
	int retcode = -1;
	Frame* verified_frame = NULL;
	APDU*  apdu = NULL;
	BOOL check_address = g_myconfig.DoesRxNeedCheckAddress();
	APDU_GroupValue_Write* apdu_gv_w;

	verified_frame = Frame::BuildFrameFromBuff(frame, frame_len, TRUE, check_address);
	//recved = TRUE;
	if ( NULL == verified_frame )
	{
		verified_frame = Frame::BuildFrameFromBuff(frame, frame_len, FALSE, check_address);
		//recved = FALSE;
	}
	if ( NULL == verified_frame )
	{
		goto _out;
	}
	apdu = APDU::BuildAPDUFromFrame(*verified_frame);
	if ( NULL == apdu )
	{
		goto _out;
	}
	apdu_gv_w = dynamic_cast<APDU_GroupValue_Write*>(apdu);
	if ( NULL == apdu_gv_w )
	{
		goto _out;
	}
	retcode = apdu_gv_w->GetParams(buff, buff_len); // 返回的长度是以bit为单位.而又没办法精确到6bits以下. 
	if ( 0 != retcode )
	{
		goto _out;
	}
	buff_len /= 8;   // 干脆转成byte单位.
	if ( 0 == buff_len ) buff_len = 1;  // 调用者清楚这个组地址里放的组对象的类型. 

	if ( NULL != grp_addr)
	{
		memcpy(grp_addr, verified_frame->GetPeerAddr(), 2);
	}

	retcode = 0;

_out:
	delete apdu;
	delete verified_frame;

	return retcode;
}

int WriteIObj_FrameParser(unsigned char* frame, int frame_len, unsigned char* from,
						   unsigned char* to, unsigned char* object_index, unsigned char* prop_id,
						   unsigned char* count, unsigned short* start, 
						   unsigned char* buff, int* buff_len)
{
	int retcode = -1;
	Frame* verified_frame = NULL;
	APDU*  apdu = NULL;
	BOOL check_address = g_myconfig.DoesRxNeedCheckAddress();
	APDU_PropertyValue_Write* apdu_pv_w;

	verified_frame = Frame::BuildFrameFromBuff(frame, frame_len, TRUE, check_address);// TRUE表示接收的frame
	//recved = TRUE;
	if ( NULL == verified_frame )
	{
		verified_frame = Frame::BuildFrameFromBuff(frame, frame_len, FALSE, check_address); // FALSE表示发送的frame.
		//recved = FALSE;
	}
	if ( NULL == verified_frame )
	{
		goto _out;
	}
	apdu = APDU::BuildAPDUFromFrame(*verified_frame);
	if ( NULL == apdu )
	{
		goto _out;
	}
	apdu_pv_w = dynamic_cast<APDU_PropertyValue_Write*>(apdu);
	if ( NULL == apdu_pv_w )
	{
		goto _out;
	}
	retcode = apdu_pv_w->GetParams(object_index, prop_id, count, start, buff, buff_len);
	if ( 0 != retcode )
	{
		goto _out;
	}

	if ( NULL != frame )
	{
		memcpy(from, verified_frame->GetHostAddr(), 2);
	}

	if ( NULL != to )
	{
		memcpy(to, verified_frame->GetPeerAddr(), 2);
	}

_out:
	delete apdu;
	delete verified_frame;

	return retcode;
}

/////////////// 管理软设备的函数 //////////////
unsigned int make_dev(const char* template_file)
{
	int retcode = -1;
	retcode = g_eib_emulator->make_dev(template_file);
	return retcode;
}

int remove_dev(unsigned int dev_id)
{
	int retcode = -1;
	retcode = g_eib_emulator->remove_dev(dev_id);
	return retcode;
}

int get_all_devs(unsigned int* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_all_devs(buff, buff_len);
	return retcode;
}

int get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_all_idpairs(dev_id, buff, buff_len);
	return retcode;
}

int set_host_addr(unsigned int dev_id, const unsigned char* hostaddr)
{
	int retcode = -1;
	retcode = g_eib_emulator->set_host_addr(dev_id, hostaddr);
	return retcode;
}

int get_host_addr(unsigned int dev_id, char* addr)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_host_addr(dev_id, addr);
	return retcode;
}

int set_map_info(unsigned int dev_id, const DevMapInfo& map_info)
{
	int retcode = -1;
	retcode = g_eib_emulator->set_map_info(dev_id, map_info);
	return retcode;
}

int get_map_info(unsigned int dev_id, DevMapInfo& map_info)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_map_info(dev_id, map_info);
	return retcode;
}

int get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_elemlen_inbit(dev_id, object_index, prop_id);
	return retcode;
}

int get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_elemcount(dev_id, object_index, prop_id);
	return retcode;
}

int get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, PropertyFields& fields)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_prop_fields(dev_id, object_index, prop_id, fields);
	return retcode;
}

int set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields)
{
	int retcode = -1;
	retcode = g_eib_emulator->set_prop_fields(dev_id, object_index, prop_id, fields);
	return retcode;
}

int add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields)
{
	int retcode = -1;
	retcode = g_eib_emulator->add_prop_fields(dev_id, object_index, prop_id, fields);
	return retcode;
}

int get_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
					unsigned char count, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_elem_values(dev_id, object_index, prop_id, start_index, count, buff, buff_len);
	return retcode;
}

int set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
					unsigned char count, unsigned char* buff, int buff_len)
{
	int retcode = -1;
	retcode = g_eib_emulator->set_elem_values(dev_id, object_index, prop_id, start_index, count, buff, buff_len);
	return retcode;
}

int get_gobj_count(unsigned int dev_id)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_gobj_count(dev_id);
	return retcode;
}

// 不能删组对象. 
//int rm_gobj_byidx(unsigned int dev_id, unsigned char index)
//{
//	int retcode = -1;
//	retcode = g_eib_emulator->rm_gobj_byidx(dev_id, index);
//	return retcode;
//}

int get_gobj_valuelen_inbit(unsigned int dev_id, unsigned char index)
{
	int retcode = -1;
	GroupObjectHead gobjh;
	retcode = g_eib_emulator->get_gobjhead_byidx(dev_id, index, gobjh);
	if ( 0 == retcode )
	{
		retcode = gobjh.GetLenInBit();
	}
	return retcode;
}

int get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len)
{
	int retcode = -1;
	retcode = g_eib_emulator->get_gobj_value(dev_id, index, buff, len);
	return retcode;
}

int set_gobj_value_byindex(unsigned int dev_id, unsigned char index, unsigned char* buff, bool only_send_when_changed)
{
	int retcode = -1;
	retcode = g_eib_emulator->set_gobj_value_byindex(dev_id, index, buff, only_send_when_changed);
	return retcode;
}

int set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, unsigned char* buff, bool only_send_when_changed)
{
	int retcode = -1;
	retcode = g_eib_emulator->set_gobj_value_bygaddr(dev_id, gaddr, buff, only_send_when_changed);
	return retcode;
}

const char* __test(const char* str)
{
	////g_eib_emulator->Init(3, 115200);
	////system("pause");
	////g_eib_emulator->Uninit();

	//SetINIPath("EIBCommNEO.ini");

	//int retcode;
	//DeviceManager manager;
	//manager.Init();
	//Frame::SetDeviceManager(&manager);

	//Frame* frame = get_a_sample_frame();
	//assert(frame);

	//SerialComm_MM comm;

	//if ( stricmp("tx", str) == 0 )
	//{
	//	retcode = comm.Init(4, 115200);
	//	printf("任意键发帧.");
	//	system("pause");
	//	for ( int i = 0; i < 10; i++ )
	//	{
	//		ResponseType type = comm.tx_async(frame);
	//		if ( i == 5 )
	//		{
	//			type = comm.tx_sync(frame);
	//		}
	//	}
	//}
	//else if ( stricmp("rx", str) == 0 )
	//{
	//	APDUType types[100];
	//	int len = 100;
	//	retcode = comm.Init(3, 115200);
	//	printf("按任意键开始query apdu");
	//	system("pause");

	//	FramePeek* peeks;
	//	int count;
	//	comm.FramePeeker(AT_UNAVAILABLE, peeks, count);
	//	comm.ClearFramePeek(peeks);
	//}

	//system("pause");

	//delete frame;

	//comm.Uninit();

	//manager.Uninit();

	return str;
}