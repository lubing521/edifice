#include "stdafx.h"
#include "CommBase.h"

DWORD WINAPI tx_thread(void* arg)
{
	CommBase* ptr = (CommBase*)arg;
	return ptr->tx_thread();
}

DWORD WINAPI rx_thread(void* arg)
{
	CommBase* ptr = (CommBase*)arg;
	return ptr->rx_thread();
}

CommBase::CommBase(void)
{
	m_inited = false;
}

CommBase::~CommBase(void)
{
	Uninit();
}

int CommBase::Init(const InitData& initdata)
{
	if ( m_inited )
	{
		return -1;
	}

	// ��Щ����ֻͬ��֯Frame�ĵ�˫�������ݽṹ�й�. 
	int retcode = 0;
	char buff[0xff] = { 0 };

	m_quit_rx_thread = FALSE;
	m_quit_tx_thread = FALSE;
	m_next_rx_id = 1;
	m_next_tx_id = 1;
	m_quit_listen = FALSE;
	m_reporter = NULL;  // �ص�����. 

	m_mutex4tx_list = CreateMutex(NULL, FALSE, NULL);
	m_mutex4rx_list = CreateMutex(NULL, FALSE, NULL);
	m_mutex4lock_peeker = CreateMutex(NULL, FALSE, NULL);
	if ( m_mutex4rx_list == NULL || m_mutex4tx_list == NULL || m_mutex4lock_peeker == NULL )
	{
		report_err_immed("CreateMutex is not expected to fail.", 0xa);
	}
	m_event4tx_list = CreateEvent(NULL, TRUE, FALSE/*���ź�*/, NULL);
	if ( m_event4tx_list == NULL )
	{
		report_err_immed("CreateEvent is not expected to fail.", 0xa);
	}

	m_thread4tx_list = CreateThread(NULL, 0, ::tx_thread, this, 0, NULL);
	m_thread4rx_list = CreateThread(NULL, 0, ::rx_thread, this, 0, NULL);
	if ( m_thread4rx_list == NULL || m_thread4tx_list == NULL )
	{
		report_err_immed("CreateThread is not expected to fail.", 0xa);
	}

	m_inited = TRUE;

	return retcode;
}

void CommBase::Uninit()
{
	if ( !m_inited )
	{
		return;
	}

	m_quit_tx_thread = TRUE;
	m_quit_rx_thread = TRUE;
	WaitForSingleObject(m_thread4tx_list, INFINITE);
	WaitForSingleObject(m_thread4rx_list, INFINITE);
	CloseHandle(m_thread4tx_list);
	m_thread4tx_list = NULL;
	CloseHandle(m_thread4rx_list);
	m_thread4rx_list = NULL;
	CloseHandle(m_mutex4tx_list);
	m_mutex4tx_list = NULL;
	CloseHandle(m_mutex4rx_list);
	m_mutex4rx_list = NULL;
	CloseHandle(m_mutex4lock_peeker);
	m_mutex4lock_peeker = NULL;
	CloseHandle(m_event4tx_list);
	m_event4tx_list = NULL;
}

ResponseType CommBase::tx_sync(const Frame * send_frame)
{
	if ( !m_inited )
	{
		return RESPONSE_UNDEFINED;
	}

	if ( NULL == send_frame )
	{
		log_me("tx_async��������ΪNULL.���ܷ���.");
		return RESPONSE_BAD_AUGUMENT;
	}

	HANDLE evt = CreateEvent(NULL, TRUE, FALSE/*���ź�*/, NULL);
	if ( evt == NULL )
	{
		return RESPONSE_ERROR_CREATEEVENT;
	}

	Frame* frame = send_frame->CloneMe();
	WaitForSingleObject(m_mutex4tx_list, INFINITE);
	TxData tx_data;
	tx_data._frame = frame;
	tx_data._id = m_next_tx_id++;
	tx_data._time_stamp = GetTickCount();
	tx_data._evt = evt;
	m_tx_list.push_back(tx_data);
	ReleaseMutex(m_mutex4tx_list);

	SetEvent(m_event4tx_list);  // ֪ͨ�����ݷ��������. 
	WaitForSingleObject(evt, INFINITE);
	CloseHandle(evt);

	return RESPONSE_SENDING_OKAY;
}

