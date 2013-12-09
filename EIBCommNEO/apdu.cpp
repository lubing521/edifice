#include "stdafx.h"
#include <vector>
#include <assert.h>
#include "generic_helper.h"
#include "APDU.h"
#include "MyConfig.h"
#include "Frame.h"

DeviceManager* APDU::ms_dev_manager;

const std::vector<APDU*>& APDUContainer::GetAPDUVec()
{
	WaitForSingleObject(m_mutex, INFINITE);
	const std::vector<APDU*>& apdu_vec = GetAPDUVec_nolock();
	ReleaseMutex(m_mutex);

	return apdu_vec;
}

const std::vector<APDUType>& APDUContainer::GetExpectedAPDUTypesForListen()
{
	WaitForSingleObject(m_mutex, INFINITE);
	if ( 0 != m_vec_expected_apdutypes4listen.size() )
	{
		goto _out;
	}
	else
	{
		int retcode;
		APDUType apdutype;
		std::vector<APDUType> adputypes;
		const std::vector<APDU*>& apdu_vec = GetAPDUVec_nolock();
		for ( std::vector<APDU*>::const_iterator ite = apdu_vec.begin();
			  ite != apdu_vec.end();
			  ++ite )
		{
			retcode = (*ite)->GetExpectedAPDUTypes(adputypes);
			if ( retcode == 0 )
			{
				apdutype = (*ite)->GetAPDUType();
				m_vec_expected_apdutypes4listen.push_back(apdutype);
			}
		}
	}
_out:
	ReleaseMutex(m_mutex);
	return m_vec_expected_apdutypes4listen;
}

APDUContainer& APDUContainer::GetInstance()
{
	static APDUContainer container;
	return container;
}


APDUContainer::APDUContainer()
{
	m_mutex = CreateMutex(NULL, false, NULL);
	if ( NULL == m_mutex )
	{
		throw "APDUContainer::APDUContainer CreateMutex error.";
	}
}

const std::vector<APDU*>& APDUContainer::GetAPDUVec_nolock()
{
	if ( 0 != m_vec_apdu.size() )
	{
		return m_vec_apdu;
	}

	// 所有的APDU都加入到这里. 
	m_vec_apdu.push_back(new APDU_ACK);
	m_vec_apdu.push_back(new APDU_NACK);
	m_vec_apdu.push_back(new APDU_BUSY);
	m_vec_apdu.push_back(new APDU_NACKBUSY);
	m_vec_apdu.push_back(new APDU_PropertyValue_Read);
	m_vec_apdu.push_back(new APDU_PropertyValue_Response);
	m_vec_apdu.push_back(new APDU_PropertyValue_Write);
	m_vec_apdu.push_back(new APDU_Property_Read);
	m_vec_apdu.push_back(new APDU_Property_Response);
	m_vec_apdu.push_back(new APDU_Property_Write);
	m_vec_apdu.push_back(new APDU_GroupValue_Read);
	m_vec_apdu.push_back(new APDU_GroupValue_Response);
	m_vec_apdu.push_back(new APDU_GroupValue_Write);
	m_vec_apdu.push_back(new APDU_Link_Read);
	m_vec_apdu.push_back(new APDU_Link_Response);
	m_vec_apdu.push_back(new APDU_Link_Write);
	m_vec_apdu.push_back(new APDU_Authorize_Request);
	m_vec_apdu.push_back(new APDU_Authorize_Response);
	m_vec_apdu.push_back(new APDU_UserMemory_Read);
	m_vec_apdu.push_back(new APDU_UserMemory_Response);
	m_vec_apdu.push_back(new APDU_UserMemory_Write);
	m_vec_apdu.push_back(new APDU_IndividualAddress_Write);
	m_vec_apdu.push_back(new APDU_IndividualAddress_Read);
	m_vec_apdu.push_back(new APDU_IndividualAddress_Response);
	m_vec_apdu.push_back(new APDU_IndividualAddressSerialNumber_Read);
	m_vec_apdu.push_back(new APDU_IndividualAddressSerialNumber_Response);
	m_vec_apdu.push_back(new APDU_IndividualAddressSerialNumber_Write);

	return m_vec_apdu;
}

APDUContainer::~APDUContainer()
{
	WaitForSingleObject(m_mutex, INFINITE);
	if ( 0 != m_vec_apdu.size() )
	{
		for ( int i = 0; i < m_vec_apdu.size(); i++ )
		{
			delete m_vec_apdu[i];
		}
	}
	ReleaseMutex(m_mutex);
}

//APDU_ACK                 apdu_ack;
//APDU_NACK                apdu_nack;
//APDU_BUSY                apdu_busy;
//APDU_NACKBUSY            apdu_nackbusy;
//APDU_PropertyValue_Read  apdu_propertyvalue_read;
//APDU_PropertyValue_Response apdu_propertyvalue_response;
//APDU_PropertyValue_Write apdu_propertyvalue_write;
//APDU_Property_Read       apdu_property_read;
//APDU_Property_Response   apdu_property_response;
//APDU_Property_Write      apdu_property_write;
//APDU_GroupValue_Read     apdu_groupvalue_read;
//APDU_GroupValue_Response apdu_groupvalue_response;
//APDU_GroupValue_Write    apdu_groupvalue_write;
//APDU_Link_Read           apdu_link_read; 
//APDU_Link_Response       apdu_link_response;
//APDU_Link_Write          apdu_link_write;
//APDU_Authorize_Request   apdu_authorize_request;
//APDU_Authorize_Response  apdu_authorize_response;
//APDU_UserMemory_Read     apdu_usermemory_read;
//APDU_UserMemory_Response apdu_usermemory_response;
//APDU_UserMemory_Write    apdu_usermemory_write;
//
//const APDU* apdu_ary[] = { 
//		&apdu_ack,  
//        &apdu_nack,
//		&apdu_busy,
//		&apdu_nackbusy,
//		&apdu_propertyvalue_read,
//		&apdu_propertyvalue_response,
//		&apdu_propertyvalue_write,
//		&apdu_property_read,
//		&apdu_property_response,
//		&apdu_property_write,
//		&apdu_authorize_request,
//		&apdu_authorize_response,
//		&apdu_groupvalue_read,
//		&apdu_groupvalue_response,
//		&apdu_groupvalue_write,
//		&apdu_link_read,
//		&apdu_link_response,
//		&apdu_link_write,
//		&apdu_usermemory_read,
//		&apdu_usermemory_response,
//		&apdu_usermemory_write
//};

