#include "stdafx.h"
#include <assert.h>
#include "EIBEmulatorImpl.h"
#include "SerialComm_MM.h"
#include "UDPComm_MM.h"
#include "DeviceManager.h"
#include "MyConfig.h"
#include "generic_helper.h"

#define SUPERNATURAL_WAITING_TIME 10

class APDUContainer;

DWORD WINAPI listen_proc(void * arg)
{
	EIBEmulatorImpl * ptr = static_cast<EIBEmulatorImpl*>(arg);
	return ptr->_listen_proc();
}

EIBEmulatorImpl::EIBEmulatorImpl(void)
{
	m_inited = false;
}

EIBEmulatorImpl::~EIBEmulatorImpl(void)
{
	Uninit();
}

int EIBEmulatorImpl::Init(const InitData& initdata)
{
	if ( m_inited )
		return -1;

	int retcode = 0;

	m_reporter = NULL;
	m_mutex_objs = NULL;
	m_listen_event = NULL; 
	m_listen_thread = NULL;

	m_comm = NULL;
	m_dev_manager = NULL;

	g_myconfig.Init(NULL);

	if ( initdata._ct == CT_SERIAL_PORT )
	{
		m_comm = new SerialComm_MM;
	}
	else if ( initdata._ct == CT_UDP )
	{
		m_comm = new UDPComm_MM;
	}
	 
	m_dev_manager = new DeviceManager;
	if ( NULL == m_comm || NULL == m_dev_manager )
	{
		retcode = -2;
		goto _out;
	}

	m_mutex_objs = CreateMutex(NULL, FALSE, NULL);
	if ( NULL == m_mutex_objs )
	{
		retcode = GetLastError();
		goto _out;
	}

	retcode = m_comm->Init(initdata);
	if ( 0 != retcode )
	{
		goto _out;
	}

	retcode = m_dev_manager->Init();
	if ( 0 != retcode )
	{
		goto _out;
	}

	// �����߳�. 
	m_quit_listen_thread = FALSE;
	// for win
	m_listen_event = CreateEvent(NULL, FALSE, FALSE, NULL);  // �ڶ�������ΪFALSE��ʾ�Զ��ָ�Event�����ź�. 
	if ( NULL == m_listen_event )
	{
		retcode = GetLastError();
		goto _out;
	}
	m_listen_thread = CreateThread(NULL, 0, listen_proc, this, 0, NULL);
	if ( NULL == m_listen_thread )
	{
		retcode = GetLastError();
		goto _out;
	}

	m_inited = true;

_out:
	if ( 0 != retcode )
	{
		if ( m_mutex_objs ) CloseHandle(m_mutex_objs);
		if ( m_listen_event ) CloseHandle(m_listen_event);
		if ( m_listen_thread ) CloseHandle(m_listen_thread);
	}
	return retcode;
}

int EIBEmulatorImpl::Uninit()
{
	if ( !m_inited )
		return -1;

	m_quit_listen_thread = true;
	m_comm->QuitRxNow();
	WaitForSingleObject(m_listen_thread, INFINITE);
	CloseHandle(m_listen_event);
	CloseHandle(m_listen_thread);
	CloseHandle(m_mutex_objs);

	delete m_comm;
	delete m_dev_manager;

	m_inited = false;

	return 0;
}

int EIBEmulatorImpl::QueryAPDUTypesInRxQueue(APDUType types[], int& sz)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode;
	retcode = m_comm->QueryAPDUTypesInRxQueue(types, sz);
	return retcode;
}

#define FRAME_PEEKER_BUSY -1      // ��һС��ʱ����û�о���������������ΪBUSY.
int EIBEmulatorImpl::FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode;
	retcode = m_comm->FramePeeker(filter, frame_peeks, count);
	return retcode;
}

int EIBEmulatorImpl::ClearFramePeek(void* addr)
{
	if ( !m_inited )
	{
		return -1;
	}

	m_comm->ClearFramePeek(addr);
	return 0;
}

int EIBEmulatorImpl::RemoveFrameFromRxQueue(const FramePeek& peek)
{
	if ( !m_inited )
	{
		return -1;
	}

	m_comm->RemoveFrameFromRxQueue(peek);
	return 0;
}


int EIBEmulatorImpl::Listen_Async()
{
	if ( !m_inited )
	{
		return -1;
	}
	
	//////  �����ź�  �����߳̿�ʼʵ�ʹ���
	SetEvent(m_listen_event);

	return 0;
}

int EIBEmulatorImpl::Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len, uint waiting_time)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode = 0;
	ResponseType response_type = RESPONSE_UNDEFINED;

	Frame* recv_frame = NULL;
	Frame* send_frame = NULL;
	std::vector<Frame*> send_frames;
	const UCHAR* frame_mem = NULL;
	UCHAR  frame_len = 0;

	int buff_used = 0;

	// ��. 
	BOOL listening = TRUE;
	const std::vector<APDUType>& vec_apdutype = APDUContainer::GetInstance().GetExpectedAPDUTypesForListen();
	response_type = m_comm->rx_sync(recv_frame, -1, &vec_apdutype, NULL, NULL, waiting_time);
	if ( response_type == RESPONSE_RECVING_TIMEOUT ) // ��ʱ
	{
		return 74;
	}

	if ( NULL == recv_frame )  // ����û���յ������. 
	{
		return -1;
	}

	recv_frame->GetFrameMem(frame_mem, frame_len);
	if ( recv_buff_len >= frame_len )  // ���ջ������㹻��. 
	{
		if ( recv_buff ) 
		{
			memcpy(recv_buff, frame_mem, frame_len);
		}
		recv_buff_len = frame_len;
	}
	else
	{
		recv_buff_len = frame_len;
		recv_buff_len = -recv_buff_len;
	}

	// ��. 

	// ���ñ���APDU���ݵ���.��Ϊbuild_send_frames_from_recv_frame�������ɻظ�֡������������п��ܻ��޸�APDU����. 
	HoldTheMutex htm_objs(m_mutex_objs);
	retcode = build_send_frames_from_recv_frame(recv_frame, send_frames);
	htm_objs.Release();