ResponseType CommBase::tx_async(const Frame* send_frame)
{
	if ( !m_inited )
	{
		return RESPONSE_UNDEFINED;
	}

	if ( NULL == send_frame )
	{
		log_me("tx_async��������ΪNULL.���ܷ���.");
		return RESPONSE_BAD_AUGUMENT;
	}

	Frame* frame = send_frame->CloneMe();
	WaitForSingleObject(m_mutex4tx_list, INFINITE);
	TxData tx_data;
	tx_data._frame = frame;
	tx_data._id = m_next_tx_id++;
	tx_data._time_stamp = GetTickCount();
	tx_data._evt = NULL;
	m_tx_list.push_back(tx_data);
	ReleaseMutex(m_mutex4tx_list);

	SetEvent(m_event4tx_list);  // ֪ͨ�����ݷ��������. 

	return RESPONSE_SENDING_INQUEUE;
}

ResponseType CommBase::rx_sync(Frame*& recv_frame, uint waiting_time)
{
	return rx_sync(recv_frame, -1, NULL, NULL, NULL, waiting_time);
}

ResponseType CommBase::rx_sync(Frame*& recv_frame, uint sending_time, const std::vector<APDUType>* expected_types, 
							   const uchar* from, const uchar* to, uint waiting_time /* = -1 */)
{
	ResponseType response_type = RESPONSE_UNDEFINED;
	int response_time = g_myconfig.GetRxResponseTime(); // ��Ӧ���ʱ��. 
	bool timeout_reached = false;
	uint time_ent = GetTickCount();   // ����ú���ʱ��ʱ��. 

	while ( true )
	{
		if ( m_quit_listen )
		{
			response_type = RESPONSE_QUIT_FORCED;
			goto _out;
		}

		Sleep(10);

		if ( m_quit_listen )
		{
			response_type = RESPONSE_QUIT_FORCED;
			goto _out;
		}

		WaitForSingleObject(m_mutex4rx_list, INFINITE);
		for ( std::list<RxData>::iterator ite = m_rx_list.begin();
			ite != m_rx_list.end();
			++ite )
		{
			// ��֡������֮ǰ��֡����֮�󵽴������Ӧʱ���Լ������Ӧʱ�� response_time ̫����Ӧ��Ϊ������. 
			// sending_time ���� -1 ʱ�Ͳ���Ҫ���. 
			if ( sending_time != (uint)-1 && ite->_time_stamp < sending_time || ite->_time_stamp - sending_time < response_time )
			{
				continue;
			}
	
			// Ҫ���֡�ķ��͵�ַʱ������͵�ַ������.��֡��û�мǵ�ַ��Ϣ.  
			const uchar* host_addr = ite->_frame->GetHostAddr();
			if ( NULL != from && NULL != host_addr && 0 != memcmp( from, host_addr, 2 ) )
			{
				continue;
			}

			// Ҫ���֡��Ŀ�ĵ�ַ�����Ŀ�ĵ�ַ������.��֡��û�мǵ�ַ��Ϣ.
			const uchar* peer_addr = ite->_frame->GetPeerAddr();
			if ( NULL != to && NULL != peer_addr && 0 != memcmp( to, peer_addr, 2 ) )
			{
				continue;
			}

			// expected_types��ΪNULL������������Ԫ����Ҫ���֡������. 
			if ( NULL != expected_types && !expected_types->empty() )
			{
				bool got_ya = false;
				for( std::vector<APDUType>::const_iterator ite2apdutype = expected_types->begin(); 
					ite2apdutype!= expected_types->end();
					++ite2apdutype)
				{
					if ( ite->_type == *ite2apdutype )
					{
						got_ya = true;
						break;
					}
				}

				if ( !got_ya )  // ������͵�û��ƥ�䵽����. 
				{
					continue;
				}
			}

			// �ҵ�һ���Ϸ���֡��. 
			if ( ite->_frame->IsOneByteFrame() )
			{
				response_type = RESPONSE_SHORT;
			}
			else
			{
				response_type = RESPONSE_NORMAL;
			}
			recv_frame = ite->_frame;  // ��֡���ڴ������ڽ���ȥ. 
			m_rx_list.erase(ite);   // �Ӷ����а���ɾ��. 
			ReleaseMutex(m_mutex4rx_list);
			goto _out;
		} // end of for loop. 
		ReleaseMutex(m_mutex4rx_list);

		uint time_now = GetTickCount();
		if ( waiting_time != -1 && time_now - time_ent >= waiting_time )
		{
			response_type = RESPONSE_RECVING_TIMEOUT;
			recv_frame = NULL;
			break;
		}
	}

_out:
	return response_type;
}