APDU::APDU(void)
{
	m_apdu_len = 0;
	m_apdu_mem = NULL;
	m_addrtype = 0;  // 单个地址. 
	m_recved_apdu = FALSE;

	m_apci_l_mask = 255;
	m_apci_h_mask = 255;

	// 这里耦合度？......去管理配置的类里拿host地址.旧代码.以后会有变动！！！ 
	g_myconfig.GetHostAddr_b(m_hostaddr, 2);
	//	int lenOfhostaddr = 2;
	//	hexs2bins(g_myconfig.GetHostAddr().c_str(), m_hostaddr, &lenOfhostaddr);
	//if ( 2 != lenOfhostaddr )
	//{
	//	std::string err = "配置文件中的 host 地址错.";
	//	report_err_immed(err.c_str(), 0xe);
	//}
}

APDU::~APDU(void)
{
	delete []m_apdu_mem;
}

APDU* APDU::BuildAPDUFromFrame(const Frame& frame)
{
	APDU*  apdu = NULL;
	const std::vector<APDU*>& apdus = APDUContainer::GetInstance().GetAPDUVec();
	const UCHAR* apdu_mem = NULL;
	UCHAR apdu_len = 0;
	frame.GetApduMem(apdu_mem, apdu_len);

	if ( NULL == apdu_mem ) // 属于确认帧. 
	{
		const OneByteFrame* obf = dynamic_cast<const OneByteFrame*>(&frame);
		if ( NULL != obf )
		{
			OneByteFrame::OneByteFrameType type = obf->GetType();
			switch ( type )
			{
			case OneByteFrame::ACK_FRAME:
				apdu = new APDU_ACK;
				break;

			case OneByteFrame::NACK_FRAME:
				apdu = new APDU_NACK;
				break;

			case OneByteFrame::BUSY_FRAME:
				apdu = new APDU_BUSY;
				break;

			case OneByteFrame::NACKBUSY_FRAME:
				apdu = new APDU_NACKBUSY;
				break;

			default:
				apdu = NULL;
				break;
			}
		}
		else
		{
			log_me("APDU::BuildAPDUFromFrame: unable to dynamic_cast frame.");
		}

		goto _out;
	}

	// Frame里包装里的是一般的apdu. 
	// int count_of_apdutype = sizeof(apdu_ary)/sizeof(APDU*);
	for ( int i = 0; i < apdus.size(); i++ )
	{
		const APDU* apdu_ptr = apdus[i];
		apdu = apdu_ptr->Spawn(apdu_mem, apdu_len); 
		// 找到了APDU的类型，于是生成了一个它的对象.
		if ( NULL != apdu )
		{
			apdu->SetHostAddr(frame.GetHostAddr());  // 设好host地址. 
			apdu->BasicInfo(0,0,7,frame.GetPeerAddr());  // 和peer地址. 
			apdu->m_recved_apdu = TRUE;
			break;
		}
	}

_out:
	return apdu;
}

DeviceManager* APDU::SetDeviceManager(DeviceManager* manager)
{
	DeviceManager* save = ms_dev_manager;
	ms_dev_manager = manager;
	return save;
}

const UCHAR* APDU::GetAPCI(UCHAR* apci_l_p, UCHAR* apci_h_p)const
{
	if ( IsStandard() )
	{
		if ( NULL != apci_l_p )
		{
			*apci_l_p = m_apci_l;
		}
		if ( NULL != apci_h_p )
		{
			*apci_h_p = m_apci_h;
		}

		static UCHAR buff[3];
		buff[0] = m_apci_l;
		buff[1] = m_apci_h;
		buff[2] = 0;

		return buff;
	}
	else
	{
		return NULL;
	}
}

APDU* APDU::Spawn(const UCHAR* apdu_mem, int len)const
{
	if ( len < 2 || !ApciMatch(apdu_mem) ) // 还要检查acpi. 
	{
		return NULL;
	}

	APDU* apdu = VConstruct();
	apdu->Resize(len);
	memcpy(apdu->m_apdu_mem, apdu_mem, len);
	return apdu;
}

int APDU::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	apdu = NULL;
	return -1;
}

int APDU::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{
	expected_bytes.clear();
	return -1;
}

int APDU::Resize(int size)
{
	if ( size > 0 )
	{
		delete []m_apdu_mem;
		m_apdu_mem = new UCHAR[size];
		m_apdu_len = size;

		if ( size >= 2  )
		{
			m_apdu_mem[0] = m_apci_l;
			m_apdu_mem[1] = m_apci_h;
		}

		return 0;
	}
	else
	{
		return -1;
	}
}

void APDU::BasicInfo(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap)
{
	m_ack_request = ack_request;
	m_priority = priority;
	m_hop_count_type = hop_count_type;
	SetPeerAddr(asap);
}

bool APDU::ApciMatch(const UCHAR* apci)const
{
	if ( NULL == apci )
	{
		return false;
	}

	uchar apci_l = (apci[0] & m_apci_l_mask);
	uchar apci_h = (apci[1] & m_apci_h_mask);
	if ( apci_l == m_apci_l && apci_h == m_apci_h )
	{
		return true;
	}

	return false;
}

int APDU_PropertyValue_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
									 UCHAR object_index, UCHAR property_id, UCHAR no_of_elem, USHORT start_index)
{
	if ( NULL == asap || no_of_elem > 15 )  // 参数检查. 
	{
		return -1;
	}

	Resize(6);  // APDU_PropertyValue_Read 的 APDU 就是 6 个 octec 长.

	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = object_index;
	m_apdu_mem[3] = property_id;
	unsigned char tedious = ( (no_of_elem<<4) | ( *((unsigned char*)&start_index + 1) & 0x0f) );
	m_apdu_mem[4] = tedious;  // 真难算. 
	m_apdu_mem[5] = *(unsigned char*)&start_index;

	return 0;
}

int APDU_PropertyValue_Read::GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, USHORT* start_index)const
{
	if ( m_apdu_len != 6 )
	{
		return -1;
	}

	if ( NULL != object_index )
	{
		*object_index = m_apdu_mem[2];
	}

	if ( NULL != property_id )
	{
		*property_id = m_apdu_mem[3];
	}

	if ( NULL != no_of_elem )
	{
		UCHAR uc = m_apdu_mem[4];
		*no_of_elem = (uc >> 4);
	}

	if ( NULL != start_index )
	{
		const char* mask = "000000000fff";
		*start_index = mask_num(m_apdu_mem, mask);
	}

	return 0;
}