//	if ( 0 == retcode && NULL != send_frame )
	if ( !send_frames.empty() && 0 == retcode )
	{
		for ( int i = 0; i < send_frames.size(); i++ )
		{
			send_frame = send_frames[i];
			response_type = m_comm->tx_sync(send_frame);  // ���ͳ�ȥ������
			send_frame->GetFrameMem(frame_mem, frame_len);
			if ( send_buff_len >= frame_len + buff_used + 1 )
			{
				send_buff[buff_used] = frame_len;
				buff_used++;
				if ( send_buff ) 
				{
					memcpy(send_buff + buff_used, frame_mem, frame_len);
				}
				buff_used += frame_len;
			}
			else
			{
				send_buff_len = frame_len + buff_used + 1 ;
				send_buff_len = -send_buff_len;
				goto _out;
			}
		}
		send_buff_len = buff_used;
	}
	else if ( 0 == retcode )
	{
		send_buff_len = 0;
	}
	else
	{
		retcode = -2;
		response_type = RESPONSE_BAD_QUERY;
		log_me("�޷����յ�������֡����һ���ظ�֡.");
	}

_out:
	// to prevent memory leak. 
	delete recv_frame;  
	for ( int i = 0; i < send_frames.size(); i++ )
	{
		delete send_frames[i];
	}

	return retcode;
}

int EIBEmulatorImpl::PropertyValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
							   unsigned char object_index, unsigned char property_id, unsigned char& count, 
							   unsigned short start, unsigned char* buff, int& buff_len)
{
	int retcode = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU_PropertyValue_Response* resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( count > 15 ) //|| numtring < 1 )
	{
		return -2;
	}

	if ( NULL == buff ||  0 >= buff_len || NULL == host || NULL == peer )
	{
		return -3;
	}

	set_host_addr(0xffffffff, host);

	APDU_PropertyValue_Read apdu;
	apdu.BuildMe(0, 3, 7, peer, object_index, property_id, count, start);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	if ( NULL == send_frame )
	{
		return -4;
	}

	int numtring = 1; // numtring �����������ʷ����.���Ǻ�����. 
	for ( int i = 0; i < numtring; i ++ )
	{
		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);
		if ( res_type == RESPONSE_NORMAL )
		{
			APDU* apdu = NULL;

			if ( NULL != recv_frame )
			{
				apdu = APDU::BuildAPDUFromFrame(*recv_frame) ;
				resp = dynamic_cast<APDU_PropertyValue_Response*>(apdu);
			}

			if ( NULL != resp )  // ����������õ��Ļظ�����. 
			{
				retcode = 0;

				UCHAR resp_obj_index;
				UCHAR resp_property_id;
				UCHAR resp_no_of_elem;
				USHORT resp_start_index;
				retcode = resp->GetParams(&resp_obj_index, &resp_property_id, &resp_no_of_elem, &resp_start_index, buff, &buff_len);

				assert(resp_obj_index==object_index);
				assert(resp_property_id==property_id);
				assert(resp_start_index==start);
			} 
			else
			{
				count = 0;
				buff_len = 0;
			}
			delete apdu;
		}
		else  // ������������ȷ�ظ�
		{
			count = 0;
			buff_len = 0;
		}

		delete recv_frame;
		recv_frame = NULL;
		resp = NULL;

		if ( 0 == retcode )
		{
			break;
		}

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	} // end of for loop. 

	delete send_frame;
	delete recv_frame;

	return retcode;
}

int EIBEmulatorImpl::PropertyValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
								unsigned char object_index, unsigned char property_id, unsigned char count, 
								unsigned short start, const unsigned char* buff, int buff_len)
{
	int retcode = 1;
	int numtring = 1; // ��ʷ��������. 

	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	// UNDO �������ĺϷ���. 
	if ( NULL == buff || count > 15 || numtring < 1 || buff_len > 249 || buff_len <= 0 || NULL == host || NULL == peer )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_PropertyValue_Write apdu;
	apdu.BuildMe(0, 3, 7, peer, object_index, property_id, count, start, buff, buff_len);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	for ( int i = 0; i < numtring; i++ )
	{
		retcode = -1;
		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);
		if ( res_type == RESPONSE_SHORT && NULL != recv_frame )
		{
			resp = APDU::BuildAPDUFromFrame(*recv_frame);

			if ( NULL != resp )  // ����������õ��Ļظ�����. 
			{
				if ( resp->IsAck() )  // ֻ�����õ��ظ���ACK�ŷ��سɹ�. 
				{
					retcode = 0;
					goto _out;
				}

				if ( resp->IsNack() ) // �лظ������ظ��ܾ�.
				{
					retcode = -1;
					goto _out;
				}
			}
		}

		// ��Ϊ1,����û���յ��ظ����߲��������Ļظ�����. 
		retcode = 1;

		delete recv_frame;
		delete resp;
		recv_frame = NULL;
		resp = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	}

_out:
	delete recv_frame;
	delete send_frame;
	delete resp;

	return retcode;
}

int EIBEmulatorImpl::PropertyValue_Read2(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
								unsigned char object_index, unsigned char property_id, unsigned short start, 
								unsigned char* buff, int& buff_len)
{
	int retcode = 0;

	unsigned char count = mask_num((unsigned char*)&start, "00f");  // α��һ������.
	start &= 0x0fff;

	retcode = PropertyValue_Read(host, peer, priority, object_index, property_id, count, start, buff, buff_len);

	return retcode;
}