ResponseType CommBase::rx_sync_ex(std::vector<Frame*>& recv_frames)
{
	return rx_sync_ex(recv_frames, -1, NULL, NULL, NULL);
}

ResponseType CommBase::rx_sync_ex(std::vector<Frame*>& recv_frames, uint sending_time, const std::vector<APDUType>* expected_types, 
								  const uchar* from, const uchar* to)
{
	ResponseType response_type = RESPONSE_UNDEFINED;
	uint time_out = g_myconfig.GetRxTimeOut();
	recv_frames.clear();
	Frame* frame;
	do
	{
		response_type = rx_sync(frame, sending_time, expected_types, from, to, time_out);
		if ( RESPONSE_NORMAL == response_type )
		{
			recv_frames.push_back(frame);
		}
		else if ( RESPONSE_RECVING_TIMEOUT == response_type )
		{
			break;
		}
	}while( true );  // Ҫһֱ�յ���ʱ. 

	return response_type;
}

Reporter CommBase::SetMonitor(Reporter r)
{
	Reporter save = m_reporter;

	m_reporter = r;

	return save;
}

int CommBase::QueryAPDUTypesInRxQueue(APDUType types[], int& sz)
{
	APDU* apdu;
	std::vector<APDUType> types_vec;

	if ( types == NULL )
	{
		return -1;
	}

	WaitForSingleObject(m_mutex4rx_list, INFINITE);
	for ( std::list<RxData>::iterator ite = m_rx_list.begin();
		ite != m_rx_list.end();
		++ite )
	{
		apdu = APDU::BuildAPDUFromFrame( *ite->_frame );
		if ( apdu )
		{
			types_vec.push_back(apdu->GetAPDUType());
			delete apdu;
			apdu = NULL;
		}
	}
	ReleaseMutex(m_mutex4rx_list);

	std::sort(types_vec.begin(), types_vec.end());
	std::vector<APDUType>::iterator ite_rm = std::unique(types_vec.begin(), types_vec.end());
	types_vec.erase(ite_rm, types_vec.end());

	if ( sz < types_vec.size() && types_vec.size() > 0 )
	{
		sz = -types_vec.size();
		return -2;
	}
	else
	{
		sz = types_vec.size();
		for ( int i = 0; i < types_vec.size(); i++ )
		{
			types[i] = types_vec[i];
		}
		return 0;
	}
}

