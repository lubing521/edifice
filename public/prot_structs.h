#ifndef _H_PROT_STRUCTS_
#define _H_PROT_STRUCTS_

#include <math.h>
#include "Pentagon.h"

//////////////////////// Property 的结构体.
#pragma pack(1)  // 告诉编译器紧凑分配结构体内存. 
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

/// EIB协议定义的四种优先级. 
enum EIB_Priority
{
	EP_SYSTEM = 0x00,
	EP_URGENCE = 0x10,
	EP_NORMAL = 0x01,
	EP_LOW = 0x00
};

/// APDU的所有类型. 
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

/// 这个结构体是检接收队列时返回的结构体. 
struct FramePeek
{
	APDUType  _type;    // Frame的类型.由Frame包含的APDU类型确定. 
	void *    _frame_handle;  // 该Frame的句柄.实际指向Frame结构体的首地址.  
	uint      _frame_id; // 在队列中的唯一标志号. 
	uint      _frame_time_stamp; // 收到帧时的时间戳.单位毫秒.表示从开机时经过的毫秒数. 8
};

struct DevMapInfo
{
	bool  _does_group_obj_head_mapped;   // 是否映射了GroupObjectHead.
	uchar _group_obj_head_map[2];    // 映射到的接口对象 接口对象索引,和属性ID. 

	bool  _does_group_obj_value_mapped; // 是否映射了GroupObject的值. 
	uchar _group_obj_value_map[2];   

	bool  _does_group_addrs_mapped;   // 是否映射了组地址的值. 
	uchar _group_addrs_map[2];

	bool  _does_group_links_mapped;   // 是否映射了组关联表. 
	uchar _group_links_map[2];

	bool  _does_user_memory_mapped;   // 是否映射了 User Memory. 
	uchar _user_memory_map[2][2];     // user memory 要映射到两个接口对象. 

	bool  _does_host_addr_mapped;     // host的设备地址也被映射到接口对象. 
	uchar _host_addr_map[2];  

	bool  _does_individual_addr_mapped;  // 给EIB协议的 IndividualAddress_* 应用层服务用.
	uchar _individual_addr_map[2];
};

// 可怕的比特. 
class DireBits
{
public:
	DireBits(int bits)
	{
		m_bits = bits;
	}

	// 重载与型的乘法操作. 
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
	int m_bits;   // 记共有多少个bits.取值不会取到0. 或bit的下标,从0开始取值.
};

// element_type字段.该字段一个byte长。定义为 3bits(单位代码) + 5bits(数值位).
// 该字段描述一个元素所占的内存长度.
class ElemTypeField
{
public:
	// 构造函数把该字段的值传入. 
	ElemTypeField(unsigned char type)
	{
		m_type = type;
		m_unit = mask_num(&type, "e0");  // 3(单位代码)为0时表示1 bit. 1 表示1 byte. 2表示 2 bytes.
		// m_unit = type & 0xe0;
		m_value = type & 0x1f;
	}

	/// unit code 占三位. 
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
	unsigned char m_type;  // 存数值. 
	unsigned char m_unit;  // 3(单位) 0比特 1 BYTE  2 SHORT
	unsigned char m_value; // 5(数值位)
};

// data_length字段.该字段两个byte长。字义为 4bits(指数的数值,隐含冥为10) + 12bits(数值位).
// 该字段描述了data区的总大长度。不能超过这个长度。
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