int EIBEmulatorImpl::PropertyValue_Write2(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
								unsigned char object_index, unsigned char property_id, unsigned short start, 
								const unsigned char* buff, int buff_len)
{
	int retcode = 0;

	unsigned char count = mask_num((unsigned char*)&start, "00f");  // α��һ������.���ɵ�PropertyValue_Write. 
	start &= 0x0fff;

	retcode = PropertyValue_Write(host, peer, priority, object_index, property_id, count, start, buff, buff_len);

	return retcode = 0;
}

int EIBEmulatorImpl::Property_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
						  unsigned char object_index, unsigned char property_id, PropertyFields& prop_fields)
{
	int retcode = -1;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU_Property_Response*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( numtring < 1 )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_Property_Read apdu;
	apdu.BuildMe(0, 0, 7, peer, object_index, property_id);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	for ( int i = 0; i < numtring; i ++ )
	{
		retcode = -1;

		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);
		if ( res_type == RESPONSE_NORMAL && NULL!=recv_frame )
		{
			APDU* apdu = APDU::BuildAPDUFromFrame(*recv_frame);
			resp = dynamic_cast<APDU_Property_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				UCHAR resp_object_index;
				UCHAR resp_property_id;
				resp->GetParams(&resp_object_index, &resp_property_id, &prop_fields);

				// #define  _DONT_ASSERT_RESPONSE_
				// #ifndef  _DONT_ASSERT_RESPONSE_
				//				assert(resp_object_index==object_index);
				//				assert(resp_property_id==property_id);
				// #endif
				retcode = 0;
				delete apdu;
				goto _out;
			}
			delete apdu;
		}

		delete recv_frame;
		recv_frame = NULL;
		resp = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	} // end of for loop. 

_out:
	delete send_frame;
	delete recv_frame;

	return retcode;	
}

int EIBEmulatorImpl::Property_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
						   unsigned char object_index, unsigned char property_id, const PropertyFields& prop_fields)
{
	int retcode = -1;
	int numtring = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( numtring < 1 )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_Property_Write apdu;
	apdu.BuildMe(0, 0, 7, peer, object_index, property_id, prop_fields);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);
	for ( int i = 0; i < numtring; i ++ )
	{
		retcode = -1;
		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);

		if ( res_type == RESPONSE_SHORT && NULL!=recv_frame )
		{
			resp = APDU::BuildAPDUFromFrame(*recv_frame);

			if ( NULL != resp )  // ����������õ��Ļظ�����. 
			{
				if ( resp->IsAck() )  // ֻ�����õ��ظ���ACK�ŷ��سɹ�. 
				{
					retcode = 0;
					goto _out;
				}

				if ( resp->IsNack() ) // �лظ������ظ��ܾ�.
				{
					retcode = -1;
					goto _out;
				}
			}
		}

		delete resp;
		delete recv_frame;
		resp = NULL;
		recv_frame = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	}

_out:
	delete resp;
	delete recv_frame;
	delete send_frame;

	return retcode;
}

int EIBEmulatorImpl::Authorize_Query(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
							const unsigned char* key, unsigned char* level)
{
	int retcode = -1;
	int numtring = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU_Authorize_Response* resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( NULL == key || NULL == level || numtring < 1 )
	{
		return -7;
	}

	set_host_addr(0xffffffff, host);

	APDU_Authorize_Request apdu;
	apdu.BuildMe(0, 0, 7, peer, key);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);
	for ( int i = 0; i < numtring; i ++ )
	{
		recv_frame = NULL;
		resp = NULL;
		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);

		if ( res_type == RESPONSE_NORMAL && NULL!=recv_frame )
		{
			APDU* apdu = APDU::BuildAPDUFromFrame(*recv_frame);
			resp = dynamic_cast<APDU_Authorize_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				resp->GetParams(level);
				retcode = 0;
				delete apdu;
				break;
			}
			delete apdu;
		}

		delete recv_frame;
		resp = NULL;
		recv_frame = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	}

	//_out:
	delete recv_frame;
	delete send_frame;

	return retcode;
}

int EIBEmulatorImpl::GroupValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
									 unsigned char* buff, int& buff_len )
{
	int retcode = -1;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	std::vector<Frame*> recv_frames;
	APDU_GroupValue_Response* resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( NULL == host || NULL == peer || NULL == buff || numtring < 1 )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_GroupValue_Read apdu;
	apdu.BuildMe(0, 0, 7, peer);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	//for ( int i = 0; i < numtring; i ++ )
	//{
		retcode = -1;

		// һֱ�յ���ʱ. 
		ResponseType res_type = tx_rx_till_timeout_sync(send_frame, recv_frames);
		if ( res_type == RESPONSE_NORMAL && 0 != recv_frames.size() )
		{
			APDU* apdu = APDU::BuildAPDUFromFrame(**recv_frames.rbegin());
			resp = dynamic_cast<APDU_GroupValue_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				resp->GetParams(buff ,buff_len);  // GetParams��
				if ( buff_len > 0 )
				{
					retcode = 0;
				}
				else // ����Ļ����������� 
				{
					retcode = -2; 
				}
				delete apdu;
				goto _out;
			}
			delete apdu;
		}
_out:
	delete send_frame;
	for ( int i = 0; i < recv_frames.size(); i++ )
	{
		delete recv_frames[i];
	}

	return retcode;	
}

int EIBEmulatorImpl::GroupValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
							 const unsigned char* buff, int buff_len_in_bit)
{
	int retcode = -1;
	Frame* send_frame = NULL;
//	Frame* recv_frame = NULL;
//	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( NULL == host || NULL == peer || NULL == buff )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_GroupValue_Write apdu;
	apdu.BuildMe(0, 0, 7, peer, buff, buff_len_in_bit);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	ResponseType resp_type = m_comm->tx_sync(send_frame);
	if ( RESPONSE_SENDING_OKAY == resp_type )
	{
		retcode = 0;
	}
	else
	{
		retcode = -3;
	}

//_out:
	delete send_frame;

	return retcode;
}