int CommBase::FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count)
{
	APDU * apdu;
	APDUType apdu_type;
	std::vector<FramePeek> frame_peek_vec;

	// 10 ���붼�ò��������.��ֱ�ӷ���BUSY.��Ϊ�ɵ�Э��ӿں�������и���. 
	DWORD ret = WaitForSingleObject(m_mutex4lock_peeker, 10);
	if ( WAIT_OBJECT_0 != ret )
	{
		return FRAME_PEEKER_BUSY;
	}

	WaitForSingleObject(m_mutex4rx_list, INFINITE);
	for ( std::list<RxData>::iterator ite = m_rx_list.begin();
		ite != m_rx_list.end();
		++ite )
	{
		apdu = APDU::BuildAPDUFromFrame( *ite->_frame );
		if ( apdu )
		{
			apdu_type = apdu->GetAPDUType();
			if ( apdu_type == filter || AT_UNAVAILABLE == filter )
			{
				FramePeek frame_peek;
				frame_peek._type = apdu_type;
				frame_peek._frame_handle = ite->_frame->CloneMe();
				frame_peek._frame_id = ite->_id;
				frame_peek._frame_time_stamp = ite->_time_stamp;
				frame_peek_vec.push_back(frame_peek);
			}

			delete apdu;
			apdu = NULL;
		}
	}
	ReleaseMutex(m_mutex4rx_list);
	ReleaseMutex(m_mutex4lock_peeker);

	int total_mem_len = 4 + sizeof(FramePeek) * frame_peek_vec.size();  // ǰ�ĸ��ֽڷ�FramePeek�ĸ���. 
	uchar* mem = new uchar[total_mem_len];
	count = frame_peek_vec.size();
	*(uint*)mem = count;
	for ( int i = 0; i < count; i++ )
	{
		FramePeek* peek = (FramePeek*)(mem + 4 + i * sizeof(FramePeek));
		peek->_type = frame_peek_vec[i]._type;
		peek->_frame_handle = frame_peek_vec[i]._frame_handle;
		peek->_frame_id = frame_peek_vec[i]._frame_id;
		peek->_frame_time_stamp = frame_peek_vec[i]._frame_time_stamp;
	}
	frame_peeks = (FramePeek*)(mem + 4);

	return 0;
}

void CommBase::ClearFramePeek(void* addr)
{
	if ( NULL == addr )
	{
		return;
	}

	FramePeek* peek;
	Frame* frame;
	void* true_addr = ((uint*)addr) - 1;
	uint cout = *((uint*)true_addr);
	for ( int i = 0; i < cout; i++ )
	{
		peek = (FramePeek*)( (uchar*)addr + i * sizeof(FramePeek) );
		frame = (Frame*)peek->_frame_handle;
		delete frame;
	}

//	delete true_addr;
	delete (uint*)true_addr;  // 'cos gcc complains that "delete void* is undefined."
}

void CommBase::RemoveFrameFromRxQueue(const FramePeek& peek)
{
	WaitForSingleObject(m_mutex4rx_list, INFINITE);
	RemoveFrameFromRxQueue_nolock(peek);
	ReleaseMutex(m_mutex4rx_list);
}

void CommBase::LockFramePeeker()
{
	WaitForSingleObject(m_mutex4lock_peeker, INFINITE);
}

void CommBase::UnlockFramePeeker()
{
	ReleaseMutex(m_mutex4lock_peeker);
}

void CommBase::RemoveFrameFromRxQueue_nolock(const FramePeek& peek)
{
	for ( std::list<RxData>::iterator ite = m_rx_list.begin();
		ite != m_rx_list.end();
		++ite )
	{
		if ( ite->_id == peek._frame_id )
		{
			delete ite->_frame;
			m_rx_list.erase(ite);
		}
	}
}

void CommBase::QuitRxNow()
{
	m_quit_listen = true;
	m_quit_rx_thread = true;
}

