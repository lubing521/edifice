#ifndef _H_PROT_STRUCTS_
#define _H_PROT_STRUCTS_

#include <math.h>
#include "Pentagon.h"

//////////////////////// Property �Ľṹ��.
#pragma pack(1)  // ���߱��������շ���ṹ���ڴ�. 
struct PropertyFields
{
	unsigned char _object_index;
	unsigned char _property_id;
	unsigned char _elem_type;
	unsigned char _perm;
	unsigned char _storage_manage;
	unsigned char _pid;
	unsigned short _data_length;
	unsigned short _elem_count;
	unsigned char _w_tag[2];
	unsigned char _data_addr[4];
};
#pragma pack()

/// EIBЭ�鶨����������ȼ�. 
enum EIB_Priority
{
	EP_SYSTEM = 0x00,
	EP_URGENCE = 0x10,
	EP_NORMAL = 0x01,
	EP_LOW = 0x00
};

/// APDU����������. 
enum APDUType{ 
	    AT_UNAVAILABLE,
	    AT_ACK, AT_NACK, AT_BUSY, AT_NACKBUSY, 
		AT_PropertyValue_Read,
		AT_PropertyValue_Response,
		AT_PropertyValue_Write,
		AT_Property_Read,
		AT_Property_Response,
		AT_Property_Write,
		AT_GroupValue_Read,
		AT_GroupValue_Response,
		AT_GroupValue_Write,
		AT_Link_Read,
		AT_Link_Response,
		AT_Link_Write,
		AT_Authorize_Request,
		AT_Authorize_Response,
		AT_UserMemory_Read,
		AT_UserMemory_Response,
		AT_UserMemory_Write,
		AT_IndividualAddress_Write,
		AT_IndividualAddress_Read,
		AT_IndividualAddress_Response,
		AT_IndividualAddressSerialNumber_Read,
		AT_IndividualAddressSerialNumber_Response,
		AT_IndividualAddressSerialNumber_Write, 
};

/// ����ṹ���Ǽ���ն���ʱ���صĽṹ��. 
struct FramePeek
{
	APDUType  _type;    // Frame������.��Frame������APDU����ȷ��. 
	void *    _frame_handle;  // ��Frame�ľ��.ʵ��ָ��Frame�ṹ����׵�ַ.  
	uint      _frame_id; // �ڶ����е�Ψһ��־��. 
	uint      _frame_time_stamp; // �յ�֡ʱ��ʱ���.��λ����.��ʾ�ӿ���ʱ�����ĺ�����. 8
};

struct DevMapInfo
{
	bool  _does_group_obj_head_mapped;   // �Ƿ�ӳ����GroupObjectHead.
	uchar _group_obj_head_map[2];    // ӳ�䵽�Ľӿڶ��� �ӿڶ�������,������ID. 

	bool  _does_group_obj_value_mapped; // �Ƿ�ӳ����GroupObject��ֵ. 
	uchar _group_obj_value_map[2];   

	bool  _does_group_addrs_mapped;   // �Ƿ�ӳ�������ַ��ֵ. 
	uchar _group_addrs_map[2];

	bool  _does_group_links_mapped;   // �Ƿ�ӳ�����������. 
	uchar _group_links_map[2];

	bool  _does_user_memory_mapped;   // �Ƿ�ӳ���� User Memory. 
	uchar _user_memory_map[2][2];     // user memory Ҫӳ�䵽�����ӿڶ���. 

	bool  _does_host_addr_mapped;     // host���豸��ַҲ��ӳ�䵽�ӿڶ���. 
	uchar _host_addr_map[2];  

	bool  _does_individual_addr_mapped;  // ��EIBЭ��� IndividualAddress_* Ӧ�ò������.
	uchar _individual_addr_map[2];
};

// ���µı���. 
class DireBits
{
public:
	DireBits(int bits)
	{
		m_bits = bits;
	}

	// �������͵ĳ˷�����. 
	const DireBits& operator * (int multiplier)
	{
		m_bits *= multiplier;
		return *this;
	}

	const DireBits& operator + (const DireBits& dbit)
	{
		m_bits += dbit.m_bits;
	}

	const DireBits& operator - (const DireBits& dbit)
	{
		m_bits -= dbit.m_bits;
	}

	int IndexInByte()const
	{
		int ret = m_bits / 8;
		return ret;
	}

	DireBits& operator --(int)
	{
		m_bits --;
		return *this;
	}

	BOOL CanDivideExactlyBy8()const
	{
		BOOL b = !( m_bits % 8 );
		return b;
	}

	int NumInByte()const
	{
		int ret = ( m_bits - 1 ) / 8 + 1;
		return ret;
	}

	int NumInBit()const
	{
		return m_bits;
	}

private:
	int m_bits;   // �ǹ��ж��ٸ�bits.ȡֵ����ȡ��0. ��bit���±�,��0��ʼȡֵ.
};

// element_type�ֶ�.���ֶ�һ��byte��������Ϊ 3bits(��λ����) + 5bits(��ֵλ).
// ���ֶ�����һ��Ԫ����ռ���ڴ泤��.
class ElemTypeField
{
public:
	// ���캯���Ѹ��ֶε�ֵ����. 
	ElemTypeField(unsigned char type)
	{
		m_type = type;
		m_unit = mask_num(&type, "e0");  // 3(��λ����)Ϊ0ʱ��ʾ1 bit. 1 ��ʾ1 byte. 2��ʾ 2 bytes.
		// m_unit = type & 0xe0;
		m_value = type & 0x1f;
	}

	/// unit code ռ��λ. 
	uint get_unit_code()
	{
		return m_unit;
	}

	bool elem_unit_in_bit()
	{
		if ( 0 == m_unit )
			return true;
		else
			return false;
	}

	ushort get_len_in_bit()
	{
		int total_bits;
		if ( 0 == m_unit )
			total_bits = m_value;
		else
			total_bits = 8 * m_unit * m_value;

		return total_bits;
	}

private:
	unsigned char m_type;  // ����ֵ. 
	unsigned char m_unit;  // 3(��λ) 0���� 1 BYTE  2 SHORT
	unsigned char m_value; // 5(��ֵλ)
};

// data_length�ֶ�.���ֶ�����byte��������Ϊ 4bits(ָ������ֵ,����ڤΪ10) + 12bits(��ֵλ).
// ���ֶ�������data�����ܴ󳤶ȡ����ܳ���������ȡ�
class DataLengthField
{
public:
	DataLengthField(unsigned short datalen)
	{
		m_datalen = datalen;
		m_index = mask_num((const unsigned char*)&m_datalen, "f");
		m_value = mask_num((const unsigned char*)&m_datalen, "0fff");
	}

	DataLengthField(const unsigned char*& addr)
	{
		memcpy(&m_datalen, addr, sizeof(unsigned short));
		m_index = mask_num((const unsigned char*)&m_datalen, "f");
		m_value = mask_num((const unsigned char*)&m_datalen, "0fff");
	}

	unsigned int get_length()
	{
		int total_len = (int)pow(10.0, m_index) * m_value;
		return total_len;
	}

private:
	unsigned short m_datalen;
	unsigned char  m_index;
	unsigned short m_value;
};

#endif