int EIBEmulatorImpl::Link_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, 
					           unsigned char group_obj_num, unsigned char start_index, unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU_Link_Response* resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( NULL == host || NULL == peer || numtring < 1 )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_Link_Read apdu;
	apdu.BuildMe(0, 0, 7, peer, group_obj_num, start_index);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	for ( int i = 0; i < numtring; i ++ )
	{
		retcode = -1;

		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);
		if ( res_type == RESPONSE_NORMAL && NULL!=recv_frame )
		{
			APDU* apdu = APDU::BuildAPDUFromFrame(*recv_frame);
			resp = dynamic_cast<APDU_Link_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				UCHAR group_object_number;
				UCHAR start_index;
				resp->GetParams(&group_object_number, &start_index, buff, &buff_len);
				
				if ( buff_len >= 0 )
				{
					retcode = 0;
				}
				else // ����Ļ����������� 
				{
					retcode = -2; 
				}
				delete apdu;
				goto _out;
			}
			delete apdu;
		}

		delete recv_frame;
		recv_frame = NULL;
		resp = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	} // end of for loop. 

_out:
	delete send_frame;
	delete recv_frame;

	return retcode;	
}

int EIBEmulatorImpl::Link_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority,
							unsigned char group_obj_num, unsigned char flags, const unsigned char* group_addr)
{
	int retcode = -1;
	int numtring = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( numtring < 1 || NULL == host || NULL == peer || NULL == group_addr )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_Link_Write apdu;
	apdu.BuildMe(0, 0, 7, peer, group_obj_num, flags, group_addr);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);
	for ( int i = 0; i < numtring; i ++ )
	{
		retcode = -1;
		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);

		if ( res_type == RESPONSE_SHORT && NULL!=recv_frame )
		{
			resp = APDU::BuildAPDUFromFrame(*recv_frame);

			if ( NULL != resp )  // ����������õ��Ļظ�����. 
			{
				if ( resp->IsAck() )  // ֻ�����õ��ظ���ACK�ŷ��سɹ�. 
				{
					retcode = 0;
					goto _out;
				}

				if ( resp->IsNack() ) // �лظ������ظ��ܾ�.
				{
					retcode = -1;
					goto _out;
				}
			}
		}

		delete resp;
		delete recv_frame;
		resp = NULL;
		recv_frame = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	}

_out:
	delete resp;
	delete recv_frame;
	delete send_frame;

	return retcode;
}

int EIBEmulatorImpl::UserMemory_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned int address, 
							unsigned char number, unsigned char* buff)
{
	int retcode = -1;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU_UserMemory_Response* resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( NULL == host || NULL == peer || numtring < 1 || address & 0xFFF00000 || number == 0 || number > 11 || NULL == buff )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_UserMemory_Read apdu;
	apdu.BuildMe(0, 0, 7, peer, number, address);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	for ( int i = 0; i < numtring; i ++ )
	{
		retcode = -1;

		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);
		if ( res_type == RESPONSE_NORMAL && NULL != recv_frame )
		{
			APDU* apdu = APDU::BuildAPDUFromFrame(*recv_frame);
			resp = dynamic_cast<APDU_UserMemory_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				UCHAR number;
				UINT address;
				UCHAR buxx[16];
				int buxx_len = sizeof(buxx);
				resp->GetParams(&number, &address, buxx, &buxx_len);
				
				memcpy(buff, buxx, buxx_len);
				retcode = 0;
				delete apdu;
				goto _out;
			}
			delete apdu;
		}

		delete recv_frame;
		delete resp;
		recv_frame = NULL;
		resp = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	} // end of for loop. 

_out:
	delete send_frame;
	delete recv_frame;

	return retcode;	
}


int EIBEmulatorImpl::UserMemory_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned int address,
							 unsigned char number, unsigned char* buff)
{
	int retcode = -1;
	int numtring = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( NULL == host || NULL == peer || numtring < 1 || address & 0xFFF00000 || number == 0 || number > 11 || NULL == buff )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_UserMemory_Write apdu;
	apdu.BuildMe(0, 0, 7, peer, number, address, buff);
	apdu.SetHostAddr(host);
	send_frame = Frame::BuildFrameFromAPDU(apdu);
	for ( int i = 0; i < numtring; i ++ )
	{
		retcode = -1;
		ResponseType res_type = tx_rx_sync(send_frame, recv_frame);

		if ( res_type == RESPONSE_SHORT && NULL!=recv_frame )
		{
			resp = APDU::BuildAPDUFromFrame(*recv_frame);

			if ( NULL != resp )  // ����������õ��Ļظ�����. 
			{
				if ( resp->IsAck() )  // ֻ�����õ��ظ���ACK�ŷ��سɹ�. 
				{
					retcode = 0;
					goto _out;
				}

				if ( resp->IsNack() ) // �лظ������ظ��ܾ�.
				{
					retcode = -1;
					goto _out;
				}
			}
		}

		delete resp;
		delete recv_frame;
		resp = NULL;
		recv_frame = NULL;

		if ( i + 1 != numtring )
			Sleep(SUPERNATURAL_WAITING_TIME);
	}

_out:
	delete resp;
	delete recv_frame;
	delete send_frame;

	return retcode;
}