int APDU_PropertyValue_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( m_recved_apdu )  // 收到的apdu才需要回复. 
	{
		int retcode = 0;
		UCHAR object_index;
		UCHAR property_id;
		UCHAR  no_of_elem;
		USHORT start_index;
		GetParams(&object_index, &property_id, &no_of_elem, &start_index);
		APDU_PropertyValue_Response* apdu_pv_r = new APDU_PropertyValue_Response;
		apdu_pv_r->SetHostAddr(m_peeraddr);
		retcode = apdu_pv_r->BuildMe(0, 0, 7, m_hostaddr, object_index, property_id, no_of_elem, start_index);
		if ( 0 == retcode )
		{
			apdu = apdu_pv_r;
		}
		else
		{
			delete apdu_pv_r;
			apdu = new APDU_NACK;
		}

		return 0;
	}
	else
	{
		apdu = NULL;
		return -2;
	}
}

int APDU_PropertyValue_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_PropertyValue_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_PropertyValue_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR *asap, 
										 UCHAR object_index, UCHAR property_id, UCHAR no_of_elem, USHORT start_index)
{
	if ( NULL == asap || no_of_elem > 15 )  // 参数检查. 
	{
		return -1;
	}

	int retcode = -1;
	unsigned char buff[1024] = { 0 };
	int buff_len = sizeof(buff);
	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_hostaddr);
	if ( NULL != dev )
	{
		retcode = dev->GetElemValues(object_index, property_id, start_index, no_of_elem, buff, buff_len);
	}
	if ( 0 == retcode )
	{
		Resize(6 + buff_len);
		BasicInfo(ack_request, priority, hop_count_type, asap);

		m_apdu_mem[2] = object_index;
		m_apdu_mem[3] = property_id;
		unsigned char tedious = ( (no_of_elem<<4) | ( *((unsigned char*)&start_index + 1) & 0x0f) );
		m_apdu_mem[4] = tedious;  // 真难算. 
		m_apdu_mem[5] = *(unsigned char*)&start_index;
		memcpy(m_apdu_mem + 6, buff, buff_len);
	}

	return retcode;
}

int APDU_PropertyValue_Response::GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, 
										   USHORT* start_index, UCHAR* buff, int* len)const
{
	if ( m_apdu_len < 6 )
	{
		return -1;
	}

	if ( NULL != object_index )
	{
		*object_index = m_apdu_mem[2];
	}

	if ( NULL != property_id )
	{
		*property_id =m_apdu_mem[3];
	}

	if ( NULL != no_of_elem )
	{
		UCHAR uc = m_apdu_mem[4];
		*no_of_elem = (uc >> 4);
	}

	if ( NULL != start_index )
	{
		const char* mask = "000000000fff";
		*start_index = mask_num(m_apdu_mem, mask);
	}

	if ( NULL != buff && NULL != len )
	{
		int data_len = m_apdu_len - 6;
		if ( *len < data_len )
		{
			*len = -data_len;
			return -1;
		}
		else
		{
			*len = data_len;
		}
		memcpy(buff, m_apdu_mem + 6, data_len);
	}

	return 0;
}

int APDU_PropertyValue_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR *asap, 
									  UCHAR object_index, UCHAR property_id, UCHAR no_of_elem, USHORT start_index, 
									  const UCHAR *buff, UCHAR buff_len)
{
	if ( NULL == asap || NULL == buff || 0 == buff_len || no_of_elem > 15 )
	{
		return -1;
	}

	Resize(6 + buff_len);

	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = object_index;
	m_apdu_mem[3] = property_id;
	unsigned char tedious = ( (no_of_elem<<4) | ( *((unsigned char*)&start_index + 1) & 0x0f) );
	m_apdu_mem[4] = tedious;  // 真难算. 
	m_apdu_mem[5] = *(unsigned char*)&start_index;

	memcpy(m_apdu_mem + 6, buff, buff_len);

	return 0;
}

int APDU_PropertyValue_Write::GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, USHORT* start_index, 
										UCHAR* buff, int* len)const
{
	if ( m_apdu_len < 6 )
	{
		return -1;
	}

	if ( NULL != object_index )
	{
		*object_index = m_apdu_mem[2];
	}

	if ( NULL != property_id )
	{
		*property_id = m_apdu_mem[3];
	}

	if ( NULL != no_of_elem )
	{
		UCHAR uc = m_apdu_mem[4];
		*no_of_elem = (uc >> 4);
	}

	if ( NULL != start_index )
	{
		const char* mask = "000000000fff";
		*start_index = mask_num(m_apdu_mem, mask);
	}

	if ( NULL != buff && NULL != len )
	{
		int data_len = m_apdu_len - 6;
		if ( *len < data_len )
		{
			*len = -1;
			return -1;
		}
		memcpy(buff, m_apdu_mem + 6, data_len);
		*len = data_len;
	}

	return 0;
}

int APDU_PropertyValue_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	int retcode = -1;
	UCHAR buff[1024] = { 0 };
	int buff_len = sizeof(buff);
	UCHAR object_index;
	UCHAR property_id;
	UCHAR no_of_elem;
	USHORT start_index;
	GetParams(&object_index, &property_id, &no_of_elem, &start_index, buff, &buff_len);	

	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_peeraddr);
	if ( NULL != dev )
	{
		retcode = dev->SetElemValues(object_index, property_id, start_index, no_of_elem, buff, buff_len);
	}

	if ( 0 == retcode )
	{
		apdu = new APDU_ACK;
	}
	else
	{
		apdu = new APDU_NACK;
	}

	return 0;
}

int APDU_PropertyValue_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_ACK);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_Property_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
								UCHAR object_index, UCHAR property_id)
{
	if ( NULL == asap )
	{
		return -1;
	}

	Resize(4);

	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = object_index;
	m_apdu_mem[3] = property_id;

	return 0;
}

int APDU_Property_Read::GetParams(UCHAR* object_index, UCHAR* property_id)const
{
	if ( m_apdu_len != 4 )
	{
		return -1;
	}

	if ( NULL != object_index )
	{
		*object_index = m_apdu_mem[2];
	}

	if ( NULL != property_id )
	{
		*property_id = m_apdu_mem[3];
	}

	return 0;
}

int APDU_Property_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -2;
	}

	int retcode = 0;
	UCHAR object_index;
	UCHAR property_id;
	GetParams(&object_index, &property_id);

	APDU_Property_Response* apdu_p_r = new APDU_Property_Response;
	apdu_p_r->SetHostAddr(m_peeraddr);
	retcode = apdu_p_r->BuildMe(0, 0, 7, m_hostaddr, object_index, property_id);
	if ( 0 == retcode )
	{
		apdu = apdu_p_r;
	}
	else
	{
		delete apdu_p_r;
		apdu = new APDU_NACK;
	}

	return 0;
}

