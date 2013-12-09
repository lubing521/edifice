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

	// 这些代码只同组织Frame的的双队列数据结构有关. 
	int retcode = 0;
	char buff[0xff] = { 0 };

	m_quit_rx_thread = FALSE;
	m_quit_tx_thread = FALSE;
	m_next_rx_id = 1;
	m_next_tx_id = 1;
	m_quit_listen = FALSE;
	m_reporter = NULL;  // 回调函数. 

	m_mutex4tx_list = CreateMutex(NULL, FALSE, NULL);
	m_mutex4rx_list = CreateMutex(NULL, FALSE, NULL);
	m_mutex4lock_peeker = CreateMutex(NULL, FALSE, NULL);
	if ( m_mutex4rx_list == NULL || m_mutex4tx_list == NULL || m_mutex4lock_peeker == NULL )
	{
		report_err_immed("CreateMutex is not expected to fail.", 0xa);
	}
	m_event4tx_list = CreateEvent(NULL, TRUE, FALSE/*无信号*/, NULL);
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
		log_me("tx_async函数参数为NULL.不能发送.");
		return RESPONSE_BAD_AUGUMENT;
	}

	HANDLE evt = CreateEvent(NULL, TRUE, FALSE/*无信号*/, NULL);
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

	SetEvent(m_event4tx_list);  // 通知有数据放入队列啦. 
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
		log_me("tx_async函数参数为NULL.不能发送.");
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

	SetEvent(m_event4tx_list);  // 通知有数据放入队列啦. 

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
	int response_time = g_myconfig.GetRxResponseTime(); // 响应最短时间. 
	bool timeout_reached = false;
	uint time_ent = GetTickCount();   // 进入该函数时的时间. 

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
			// 这帧数据在之前的帧发送之后到达，但是响应时间比约定的向应时间 response_time 太短则应判为不符合. 
			// sending_time 等于 -1 时就不需要检查. 
			if ( sending_time != (uint)-1 && ite->_time_stamp < sending_time || ite->_time_stamp - sending_time < response_time )
			{
				continue;
			}
	
			// 要检查帧的发送地址时这个发送地址不符合.短帧里没有记地址信息.  
			const uchar* host_addr = ite->_frame->GetHostAddr();
			if ( NULL != from && NULL != host_addr && 0 != memcmp( from, host_addr, 2 ) )
			{
				continue;
			}

			// 要检查帧的目的地址是这个目的地址不符合.短帧里没有记地址信息.
			const uchar* peer_addr = ite->_frame->GetPeerAddr();
			if ( NULL != to && NULL != peer_addr && 0 != memcmp( to, peer_addr, 2 ) )
			{
				continue;
			}

			// expected_types不为NULL，且容器里有元素则要检查帧的类型. 
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

				if ( !got_ya )  // 检查类型但没有匹配到类型. 
				{
					continue;
				}
			}

			// 找到一个合法的帧了. 
			if ( ite->_frame->IsOneByteFrame() )
			{
				response_type = RESPONSE_SHORT;
			}
			else
			{
				response_type = RESPONSE_NORMAL;
			}
			recv_frame = ite->_frame;  // 把帧的内存生命期交出去. 
			m_rx_list.erase(ite);   // 从队列中把它删除. 
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
	}while( true );  // 要一直收到超时. 

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

	// 10 毫秒都得不到这个锁.则直接返回BUSY.因为旧的协议接口函数会持有该锁. 
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

	int total_mem_len = 4 + sizeof(FramePeek) * frame_peek_vec.size();  // 前四个字节放FramePeek的个数. 
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
	int interval_time = g_myconfig.GetTxIntervalTime();  // 两个发送帧之间的间隔. 

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
			ResetEvent(m_event4tx_list);  // 发现发送队列里没有帧了.就把相关Event重置. 
			ReleaseMutex(m_mutex4tx_list);
			continue;
		}

		// 现在 m_tx_list 非空. 
		TxData& tx_data = m_tx_list.front();  // 取队列中的第一个. 
		Frame* send_frame = tx_data._frame;

		big_buff_len = sizeof(big_buff);

		const UCHAR* frame_mem;
		UCHAR  frame_len;
		send_frame->GetFrameMem(frame_mem, frame_len);

		bins2hexs(frame_mem, frame_len, big_buff, big_buff_len);
		log_me("tx: 开始发送帧. %s", big_buff);

		// 把帧发出去. 
		int sending_okay = _send_frame(send_frame);
		if ( 0 == sending_okay )  // 发送成功.
		{
			// 发送出去了.向回调函数报告. 
			log_me("tx: 帧已经被发送成功.");
			if ( NULL != m_reporter )
			{
				m_reporter( (unsigned char*)frame_mem, frame_len, "t" );
			}
		}
		else
		{
			log_me("tx: 帧发送失败.");
		}

		Sleep( interval_time );