int EIBEmulatorImpl::IndividualAddress_Read(const unsigned char* host, unsigned char priority,  unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	std::vector<Frame*> recv_frames;
	unsigned char buxx[1024];
	const int buxx_len = sizeof(buxx);
	int buxx_used = 0;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( NULL == host || NULL == buff || numtring < 1 )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_IndividualAddress_Read apdu;
	apdu.BuildMe(0, 0, 7);
	apdu.SetHostAddr(host);
	apdu.SetPeerAddr(g_myconfig.GetBroadcastAddr());
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	//for ( int i = 0; i < numtring; i ++ )
	//{
	// һֱ�յ���ʱ. 
	ResponseType res_type = tx_rx_till_timeout_sync(send_frame, recv_frames);
	if ( res_type == RESPONSE_NORMAL && 0 != recv_frames.size() )
	{
		APDU_IndividualAddress_Response* resp = NULL;
		for ( int i = 0; i < recv_frames.size(); i++ )
		{
			Frame* frame = recv_frames[i];
			APDU* apdu = APDU::BuildAPDUFromFrame(*frame);
			resp = dynamic_cast<APDU_IndividualAddress_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				memcpy(buxx+buxx_used, frame->GetHostAddr(), 2);
				buxx_used += 2;
			}
			delete apdu;
		}
		if ( buff_len < buxx_used )
		{
			buff_len = -buxx_used;
			retcode = -3;
			goto _out;
		}
		memcpy(buff, buxx, buxx_used);
		buff_len = buxx_used;
		retcode = 0;
	}

_out:
	delete send_frame;
	for ( int i = 0; i < recv_frames.size(); i++ )
	{
		delete recv_frames[i];
	}

	return retcode;	
}


int EIBEmulatorImpl::IndividualAddress_Write(const unsigned char* host, unsigned char priority, const unsigned char* new_address)
{
	int retcode = -1;
	int numtring = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( NULL == host || numtring < 1 || NULL == new_address )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_IndividualAddress_Write apdu;
	apdu.BuildMe(0, 0, 7, new_address);
	apdu.SetHostAddr(host);
	apdu.SetPeerAddr(g_myconfig.GetBroadcastAddr());
	send_frame = Frame::BuildFrameFromAPDU(apdu);
	//for ( int i = 0; i < numtring; i ++ )
	//{
		ResponseType resp_type = m_comm->tx_sync(send_frame);
		if ( RESPONSE_SENDING_OKAY == resp_type )
		{
			retcode = 0;
		}
		else
		{
			retcode = -3;
		}
	//}

	delete send_frame;
	return retcode;
}


int EIBEmulatorImpl::IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char priority, unsigned char* serial_num,
											   unsigned char* buff, int& buff_len)
{
	int retcode = -1;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	std::vector<Frame*> recv_frames;
	unsigned char buxx[1024];
	const int buxx_len = sizeof(buxx);
	int buxx_used = 0;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	if ( NULL == host || NULL == buff || numtring < 1 || NULL == serial_num )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_IndividualAddressSerialNumber_Read apdu;
	apdu.BuildMe(0, 0, 7, serial_num);
	apdu.SetHostAddr(host);
	apdu.SetPeerAddr(g_myconfig.GetBroadcastAddr());
	send_frame = Frame::BuildFrameFromAPDU(apdu);

	//for ( int i = 0; i < numtring; i ++ )
	//{
	// һֱ�յ���ʱ. 
	ResponseType res_type = tx_rx_till_timeout_sync(send_frame, recv_frames);
	if ( res_type == RESPONSE_NORMAL && 0 != recv_frames.size() )
	{
		APDU_IndividualAddressSerialNumber_Response* resp = NULL;
		for ( int i = 0; i < recv_frames.size(); i++ )
		{
			Frame* frame = recv_frames[i];
			APDU* apdu = APDU::BuildAPDUFromFrame(*frame);
			resp = dynamic_cast<APDU_IndividualAddressSerialNumber_Response*>(apdu);
			if ( NULL != resp )  // �õ�����ȷ�Ļظ�APDU. 
			{
				uchar serialno[6] = { 0 };
				resp->GetParams(serialno);
				if ( 0 == memcmp(serialno, serial_num, 6) )
				{
					memcpy(buxx+buxx_used, frame->GetHostAddr(), 2);
					buxx_used += 2;
				}
			}
			delete apdu;
		}
		if ( buff_len < buxx_used )
		{
			buff_len = -buxx_used;
			retcode = -3;
			goto _out;
		}
		memcpy(buff, buxx, buxx_used);
		buff_len = buxx_used;
		retcode = 0;
	}

_out:
	delete send_frame;
	for ( int i = 0; i < recv_frames.size(); i++ )
	{
		delete recv_frames[i];
	}

	return retcode;	
}

int EIBEmulatorImpl::IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char priority, 
														 const unsigned char* serial_num, const unsigned char* new_addr)
{
	int retcode = -1;
	int numtring = 1;
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	APDU*  resp = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited ) // δ��ɳ�ʼ��. 
	{
		return -1;  // return immediately. 
	}

	if ( NULL == host || numtring < 1 || NULL == new_addr )
	{
		return -2;
	}

	set_host_addr(0xffffffff, host);

	APDU_IndividualAddressSerialNumber_Write apdu;
	apdu.BuildMe(0, 0, 7, serial_num, new_addr);
	apdu.SetHostAddr(host);
	apdu.SetPeerAddr(g_myconfig.GetBroadcastAddr());
	send_frame = Frame::BuildFrameFromAPDU(apdu);
	//for ( int i = 0; i < numtring; i ++ )
	//{
	ResponseType resp_type = m_comm->tx_sync(send_frame);
	if ( RESPONSE_SENDING_OKAY == resp_type )
	{
		retcode = 0;
	}
	else
	{
		retcode = -3;
	}
	//}

	delete send_frame;
	return retcode;
}