int APDU_Property_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_Property_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_Property_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR *asap, 
									UCHAR object_index, UCHAR property_id)
{
	if ( NULL == asap )
	{
		return -1;
	}

	int retcode = -1;
	PropertyFields fields;
	Resize(2 + sizeof(PropertyFields));
	BasicInfo(ack_request, priority, hop_count_type, asap);

	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_hostaddr);
	if ( NULL != dev )
	{
		retcode = dev->GetPropFields(object_index, property_id, fields);
	}
	if ( 0 == retcode )
	{
		memcpy(m_apdu_mem + 2, &fields, sizeof(PropertyFields));
	}

	return retcode;
}

int APDU_Property_Response::GetParams(UCHAR* object_index, UCHAR* property_id, PropertyFields* fields)const
{
	if ( m_apdu_len != 2 + sizeof(PropertyFields) )
	{
		return -1;
	}

	if ( NULL != object_index )
	{
		*object_index = m_apdu_mem[2];
	}

	if ( NULL != property_id )
	{
		*property_id = m_apdu_mem[3];
	}

	if ( NULL != fields )
	{
		memcpy(fields, m_apdu_mem + 2, sizeof(PropertyFields));
	}

	return 0;
}

int APDU_Property_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR *asap,
								 UCHAR object_index, UCHAR property_id, const PropertyFields &fields)
{
	if ( NULL == asap )
	{
		return -1;
	}

	Resize(2 + sizeof(PropertyFields));

	BasicInfo(ack_request, priority, hop_count_type, asap);

	memcpy(m_apdu_mem + 2, &fields, sizeof(fields));

	return 0;
}

int APDU_Property_Write::GetParams(UCHAR* object_index, UCHAR* property_id, PropertyFields* fields)const
{
	if ( m_apdu_len != 2 + sizeof(PropertyFields) )
	{
		return -1;
	}

	if ( NULL != object_index )
	{
		*object_index = m_apdu_mem[2];
	}

	if ( NULL != property_id )
	{
		*property_id = m_apdu_mem[3];
	}

	if ( NULL != fields )
	{
		memcpy(fields, m_apdu_mem + 2, sizeof(PropertyFields));
	}

	return 0;
}

int APDU_Property_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	int retcode = -1;
	UCHAR object_index;
	UCHAR prop_id;
	PropertyFields fields;
	GetParams(&object_index, &prop_id, &fields);
	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_peeraddr);
	if ( NULL != dev )
	{
		retcode = dev->SetPropFields(object_index, prop_id, fields);
	}
	if ( 0 == retcode )
	{
		apdu = new APDU_ACK;
	}
	else
	{
		apdu = new APDU_NACK;
	}

	return 0;
}

int APDU_Property_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_ACK);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_GroupValue_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap)
{
	if ( NULL == asap )
	{
		return -1;
	}

	Resize(2);
	BasicInfo(ack_request, priority, hop_count_type, asap);

	return 0;
}

int APDU_GroupValue_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -2;
	}

	apdu = NULL;
	apdus.clear();

	unsigned int buff[56] = { 0 };
	int buff_len = sizeof(buff)/sizeof(int);
	int dev_count = ms_dev_manager->GetAllDevices(buff, buff_len);
	APDU_GroupValue_Response* apdu_g_r = NULL;
	for ( int i = 0; i < dev_count; i++ )
	{
		int  okay;
		uint dev_id;
		dev_id = *(UINT*)( buff + i );
		apdu_g_r = new APDU_GroupValue_Response;
		okay = apdu_g_r->BuildMe(0, 0, 7, m_peeraddr, dev_id);
		if ( 0 == okay )
		{
			apdu_g_r->SetPeerAddr(m_peeraddr);
			apdus.push_back(apdu_g_r);
		}
		else
		{
			delete apdu_g_r;
		}
	}

	return 0;	
}

int APDU_GroupValue_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_GroupValue_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_GroupValue_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, 
									  const UCHAR* grp_addr, UINT dev_id)
{
	if ( NULL == grp_addr )
	{
		return -1;
	}

	IDevice* dev = ms_dev_manager->GetDeviceByID(dev_id);
	if ( NULL == dev )
	{
		return -1;
	}

	unsigned char buff[256] = { 0 };
	int buff_len = sizeof(buff);
	int grp_addr_count = dev->GetGroupAddrCount();
	int grp_addr_index = -1;
	for ( int i = 0; i < grp_addr_count; i++ )
	{
		const unsigned char* addr = dev->GetGroupAddrByIndex(i);
		if ( addr[0] == grp_addr[0] && addr[1] == grp_addr[1] )
		{
			grp_addr_index = i;
			break;
		}
	}
	if ( grp_addr_index == -1 )  // 该设备没有加入这个组地址. 
	{
		return -1;
	}

	int link_count = dev->GetLinkCount();
	int grp_obj_index = -1;
	for ( int i = 0; i < link_count; i++ )
	{
		uchar gobj_index = -1;
		uchar gaddr_index = -1;
		dev->GetLinkByIndex(i, gobj_index, gaddr_index);
		if ( grp_addr_index == gaddr_index )
		{
			grp_obj_index = gobj_index;
			break;
		}
	}
	if ( grp_obj_index == -1 )
	{
		return -1;
	}

	GroupObjectHead gobj_head;
	int retcode = dev->GetGroupObjectHeadByIndex(grp_obj_index, gobj_head);
	if ( 0 != retcode )
	{
		return -1;
	}
	uint gval_len_in_bit = gobj_head.GetLenInBit();
	dev->GetGroupObjectValue(grp_obj_index, buff, &buff_len);

	int apdu_len;
	if ( gval_len_in_bit > 6 )  // 大于6比特. 
	{ 
		apdu_len = 2 + gval_len_in_bit / 8;
		if ( 2 == apdu_len ) apdu_len = 3;

		Resize(apdu_len);
		BasicInfo(ack_request, priority, hop_count_type);
		memcpy( m_apdu_mem + 2, buff, buff_len);
	}
	else  // 小于6BITs
	{
		apdu_len = 2;

		Resize(apdu_len);
		BasicInfo(ack_request, priority, hop_count_type);
		m_apdu_mem[1] &= 0xC0;
		m_apdu_mem[1] |= ( 0x3F & buff[0] );
	}
	const unsigned char* resp_dev_addr = dev->GetHostAddr(NULL);
	SetHostAddr(resp_dev_addr);

	return 0;
}

