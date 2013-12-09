#include "StdAfx.h"
#include "Frame.h"
#include "MyConfig.h"

DeviceManager* Frame::ms_dev_manager;

Frame::Frame()
{
	m_frame_mem = NULL;
	m_frame_len = 0;
}

Frame::Frame(const Frame& frame)
{
	m_frame_len = frame.m_frame_len;
	if ( 0 == m_frame_len )
	{
		return;
	}
	m_frame_mem = new uchar[m_frame_len];
	memcpy(m_frame_mem, frame.m_frame_mem, m_frame_len);
}

void Frame::CopyOfFrameData(UCHAR*& mem, UCHAR& len)const
{
	mem = new UCHAR[m_frame_len];
	len = m_frame_len;
	memcpy(mem, m_frame_mem, m_frame_len);
}

const Frame& Frame::operator=(const Frame& frame)
{
	delete []m_frame_mem;
	m_frame_len = frame.m_frame_len;
	if ( 0 == m_frame_len )
	{
		m_frame_mem = NULL;
	}
	else
	{
		m_frame_mem = new uchar[m_frame_len];
		memcpy(m_frame_mem, frame.m_frame_mem, m_frame_len);
	}
	return *this;
}

Frame::~Frame(void)
{
	delete []m_frame_mem;
}

Frame* Frame::BuildFrameFromAPDU(const APDU& apdu)
{
	int len = apdu.GetApduLen();
	Frame* frame = NULL;

	if ( 0 == len )
	{
		frame = new OneByteFrame(apdu);
	}
//	else if ( len <= 16 && len >= 1 )  // 生成标准帧. 
	else if ( len < 16 && len >= 1 )  // 生成标准帧. 
	{
		frame = new StandardFrame(apdu);
	}
	else  // 生成扩展帧. 
	{
		frame = new ExtendedFrame(apdu);
	}

	return frame;
}

Frame* Frame::BuildFrameFromMem(const UCHAR* mem, UCHAR mem_len, BOOL recved)
{
	Frame* frame = NULL;

	frame = StandardFrame::BuildFrameFromMem(mem, mem_len, recved);
	if ( frame )
		return frame;

	frame = ExtendedFrame::BuildFrameFromMem(mem, mem_len, recved);
	if ( frame )
		return frame;

	frame = OneByteFrame::BuildFrameFromMem(mem, mem_len);
	if ( frame )
		return frame;

	return frame;
}

Frame* Frame::BuildFrameFromBuff(const UCHAR* buff, UCHAR buff_len, BOOL recved /* = TRUE */, BOOL check_address)
{
	Frame* frame = NULL;

	frame = StandardFrame::BuildFrameFromBuff(buff, buff_len, recved, check_address);
	if ( frame )
		return frame;

	frame = ExtendedFrame::BuildFrameFromBuff(buff, buff_len, recved, check_address);
	if ( frame )
		return frame;

	frame = OneByteFrame::BuildFrameFromBuff(buff, buff_len);
	if ( frame )
		return frame;

	return frame;
}

DeviceManager* Frame::SetDeviceManager(DeviceManager* manager)
{
	DeviceManager* save = ms_dev_manager;
	ms_dev_manager = manager;
	return save;
}

const UCHAR* Frame::GetFrameMem(const UCHAR*& frame_mem, UCHAR& frame_len)const
{
	frame_mem = m_frame_mem;
	frame_len = m_frame_len;

	return frame_mem;
}

APDUType Frame::GetAPDUType()
{
	APDU* apdu = APDU::BuildAPDUFromFrame(*this);
	if ( NULL != apdu )
	{
		APDUType apdu_type = apdu->GetAPDUType();
		delete apdu;
		return apdu_type;
	}
	else
	{
		return AT_UNAVAILABLE;
	}
}

