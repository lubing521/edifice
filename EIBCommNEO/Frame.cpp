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
//	else if ( len <= 16 && len >= 1 )  // ���ɱ�׼֡. 
	else if ( len < 16 && len >= 1 )  // ���ɱ�׼֡. 
	{
		frame = new StandardFrame(apdu);
	}
	else  // ������չ֡. 
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

int Frame::Response(Frame*& resp)const  // �ɴ���.�Ժ�Ҫ������������
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

// Response���°汾. 
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

OneByteFrame* OneByteFrame::BuildFrameFromMem(const UCHAR* mem, const UCHAR len) // �ɰ�,�����ᱻ����. 
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

	// ��Ϊ����Frame���õ���delete []�ͷ��ڴ�.���������new UCHAR�ͻ��� new �� delete []��ƥ�������.��Ȼ����ͨ������
	// Ҳû������ʱ���󡣵�ȴ��C++��׼���ᳫ��. 
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

	// ���ÿ�����. 
	UCHAR& ctrl_code = m_frame_mem[0];
	ctrl_code |= 0x80;   // ��һλ��һ���Ǳ�׼֡. 
	ctrl_code |= 0x20;   // ���ظ���Frame. 
	ctrl_code |= 0x10;   // ����֡. 
	UCHAR priority = apdu.GetPriority();
	priority <<= 2;
	ctrl_code |= priority;  // �����ȼ�. 

	// ������Դ��ַ.  
	const UCHAR* hostaddr = apdu.GetHostAddr();
	m_frame_mem[1] = hostaddr[0];
	m_frame_mem[2] = hostaddr[1];

	// ������Ŀ���ַ.  
	const UCHAR* peeraddr = apdu.GetPeerAddr();
	m_frame_mem[3] = peeraddr[0];
	m_frame_mem[4] = peeraddr[1];

	// ���ð�λ��5. 
	UCHAR& octec5 = m_frame_mem[5];
	if ( apdu.GetAddrType() ) // ������ַʱ���� 0. ��׼�����ʱ���� 1
	{
		octec5 |= 0x80;
	}
	octec5 |= 0x70;  // hop����д����7.
	octec5 |= apdu.GetApduLen();

	// ��APDU����.
	memcpy(m_frame_mem + 6, apdu.GetApduMem(), apdu.GetApduLen());

	// ��checksum
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
	m_frame_mem[0] &= 0xDF;   // ���ظ����.�ظ������bit5 Ϊ 0.
}

StandardFrame* StandardFrame::BuildFrameFromMem(const UCHAR* mem, UCHAR len, BOOL recved)
{
	StandardFrame* frame = NULL;
	int host_addr_pos = 0;
	int apdu_len, frame_expected_len;
	
	if ( len > 22 || len <= 7 || NULL == mem )
		goto _out;

	if ( 0 == (mem[0] & 0x80) ) // ��׼֡��bit7λ��1
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

	if ( 0 == (*buff & 0x80) ) // ��׼֡��bit7λ��1
	{
		goto _out;
	}

	check_group_address = FALSE; // �Ƿ�ѵ�ַ�������ַ����. 
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

	if ( check_address )  // �Ƿ����ַ��Ϣ. 
	{
		BOOL matched = FALSE;	
		if ( !check_group_address )  // ��鵥����ַ. 
		{
			const UCHAR* host_addr_bin = NULL;
			for ( IDevice* dev = ms_dev_manager->NextDevice(true);
				NULL != dev;
				dev = ms_dev_manager->NextDevice(false))
			{
				host_addr_bin = dev->GetHostAddr(NULL); // û�����õ�ַ���豸�п��ܷ���NULL.
				if ( host_addr_bin && 0 == memcmp(host_addr_bin, buff + addr_checked_pos, 2) )
				{
					matched = TRUE;
					break;
				}
			}

			// �͹㲥��ַ�Ƚ�. 
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
	frame_expected_len = 5 + 1 + apdu_len + 1;  // ���ȱ��뾫ȷһ��. 
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
	if ( ch )  // ���ַ. 
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

	// ���ÿ�����. 
	UCHAR& ctrl_code = m_frame_mem[0];
	ctrl_code &= 0x7F;   // ��һλ��0������չ֡. 
	ctrl_code |= 0x20;   // ���ظ���Frame. 
	ctrl_code |= 0x10;   // ����֡. 
	UCHAR priority = apdu.GetPriority();
	priority <<= 2;
	ctrl_code |= priority;  // �����ȼ�. 

	// ������չ��. 
	UCHAR& ctrle_code = m_frame_mem[1];
	if ( apdu.GetAddrType() )
	{
		ctrle_code |= 0x80;
	}
	ctrle_code |= 0x70;  // Hop��д��Ϊ7.

	// ������Դ��ַ. 
	const UCHAR* hostaddr = apdu.GetHostAddr();
	m_frame_mem[2] = hostaddr[0];
	m_frame_mem[3] = hostaddr[1];

	// ������Ŀ���ַ.  
	const UCHAR* peeraddr = apdu.GetPeerAddr();
	m_frame_mem[4] = peeraddr[0];
	m_frame_mem[5] = peeraddr[1];

	// ���ó���. 
	m_frame_mem[6] = apdu.GetApduLen();

	// ��APDU����. 
	memcpy(m_frame_mem + 7, apdu.GetApduMem(), apdu.GetApduLen());

	// ��checksum. 
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
	m_frame_mem[0] |= 0x40;   // ���ظ����. 
}

ExtendedFrame* ExtendedFrame::BuildFrameFromMem(const UCHAR* mem, const UCHAR len, BOOL recved)
{
	ExtendedFrame* frame = NULL;
	int host_addr_pos = 0;
	UCHAR buff[2];
	int apdu_len, frame_expected_len;

	if ( len <= 22 || NULL == mem )
		goto _out;

	if ( 0 != ( mem[0] & 0x80 ) ) // ��չ֡��bit7λ��0
	{
		goto _out;
	}

	// ������չ�ֶε��߼�.��ʱľ�С�
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

	if ( 0 != ( *buff & 0x80 ) ) // ��չ֡��bit7λ��0
	{
		goto _out;
	}

	// ������չ�ֶε��߼�.��ʱľ�С�
	if ( buff[1] )
	{
		// ...
	}

	check_group_address = FALSE; // �Ƿ�ѵ�ַ�������ַ����. 
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
	if ( ch )  // ���ַ. 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}