int APDU_GroupValue_Response::GetParams(UCHAR* buff, int& buff_len)const
{
	int expected_len = m_apdu_len - 2;
	if ( expected_len == 0 )
	{
		expected_len = 1;
	}

	if ( buff_len < expected_len || NULL == buff )
	{
		buff_len = -expected_len;
		return -1;
	}

	if ( m_apdu_len > 2 )
	{
		memcpy(buff, m_apdu_mem + 2, m_apdu_len - 2 );
		buff_len = m_apdu_len - 2;
	}
	else
	{
		*buff = ( m_apdu_mem[2] & 0x3F );
		buff_len = 1;
	}

	return 0;
}

int APDU_GroupValue_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
								   const UCHAR* buff, USHORT data_len_in_bit)
{
	if ( NULL == asap )
	{
		return -1;
	}

	int data_len = 0;
	if ( data_len_in_bit > 6 )  // 大于六比特. 
	{
		data_len = data_len_in_bit / 8;
		if ( data_len_in_bit % 8 )
		{
			data_len ++;
		}
	}

	//	m_data_len_in_bit = data_len_in_bit;

	Resize(2 + data_len);

	BasicInfo(ack_request, priority, hop_count_type, asap);

	if ( 0 != data_len )
	{
		memcpy(m_apdu_mem + 2, buff, data_len);
	}
	else 
	{
		m_apdu_mem[1] &= 0xC0;
		m_apdu_mem[1] |= ( 0x3F & buff[0] );
	}

	return 0;
}

int APDU_GroupValue_Write::GetParams(UCHAR* buff, USHORT& data_len_in_bit)const
{
	if ( NULL == buff )
	{
		return -1;
	}

	if ( m_apdu_len > 2 )
	{
		if ( data_len_in_bit < ( m_apdu_len - 2 )*8 )  // 缓冲不够. 
		{
			return -1;
		}
		memcpy(buff, m_apdu_mem + 2, m_apdu_len - 2 );
		data_len_in_bit = ( m_apdu_len - 2 )*8;
	}
	else // 只拷贝6bits. 
	{
		*buff = ( m_apdu_mem[1] & 0x3F );
		data_len_in_bit = 6; // 现实上可能没有6个bit的数据.但是代码在这里没办法区分.应用层调用者知道只有多少个bit是有效的. 
	}

	return 0;
}

int APDU_GroupValue_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		return -1;
	}

	UCHAR buff[256] = { 0 };
	apdu = NULL;
	apdus.clear();

	const UCHAR* group_addr = m_peeraddr;
	BOOL been_written = FALSE;
	// 遍历每个设备. 
	for ( IDevice* dev = ms_dev_manager->NextDevice(TRUE); NULL != dev; dev = ms_dev_manager->NextDevice() )
	{
		int gaddr_index = -1;
		int gaddr_count = dev->GetGroupAddrCount();
		for ( int i = 0; i < gaddr_count; i++ )  // 遍历这个设备所有的组地址. 
		{
			const UCHAR* this_gaddr = dev->GetGroupAddrByIndex(i);
			if ( 0 == memcmp(this_gaddr, group_addr, 2) ) // 这个设备有存这个组地址. 
			{
				gaddr_index = i;
				break;
			}
		}

		if ( -1 == gaddr_index ) // 这个设备没有要写值的组地址. 
		{
			continue;
		}

		// 这个设备有要写值的地址. 
		int link_count = dev->GetLinkCount();
		for ( int i = 0; i < link_count; i++ )
		{
			UCHAR gobj_idx;
			UCHAR gaddr_idx;
			dev->GetLinkByIndex(i, gobj_idx, gaddr_idx);
			if ( gaddr_index == gaddr_idx ) // 匹配. 
			{
				USHORT len = sizeof(buff) * 8;
				GetParams(buff, len);
				// 于是改写这个组对象的值. 
				dev->SetGroupObjectValue(gobj_idx, buff);

				been_written = TRUE;
			}
		}
	}

	if ( been_written )  // 有组对象被更改过值时的逻辑.  
	{
		// do nothing ...
	}

	return 0;
}

int APDU_GroupValue_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_ACK);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_Link_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
							UCHAR group_object_number, UCHAR start_index)
{
	if ( NULL == asap )
	{
		return -1;
	}

	Resize(4);

	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = group_object_number;
	m_apdu_mem[3] = ( 0x0f & start_index);

	return 0;
}

int APDU_Link_Read::GetParams(UCHAR* group_object_number, UCHAR* start_index)const
{
	if ( NULL != group_object_number )
	{
		*group_object_number = m_apdu_mem[2];
	}

	if ( NULL != start_index )
	{
		*start_index = m_apdu_mem[3];
	}

	return 0;
}

int APDU_Link_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	UCHAR group_object_num;
	UCHAR start_index;
	GetParams(&group_object_num, &start_index);

	APDU_Link_Response* apdu_l_r = new APDU_Link_Response;
	apdu_l_r->SetHostAddr(m_peeraddr);
	int retcode = apdu_l_r->BuildMe(0, 0, 7, m_hostaddr, group_object_num, start_index);
	if ( 0 == retcode )
	{
		apdu = apdu_l_r;
	}
	else
	{
		delete apdu_l_r;
		apdu = new APDU_NACK;
	}

	return 0;
}

int APDU_Link_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_Link_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_Link_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
								UCHAR group_object_number, UCHAR start_index)
{	
	if ( NULL == asap )
	{
		return -1;
	}

	start_index &= 0x0f;

	int retcode = 0;
	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_hostaddr);
	if ( NULL == dev ) // 没有找到这个设备. 
	{
		return -2;
	}

	// 下面是拿到相应links的逻辑.最多可以存六个组地址. 
	UCHAR gobj_idx;
	UCHAR gaddr_idx;
	UCHAR gaddr_idx_start = 0xff;
	std::vector<UCHAR> gaddr_vec; // 用来存符合要求的组地址序号. 
	int link_count = dev->GetLinkCount();
	for ( int i = 0; i < link_count; i++ )
	{
		dev->GetLinkByIndex(i, gobj_idx, gaddr_idx);
		// 不仅要组对象序号匹配.而且组地址的序号不能小于start_index. 
		if ( gobj_idx == group_object_number &&  gaddr_idx >= start_index )
		{
			gaddr_vec.push_back(gaddr_idx);
			if ( gaddr_idx_start > gaddr_idx )
			{
				gaddr_idx_start = gaddr_idx;
			}
		}
	}
	// 一共要传送多少个组地址. 
	int gaddr_2b_tx = gaddr_vec.size(); 
	if ( gaddr_2b_tx > 6 ) // 不可超过6个.  
	{
		gaddr_2b_tx = 6;
	}

	Resize(4 + 2 * gaddr_2b_tx);
	BasicInfo(ack_request, priority, hop_count_type, asap);
	m_apdu_mem[2] = group_object_number;
	m_apdu_mem[3] = start_index;
	for ( int i = 0; i < gaddr_2b_tx; i++ )
	{
		const unsigned char* ptr = dev->GetGroupAddrByIndex(gaddr_vec[i]);
		memcpy( m_apdu_mem + 4 + 2 * i, ptr, 2 );
	}

	return 0;
}