BOOL Frame::DosePeerUseBroadcastAddress()const
{
	const uchar* broadcast_addr = g_myconfig.GetBroadcastAddr();
	const uchar* peer_addr = GetPeerAddr();
	if ( 0 == memcmp(broadcast_addr, peer_addr, 2) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL Frame::IsOneByteFrame()const
{
	return m_frame_len == 1;
}

int Frame::Response(Frame*& resp)const  // 旧代码.以后要废弃！！！！
{
	int retcode = 0;
	APDU *apdu = NULL, *apdu2 = NULL;
	std::vector<APDU*> apdus;

	apdu = APDU::BuildAPDUFromFrame(*this);
	if ( NULL == apdu )
	{
		retcode = -11;
		goto _out;
	}

	retcode = apdu->Response(apdu2, apdus);
	if ( 0 != retcode || NULL == apdu2 )
	{
		retcode = -12;
		goto _out;
	}

	resp = Frame::BuildFrameFromAPDU(*apdu2);

_out:
	delete apdu2;
	delete apdu;
	for ( int i = 0; i < apdus.size(); i++ )
	{
		delete apdus[i];
	}

	return retcode;
}

// Response的新版本. 
int Frame::Response(std::vector<Frame*>& resps)const
{
	int retcode = 0;
	APDU *apdu = NULL, *apdu2 = NULL;
	std::vector<APDU*> apdus;
	Frame* frame = NULL;

	resps.clear();

	apdu = APDU::BuildAPDUFromFrame(*this);
	if ( NULL == apdu )
	{
		retcode = -11;
		goto _out;
	}

	retcode = apdu->Response(apdu2, apdus);
	if ( 0 != retcode && NULL == apdu2 && apdus.empty() )
	{
		retcode = -12;
		goto _out;
	}

	if ( NULL != apdu2 )
	{
		frame = Frame::BuildFrameFromAPDU(*apdu2);
		if ( NULL != frame )
		{
			resps.push_back(frame);
		}
	}

	for ( int i = 0; i < apdus.size(); i++ )
	{
		frame = Frame::BuildFrameFromAPDU(*apdus[i]);
		if ( NULL != frame )
		{
			resps.push_back(frame);
		}
	}

_out:
	delete apdu2;
	delete apdu;
	for ( int i = 0; i < apdus.size(); i++ )
	{
		delete apdus[i];
	}

	return retcode;
}

OneByteFrame::OneByteFrame(const APDU& apdu)
{
	m_frame_len = 1;
	m_frame_mem = new UCHAR[1];
	
	if ( apdu.IsAck() )
	{
		m_frame_mem[0] = 0xCC;
		m_type = ACK_FRAME;
	}
	else if ( apdu.IsNack() )
	{
		m_frame_mem[0] = 0x0C;
		m_type = NACK_FRAME;
	}
	else if ( apdu.IsBusy() )
	{
		m_frame_mem[0] = 0xC0;
		m_type = BUSY_FRAME;
	}
	else if ( apdu.IsNackbusy() )
	{
		m_frame_mem[0] = 0x00;
		m_type = NACKBUSY_FRAME;
	}
	else
	{
		// ...
		abort();
	}
}

const UCHAR* OneByteFrame::GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const
{
	apdu_mem = NULL;
	apdu_len = 0;

	return apdu_mem;
}

OneByteFrame* OneByteFrame::BuildFrameFromMem(const UCHAR* mem, const UCHAR len) // 旧版,将来会被废弃. 
{
	return BuildFrameFromBuff(mem, len);
}

OneByteFrame* OneByteFrame::BuildFrameFromBuff(const UCHAR* buff, const UCHAR len)
{
	OneByteFrame* frame = NULL;
	if ( 1 == len && NULL != buff )
	{
		switch ( buff[0] )
		{
		case 0xCC:
			frame = new OneByteFrame(ACK_FRAME);
			break;

		case 0x0C:
			frame = new OneByteFrame(NACK_FRAME);
			break;

		case 0xC0:
			frame = new OneByteFrame(BUSY_FRAME);
			break;

		case 0x00:
			frame = new OneByteFrame(NACKBUSY_FRAME);
			break;

		default:
			frame = NULL;
			break;
		}
	}

	return frame;
}

OneByteFrame::OneByteFrame(OneByteFrameType type)
{ 
	m_type = type;

	// 因为基类Frame里用的是delete []释放内存.这里如果是new UCHAR就会有 new 和 delete []不匹配的问题.虽然可能通过编译
	// 也没有运行时错误。但却是C++标准不提倡的. 
	m_frame_mem = new UCHAR[1];  

	m_frame_len = 1;

	switch (type)
	{
	case ACK_FRAME:
		*m_frame_mem = 0xCC;
		break;

	case NACK_FRAME:
		*m_frame_mem = 0x0C;
		break;

	case BUSY_FRAME:
		*m_frame_mem = 0xC0;
		break;

	case NACKBUSY_FRAME:
		*m_frame_mem = 0x00;
		break;

	default:
		break;
	}
}

OneByteFrame::OneByteFrame(const OneByteFrame& frame)
{
	m_type = frame.m_type;
	frame.CopyOfFrameData(m_frame_mem, m_frame_len);
}

BOOL OneByteFrame::DoesPeerUseGroupAddress()const
{
	return FALSE;
}

Frame* OneByteFrame::CloneMe()const
{
	OneByteFrame* one_byte_frame = new OneByteFrame(*this);
	return one_byte_frame;
}

StandardFrame::StandardFrame(const APDU& apdu)
{
	m_frame_len = 6 + apdu.GetApduLen() + 1;
	m_frame_mem = new UCHAR[m_frame_len];
	memset(m_frame_mem, 0, m_frame_len);

	// 设置控制字. 
	UCHAR& ctrl_code = m_frame_mem[0];
	ctrl_code |= 0x80;   // 第一位置一就是标准帧. 
	ctrl_code |= 0x20;   // 非重复的Frame. 
	ctrl_code |= 0x10;   // 数据帧. 
	UCHAR priority = apdu.GetPriority();
	priority <<= 2;
	ctrl_code |= priority;  // 设优先级. 

	// 下面设源地址.  
	const UCHAR* hostaddr = apdu.GetHostAddr();
	m_frame_mem[1] = hostaddr[0];
	m_frame_mem[2] = hostaddr[1];

	// 下面设目标地址.  
	const UCHAR* peeraddr = apdu.GetPeerAddr();
	m_frame_mem[3] = peeraddr[0];
	m_frame_mem[4] = peeraddr[1];

	// 设置八位组5. 
	UCHAR& octec5 = m_frame_mem[5];
	if ( apdu.GetAddrType() ) // 单个地址时返回 0. 标准组地组时返回 1
	{
		octec5 |= 0x80;
	}
	octec5 |= 0x70;  // hop数被写死成7.
	octec5 |= apdu.GetApduLen();

	// 把APDU放入.
	memcpy(m_frame_mem + 6, apdu.GetApduMem(), apdu.GetApduLen());

	// 算checksum
	m_frame_mem[m_frame_len-1] = xor_check_sum(m_frame_mem, m_frame_len-1);
}

StandardFrame::StandardFrame(const StandardFrame& frame)
{
	frame.CopyOfFrameData(m_frame_mem, m_frame_len);
}

const UCHAR* StandardFrame::GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const
{
	apdu_mem = m_frame_mem + 6;
	apdu_len = m_frame_len - 6 - 1;
	return apdu_mem;
}

void StandardFrame::SetReptBit()
{
	m_frame_mem[0] &= 0xDF;   // 置重复标记.重复标记是bit5 为 0.
}

StandardFrame* StandardFrame::BuildFrameFromMem(const UCHAR* mem, UCHAR len, BOOL recved)
{
	StandardFrame* frame = NULL;
	int host_addr_pos = 0;
	int apdu_len, frame_expected_len;
	
	if ( len > 22 || len <= 7 || NULL == mem )
		goto _out;

	if ( 0 == (mem[0] & 0x80) ) // 标准帧的bit7位是1
	{
		goto _out;
	}

	if ( recved )
	{
		host_addr_pos = 3;
	}
	else
	{
		host_addr_pos = 1;
	}

	UCHAR buff[2];
	if ( 0 != memcmp(mem + host_addr_pos, g_myconfig.GetHostAddr_b(buff, sizeof(buff)), 2 ) )
	{
		goto _out;
	}

	apdu_len = ( mem[5] & 0x0f );
	frame_expected_len = 5 + 1 + apdu_len + 1;
	if ( frame_expected_len != len || mem[len-1] != xor_check_sum(mem, len-1) )
	{
		goto _out;
	}

	frame = new StandardFrame;
	frame->m_frame_len = len;
	frame->m_frame_mem = new UCHAR[len];
	memcpy(frame->m_frame_mem, mem, len);

_out:
	return frame;
}

StandardFrame* StandardFrame::BuildFrameFromBuff(const UCHAR* buff, UCHAR len, BOOL recved, BOOL check_address)
{
	StandardFrame* frame = NULL;
	int addr_checked_pos = 0;
	int apdu_len, frame_expected_len;
	BOOL check_group_address;
	
	if ( len > 22 || len <= 7 || NULL == buff )
		goto _out;

	if ( 0 == (*buff & 0x80) ) // 标准帧的bit7位是1
	{
		goto _out;
	}

	check_group_address = FALSE; // 是否把地址当作组地址看待. 
	if ( recved )
	{
		addr_checked_pos = 3;
		if ( buff[5] & 0x80 )
		{
			check_group_address = TRUE;
		}
	}
	else
	{
		addr_checked_pos = 1;
	}

	if ( check_address )  // 是否检查地址信息. 
	{
		BOOL matched = FALSE;	
		if ( !check_group_address )  // 检查单个地址. 
		{
			const UCHAR* host_addr_bin = NULL;
			for ( IDevice* dev = ms_dev_manager->NextDevice(true);
				NULL != dev;
				dev = ms_dev_manager->NextDevice(false))
			{
				host_addr_bin = dev->GetHostAddr(NULL); // 没有设置地址的设备有可能访问NULL.
				if ( host_addr_bin && 0 == memcmp(host_addr_bin, buff + addr_checked_pos, 2) )
				{
					matched = TRUE;
					break;
				}
			}

			// 和广播地址比较. 
			if ( !matched && 0 == memcmp(g_myconfig.GetBroadcastAddr(), buff + addr_checked_pos, 2) )
			{
				matched = TRUE;
			}
		}

		if ( check_group_address )
		{
			matched = ms_dev_manager->ExistThisGroupAddr(buff + addr_checked_pos);
		}

		if ( !matched )
		{
			goto _out;
		}
	}

	apdu_len = ( buff[5] & 0x0f );
	frame_expected_len = 5 + 1 + apdu_len + 1;  // 长度必须精确一致. 
	if ( frame_expected_len != len || buff[len-1] != xor_check_sum(buff, len-1) )
	{
		goto _out;
	}

	frame = new StandardFrame;
	frame->m_frame_len = len;
	frame->m_frame_mem = new UCHAR[len];
	memcpy(frame->m_frame_mem, buff, len);

_out:
	return frame;
}

const UCHAR* StandardFrame::GetPeerAddr()const
{
	return m_frame_mem + 3;
}

const UCHAR* StandardFrame::GetHostAddr()const
{
	return m_frame_mem + 1;
}

BOOL StandardFrame::DoesPeerUseGroupAddress()const
{
	UCHAR ch = m_frame_mem[5];
	ch &= 0x80;
	if ( ch )  // 组地址. 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Frame* StandardFrame::CloneMe()const
{
	StandardFrame* standard_frame = new StandardFrame(*this);
	return standard_frame;
}

ExtendedFrame::ExtendedFrame(const APDU& apdu)
{
	m_frame_len = 7 + apdu.GetApduLen() + 1;
	m_frame_mem = new UCHAR[m_frame_len];
	memset(m_frame_mem, 0, m_frame_len);

	// 设置控制字. 
	UCHAR& ctrl_code = m_frame_mem[0];
	ctrl_code &= 0x7F;   // 第一位置0就是扩展帧. 
	ctrl_code |= 0x20;   // 非重复的Frame. 
	ctrl_code |= 0x10;   // 数据帧. 
	UCHAR priority = apdu.GetPriority();
	priority <<= 2;
	ctrl_code |= priority;  // 设优先级. 

	// 设置扩展字. 
	UCHAR& ctrle_code = m_frame_mem[1];
	if ( apdu.GetAddrType() )
	{
		ctrle_code |= 0x80;
	}
	ctrle_code |= 0x70;  // Hop数写死为7.

	// 下面设源地址. 
	const UCHAR* hostaddr = apdu.GetHostAddr();
	m_frame_mem[2] = hostaddr[0];
	m_frame_mem[3] = hostaddr[1];

	// 下面设目标地址.  
	const UCHAR* peeraddr = apdu.GetPeerAddr();
	m_frame_mem[4] = peeraddr[0];
	m_frame_mem[5] = peeraddr[1];

	// 设置长度. 
	m_frame_mem[6] = apdu.GetApduLen();

	// 把APDU放入. 
	memcpy(m_frame_mem + 7, apdu.GetApduMem(), apdu.GetApduLen());

	// 算checksum. 
	m_frame_mem[m_frame_len-1] = xor_check_sum(m_frame_mem, m_frame_len-1);
}

ExtendedFrame::ExtendedFrame(const ExtendedFrame& frame)
{
	frame.CopyOfFrameData(m_frame_mem, m_frame_len);
}

Frame* ExtendedFrame::CloneMe()const
{
	ExtendedFrame* extended_frame = new ExtendedFrame(*this);
	return extended_frame;
}

const UCHAR* ExtendedFrame::GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const
{
	apdu_mem = m_frame_mem + 7;
	apdu_len = m_frame_len - 7 - 1;
	return apdu_mem;
}

void ExtendedFrame::SetReptBit()
{
	m_frame_mem[0] |= 0x40;   // 置重复标记. 
}

ExtendedFrame* ExtendedFrame::BuildFrameFromMem(const UCHAR* mem, const UCHAR len, BOOL recved)
{
	ExtendedFrame* frame = NULL;
	int host_addr_pos = 0;
	UCHAR buff[2];
	int apdu_len, frame_expected_len;

	if ( len <= 22 || NULL == mem )
		goto _out;

	if ( 0 != ( mem[0] & 0x80 ) ) // 扩展帧的bit7位是0
	{
		goto _out;
	}

	// 处理扩展字段的逻辑.暂时木有。
	if ( mem[1] )
	{
		// ...
	}

	if ( recved )
	{
		host_addr_pos = 4;
	}
	else
	{
		host_addr_pos = 2;
	}

	if ( 0 != memcmp(mem + host_addr_pos, g_myconfig.GetHostAddr_b(buff, sizeof(buff)), 2 ) )
	{
		goto _out;
	}

	apdu_len = mem[6];
	frame_expected_len = 6 + 1 + apdu_len + 1;
	if ( frame_expected_len != len || mem[len-1] != xor_check_sum(mem, len-1) )
	{
		goto _out;
	}

	frame = new ExtendedFrame;
	frame->m_frame_len = len;
	frame->m_frame_mem = new UCHAR[len];
	memcpy(frame->m_frame_mem, mem, len);

_out:
	return frame;
}

ExtendedFrame* ExtendedFrame::BuildFrameFromBuff(const UCHAR* buff, const UCHAR len, BOOL recved, BOOL check_address)
{
	ExtendedFrame* frame = NULL;
	int addr_checked_pos = 0;
	int apdu_len, frame_expected_len;
	BOOL check_group_address;

	if ( len <= 22 || NULL == buff )
		goto _out;

	if ( 0 != ( *buff & 0x80 ) ) // 扩展帧的bit7位是0
	{
		goto _out;
	}

	// 处理扩展字段的逻辑.暂时木有。
	if ( buff[1] )
	{
		// ...
	}

	check_group_address = FALSE; // 是否把地址当作组地址看待. 
	if ( recved )
	{
		addr_checked_pos = 4;
		if ( buff[1] & 0x80 )
		{
			check_group_address = TRUE;
		}
	}
	else
	{
		addr_checked_pos = 2;
	}

	if ( check_address )
	{
		BOOL matched = FALSE;
		if ( !check_group_address )
		{
			const UCHAR* host_addr_bin = NULL;

			for ( IDevice* dev = ms_dev_manager->NextDevice(true);
				NULL != dev;
				dev = ms_dev_manager->NextDevice(false))
			{
				host_addr_bin = dev->GetHostAddr(NULL);
				if ( host_addr_bin && 0 == memcmp(host_addr_bin, buff + addr_checked_pos, 2) )
				{
					matched = TRUE;
					break;
				}
			}

			if ( !matched && 0 == memcmp(g_myconfig.GetBroadcastAddr(), buff + addr_checked_pos, 2) )
			{
				matched = TRUE;
			}
		}

		if ( check_group_address )
		{
			matched = ms_dev_manager->ExistThisGroupAddr(buff+addr_checked_pos);
		}

		if ( !matched )
		{
			goto _out;
		}
	}

	apdu_len = buff[6];
    frame_expected_len = 6 + 1 + apdu_len + 1;
	if ( frame_expected_len != len || buff[len-1] != xor_check_sum(buff, len-1) )
	{
		goto _out;
	}

	frame = new ExtendedFrame;
	frame->m_frame_len = len;
	frame->m_frame_mem = new UCHAR[len];
	memcpy(frame->m_frame_mem, buff, len);

_out:
	return frame;
}

const UCHAR* ExtendedFrame::GetPeerAddr()const
{
	return m_frame_mem + 4;
}

const UCHAR* ExtendedFrame::GetHostAddr()const
{
	return m_frame_mem + 2;
}

BOOL ExtendedFrame::DoesPeerUseGroupAddress()const
{
	UCHAR ch = m_frame_mem[1];
	ch &= 0x80;
	if ( ch )  // 组地址. 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}