int EIBEmulatorImpl::Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, int& recv_buff_len)
{
	int retcode = 0;
	int numtring = 1; // ��ʷ��������. 
	Frame* send_frame = NULL;
	Frame* recv_frame = NULL;
	unsigned const char* hostaddr = NULL;

	HoldTheMutex htm_objs(m_mutex_objs);
	FramePeekerLocker peeker_locker(m_comm);

	if ( !m_inited || NULL == raw_data || NULL == recv_buff || numtring < 1 )
	{
		retcode = SEND_RAWDATA_ILLEGAL_PARAMS;
		goto _out;
	}

	send_frame = Frame::BuildFrameFromBuff(raw_data, raw_data_len, FALSE);
	if ( NULL == send_frame )
	{
		retcode = SEND_RAWDATA_NOT_A_FRAME;
		goto _out;
	}

	hostaddr = send_frame->GetHostAddr();
	set_host_addr(0xffffffff, hostaddr);

	for ( int i = 0; i < numtring; i++ )
	{
		retcode = tx_rx_sync(send_frame, recv_frame);
		if ( NULL != recv_frame )
		{
			const unsigned char* frame_mem;
			unsigned char frame_len;
			// unsigned char copied_len;
			recv_frame->GetFrameMem(frame_mem, frame_len);

			if ( recv_buff_len < frame_len )
			{
				recv_buff_len = -frame_len;
			}
			else
			{
				recv_buff_len = frame_len;
				memcpy(recv_buff, frame_mem, frame_len);
			}
			retcode = 0;
			goto _out;
		}
		else
		{
			delete recv_frame;
			recv_frame = NULL;
			retcode = SEND_RAWDATA_NO_LEGAL_RESPONSE;
		}
	}

_out:
	delete recv_frame;
	delete send_frame;

	return retcode;
}

ResponseType EIBEmulatorImpl::tx_rx_sync(const Frame* send_frame,  Frame*& recv_frame)
{
	ResponseType response_type = RESPONSE_UNDEFINED;

	std::vector<APDUType> expected_apdutypes;
	const uchar* check_from;
	const uchar* check_to;
	uint sending_time, time_out;

	if ( send_frame == NULL )
	{
		goto _out;
	}

	// ��
	sending_time = GetTickCount();
	response_type = m_comm->tx_sync(send_frame);
	if ( RESPONSE_SENDING_TIMEOUT == response_type )
	{
		goto _out;
	}

	impose_response_restrictions(send_frame, expected_apdutypes, check_from, check_to);

	// ��
	time_out = g_myconfig.GetRxTimeOut();
	response_type = m_comm->rx_sync(recv_frame, sending_time, &expected_apdutypes, check_from, check_to, time_out);

_out:
	return response_type;
}

ResponseType EIBEmulatorImpl::tx_rx_till_timeout_sync(const Frame* send_frame, std::vector<Frame*>& recv_frames)
{
	ResponseType response_type = RESPONSE_UNDEFINED;

	Frame* recv_frame = NULL;
	recv_frames.clear();

	std::vector<APDUType> expected_apdutypes;
	const uchar* check_from;
	const uchar* check_to;

	// ��
	uint sending_time = GetTickCount();
	response_type = m_comm->tx_sync(send_frame);
	if ( RESPONSE_SENDING_TIMEOUT == response_type )
	{
		goto _out;
	}

	impose_response_restrictions(send_frame, expected_apdutypes, check_from, check_to);

	//do 
	//{
	//	response_type = m_comm->rx_sync(recv_frame);
	//	recv_frames.push_back(recv_frame);
	//} while ( RESPONSE_RECVING_TIMEOUT != response_type );
	// ��
	response_type = m_comm->rx_sync_ex(recv_frames, sending_time, &expected_apdutypes, check_from, check_to);
	// rx_sync_ex �������߼������յ���ʱΪֹ. 
	if ( response_type == RESPONSE_RECVING_TIMEOUT )
	{
		// ��������ѳ�ʱ�ĳ� NORMAL. 
		response_type = RESPONSE_NORMAL;
	}

_out:
	return response_type;
}

void EIBEmulatorImpl::impose_response_restrictions(const Frame* send_frame, std::vector<APDUType>& expected_apdutypes, 
												   const uchar*& check_from, const uchar*& check_to)
{
	APDU* apdu = NULL;
	const uchar* host_addr;
	const uchar* peer_addr;

	expected_apdutypes.clear();
	check_from = NULL;
	check_to = NULL;

	if ( NULL == send_frame )
	{
		goto _out;
	}

	apdu = APDU::BuildAPDUFromFrame(*send_frame);
	if ( NULL == apdu )
	{
		goto _out;
	}

	apdu->GetExpectedAPDUTypes(expected_apdutypes);
	host_addr = send_frame->GetHostAddr();
	peer_addr = send_frame->GetPeerAddr();

	// ���peer��ַ�Ǹ����ַ.���ԾͲ����ظ�֡�ĵ�ַ��Ϣ.
	// ��ʵ��,������鲥.��APDU��Response��������Լ��Ƿ�������������Ƿ���Ӧ. 
	if ( send_frame->DoesPeerUseGroupAddress() )  
	{
		check_from = NULL;
		check_to = NULL;
	}
	// ���peer��ַ�Ǹ��㲥��ַ.ֻ��Ҫ��鷵��֡��host��ַ(Ҳ�����ĸ��豸������host��ַ).
	// else if ( send_frame->DosePeerUseBroadcastAddress() )
	// ����һ�����������peer��ַ���˹㲥��ַ�Ͳ���鷵��֡�ķ�����ַ.����Э�鲢��֧��ÿ�෽��ʹ�ù㲥��ַ.�͵���չ����. 
	else if ( send_frame->DosePeerUseBroadcastAddress() || 0 == memcmp(g_myconfig.GetBroadcastAddr(), peer_addr, 2) )
	{
		check_from = NULL;
		check_to = host_addr;
	}
	else  // ����������ַ��Ҫ���. 
	{
		check_from = peer_addr;
		check_to = host_addr;
	}

_out:
	delete apdu;
	return;
}