int APDU_Link_Response::GetParams(UCHAR* group_object_number, UCHAR* start_index, UCHAR* links_buff, int* buff_len)const
{
	if ( group_object_number )
	{
		*group_object_number = m_apdu_mem[2];
	}

	if ( start_index )
	{
		*start_index = m_apdu_mem[3];
	}

	if ( links_buff && buff_len )
	{
		int link_count = (m_apdu_len - 4) / 2; // 一共有多少个link. 
		if ( *buff_len < 2 * link_count )
		{
			*buff_len = -link_count*2;
		}
		else
		{
			memcpy(links_buff, m_apdu_mem + 4, link_count * 2);
			*buff_len = link_count*2;
		}	
	}

	return 0;
}

int APDU_Link_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
							 UCHAR group_object_number, UCHAR flags, const unsigned char *group_address)
{
	//	int retcode = 0;

	if ( NULL == asap || NULL == group_address || NULL == group_address )
	{
		return -1;
	}

	flags &= 0x03;

	Resize(6);
	BasicInfo(ack_request, priority, hop_count_type, asap);
	m_apdu_mem[2] = group_object_number;
	m_apdu_mem[3] = flags;
	m_apdu_mem[4] = group_address[0];
	m_apdu_mem[5] = group_address[1];

	return 0;
	//	return retcode;
}

int APDU_Link_Write::GetParams(UCHAR* group_object_number, UCHAR* flags, const unsigned char** group_address)const
{
	if ( group_object_number )
	{
		*group_object_number = m_apdu_mem[2];
	}

	if ( flags )
	{
		*flags = m_apdu_mem[3];
	}

	if ( group_address )
	{
		*group_address = m_apdu_mem + 4;
	}

	return 0;
}

int APDU_Link_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	UCHAR group_obj_num;
	UCHAR flags;
	const unsigned char* group_address;
	GetParams(&group_obj_num, &flags, &group_address);

	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_peeraddr);
	if ( NULL != dev )
	{
		if ( 0 == (0x01 & flags) )  // 删除链接. 第一bit位是0. 
		{
			BOOL go_on = TRUE;
			while ( go_on )
			{
				go_on = FALSE;
				int link_count = dev->GetLinkCount();
				for ( int i = 0; i < link_count; i++ )
				{
					UCHAR gobj_idx;
					UCHAR gaddr_idx;
					dev->GetLinkByIndex(i, gobj_idx, gaddr_idx);
					const UCHAR* gaddr = dev->GetGroupAddrByIndex(gaddr_idx);
					if ( gobj_idx == group_obj_num && 0 == memcmp(gaddr, group_address, 2) )
					{
						dev->RemoveLinkByIndex(i);
						go_on = TRUE;
						break;
					}
				}
			} // 每删一次ID号就变佐.就要删多次. 

			apdu = new APDU_ACK;
		}
		else if ( 1 == (0x01 & flags) ) // 增加链接. 
		{
			int gaddr_count = dev->GetGroupAddrCount();
			int gaddr_index = -1; 
			for ( int i = 0; i < gaddr_count; i++ )
			{
				const unsigned char* gaddr = dev->GetGroupAddrByIndex(i);
				if ( 0 == memcmp(gaddr, group_address, 2) )
				{
					gaddr_index = i;
					break;
				}
			}

			// 没有找到组地址下标.就要加多一个. 
			if ( -1 == gaddr_index ) 
			{
				dev->AddGroupAddr(group_address);
				gaddr_index = gaddr_count;
			}

			dev->AddLink(group_obj_num, gaddr_index);

			apdu = new APDU_ACK;
		}
	}
	else
	{
		apdu = new APDU_NACK;
	}

	return 0;
}

int APDU_Link_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_ACK);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_Authorize_Request::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, const UCHAR* key)
{
	if ( NULL == asap || NULL == key )
	{
		return -1;
	}

	int size = 7;
	int key_pos = 3;
	if ( g_myconfig.UseFaultyAuthorize() )  // 兼容硬件的帧错误. 
	{
		size = 8;
		key_pos = 4;
	}

	Resize(size);
	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = 0;
	m_apdu_mem[3] = 0;
	memcpy(m_apdu_mem + key_pos, key, 4);  // 4 bytes长的key. 

	return 0;
}

int APDU_Authorize_Request::GetParams(UCHAR* key)const
{
	int size = 7;
	int key_pos = 3;
	if ( g_myconfig.UseFaultyAuthorize() )  // 兼容硬件的帧错误. 
	{
		size = 8;
		key_pos = 4;
	}

	if ( NULL == key || NULL == m_apdu_mem || m_apdu_len != size )
	{
		return -1;
	}

	memcpy(key, m_apdu_mem + key_pos, 4);  // 把key拷出. 

	return 0;
}

int APDU_Authorize_Request::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	int retcode = 0;
	UCHAR key[8] = { 0 };
	GetParams(key);
	APDU_Authorize_Response* apdu_a_r = new APDU_Authorize_Response;
	apdu_a_r->SetHostAddr(m_peeraddr);
	retcode = apdu_a_r->BuildMe(0, 0, 7, m_hostaddr, key);
	if ( 0 == retcode )
	{
		apdu = apdu_a_r;
	}
	else
	{
		delete apdu_a_r;
		apdu = new APDU_NACK;
	}
	return 0;
}

int APDU_Authorize_Request::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_Authorize_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_Authorize_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, const UCHAR* key)
{
	if ( NULL == asap )
	{
		return -1;
	}

	int retcode = -1;
	UCHAR level;
	Resize(3);
	BasicInfo(ack_request, priority, hop_count_type, asap);
	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_hostaddr);
	if ( NULL != dev )
	{
		retcode = dev->GetAuthorize(key, level);
	}
	if ( 0 == retcode )
	{
		m_apdu_mem[2] = level;
	}

	return retcode;
}	

int APDU_Authorize_Response::GetParams(UCHAR* level)const
{
	if ( NULL == level )
	{
		return -1;
	}

	*level = m_apdu_mem[2];

	return 0;
}

int APDU_UserMemory_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, 
								  const UCHAR* asap, UCHAR number, UINT address)
{
	if ( asap == NULL )
	{
		return -1;
	}

	Resize(5);
	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = (number & 0x0F);
	m_apdu_mem[2] |= ((address & 0x000F0000) >> 12);
	m_apdu_mem[3] = ((address & 0x0000FF00) >> 8);
	m_apdu_mem[4] = (address & 0x000000FF); 

	return 0;
}