DWORD CommBase::tx_thread()
{
	char big_buff[1024];
	int big_buff_len = sizeof(big_buff);
	int interval_time = g_myconfig.GetTxIntervalTime();  // ��������֮֡��ļ��. 

	while (!m_quit_tx_thread)
	{
		if ( !m_inited )
		{
			Sleep(10);
			continue;
		}
		WaitForSingleObject(m_event4tx_list, 25);

		WaitForSingleObject(m_mutex4tx_list, INFINITE);
		if ( m_tx_list.empty() )
		{
			ResetEvent(m_event4tx_list);  // ���ַ��Ͷ�����û��֡��.�Ͱ����Event����. 
			ReleaseMutex(m_mutex4tx_list);
			continue;
		}

		// ���� m_tx_list �ǿ�. 
		TxData& tx_data = m_tx_list.front();  // ȡ�����еĵ�һ��. 
		Frame* send_frame = tx_data._frame;

		big_buff_len = sizeof(big_buff);

		const UCHAR* frame_mem;
		UCHAR  frame_len;
		send_frame->GetFrameMem(frame_mem, frame_len);

		bins2hexs(frame_mem, frame_len, big_buff, big_buff_len);
		log_me("tx: ��ʼ����֡. %s", big_buff);

		// ��֡����ȥ. 
		int sending_okay = _send_frame(send_frame);
		if ( 0 == sending_okay )  // ���ͳɹ�.
		{
			// ���ͳ�ȥ��.��ص���������. 
			log_me("tx: ֡�Ѿ������ͳɹ�.");
			if ( NULL != m_reporter )
			{
				m_reporter( (unsigned char*)frame_mem, frame_len, "t" );
			}
		}
		else
		{
			log_me("tx: ֡����ʧ��.");
		}

		Sleep( interval_time );

_out:
		if ( NULL != tx_data._evt )
		{
			SetEvent(tx_data._evt);
		}
		delete send_frame;  // ������ͷ��ڴ�. 
		m_tx_list.pop_front();  // �ѷ��ͳ�ȥ�ĵ���. 

		ReleaseMutex(m_mutex4tx_list);
	}  // end of while(!quit_tx)

	return 0;
}

DWORD CommBase::rx_thread()
{
	int retcode;
	int bytes_recved_count;
	uchar big_buff[1024] = { 0 };
	char buxx[1024];
	const int big_buff_len = sizeof(big_buff);
	int big_buff_occupied = 0;   // �յ������ݶ��ᱻ���뵽big_buff����,��big_buff_occupiedָʾ����������ж�����������.
	std::list<RxData> data_list;
	BOOL rx_check_address = g_myconfig.DoesRxNeedCheckAddress();

	while (!m_quit_rx_thread)
	{
		if ( !m_inited )
		{
			Sleep(5);
			continue;
		}

		mem_t mem;
		Sleep(1);
		int recving_okay = _recv_bytes(mem);
		if ( 0 != recving_okay )
		{
			continue;
		}

		bytes_recved_count = mem.get_size();

		if ( 0 != bytes_recved_count )
		{
			bins2hexs(mem.address(), mem.get_size(), buxx, sizeof(buxx));
			log_me("���ŵ��ж������� %s", buxx);
		}

		if ( 0 == bytes_recved_count )
		{
			continue;
		}
		else if ( 1 == bytes_recved_count ) // ֻ�յ�һ���ֽ�.�ж��ǲ��Ƕ�֡�� 
		{
			retcode = 0;
			data_list.clear();
			Frame* short_frame = OneByteFrame::BuildFrameFromBuff(mem.address(), 1);
			if ( short_frame )
			{
				RxData rx_data;
				rx_data._frame = short_frame;
				rx_data._type = rx_data._frame->GetAPDUType();
				rx_data._id = m_next_rx_id++;
				rx_data._time_stamp = GetTickCount();
				data_list.push_back(rx_data);
			}
		}
		else
		{
			// ������յ�������֡�� rx_buff ���� big_buff Ȼ����������֡. 
			memcpy(big_buff + big_buff_occupied, mem.address(), mem.get_size());
			big_buff_occupied += mem.get_size(); 

			// ���������̫����ŵ����յ��������˵�ȴû�д������������ȷ��֡����.���������յ��������Ѿ�
			// �����ܷ�����һ���Ϸ���֡��.����ܿ��Ҫ���꣬����ɻ������������������ʱ����.����������߼�
			// ����������յ���256���ֽ�. 
			if ( big_buff_occupied > sizeof(big_buff) - 256 )
			{
				memmove(big_buff, big_buff + 256, sizeof(big_buff) - 256);
				big_buff_occupied -= 256;
			}

			retcode = FrameParser(big_buff, big_buff_occupied, data_list, rx_check_address);
		}

		// ���Բ����ն�����. 
		if ( 0 == retcode && !data_list.empty() )
		{
			// �������յ���֡.
			for ( std::list<RxData>::iterator ite = data_list.begin();
				ite != data_list.end();
				++ite )
			{
				const uchar* frame_mem;
				uchar frame_len;
				ite->_frame->GetFrameMem(frame_mem, frame_len);

				if ( NULL != m_reporter )
				{
					m_reporter( (unsigned char*)frame_mem, frame_len, "r" );
				}

				bins2hexs(frame_mem, frame_len, buxx, sizeof(buxx));
				log_me("rx: �յ�֡%s", buxx);
			}
			WaitForSingleObject(m_mutex4rx_list, INFINITE);
			m_rx_list.splice(m_rx_list.end(), data_list);  // ������յ���֡���������. 
			ReleaseMutex(m_mutex4rx_list);
		}

		// ������ڵ�֡. 
		ClearOldFrames();
	}

	return 0;
}