_out:
		if ( NULL != tx_data._evt )
		{
			SetEvent(tx_data._evt);
		}
		delete send_frame;  // 发完后释放内存. 
		m_tx_list.pop_front();  // 把发送出去的弹出. 

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
	int big_buff_occupied = 0;   // 收到的数据都会被填入到big_buff里面,而big_buff_occupied指示这个缓冲里有多少有用数据.
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
			log_me("从信道中读到数据 %s", buxx);
		}

		if ( 0 == bytes_recved_count )
		{
			continue;
		}
		else if ( 1 == bytes_recved_count ) // 只收到一个字节.判断是不是短帧。 
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
			// 把这次收到的数据帧从 rx_buff 拷入 big_buff 然后分析里面的帧. 
			memcpy(big_buff + big_buff_occupied, mem.address(), mem.get_size());
			big_buff_occupied += mem.get_size(); 

			// 缓冲里存了太多从信道里收到的数据了但却没有从里面分析出正确的帧数据.而且最先收到的数据已经
			// 不可能分析出一个合法的帧了.缓冲很快就要用完，会造成缓冲区溢出的严重运行时错误.于是下面的逻辑
			// 清理掉最先收到的256个字节. 
			if ( big_buff_occupied > sizeof(big_buff) - 256 )
			{
				memmove(big_buff, big_buff + 256, sizeof(big_buff) - 256);
				big_buff_occupied -= 256;
			}

			retcode = FrameParser(big_buff, big_buff_occupied, data_list, rx_check_address);
		}

		// 可以操作收队列了. 
		if ( 0 == retcode && !data_list.empty() )
		{
			// 遍历刚收到的帧.
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
				log_me("rx: 收到帧%s", buxx);
			}
			WaitForSingleObject(m_mutex4rx_list, INFINITE);
			m_rx_list.splice(m_rx_list.end(), data_list);  // 把这次收到的帧存入队列中. 
			ReleaseMutex(m_mutex4rx_list);
		}

		// 清掉过期的帧. 
		ClearOldFrames();
	}

	return 0;
}

int CommBase::FrameParser(unsigned char* mem, int& mem_len, std::list<RxData>& data_list, BOOL check_address)
{
	std::vector<std::pair<int,int> > twigs; // pair存了下标和长度.这里的>>之间必须有个空格，否则gcc编译器不让过. 
	data_list.clear();

	for ( int i = 0; i < mem_len; i++ )
	{
		for ( int j = mem_len - i; j > 0; j-- )
		{
			twigs.push_back( std::pair<int,int>(i, j) );
		}
	}

	int retcode = -1, index, len;
	std::vector<Frame*> parsing_vec;  // 存分析到的帧. 
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

	// 分析得到了帧. 
	for ( int i = 0; i < parsing_vec.size(); i++ )
	{
		RxData rx_data;
		rx_data._frame = parsing_vec[i];
		rx_data._type = rx_data._frame->GetAPDUType();
		rx_data._id = m_next_rx_id++;
		rx_data._time_stamp = GetTickCount();
		data_list.push_back(rx_data);
	}

	// 把分析过确认是帧的数据连同这段数据之前收到的数据销毁.  
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
	std::list<RxData>::iterator ite_rm = m_rx_list.end();  // 指示要不要删帧.如果这个值指向end()就不要删. 
	for ( std::list<RxData>::iterator ite = m_rx_list.begin();
		ite != m_rx_list.end();
		++ite )
	{
		recvd_time = ite->_time_stamp;
		elapsed_time = now - recvd_time;
		if ( elapsed_time >= frame_life_time * 1000 )  // 因为 frame_life_time 的单位是秒. 
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
		if (!suppress_log) log_me("parse_recv_buff_ex 收到NULL指针。没有数据可供分析.");
		retcode = -1;
		goto _out;
	}

	bins2hexs(buff, len, big_buff, big_buff_len);
	if (!suppress_log) log_me_nn("parse_recv_buff_ex 函数开始分析数据 %s ", big_buff);

	// 处理非短帧的情况. 
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

	// 处理短帧的情况移到从信道收字节的函数刚返回时. 
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
		if (!suppress_log) log_me("parse_recv_buff 得到 %u 个正确帧的回复.\n", get_frames.size());
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
		if (!suppress_log) log_me("parse_recv_buff 未得解析到回复.");
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
		!suppress_log && log_me("parse_recv_buff 收到NULL指针。没有数据可供分析.");
	}

	bins2hexs(buff, len, big_buff, big_buff_len);
	!suppress_log && log_me_nn("parse_recv_buff 函数开始分析数据 %s ", big_buff);

	get_frame = Frame::BuildFrameFromBuff(buff, len, TRUE, check_address);

	if ( NULL != get_frame )
	{
		!suppress_log && log_me("parse_recv_buff 得到正确回复.");
	}
	else
	{
		!suppress_log && log_me("parse_recv_buff 未得解析到回复.");
	}

	return retcode;
}