int APDU_UserMemory_Read::GetParams(UCHAR* number, UINT* address)const
{
	if ( m_apdu_len != 5 )  // APDU_UserMemory_Read 未被 Build. 
	{
		return -1;
	}

	if ( number )
	{
		*number = (m_apdu_mem[2] & 0x0F);
	}

	if ( address )
	{
		*address = 0;
		*address |= m_apdu_mem[4];
		*address |= ((uint)m_apdu_mem[3] << 8);
		*address |= ((uint)(m_apdu_mem[2] & 0xF0) << 12);
	}

	return 0;
}

int APDU_UserMemory_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( 5 != m_apdu_len || !m_recved_apdu )
	{
		return -1;
	}

	apdus.clear();

	int retcode;
	UCHAR number;
	UINT address;
	GetParams(&number, &address);

	APDU_UserMemory_Response* apdu_u_r = new APDU_UserMemory_Response;
	apdu_u_r->SetHostAddr(m_peeraddr);
	retcode = apdu_u_r->BuildMe(0, 0, 7, m_hostaddr, number, address);
	if ( 0 == retcode )
	{
		apdu = apdu_u_r;
	}
	else
	{
		delete apdu_u_r;
		apdu = NULL;
	}

	return retcode;
}

int APDU_UserMemory_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_UserMemory_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_UserMemory_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, 
									  const UCHAR* asap, UCHAR number, UINT address)
{
	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_hostaddr);
	if ( NULL == dev )
	{
		return -1;
	}

	address &= 0x000FFFFF;
	if ( number > 11 )
	{
		number = 11;
	}

	if ( address + number > MAX_USER_MEMORY_SIZE )
	{
		return -1;
	}

	const uchar* mem = dev->GetUserMemory();
	if ( NULL == mem )
	{
		return -1;
	}

	Resize(5 + number);
	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = (number & 0x0F);
	m_apdu_mem[2] |= ((address & 0x000F0000) >> 12);
	m_apdu_mem[3] = ((address & 0x0000FF00) >> 8);
	m_apdu_mem[4] = (address & 0x000000FF);

	memcpy(m_apdu_mem + 5, mem + address, number);

	return 0;
}

int APDU_UserMemory_Response::GetParams(UCHAR* number, UINT* address, UCHAR* buff, int* buff_len)const
{
	if ( m_apdu_len < 5 )
	{
		return -1;
	}

	if ( number )
	{
		*number = (m_apdu_mem[2] & 0x0F);
	}

	if ( address )
	{
		*address = 0;
		*address |= m_apdu_mem[4];
		*address |= ((uint)m_apdu_mem[3] << 8);
		*address |= (uint)(m_apdu_mem[2] & 0xF0 << 12);
	}

	if ( buff && buff_len )
	{
		int copied_num = m_apdu_len - 5;
		assert(copied_num == *number);
		if ( NULL == buff || *buff_len < copied_num )
		{
			*buff_len = -copied_num;
			return -1;
		}

		memcpy(buff, m_apdu_mem + 5, copied_num);
		*buff_len = copied_num;
	}

	return 0;
}

int APDU_UserMemory_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
								   UCHAR number, UINT address, UCHAR* buff)
{
	address &= 0x000FFFFF;
	if ( number > 11 )
	{
		number = 11;
	}

	if ( address + number > MAX_USER_MEMORY_SIZE )
	{
		return -1;
	}

	Resize(5 + number);
	BasicInfo(ack_request, priority, hop_count_type, asap);

	m_apdu_mem[2] = (number & 0x0F);
	m_apdu_mem[2] |= ((address & 0x000F0000) >> 12);
	m_apdu_mem[3] = ((address & 0x0000FF00) >> 8);
	m_apdu_mem[4] = (address & 0x000000FF);

	memcpy(m_apdu_mem + 5, buff, number);

	return 0;
}

int APDU_UserMemory_Write::GetParams(UCHAR* number, UINT* address, UCHAR* buff, int* buff_len)const
{
	if ( m_apdu_len < 5 )
	{
		return -1;
	}

	if ( number )
	{
		*number = (m_apdu_mem[2] & 0x0F);
	}

	if ( address )
	{
		*address = 0;
		*address |= m_apdu_mem[4];
		*address |= ((uint)m_apdu_mem[3] << 8);
		*address |= (uint)(m_apdu_mem[2] & 0xF0 << 12);
	}

	if ( buff && buff_len )
	{
		int copied_num = m_apdu_len - 5;
		assert(copied_num == *number);
		if ( NULL == buff || *buff_len < copied_num )
		{
			*buff_len = -copied_num;
			return -1;
		}

		memcpy(buff, m_apdu_mem + 5, copied_num);
		*buff_len = copied_num;
	}

	return 0;
}

int APDU_UserMemory_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	UCHAR number;
	UINT  address;
	UCHAR buff[56];
	int buff_len = sizeof(buff);
	GetParams(&number, &address, buff, &buff_len);

	IDevice* dev = ms_dev_manager->GetDeviceByDAddr(m_peeraddr);
	if ( NULL != dev )
	{
		dev->WriteUserMemory(address, buff, buff_len);
		apdu = new APDU_ACK;
	}
	else
	{
		apdu = new APDU_NACK;
	}

	return 0;
}

int APDU_UserMemory_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_ACK);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_IndividualAddress_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* new_address)
{
	if ( NULL == new_address )
	{
		return -1;
	}

	Resize(4);
	const UCHAR* broadcast_addr = g_myconfig.GetBroadcastAddr();
	BasicInfo(ack_request, priority, hop_count_type, broadcast_addr);

	m_apdu_mem[2] = new_address[0];
	m_apdu_mem[3] = new_address[1];

	return 0;
}

int APDU_IndividualAddress_Write::GetParams(UCHAR* new_address)const
{
	if ( NULL == new_address )
	{
		return -1;
	}

	memcpy(new_address, m_apdu_mem + 2, 2);

	return 0;
}

int APDU_IndividualAddress_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	apdu = NULL;
	apdus.clear();

	IDevice* dev;
	unsigned int buff[1024] = { 0 };
	int buff_len = sizeof(buff)/sizeof(uint);
	int dev_count = ms_dev_manager->GetAllDevices(buff, buff_len);
	for ( int i = 0; i < dev_count; i++ )
	{
		dev = ms_dev_manager->GetDeviceByID(buff[i]);
		if ( dev && dev->IsIndividualAddressWriteEnabled() )
		{
			dev->SetHostAddr(m_apdu_mem+2);  // 更新地址. 
		}
	}

	return 0;
}