int CommBase::FrameParser(unsigned char* mem, int& mem_len, std::list<RxData>& data_list, BOOL check_address)
{
	std::vector<std::pair<int,int> > twigs; // pair�����±�ͳ���.�����>>֮������и��ո񣬷���gcc���������ù�. 
	data_list.clear();

	for ( int i = 0; i < mem_len; i++ )
	{
		for ( int j = mem_len - i; j > 0; j-- )
		{
			twigs.push_back( std::pair<int,int>(i, j) );
		}
	}

	int retcode = -1, index, len;
	std::vector<Frame*> parsing_vec;  // ���������֡. 
	for ( int i = 0; i < twigs.size(); i++ )
	{
		index = twigs[i].first;
		len = twigs[i].second;
		retcode = parse_recv_buff_ex(mem + index, len, parsing_vec, TRUE, check_address);
		if ( retcode == 0 )
		{
			break;
		}
	}

	if ( retcode != 0 )
	{
		return retcode;
	}

	// �����õ���֡. 
	for ( int i = 0; i < parsing_vec.size(); i++ )
	{
		RxData rx_data;
		rx_data._frame = parsing_vec[i];
		rx_data._type = rx_data._frame->GetAPDUType();
		rx_data._id = m_next_rx_id++;
		rx_data._time_stamp = GetTickCount();
		data_list.push_back(rx_data);
	}

	// �ѷ�����ȷ����֡��������ͬ�������֮ǰ�յ�����������.  
	int index_of_head_byte = index + len;
	int count_of_unattended_bytes_left = mem_len - index_of_head_byte;
	assert(count_of_unattended_bytes_left >= 0);
	memmove(mem, mem + index_of_head_byte, count_of_unattended_bytes_left);
	mem_len = count_of_unattended_bytes_left;

	return 0; 
}

void CommBase::ClearOldFrames()
{
	int cout_deleted = 0;
	int frame_life_time = g_myconfig.GetFrameLifeTime();
	uint now = GetTickCount();
	uint recvd_time;
	uint elapsed_time;

	WaitForSingleObject(m_mutex4rx_list, INFINITE);
	std::list<RxData>::iterator ite_rm = m_rx_list.end();  // ָʾҪ��Ҫɾ֡.������ֵָ��end()�Ͳ�Ҫɾ. 
	for ( std::list<RxData>::iterator ite = m_rx_list.begin();
		ite != m_rx_list.end();
		++ite )
	{
		recvd_time = ite->_time_stamp;
		elapsed_time = now - recvd_time;
		if ( elapsed_time >= frame_life_time * 1000 )  // ��Ϊ frame_life_time �ĵ�λ����. 
		{
	    	ite_rm = ite;
		}
		else
		{
			break;
		}
	}
	if ( ite_rm != m_rx_list.end() )
	{
		++ite_rm;  // now point to the element beyond the last element that is gonna be deleted. 
		for ( std::list<RxData>::iterator ite = m_rx_list.begin(); 
			ite != ite_rm;
			++ite )
		{
			delete ite->_frame;
		}
		m_rx_list.erase(m_rx_list.begin(), ite_rm);
	}
	ReleaseMutex(m_mutex4rx_list);
}