ResponseType EIBEmulatorImpl::rx_tx_sync(uint waiting_time)
{
	int retcode = 0;
	ResponseType response_type = RESPONSE_UNDEFINED;

	Frame* recv_frame = NULL;
//	Frame* send_frame = NULL;
	std::vector<Frame*> send_frames;

	// ��. 
	const std::vector<APDUType>& apdutypes = APDUContainer::GetInstance().GetExpectedAPDUTypesForListen();
	response_type = m_comm->rx_sync(recv_frame, -1, &apdutypes, NULL, NULL, waiting_time);

	if ( NULL == recv_frame || response_type == RESPONSE_RECVING_TIMEOUT )
		goto _out;

	// ��. 
	retcode = build_send_frames_from_recv_frame(recv_frame, send_frames);
	if ( retcode == 0 )
	{
		for ( int i = 0; i < send_frames.size(); i++ )
		{
			response_type = m_comm->tx_sync(send_frames[i]);
		}
		response_type = RESPONSE_NORMAL;
	}
	else
	{
		response_type = RESPONSE_BAD_QUERY;
		log_me("�յ�������֡��Ч.");
	}

_out:
	for ( int i = 0; i < send_frames.size(); i++ )
	{
		delete send_frames[i];
	}
	// to prevent memory leak. 
	delete recv_frame;  

	return response_type;
}

int EIBEmulatorImpl::build_send_frames_from_recv_frame(Frame* recv_frame, std::vector<Frame*>& send_frames)
{
	int retcode = -1;
	send_frames.clear();

	if ( NULL == recv_frame )
	{
		retcode = -7;
	}
	else
	{
		retcode = recv_frame->Response(send_frames);
	}

	return retcode;
}

uint EIBEmulatorImpl::_listen_proc()
{
	uint ret = 0;
	int puppet0, puppet1; 

	while ( true )
	{
		ret = WaitForSingleObject(m_listen_event, 10);
		if ( ret == WAIT_OBJECT_0 )
		{
			break;
			//report_err_immed("EIBEmulatorImpl::_listen_proc: WaitForSingleObject returns unexpectly.",0xE);
		}
		if ( m_quit_listen_thread )
		{
			goto _out;
		}
	}

	while ( !m_quit_listen_thread )
	{		
		HoldTheMutex htm_objs(m_mutex_objs);		
		rx_tx_sync(250);

		//Frame* recv_frame = NULL;
		//Frame* send_frame = NULL;

		//// ��. 
		//BOOL listening = TRUE;
		//response_type = m_comm->rx_sync(recv_frame, listening);

		//if ( NULL == recv_frame )
		//	continue;

		//// ��. 
		//int retcode = build_send_frames_from_recv_frame(recv_frame, send_frame);
		//if ( 0 == retcode && NULL != send_frame )
		//{
		//	response_type = m_comm->tx_sync(send_frame);
		//}
		//else
		//{
		//	response_type = RESPONSE_BAD_QUERY;
		//	log_me("�յ�������֡��Ч.");
		//}

		//// to prevent memory leak. 
		//delete recv_frame;  
		//delete send_frame;
	}

_out:
	return ret;
}

unsigned int EIBEmulatorImpl::make_dev(const char* template_file)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	retcode = m_dev_manager->CreateNewDev(template_file);
	return retcode;
}

int EIBEmulatorImpl::remove_dev(unsigned int dev_id)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	retcode = m_dev_manager->RemoveDev(dev_id);
	return retcode;
}

int EIBEmulatorImpl::get_all_devs(unsigned int* buff, int& buff_len)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	retcode = m_dev_manager->GetAllDevices(buff, buff_len);
	return retcode;
}

int EIBEmulatorImpl::get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetAllIDPairs(buff, buff_len);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetPerElemLen_inbit(object_index, prop_id);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::set_host_addr(unsigned int dev_id, const unsigned char* hostaddr)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->SetHostAddr(hostaddr);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::get_host_addr(unsigned int dev_id, char* addr)
{
	if ( !m_inited || NULL == addr ) 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		const uchar* ptr = dev->GetHostAddr(NULL);
		if ( NULL != ptr )
		{
			addr[0] = ptr[0];
			addr[1] = ptr[1];
		}
		return 0;
	}

	return -1;
}

int EIBEmulatorImpl::get_map_info(unsigned int dev_id, DevMapInfo& map_info)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		dev->GetMapInfo(map_info);
		return 0;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::set_map_info(unsigned int dev_id, const DevMapInfo& map_info)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		dev->SetMapInfo(map_info);
		return 0;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetElemCount(object_index, prop_id);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id,
									 PropertyFields& fields)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetPropFields(object_index, prop_id, fields);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, 
									 const PropertyFields& fields)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->SetPropFields(object_index, prop_id, fields);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, 
									 const PropertyFields& fields)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->AddPropFields(object_index, prop_id, fields);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::get_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
							unsigned char count, unsigned char* buff, int& buff_len)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetElemValues(object_index, prop_id, start_index, count, buff, buff_len);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
							unsigned char count, unsigned char* buff, int buff_len)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->SetElemValues(object_index, prop_id, start_index, count, buff, buff_len);
		return retcode;
	}
	else
	{
		return -1;
	}
}

int EIBEmulatorImpl::get_gobj_count(unsigned int dev_id)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetGroupObjectCount();
		return retcode;
	}
	else
	{
		return -1;
	}
}

// ����ɾ�������. 
//int EIBEmulatorImpl::rm_gobj_byidx(unsigned int dev_id, unsigned char index)
//{
//	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
//	{
//		return -1;
//	}
//
//	int retcode = 0;
//	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
//	if ( NULL != dev )
//	{
//		retcode = dev->RemoveGroupObjectByIndex(index);
//		return retcode;
//	}
//	else
//	{
//		return -1;
//	}
//}