int APDU_IndividualAddress_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	return 0;
}


int APDU_IndividualAddress_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type)
{
	Resize(2);
	const UCHAR* broadcast_addr = g_myconfig.GetBroadcastAddr();
	BasicInfo(ack_request, priority, hop_count_type, broadcast_addr);

	return 0;
}

int APDU_IndividualAddress_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	apdu = NULL;
	apdus.clear();

	IDevice* dev;
	APDU_IndividualAddress_Response* apdu_i_r;
	const uchar* host_addr;
	const uchar* peer_addr = m_hostaddr; // 回复帧的的peer地址就是收到帧的host地址啦. 
	unsigned int buff[1024] = { 0 };
	int buff_len = sizeof(buff)/sizeof(uint);
	int dev_count = ms_dev_manager->GetAllDevices(buff, buff_len);
	for ( int i = 0; i < dev_count; i++ )
	{
		dev = ms_dev_manager->GetDeviceByID(buff[i]);
		if ( dev && dev->IsIndividualAddressReadEnabled() )
		{
			host_addr = dev->GetHostAddr(NULL);
			apdu_i_r = new APDU_IndividualAddress_Response;
			apdu_i_r->BuildMe(0, 0, 0);
			apdu_i_r->SetHostAddr(host_addr);
			apdu_i_r->SetPeerAddr(peer_addr);
			apdus.push_back(apdu_i_r);
		}
	}

	return 0;
}

int APDU_IndividualAddress_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_IndividualAddress_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_IndividualAddress_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type)
{
	Resize(2);
	BasicInfo(ack_request, priority, hop_count_type);

	return 0;
}

// 序列号有六个octecs. 
int APDU_IndividualAddressSerialNumber_Read::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* serial_num)
{
	if ( NULL == serial_num )
	{
		return -1;
	}

	const UCHAR* broadcast_addr = g_myconfig.GetBroadcastAddr();  // peer地址是广播地址. 
	Resize(8);
	BasicInfo(ack_request, priority, hop_count_type, broadcast_addr);
	memcpy(m_apdu_mem+2, serial_num, 6);

	return 0;
}

int APDU_IndividualAddressSerialNumber_Read::GetParams(UCHAR* serial_num)const
{
	if ( NULL == serial_num )
	{
		return -1;
	}

	memcpy(serial_num, m_apdu_mem+2, 6);

	return 0;
}

int APDU_IndividualAddressSerialNumber_Read::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	apdu = NULL;
	apdus.clear();

	IDevice* dev;
	APDU_IndividualAddressSerialNumber_Response* apdu_ias_r;
	const uchar* host_addr;
	const uchar* peer_addr = m_hostaddr; // 回复帧的的peer地址就是收到帧的host地址啦. 
	const uchar* serial_num;
	unsigned int buff[1024] = { 0 };
	int buff_len = sizeof(buff)/sizeof(uint);
	int dev_count = ms_dev_manager->GetAllDevices(buff, buff_len);
	for ( int i = 0; i < dev_count; i++ )
	{
		dev = ms_dev_manager->GetDeviceByID(buff[i]);
		if ( NULL == dev )
		{
			continue;
		}
		serial_num = dev->GetSerialNum(NULL);
		if ( serial_num && 0 == memcmp(m_apdu_mem+2, serial_num, 6) )
		{
			host_addr = dev->GetHostAddr(NULL);
			apdu_ias_r = new APDU_IndividualAddressSerialNumber_Response;
			apdu_ias_r->BuildMe(0, 0, 0, serial_num);
			apdu_ias_r->SetHostAddr(host_addr);
			apdu_ias_r->SetPeerAddr(peer_addr);
			apdus.push_back(apdu_ias_r);
		}
	} // end of for.

	return 0;
}

int APDU_IndividualAddressSerialNumber_Read::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	expected_bytes.push_back(AT_IndividualAddressSerialNumber_Response);
	expected_bytes.push_back(AT_NACK);
	expected_bytes.push_back(AT_BUSY);
	expected_bytes.push_back(AT_NACKBUSY);
	return 0;
}

int APDU_IndividualAddressSerialNumber_Response::GetParams(uchar* serial_num)const
{
	if ( serial_num == NULL )
	{
		return -1;
	}
	memcpy(serial_num, m_apdu_mem+2, 6);
	return 0;
}

int APDU_IndividualAddressSerialNumber_Response::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* serialnum)
{
	if ( NULL == serialnum )
	{
		return -1;
	}

	Resize(12);
	BasicInfo(ack_request, priority, hop_count_type);
	memcpy(m_apdu_mem+2, serialnum, 6);

	return 0;
}

int APDU_IndividualAddressSerialNumber_Write::BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, 
													  const UCHAR* serial_num, const UCHAR* new_address)
{
	if ( NULL == serial_num || NULL == new_address )
	{
		return -1;
	}

	Resize(14);
	BasicInfo(ack_request, priority, hop_count_type);
	memcpy(m_apdu_mem+2, serial_num, 6);
	memcpy(m_apdu_mem+8, new_address, 2);

	return 0;
}

int APDU_IndividualAddressSerialNumber_Write::GetParams(UCHAR* serial_num, UCHAR* new_address)const
{
	if ( serial_num )
	{
		memcpy(serial_num, m_apdu_mem+2, 6);
	}

	if ( new_address )
	{
		memcpy(new_address, m_apdu_mem+8, 2);
	}

	return 0;
}

int APDU_IndividualAddressSerialNumber_Write::Response(APDU*& apdu, std::vector<APDU*>& apdus)const
{
	if ( !m_recved_apdu )
	{
		apdu = NULL;
		return -1;
	}

	apdu = NULL;
	apdus.clear();

	uchar serial_num[6];
	uchar new_addr[2];
	GetParams(serial_num, new_addr);

	IDevice* dev;
	unsigned int buff[1024] = { 0 };
	int buff_len = sizeof(buff)/sizeof(uint);
	int dev_count = ms_dev_manager->GetAllDevices(buff, buff_len);
	for ( int i = 0; i < dev_count; i++ )
	{
		dev = ms_dev_manager->GetDeviceByID(buff[i]);
		if ( NULL == dev )
		{
			continue;
		}
		const uchar* my_serialno = dev->GetSerialNum(NULL);
		if ( my_serialno && 0 == memcmp(serial_num, my_serialno, 6) )
		{
			dev->SetHostAddr(new_addr);
		}
	} // end of for. 

	return 0;
}

int APDU_IndividualAddressSerialNumber_Write::GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const
{ 
	expected_bytes.clear();
	return 0;
}