int CommBase::parse_recv_buff(const unsigned char* buff, int len, Frame*& get_frame, BOOL check_address)
{
	int retcode = 0;
	retcode = _parse_recv_buff(buff, len, get_frame, FALSE, check_address);
	return retcode;
}

int CommBase::parse_recv_buff_ex(const unsigned char* buff, int len, std::vector<Frame*>& get_frames, 
								 BOOL suppress_log, BOOL check_address)
{
	int retcode = -1;
	Frame* tmp_frame = NULL;
	get_frames.clear();

	char big_buff[1024] = { 0 };
	int big_buff_len = sizeof(big_buff);

	if ( NULL == buff )
	{
		if (!suppress_log) log_me("parse_recv_buff_ex �յ�NULLָ�롣û�����ݿɹ�����.");
		retcode = -1;
		goto _out;
	}

	bins2hexs(buff, len, big_buff, big_buff_len);
	if (!suppress_log) log_me_nn("parse_recv_buff_ex ������ʼ�������� %s ", big_buff);

	// ����Ƕ�֡�����. 
	for ( int flen = 9; flen <= len; flen++ )
	{
		BOOL perfect = TRUE;
		for ( int pos = 0; pos < len; pos += flen )
		{
			if ( 0 != _parse_recv_buff(buff + pos, flen, tmp_frame, TRUE, check_address) || NULL == tmp_frame )
			{
				perfect = FALSE;
				break;
			}
			get_frames.push_back(tmp_frame);
		}
		if ( perfect )
		{
			retcode = 0;
			break;
		}
		get_frames.clear();
	}

	// �����֡������Ƶ����ŵ����ֽڵĺ����շ���ʱ. 
	//if ( len == 1 )
	//{
	//	tmp_frame = OneByteFrame::BuildFrameFromBuff(buff, 1);
	//	if ( NULL != tmp_frame )
	//	{
	//		get_frames.push_back(tmp_frame);
	//		retcode = 0;
	//	}
	//}

	if ( get_frames.size() )
	{
		if (!suppress_log) log_me("parse_recv_buff �õ� %u ����ȷ֡�Ļظ�.\n", get_frames.size());
		for ( int i = 0; i < get_frames.size(); i++ )
		{
			const UCHAR* frame_mem;
			UCHAR  frame_len;
			get_frames[i]->GetFrameMem(frame_mem, frame_len);
			bins2hexs(frame_mem, frame_len, big_buff, sizeof(big_buff));
			if (!suppress_log) log_me("%s\n", big_buff);
		}
	}
	else
	{
		if (!suppress_log) log_me("parse_recv_buff δ�ý������ظ�.");
	}

_out:
	return retcode;
}

int CommBase::_parse_recv_buff(const unsigned char* buff, int len, Frame*& get_frame, BOOL suppress_log, BOOL check_address)
{
	int retcode = 0;

	char big_buff[1024];
	int big_buff_len = sizeof(big_buff);

	if ( NULL == buff )
	{
		!suppress_log && log_me("parse_recv_buff �յ�NULLָ�롣û�����ݿɹ�����.");
	}

	bins2hexs(buff, len, big_buff, big_buff_len);
	!suppress_log && log_me_nn("parse_recv_buff ������ʼ�������� %s ", big_buff);

	get_frame = Frame::BuildFrameFromBuff(buff, len, TRUE, check_address);

	if ( NULL != get_frame )
	{
		!suppress_log && log_me("parse_recv_buff �õ���ȷ�ظ�.");
	}
	else
	{
		!suppress_log && log_me("parse_recv_buff δ�ý������ظ�.");
	}

	return retcode;
}