int EIBEmulatorImpl::get_gobjhead_byidx(unsigned int dev_id, unsigned char index, GroupObjectHead& gobj_head)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return NULL;
	}

	int retcode = 0;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetGroupObjectHeadByIndex(index, gobj_head);
	}
	
	return retcode;
}

int EIBEmulatorImpl::get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = -1;
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL != dev )
	{
		retcode = dev->GetGroupObjectValue(index, buff, &len);
	}
	return retcode;
}

int EIBEmulatorImpl::set_gobj_value_byindex(unsigned int dev_id, unsigned char index, unsigned char* buff, bool only_send_when_changed)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	int retcode = -1;
	IDevice*  dev = m_dev_manager->GetDeviceByID(dev_id);
	GroupObjectHead gobjh;
	retcode = get_gobjhead_byidx(dev_id, index, gobjh);
	if ( 0 != retcode )
	{
		return retcode;
	}
	if ( NULL != dev )
	{
		unsigned char buxx[64] = { 0 };
		int buxx_len = sizeof(buxx);
		retcode = dev->GetGroupObjectValue(index, buxx, &buxx_len);
		if ( 0 != retcode )  // ��������������. 
		{
			goto _out;
		}

		//retcode = gobj->SetValue(buff); // ��ֵ���. 
		retcode = dev->SetGroupObjectValue(index, buff);
		if ( 0 != retcode ) // û�����óɹ�. 
		{
			goto _out;
		}

		// ���óɹ���. 

		// ������дֵ��û�и���ԭʼֵ�Ƿ���Ҫ�����µ��������Ϸ�֡������������������ֵ���߼�. 
		bool send_frame_on_bus = true;
		if ( only_send_when_changed )
		{
			if ( 0 == memcmp_inbit(buxx, buff, buxx_len) )
			{
				send_frame_on_bus = false;
			}
		}
		
		uchar gaddrs[1024] = { 0 };
		int len = sizeof(gaddrs);
		int gaddr_count = dev->GetGaddrsOfGobjs(index, gaddrs, len);  // �ҵ����±��������Ӧ���������ַ. 
		for ( int i = 0; i < gaddr_count; i++ )
		{
			// Ȼ��ͨ��Э������������豸����Ӧ�����. 
			
			// STEP ONE.
			// ģ�������������豸(���豸֮��û��ʵ�ʵ������ŵ�����)�����յ���д�����ֵ��֡. 
			APDU_GroupValue_Write apdu_gv_w;
			// �õ����ַ. 
			const uchar* gaddr = gaddrs + i*2;
			// ����󳤶�. 
			int elemlen_in_bit = gobjh.GetLenInBit();
			retcode = apdu_gv_w.BuildMe(0, 0, 0, gaddr, buff, elemlen_in_bit);
			apdu_gv_w.m_recved_apdu = TRUE;

			if ( send_frame_on_bus )
			{
				APDU* apdu;
				std::vector<APDU*> apdus;
				apdu_gv_w.Response(apdu, apdus);

				// STEP TWO. 
				// ��ʵ�������Ϸ�д��ֵ��֪֡ͨ�����豸.  
				// NOTICE. �����п��ܻ���������N���֡�. 
				retcode = GroupValue_Write(dev->GetHostAddr(NULL), gaddr, 0, buff, elemlen_in_bit);
			}
		}
	}

_out:
	return retcode;
}

int EIBEmulatorImpl::set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, unsigned char* buff, bool only_send_when_changed)
{
	if ( NULL == gaddr )
	{
		return -1;
	}

	int retcode = -1;
	uchar buxx[1024];
	int buxx_len = sizeof(buff);
	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	int gobj_count;

	if ( NULL == dev )
	{
		retcode = -1;
		goto _out;
	}

	gobj_count = dev->GetGroupObjectCount();
	for ( int i = 0; i < gobj_count; i++ )
	{
		buxx_len = sizeof(buff);
		int gaddr_count = dev->GetGaddrsOfGobjs(i, buxx, buxx_len);
		assert(gaddr_count*2==buxx_len);
		
		for ( int j = 0; j < gaddr_count; j++ )
		{
			if ( 0 == memcmp(buxx+j*2, gaddr, 2) ) // ��.�������������������ַ. 
			{
				retcode = set_gobj_value_byindex(dev_id, i, buff, only_send_when_changed);
			}
		}
	}

_out:
	return retcode;
}

bool EIBEmulatorImpl::DoesThisSoftUnitExist(unsigned int dev_id, const char* softunit_name)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return false;
	}

	IDevice* dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL == dev )
	{
		return false;
	}

	bool b = dev->DoesThisSoftUnitExist(softunit_name);
	return b;
}

const uchar* EIBEmulatorImpl::ReadSoftUnit(unsigned int dev_id, const char* softunit_name, uchar* buff)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return NULL;
	}

	IDevice*  dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL == dev )
	{
		return NULL;
	}

	const uchar* ptr;
	ptr = dev->ReadSoftUnit(softunit_name, buff);
	return ptr;
}

int EIBEmulatorImpl::WriteSoftUnit(unsigned int dev_id, const char* softunit_name, const uchar* value)
{
	if ( !m_inited ) // δ��ɳ�ʼ��ѽ. 
	{
		return -1;
	}

	IDevice*  dev = m_dev_manager->GetDeviceByID(dev_id);
	if ( NULL == dev )
	{
		return -1;
	}

	int retcode = dev->WriteSoftUnit(softunit_name, value);
	return retcode;
}

Reporter EIBEmulatorImpl::SetMonitor(Reporter r)
{
	Reporter s = m_comm->SetMonitor(r);
	
	return s;
}