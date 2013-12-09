#include "stdafx.h"
#include <errno.h>
#include <set>
#include <algorithm>
#include "DeviceImpl_TXT.h"
#include "MyConfig.h"
#include "generic_helper.h"
#include "deelx.h"
#include "assert.h"
// #include <algorithm>

#define SEPARATOR_CHAR     ':'
#define SEPARATOR_STR      ":"

#define GROUP_OBJ_HEAD_INDI  "$GROUP_OBJ_HEAD"
#define GROUP_OBJ_VALUE_INDI "$GROUP_OBJ_VALUE"
#define GROUP_ADDRS_INDI     "$GROUP_ADDR"
#define GROUP_LINKS_INDI     "$GROUP_LINK"
#define USER_MEMORY_INDI     "$USER_MEMORY"
#define HOST_ADDR_INDI       "$HOST_ADDR"
#define INDIVIDUAL_ADDR_INDI "$INDIVIDUAL_ADDR"
#define SOFTUNIT_X_INDI      "$SOFTUNIT_X"
#define SOFTUNIT_Y_INDI      "$SOFTUNIT_Y"
#define SOFTUNIT_M_INDI      "$SOFTUNIT_M"
#define SOFTUNIT_S_INDI      "$SOFTUNIT_S"
#define SOFTUNIT_C_INDI      "$SOFTUNIT_C"
#define SOFTUNIT_T_INDI      "$SOFTUNIT_T"
#define SOFTUNIT_D_INDI      "$SOFTUNIT_D"

const char* key_words[] = {
	GROUP_OBJ_HEAD_INDI,
	GROUP_OBJ_VALUE_INDI,
	GROUP_ADDRS_INDI,
	GROUP_LINKS_INDI,
	USER_MEMORY_INDI,
	HOST_ADDR_INDI,
	INDIVIDUAL_ADDR_INDI,
	SOFTUNIT_X_INDI,
	SOFTUNIT_Y_INDI,
	SOFTUNIT_M_INDI,
	SOFTUNIT_S_INDI,
	SOFTUNIT_C_INDI,
	SOFTUNIT_T_INDI,
	SOFTUNIT_D_INDI
};

// 描述了 PropertyFields 结构体中的元素依次所占的字节数. 
static const unsigned int propertyFieldsShiftNum[] = {
	1, 1, 1, 1, 1, 1, 2, 2, 2, 4
};

DeviceImpl_TXT::DeviceImpl_TXT(void)
{
	m_inited = FALSE;
}

DeviceImpl_TXT::~DeviceImpl_TXT(void)
{
	Uninit();
}

int DeviceImpl_TXT::Init(const char* file_path, uint* dev_id)
{
	int retcode = 0;
	std::string info;

	if ( m_inited || NULL == file_path )
	{
		return -1;
	}

	//m_mutex_objs = NULL;
	//m_mutex_comm = NULL;

	if ( NULL != dev_id )
	{
		m_dev_id = *dev_id;
	}
	else
	{
		const char* basename = get_basename(file_path);
		m_dev_id = strtoul(basename, NULL, 16);
	}

	retcode = ReadInFile(file_path);
	if ( 0 != retcode )
	{
		goto _out;
	}

	// 检查相关数据是否合法. 
	if ( !CheckValidity(&info) )
	{
		std::string err_str = std::string(file_path) + ": " + info;
		report_err_immed(err_str.c_str(), 0xF);
	}

	//m_mutex_objs = CreateMutex(NULL, FALSE, NULL);
	//if ( NULL == m_mutex_objs )
	//{
	//	retcode = GetLastError();
	//	goto _out;
	//}
	//m_mutex_objs = CreateMutex(NULL, FALSE, NULL);
	//if ( NULL == m_mutex_objs )
	//{
	//	retcode = GetLastError();
	//	goto _out;
	//}

	retcode = 0;

_out:
	//if ( 0 != retcode )  // 没有初始化成功的话就要清理已经申请到的资源. 
	//{
	//	if ( m_mutex_objs ) CloseHandle(m_mutex_objs);
	//	if ( m_mutex_comm ) CloseHandle(m_mutex_comm);
	//}

	return retcode;
}

int DeviceImpl_TXT::Uninit()
{
	if ( !m_inited )
	{
		return -1;
	}
	
	int retcode;
	retcode = WriteBackFile(m_dev_desc_path.c_str());
	if ( 0 == retcode )
	{
		m_data.clear();
		//m_vec_gobjs.clear();
		//m_vec_gaddrs.clear();
		//m_vec_links.clear();

		//CloseHandle(m_mutex_objs);
		//CloseHandle(m_mutex_comm);

		m_inited = FALSE;
	}

	return retcode;
}

int DeviceImpl_TXT::RemoveIObject(unsigned char object_index, unsigned char prop_id)
{
	// 这里加上不能删除同组对象有映射关系的接口对象更合理. 
	DataContainer::iterator ite = MapID2FairyIte(object_index, prop_id);
	if ( ite != m_data.end() )
	{
		m_data.erase(ite);
		return 0;
	}
	else
	{
		return -1;
	}
}

int DeviceImpl_TXT::GetPropFields(unsigned char object_index, unsigned char prop_id, PropertyFields& fields)
{
	if ( !m_inited )
	{
		return -1;
	}

	DataContainer::iterator ite = MapID2FairyIte(object_index, prop_id);
	if ( ite != m_data.end() )  // 找到了. 
	{
		memcpy(&fields, ite->second.address(), sizeof(PropertyFields));
		return 0;
	}
	else
	{
		return -1;
	}
}

int DeviceImpl_TXT::SetPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields)
{
	if ( !m_inited )
	{
		return -1;
	}

	DataContainer::iterator ite = MapID2FairyIte(object_index, prop_id);
	if ( ite != m_data.end() )
	{
		int new_length = ceil_divide( ElemTypeField(fields._elem_type).get_len_in_bit() * fields._elem_count, 8);
		memcpy(const_cast<UCHAR*>(ite->second.address())+2, (UCHAR*)&fields+2, sizeof(PropertyFields)-2);
		ite->second.resize(sizeof(PropertyFields) + new_length);
		return 0;
	}
	else
	{
		return -1;
	}
}

int DeviceImpl_TXT::AddPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields)
{
	if ( !m_inited )
	{
		return -1;
	}

	DataContainer::iterator ite = MapID2FairyIte(object_index, prop_id);
	if ( ite != m_data.end() )
	{
		return -1; // 已经存在了.
	}
	else
	{
		std::pair<unsigned char, unsigned char> key(fields._object_index, fields._property_id);
		uint total_elem_len_in_byte;
		propfields_parser(fields, NULL, NULL, NULL, NULL, &total_elem_len_in_byte);

		mem_t m;
		m.cp((unsigned char*)&fields, sizeof(PropertyFields));
		m.resize(sizeof(PropertyFields) + total_elem_len_in_byte);

		m_data[key] = m;

		return 0;
	}
}

int DeviceImpl_TXT::GetPerElemLen_inbit(unsigned char object_index, unsigned char prop_id)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode = 0;
	PropertyFields fields;

	retcode = GetPropFields(object_index, prop_id, fields);
	if ( 0 == retcode )
	{
		ElemTypeField etf(fields._elem_type);
		return etf.get_len_in_bit();
	}
	else
	{
		return -1;
	}
}

int DeviceImpl_TXT::GetElemCount(unsigned char object_index, unsigned char prop_id)
{
	int retcode = 0;
	PropertyFields fields;

	retcode = GetPropFields(object_index, prop_id, fields);
	if ( 0 == retcode )
	{
		ushort count = fields._elem_count;
		return count;
	}
	else
	{
		return -1;
	}

	return 0;
}

int DeviceImpl_TXT::GetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, unsigned char count,
				    unsigned char* buff, int& buff_len)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode = -1;
	unsigned char* tmp_mem = NULL;
	DataContainer::iterator ite = MapID2FairyIte(object_index, prop_id);
	if ( ite != m_data.end() )  // 找到了. 
	{
		PropertyFields fields;
		const unsigned char* prop_mem = ite->second.address();
		const unsigned char* value_mem = prop_mem + sizeof(PropertyFields);
		memcpy(&fields, prop_mem, sizeof(PropertyFields));

		ushort elem_count;
		ushort elem_len_in_bit;
		uint total_elem_len_in_byte;
		propfields_parser(fields, &elem_count, NULL, &elem_len_in_bit, NULL, &total_elem_len_in_byte);
		uint start_pos_inbit = (start_index - 1) * elem_len_in_bit; // 起始bit位. 
		uint coping_len_inbit = elem_len_in_bit * count;   // 要拷贝多少个bit位. 
		uint end_pos_inbit = start_pos_inbit + coping_len_inbit-1; //终止bit位. 
		uint start_pos_inbyte = start_pos_inbit / 8;
		uint end_pos_inbyte = end_pos_inbit / 8;
		uint coping_len_inbyte = end_pos_inbyte - start_pos_inbyte + 1;
		uint left_secede = start_pos_inbit % 8; // 第一个byte的不使用的bit位数(低bit位). 
		uint right_secede = 8 - end_pos_inbit % 8 - 1; // 最后一个byte的不使用的bit位数(高bit位). 
		tmp_mem = new unsigned char[coping_len_inbyte];  // 临时用内存区. 
		memset(tmp_mem, 0, coping_len_inbyte);

		if ( start_index + count - 1 > elem_count )  // 越界.没有这么多元素. 
		{
			retcode = -1;
			goto _out;
		}

		if ( buff_len * 8 < coping_len_inbit ) // 缓冲区不够写. 
		{
			buff_len = - ceil_divide(coping_len_inbit, 8); // 把传入的缓冲区长度参数置为期望的长度. 
			retcode = -2;
			goto _out;
		}

		memcpy(tmp_mem, value_mem + start_pos_inbyte, coping_len_inbyte);
//		left_shift_mem(tmp_mem, coping_len_inbyte, left_secede);
//		right_shift_mem(tmp_mem, coping_len_inbyte, 8 - left_secede);
		left_shift_bitstream(tmp_mem, coping_len_inbyte, left_secede);

//		if ( 0 != left_secede && left_secede >= right_secede )
		if ( 0 != left_secede && 1 != coping_len_inbyte && left_secede >= ( 8 - right_secede ) )
		{
			buff_len = coping_len_inbyte - 1;
		}
		else
		{
			buff_len = coping_len_inbyte;
		}
		memcpy(buff, tmp_mem, buff_len);
		retcode = 0;
	}

_out:
	delete []tmp_mem;

	return retcode;
}

int DeviceImpl_TXT::SetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
								  unsigned char count, const unsigned char* buff, int buff_len)
{
	int retcode = 0;
	
	retcode = SetElemValues_Ex(object_index, prop_id, start_index, count, buff, buff_len, FALSE);
	return retcode;
}

int DeviceImpl_TXT::SetElemValues_Ex(unsigned char object_index, unsigned char prop_id, unsigned short start_index, unsigned char count,
								  const unsigned char* buff, int buff_len, BOOL enable_append)
{
	if ( !m_inited ||  start_index + count < count ) // 未初始化或ushort变量溢出. 
	{
		return -1;
	}

	int retcode = -1;
	unsigned char* tmp_mem = NULL;
	DataContainer::iterator ite = MapID2FairyIte(object_index, prop_id);
	if ( ite != m_data.end() )  // 找到了. 
	{
		PropertyFields* f = (PropertyFields*)ite->second.address();
		ushort elem_count;
		ushort elem_len_in_bit;
		uint total_elem_len_in_byte;
		propfields_parser(*f, &elem_count, NULL, &elem_len_in_bit, NULL, &total_elem_len_in_byte);
		if ( start_index + count - 1 > elem_count ) // 要增加元素个数. 
		{
			if ( !enable_append )  // 不能增加. 
			{
				return -1;
			}
			ushort new_count = start_index + count - 1;
			memcpy(&f->_elem_count, &new_count, 2);
			uint new_length = ceil_divide( elem_len_in_bit * new_count, 8 );
			ite->second.resize( sizeof(PropertyFields) + new_length);  // 增加内存. 
			f = (PropertyFields*)ite->second.address();
			memcpy(&f->_elem_count, &new_count, sizeof(ushort));
		}

		const unsigned char* prop_mem = ite->second.address();
		unsigned char* value_mem = const_cast<uchar*>(prop_mem + sizeof(PropertyFields));

		propfields_parser(*f, &elem_count, NULL, &elem_len_in_bit, NULL, &total_elem_len_in_byte);
		uint start_pos_inbit = (start_index - 1) * elem_len_in_bit; // 起始bit位. 
		uint coping_len_inbit = elem_len_in_bit * count;   // 要拷贝多少个bit位. 
		uint end_pos_inbit = start_pos_inbit + coping_len_inbit-1; //终止bit位. 
		uint start_pos_inbyte = start_pos_inbit / 8;
		uint end_pos_inbyte = end_pos_inbit / 8;
		uint coping_len_inbyte = end_pos_inbyte - start_pos_inbyte + 1;
		uint coping_len_from_buff = ceil_divide(coping_len_inbit, 8); 
		uint left_secede = start_pos_inbit % 8; // 第一个byte的不使用的bit位数(低bit位). 
		uint right_secede = 8 - end_pos_inbit % 8 - 1; // 最后一个byte的不使用的bit位数(高bit位). 
		tmp_mem = new unsigned char[coping_len_inbyte + 1];  // 临时用内存区. 
		memset(tmp_mem, 0, coping_len_inbyte + 1);
	
		if ( buff_len * 8 < coping_len_inbit ) // 缓冲区里的数据不够. 
		{
			buff_len = coping_len_from_buff;  // 把传入的缓冲区长度参数置为期望的长度. 
			buff_len = -buff_len;
			retcode = -1;
			goto _out;
		}	

		memcpy(tmp_mem + 1, buff, coping_len_from_buff);
        left_shift_bitstream(tmp_mem, coping_len_inbyte + 1, 8 - left_secede);

		//uchar left_mask = build_mask_code(left_secede);
		//uchar right_mask = ~build_mask_code(8-right_secede);
		//right_mask <<= (8-right_secede);

		uchar left_mask = ~build_mask_code(8-left_secede);  // 用1标记低bit无关位. 
		uchar right_mask = build_mask_code(right_secede);  // 用1标记高bit无关位. 
		tmp_mem[0] &= ~left_mask;                     //  把无关位擦掉.
 		tmp_mem[coping_len_inbyte-1] &= ~right_mask;  // 同上
		
		if ( start_pos_inbyte == end_pos_inbyte )
		{
			value_mem[start_pos_inbyte] &= ( left_mask | right_mask );
		}
		else
		{
			value_mem[start_pos_inbyte] &= left_mask;  // 要改写的内存区的两边.用掩码把不能覆写的bit位留下.
			value_mem[end_pos_inbyte] &= right_mask;
		}
		
		int middle_len = end_pos_inbyte-start_pos_inbyte-1;
		if ( middle_len > 0 )
		{
			memset(value_mem+start_pos_inbyte+1, 0, middle_len);  // 中间部分全部置0.
		}

		for ( int i = 0; i < coping_len_inbyte; i++ )
		{
			value_mem[start_pos_inbyte+i] |= tmp_mem[i];
		}
		retcode = 0;
	}

_out:
	delete []tmp_mem;

	return retcode;
}

int DeviceImpl_TXT::GetAllIDPairs(unsigned char* buff, int& buff_len)
{
	uint count = m_data.size();
	if ( NULL != buff && buff_len >= 2 * count ) // 足够长. 
	{
		buff_len = 0;
		for ( DataContainer::iterator ite = m_data.begin();
			ite != m_data.end();
			++ite )
		{
			buff[buff_len++] = ite->first.first;
			buff[buff_len++] = ite->first.second;
		}	
	}
	else
	{
		buff_len = - 2 * count;
	}

	return count;
}

DeviceImpl_TXT::DataContainer::iterator DeviceImpl_TXT::MapID2FairyIte(unsigned char object_index, unsigned char prop_id)
{
	std::pair<unsigned char, unsigned char> key(object_index, prop_id);
	DataContainer::iterator ite = m_data.find(key);

	return ite;
}

DeviceImpl_TXT::Mediator::iterator DeviceImpl_TXT::MapKeyword2DuplexesIte(std::string keyword)
{
	Mediator::iterator ite = m_mediator.find(keyword);

	return ite;
}

const unsigned char* DeviceImpl_TXT::GetHostAddr(std::string* h)
{
	char buff[8] = { 0 };
	Mediator::iterator ite2addrid = MapKeyword2DuplexesIte(HOST_ADDR_INDI);
	unsigned char addr_obj_index;
	unsigned char addr_prop_id;
	if ( m_mediator.end() != ite2addrid )
	{
		addr_obj_index = ite2addrid->second[0]._obj_index;
		addr_prop_id =ite2addrid->second[0]._prop_id;
	}
	else
	{
		addr_obj_index = g_myconfig.GetAddrObjIndex(); 
		addr_prop_id = g_myconfig.GetAddrPropID();
	}

	std::pair<unsigned char, unsigned char> key(addr_obj_index, addr_prop_id);
	DataContainer::iterator ite = m_data.find(key);
	// 找到了.并且存了地址. 
	if ( ite != m_data.end() && ite->second.get_size() >= sizeof(PropertyFields)+2 )
	{
		const unsigned char* addr = ite->second.address()+sizeof(PropertyFields);
		bins2hexs(addr, 2, buff, sizeof(buff));
		if ( h )
		{
			*h = buff;
		}
		return addr;
	}
	else
	{
		return NULL;
	}
}

const unsigned char* DeviceImpl_TXT::GetSerialNum(std::string* h)
{
	char  buff[16] = { 0 };
	uchar serialnum_obj_index = g_myconfig.GetSerialNumObjIndex(); 
	uchar serialnum_prop_id = g_myconfig.GetSerialNumPropID();

	std::pair<unsigned char, unsigned char> key(serialnum_obj_index, serialnum_prop_id);
	DataContainer::iterator ite = m_data.find(key);
	// 找到了.并且存了SerialNum. 
	if ( ite != m_data.end() && ite->second.get_size() >= sizeof(PropertyFields)+6 )
	{
		const unsigned char* serialnum = ite->second.address()+sizeof(PropertyFields);
		bins2hexs(serialnum, 6, buff, sizeof(buff));
		if ( h )
		{
			*h = buff;
		}
		return serialnum;
	}
	else
	{
		return NULL;
	}
}

int DeviceImpl_TXT::SetHostAddr(const unsigned char* addr)
{
	if ( NULL == addr )
	{
		return -1;
	}

	Mediator::iterator ite2addrid = MapKeyword2DuplexesIte(HOST_ADDR_INDI);
	unsigned char addr_obj_index;
	unsigned char addr_prop_id;
	// 指定存设备地址的接口对象ID优先选取配置在设备描述文件中的.其次选取配置在ini文件中的. 
	if ( m_mediator.end() != ite2addrid )
	{
		addr_obj_index = ite2addrid->second[0]._obj_index;
		addr_prop_id =ite2addrid->second[0]._prop_id;
	}
	else
	{
		addr_obj_index = g_myconfig.GetAddrObjIndex(); 
		addr_prop_id = g_myconfig.GetAddrPropID();
	}

	std::pair<unsigned char, unsigned char> key(addr_obj_index, addr_prop_id);
	DataContainer::iterator ite = m_data.find(key);

	if ( ite != m_data.end() )
	{
//		len = ite->second.get_size();
		// 假设存地址的接口对象的元素只有一个，并且长度为2.如果这个假设不成立.这里就有BUG. 
		ite->second.resize(sizeof(PropertyFields));
		ite->second.append(addr, 2);
	}
	else  // 不存在存地址的接口对象. 
	{
		PropertyFields fields;
		fields._object_index = addr_obj_index;
		fields._property_id = addr_prop_id;
		fields._elem_count = 1;
		fields._elem_type = 0x41;
		AddPropFields(addr_obj_index, addr_prop_id, fields);
		SetElemValues(addr_obj_index, addr_prop_id, 1, 1, addr, 2);
	}

	return 0;
}

UINT DeviceImpl_TXT::GetDevID()
{
	return m_dev_id;
}

void DeviceImpl_TXT::ResetDevDescFilePath(const char* file_path)
{
	if ( file_path )
	{
		m_dev_desc_path = file_path;
	}
}

int DeviceImpl_TXT::AppendElemValue(unsigned char obj_index, unsigned char prop_id, const unsigned char* buff, int buff_len)
{
	int retcode = -1;
	PropertyFields fields;
	int new_count;
	
	retcode = GetPropFields(obj_index, prop_id, fields);
	if ( 0 != retcode )
	{
		goto _out;
	}

	new_count = fields._elem_count + 1;
	retcode = SetElemValues_Ex(obj_index, prop_id, new_count, 1, buff, buff_len, TRUE);

_out:
	return retcode;
}

bool DeviceImpl_TXT::DoesThisSoftUnitExist(const char* softunit_name)
{
	uchar buff[16], obj_index, prop_id;
	bool b = _DoesThisSoftUnitExist(softunit_name, obj_index, prop_id, buff);
	return b;
}

bool DeviceImpl_TXT::_DoesThisSoftUnitExist(const char* softunit_name, uchar& obj_index, uchar& prop_id, uchar* buff)
{
	if ( NULL == softunit_name || NULL == buff )
	{
		return false;
	}

	const char* pattern = "[XYMSCTDG][0-9]+";
	CRegexpT <char> regexp(pattern);
	MatchResult result = regexp.MatchExact(softunit_name);
	
	if( !result.IsMatched() )  // 不匹配软单元的命名约定. 
		return false;

	int unit_num = strtol(softunit_name+1, NULL, 10);

	Mediator::iterator ite;
	switch ( softunit_name[0] )
	{
	case 'X':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_X_INDI);
		break;

	case 'Y':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_Y_INDI);
		break;

	case 'M':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_M_INDI);
		break;

	case 'S':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_S_INDI);
		break;

	case 'C':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_C_INDI);
		break;

	case 'T':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_T_INDI);
		break;

	case 'D':
		ite = MapKeyword2DuplexesIte(SOFTUNIT_D_INDI);
		break;

	case 'G':
		ite = MapKeyword2DuplexesIte(GROUP_OBJ_HEAD_INDI);
		break;

	default:
		report_err_immed("DeviceImpl_TXT::DoesThisSoftUnitExist reaches unexpected position.", 0x10);
		break;
	}

	if ( ite == m_mediator.end() )  // 没有映射这个软单元. 
	{
		return false;
	}

	obj_index = ite->second[0]._obj_index;
	prop_id = ite->second[0]._prop_id;

	int retcode;
	int buff_len = 0xff;
	if ( softunit_name[0] == 'G' )  // 操作组对象. 
	{
		retcode = GetGroupObjectValue(unit_num, buff, &buff_len);
		//if ( buff_len != 1 )  // 必须只有一个bit位.软单元里支持的组对象都应该是1bit的值. 
		//	retcode = -1;
	}
	else
	{
		retcode = GetElemValues(obj_index, prop_id, unit_num + 1, 1, buff, buff_len);
	}
	if ( 0 == retcode )
	{
		return true;
	}
	else
	{
		return false;
	}
}

const uchar* DeviceImpl_TXT::ReadSoftUnit(const char* softunit_name, uchar* buff)
{
	uchar obj_index;
	uchar prop_id;
	if ( NULL != buff )
	{
		bool exist = _DoesThisSoftUnitExist(softunit_name, obj_index, prop_id, buff);
		if ( !exist )
			return NULL;
	}
	return buff;
}

int DeviceImpl_TXT::WriteSoftUnit(const char* softunit_name, const uchar* value)
{
	uchar buff[16];
	uchar obj_index;
	uchar prop_id;
	bool  exist;

	if ( NULL == value || NULL == softunit_name )
	{
		return -1;
	}
	exist = _DoesThisSoftUnitExist(softunit_name, obj_index, prop_id, buff);
	if ( !exist ) // 不存在就报失败. 
	{
		return -1;
	}

	char  unit_type = softunit_name[0];
	uchar unit_num = strtol(softunit_name+1, NULL, 10);
	if ( unit_type == 'G' )
	{
		int retcode = SetGroupObjectValue(unit_num, value);
		return retcode;
	}
	else
	{
		// 最后一个参数填16.因为不会有那个元素的长度会超过16byte.而SetElemValues函数会从value开始拷贝适当的长度作为新值. 
		// 由调用者保证value指向合法的数据.
		int retcode = SetElemValues(obj_index, prop_id, unit_num+1, 1, value, 16);
		return retcode;
	}
}

int DeviceImpl_TXT::GetAuthorize(const unsigned char* key, unsigned char& level)
{
	// 检查 key. 
	char legal_key[4] = { '\x59', '\x4d', '\x4b', '\x4a' };
	if ( 0 == memcmp(key, legal_key, 4) )
	{
		level = 0xff;
		return 0;
	}
	else
	{
		return -1;
	}
}

void DeviceImpl_TXT::GetMapInfo(DevMapInfo& map_info)
{
	memset(&map_info, 0, sizeof(map_info));

	for ( Mediator::iterator ite = m_mediator.begin();
		  ite != m_mediator.end();
		  ++ite )
	{
		 if ( 0 == stricmp(GROUP_OBJ_HEAD_INDI, ite->first.c_str()) )
		 {
			map_info._does_group_obj_head_mapped = true;
			map_info._group_obj_head_map[0] = ite->second[0]._obj_index;
			map_info._group_obj_head_map[1] = ite->second[0]._prop_id;
		 }
		 else if ( 0 == stricmp(GROUP_OBJ_VALUE_INDI, ite->first.c_str()) )
		 {
			map_info._does_group_obj_value_mapped = true;
			map_info._group_obj_value_map[0] = ite->second[0]._obj_index;
			map_info._group_obj_value_map[1] = ite->second[0]._prop_id;
		 }
		 else if ( 0 == stricmp(GROUP_ADDRS_INDI, ite->first.c_str()) )
		 {
			map_info._does_group_addrs_mapped = true;
			map_info._group_addrs_map[0] = ite->second[0]._obj_index;
			map_info._group_addrs_map[1] = ite->second[0]._prop_id;
		 }
		 else if ( 0 == stricmp(GROUP_LINKS_INDI, ite->first.c_str()) )
		 {
			map_info._does_group_links_mapped = true;
			map_info._group_links_map[0] = ite->second[0]._obj_index;
			map_info._group_links_map[1] = ite->second[0]._prop_id;
		 }
		 else if ( 0 == stricmp(USER_MEMORY_INDI, ite->first.c_str()) )
		 {
			map_info._does_user_memory_mapped = true;
			map_info._user_memory_map[0][0] = ite->second[0]._obj_index;
			map_info._user_memory_map[0][1] = ite->second[0]._prop_id;
			map_info._user_memory_map[1][0] = ite->second[1]._obj_index;
			map_info._user_memory_map[1][1] = ite->second[1]._prop_id;			
		 }
		 else if ( 0 == stricmp(HOST_ADDR_INDI, ite->first.c_str()) )
		 {
			map_info._does_host_addr_mapped = true;
			map_info._host_addr_map[0] = ite->second[0]._obj_index;
			map_info._host_addr_map[1] = ite->second[0]._prop_id;
		 }
		 else if ( 0 == stricmp(INDIVIDUAL_ADDR_INDI, ite->first.c_str()) )
		 {
			map_info._does_individual_addr_mapped = true;
			map_info._individual_addr_map[0] = ite->second[0]._obj_index;
			map_info._individual_addr_map[0] = ite->second[0]._prop_id;
		 }
		 else 
		 {
			 assert("不能识别的映射");
		 }
	}  // end of for loop. 
}

void DeviceImpl_TXT::SetMapInfo(const DevMapInfo& map_info)
{
	uchar obj_index, prop_id;
	PropertyFields fields;
	memset(&fields, 0, sizeof(fields));

	if ( map_info._does_group_obj_head_mapped )
	{
		m_mediator[GROUP_OBJ_HEAD_INDI].resize(1);
		m_mediator[GROUP_OBJ_HEAD_INDI][0]._obj_index = map_info._group_obj_head_map[0];
		m_mediator[GROUP_OBJ_HEAD_INDI][0]._prop_id = map_info._group_obj_head_map[1];
	}

	if ( map_info._does_group_obj_value_mapped )
	{
		m_mediator[GROUP_OBJ_VALUE_INDI].resize(1);
		m_mediator[GROUP_OBJ_VALUE_INDI][0]._obj_index = map_info._group_obj_value_map[0];
		m_mediator[GROUP_OBJ_VALUE_INDI][0]._prop_id = map_info._group_obj_value_map[1];
	}
	
	if ( map_info._does_group_addrs_mapped )  // 组地址. 
	{
		obj_index = map_info._group_addrs_map[0];
		prop_id = map_info._group_addrs_map[1];

		m_mediator[GROUP_ADDRS_INDI].resize(1);
		m_mediator[GROUP_ADDRS_INDI][0]._obj_index = obj_index;
		m_mediator[GROUP_ADDRS_INDI][0]._prop_id = prop_id;

		// 对于组地址.确保存在映射到的接口对象.其它的懒得加了. 
		fields._elem_type = 0x22;
		fields._object_index = obj_index;
		fields._property_id = prop_id;
		AddPropFields(obj_index, prop_id, fields);
	}
	
	if ( map_info._does_group_links_mapped )
	{
		m_mediator[GROUP_LINKS_INDI].resize(1);
		m_mediator[GROUP_LINKS_INDI][0]._obj_index = map_info._group_links_map[0];
		m_mediator[GROUP_LINKS_INDI][0]._prop_id = map_info._group_links_map[1];
	}
	
	if ( map_info._does_user_memory_mapped )
	{
		m_mediator[USER_MEMORY_INDI].resize(2);
		m_mediator[USER_MEMORY_INDI][0]._obj_index = map_info._user_memory_map[0][0];
		m_mediator[USER_MEMORY_INDI][0]._prop_id = map_info._user_memory_map[0][1];
		m_mediator[USER_MEMORY_INDI][1]._obj_index = map_info._user_memory_map[1][0];
		m_mediator[USER_MEMORY_INDI][1]._prop_id = map_info._user_memory_map[1][1];
	}

	if ( map_info._does_host_addr_mapped )
	{
		m_mediator[HOST_ADDR_INDI].resize(1);
		m_mediator[HOST_ADDR_INDI][0]._obj_index = map_info._host_addr_map[0];
		m_mediator[HOST_ADDR_INDI][0]._prop_id = map_info._host_addr_map[1];
	}
	
	if ( map_info._does_individual_addr_mapped )
	{
		m_mediator[INDIVIDUAL_ADDR_INDI].resize(1);
		m_mediator[INDIVIDUAL_ADDR_INDI][0]._obj_index = map_info._individual_addr_map[0];
		m_mediator[INDIVIDUAL_ADDR_INDI][0]._prop_id = map_info._individual_addr_map[1];
	}
}

int DeviceImpl_TXT::CommitSuicide()
{
	int retcode = remove(m_dev_desc_path.c_str());
	if ( 0 == retcode )
	{
		m_inited = false;
		m_data.clear();
	}
	return retcode;
}

int DeviceImpl_TXT::GetGroupObjectCount()
{
	Mediator::iterator ite = m_mediator.find(GROUP_OBJ_HEAD_INDI);
	if ( ite == m_mediator.end() )  // 没有记录组对象的信息就返回0个. 
	{
		return 0;
	}

	const Duplex& duplex = ite->second[0];
	PropertyFields fields;
	GetPropFields(duplex._obj_index, duplex._prop_id, fields);
	int count = fields._elem_count;

	return count;
}

int DeviceImpl_TXT::GetGroupObjectHeadByIndex(unsigned char index, GroupObjectHead& gobj_head)
{
	int retcode = -1;
	Mediator::iterator ite = m_mediator.find(GROUP_OBJ_HEAD_INDI);
	if ( ite == m_mediator.end() )  // 没有记录组对象的信息.  
	{
		return retcode;
	}

	PropertyFields fields;
	const Duplex& duplex = ite->second[0];
	GetPropFields(duplex._obj_index, duplex._prop_id, fields);
	int count = fields._elem_count;
	if ( index >= count )
	{
		return retcode;
	}

	uchar mem[4];
	int mem_len = sizeof(mem);
	GetElemValues(duplex._obj_index, duplex._prop_id, index+1, 1, mem, mem_len); // start pos从1开始算,而index从0开始算.  
	assert(sizeof(mem) == mem_len);

	gobj_head.Init(mem[0], mem[1], *(ushort*)(mem+2));
	retcode = 0;

	return retcode;
}

int DeviceImpl_TXT::SetGroupObjectValue(unsigned char index, const unsigned char* val)
{
	int retcode = -1;
	if ( NULL == val )
	{
		return retcode;
	}
	GroupObjectHead gobj_head;
	retcode = GetGroupObjectHeadByIndex(index, gobj_head);
	if ( 0 != retcode )
	{
		return retcode;
	}
	Mediator::iterator ite = m_mediator.find(GROUP_OBJ_VALUE_INDI);
	if ( ite == m_mediator.end() )  // 没有记录组对象的信息.  
	{
		retcode = -1;
		return retcode;
	}

	const Duplex& duplex = ite->second[0];  // 组对象的值存在哪个接口对象. 
	int len_in_bit = gobj_head.GetLenInBit();
	int len_in_byte = gobj_head.GetLenInByte();
	// start pos 从1开始算, 而index从 0 开始算. 
	retcode = SetElemValues(duplex._obj_index, duplex._prop_id, gobj_head.m_start_pos, len_in_bit, val, len_in_byte);

	return retcode;
}

int DeviceImpl_TXT::GetGroupObjectValue(unsigned char index, unsigned char* val, int* len)
{
	int retcode = -1;
	if ( NULL == val || NULL == len )
	{
		return retcode;
	}

	GroupObjectHead gobj_head;
	retcode = GetGroupObjectHeadByIndex(index, gobj_head);
	if ( 0 != retcode )
	{
		return retcode;
	}

	Mediator::iterator ite = m_mediator.find(GROUP_OBJ_VALUE_INDI);
	if ( ite == m_mediator.end() )  // 没有记录组对象的信息.  
	{
		retcode = -1;
		return retcode;
	}

	const Duplex& duplex = ite->second[0];  // 组对象的值存在哪个接口对象. 
	int len_in_bit = gobj_head.GetLenInBit();
	int len_in_byte = gobj_head.GetLenInByte();

	// start pos 从1开始算, 而index从 0 开始算. 
	retcode = GetElemValues(duplex._obj_index, duplex._prop_id, gobj_head.m_start_pos, len_in_bit, val, *len);
	if ( 0 == retcode )
	{
		*len = len_in_bit;
	}

//_out:
	return retcode;
}

int DeviceImpl_TXT::AddGroupObject(GroupObjectHead* gobjh, unsigned char* val)
{
	int retcode = -1;
	unsigned char buff[56];
	Duplex duplex;
	uchar  obj_idx;
	uchar  prop_id;

	if ( NULL == gobjh )
	{
		retcode = -1;
		return retcode;
	}

	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_OBJ_HEAD_INDI);
	if ( ite == m_mediator.end() )  // 没有记录组对象的映射. 
	{
		retcode = -1;
		return retcode;
	}
	duplex = ite->second[0];
	obj_idx = duplex._obj_index;
	prop_id = duplex._prop_id;
	buff[0] = gobjh->m_type;
	buff[1] = gobjh->m_state;
	memcpy(buff+2, &gobjh->m_start_pos, 2);
	AppendElemValue(obj_idx, prop_id, buff, 4);

	ite = m_mediator.find(GROUP_OBJ_VALUE_INDI);
	if ( ite == m_mediator.end() )  // 没有记录组对象的信息.  
	{
		retcode = -1;
		return retcode;
	}
	duplex = ite->second[0];  // 组对象的值存在哪个接口对象. 
	obj_idx = duplex._obj_index;
	prop_id = duplex._prop_id;
	int len_in_bit = gobjh->GetLenInBit();
	int len_in_byte = gobjh->GetLenInByte();
	// start pos 从1开始算, 而index从 0 开始算.把组对象的值写入. 
	retcode = SetElemValues_Ex(duplex._obj_index, duplex._prop_id, gobjh->m_start_pos, len_in_bit, val, len_in_byte, TRUE);

	return retcode;
}

BOOL DeviceImpl_TXT::ExistThisGroupAddr(const unsigned char* g_addr)
{
	if ( NULL == g_addr )
	{
		return FALSE;
	}

	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( m_mediator.end() == ite )
	{
		return FALSE;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return FALSE;
	}

	const unsigned char* ptr2addr = ite2iobj->second.address() + sizeof(PropertyFields);
	int addresses_len = ite2iobj->second.get_size() - sizeof(PropertyFields);
	for ( int i = 0; i < addresses_len; i += 2 )
	{
		if ( 0 == memcmp(ptr2addr+i, g_addr, 2) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

int DeviceImpl_TXT::GetGroupAddrCount()
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( m_mediator.end() == ite )
	{
		return FALSE;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	PropertyFields fields;
	GetPropFields(obj_index, prop_id, fields);
	int count = fields._elem_count;

	return count;
}


const unsigned char* DeviceImpl_TXT::GetGroupAddrByIndex(unsigned char index)
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( m_mediator.end() == ite )
	{
		return NULL;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return NULL;
	}

	PropertyFields* ptr2fields = (PropertyFields*)ite2iobj->second.address();
	const unsigned char* ptr2addres = ite2iobj->second.address() + sizeof(PropertyFields);
	if ( index >= ptr2fields->_elem_count )  // 下标越界. 
	{
		return NULL;
	}
	
	const unsigned char* ptr2addr = ptr2addres + 2 * index;
	return ptr2addr;
}

int DeviceImpl_TXT::RemoveGroupAddrByIndex(unsigned char index)
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( m_mediator.end() == ite )
	{
		return -1;
	}

	Duplex duplex = ite->second[0];
	uchar obj_index = duplex._obj_index;
	uchar prop_id = duplex._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return -1;
	}

	PropertyFields* ptr2fields = (PropertyFields*)ite2iobj->second.address();
	ushort& ref2elem_count = ptr2fields->_elem_count;
	const unsigned char* ptr2addres = ite2iobj->second.address() + sizeof(PropertyFields);
	if ( index >= ref2elem_count )  // 下标越界. 
	{
		return -1;
	}

	// 删掉这个组地址. 
	const unsigned char* ptr2addr = ptr2addres + 2 * index;
	memmove((void*)ptr2addr, ptr2addr+2, 2*(ref2elem_count-index-1));
	ref2elem_count--;
	ite2iobj->second.resize( ite2iobj->second.get_size() - 2 );

	// 调整关联项.因为删除了组地址.很多关联项就失效了.  
	ite = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( m_mediator.end() == ite ) // 没有关联项.
	{
		return 0;  // 直接返回好了。
	}

	obj_index = ite->second[0]._obj_index;
	prop_id = ite->second[0]._prop_id;
	ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return -1;
	}

	std::vector<ushort> links2Bremoved;
	ptr2fields = (PropertyFields*)ite2iobj->second.address();
	uchar* ptr2links = ite2iobj->second.address() + sizeof(PropertyFields);
	for ( int i = 0; i < ref2elem_count; i++ )
	{
		uchar& gobj_idx = *(ptr2links + i * 2);
		uchar& gaddr_idx = *(ptr2links + i * 2 + 1);
		if ( gaddr_idx == index )
		{
			links2Bremoved.push_back(gaddr_idx); // 把要删除的link的索引号记下来. 
		}
		else if ( gaddr_idx > index )
		{
			gaddr_idx--;
		}
	}
	for ( int i = 0; i < links2Bremoved.size(); i++ )
	{
		RemoveLinkByIndex(links2Bremoved[i]);
	}

	return 0;
}


int DeviceImpl_TXT::AddGroupAddr(const unsigned char* g_addr)
{
	if ( NULL == g_addr )
	{
		return -1;
	}
	
	if ( ExistThisGroupAddr(g_addr) )
	{
		return -1;
	}

	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( m_mediator.end() == ite )
	{
		return -1;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);

	int retcode = AppendElemValue(obj_index, prop_id, g_addr, 2);

	return retcode;
}

int DeviceImpl_TXT::GetLinkCount()
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( m_mediator.end() == ite )
	{
		return 0;  // 有 0 个 LinkCount.
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return 0;  // 有 0 个 LinkCount
	}

	PropertyFields* ptr2fields = (PropertyFields*)ite2iobj->second.address();
	int count = ptr2fields->_elem_count;

	return count;
}

int DeviceImpl_TXT::GetLinkByIndex(unsigned char index, unsigned char& gobj_idx, unsigned char& gaddr_idx)
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( m_mediator.end() == ite )
	{
		return -1;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return -1;
	}

	PropertyFields* ptr2fields = (PropertyFields*)ite2iobj->second.address();
	ushort count = ptr2fields->_elem_count;
	uchar* ptr2elems = ite2iobj->second.address() + sizeof(PropertyFields);
	if ( index >= count )
	{
		return -1;
	}

	gobj_idx = *(ptr2elems + index * 2);
	gaddr_idx = *(ptr2elems + index * 2 + 1);

	return 0;
}

int DeviceImpl_TXT::RemoveLinkByIndex(unsigned char index)
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( m_mediator.end() == ite )
	{
		return -1;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return -1;
	}

	PropertyFields* ptr2fields = (PropertyFields*)ite2iobj->second.address();
	ushort& ref2count = ptr2fields->_elem_count;
	uchar* ptr2elems = ite2iobj->second.address() + sizeof(PropertyFields);
	if ( index >= ref2count )
	{
		return -1;
	}

	memmove(ptr2elems+2*index, ptr2elems+2*index+2, 2*(ref2count-index-1) );
	ref2count--;
	ite2iobj->second.resize( sizeof(PropertyFields) + ref2count*2 );

	return 0;
}

int DeviceImpl_TXT::AddLink(unsigned char gobj_idx, unsigned char gaddr_idx)
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( m_mediator.end() == ite )
	{
		return 0;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;

	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( m_data.end() == ite2iobj )
	{
		return -1;
	}
	PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
	const uchar* elems = ite2iobj->second.address() + sizeof(PropertyFields);
	for ( int i = 0; i < fields->_elem_count; i++ )
	{
		const uchar* elem = elems + i*2;
		if ( gobj_idx == elem[0] && gaddr_idx == elem[1] )  // 已经存在这样一个Link了. 
		{
			return 0;
		}
	}

	uchar buff[2];
	buff[0] = gobj_idx;
	buff[1] = gaddr_idx;
	int retcode = AppendElemValue(obj_index, prop_id, buff, 2);

	return retcode;
}

int DeviceImpl_TXT::GetGaddrsOfGobjs(unsigned char gobj_idx, unsigned char* buff, int& buff_len)
{
	if ( NULL == buff )
	{
		return -1;
	}

	// 找映射地址的接口对象. 
	Mediator::iterator ite = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( m_mediator.end() == ite )
	{
		return -1;
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	if ( ite2iobj == m_data.end() )
	{
		return -1;
	}

	PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
	const uchar* ptr2addres = ite2iobj->second.address() + sizeof(PropertyFields);
	int count_of_addres = (ite2iobj->second.get_size() - sizeof(PropertyFields))/2;

	// 找映射link的接口对象. 
	Mediator::iterator ite2link = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( m_mediator.end() == ite2link )
	{
		return -1;
	}

	unsigned char obj_index_of_link = ite2link->second[0]._obj_index;
	unsigned char prop_id_of_link = ite2link->second[0]._prop_id;
	DataContainer::iterator ite2iobj_of_link = MapID2FairyIte(obj_index_of_link, prop_id_of_link);
	if ( ite2iobj_of_link == m_data.end() )
	{
		return -1;
	}

	PropertyFields* fields_of_link = (PropertyFields*)ite2iobj_of_link->second.address();
	const uchar* ptr2addres_of_link = ite2iobj_of_link->second.address() + sizeof(PropertyFields);
	int count_of_link = (ite2iobj_of_link->second.get_size() - sizeof(PropertyFields))/2;

	int addr_count_returned = 0;
	for ( int i = 0; i < count_of_link; i++ )  // 计算出要返回多少个组地址. 
	{
		if ( gobj_idx == ptr2addres_of_link[i*2] )
		{
			addr_count_returned++;
		}
	}

	if ( buff_len < addr_count_returned*2 )
	{
		buff_len = -addr_count_returned*2;
		return -1;
	}

	int num_copied = 0;
	for ( int i = 0; i < count_of_link; i++ )
	{
		if ( gobj_idx == ptr2addres_of_link[i*2] )
		{
			uchar idx_of_gaddr = ptr2addres_of_link[i*2+1];
			buff[num_copied*2] = ptr2addres[idx_of_gaddr*2];
			buff[num_copied*2+1] = ptr2addres[idx_of_gaddr*2+1];
			num_copied ++;
		}
	}

	buff_len = num_copied*2;

	assert(num_copied == addr_count_returned);

	return addr_count_returned;
}

const unsigned char* DeviceImpl_TXT::GetUserMemory()
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(USER_MEMORY_INDI);
	if ( m_mediator.end() == ite )
	{
		return NULL;
	}

	m_mem.clear();

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	const uchar* user_mem = ite2iobj->second.address() + sizeof(PropertyFields);
	int user_mem_len = ite2iobj->second.get_size() - sizeof(PropertyFields);
	if ( ite2iobj != m_data.end() )
	{
		m_mem.cp(user_mem, user_mem_len);
	}

	obj_index = ite->second[1]._obj_index;
	prop_id = ite->second[1]._prop_id;
	ite2iobj = MapID2FairyIte(obj_index, prop_id);
	user_mem = ite2iobj->second.address() + sizeof(PropertyFields);
	user_mem_len = ite2iobj->second.get_size() - sizeof(PropertyFields);
	if ( ite2iobj != m_data.end() )
	{
		m_mem.append(user_mem, user_mem_len);
	}

	const unsigned char* m = m_mem.address();
	return m;
}

int DeviceImpl_TXT::WriteUserMemory(unsigned int shift, unsigned char* buff, int buff_len)
{
	int retcode = -1;
	int mem_size_atleast = shift + buff_len;

	Mediator::iterator ite = MapKeyword2DuplexesIte(USER_MEMORY_INDI);
	if ( ite == m_mediator.end() )
	{
		goto _out;
	}
	
	// user memory 的一兆内存分别存在两个接口对象中. 
	if ( mem_size_atleast <= MAX_USER_MEMORY_SIZE )
	{
		if ( shift < 0x80000 )  // 需要写第一个接口对象. 
		{
			unsigned char obj_index = ite->second[0]._obj_index;
			unsigned char prop_id = ite->second[0]._prop_id;
			DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
			uchar* user_mem = ite2iobj->second.address() + sizeof(PropertyFields);
			int user_mem_len = ite2iobj->second.get_size() - sizeof(PropertyFields);
			assert(user_mem_len==0x80000);

			int byte_copied = buff_len;
			int byte_copied_at_most = 0x80000 - shift;
			if ( buff_len > byte_copied_at_most )
			{
				byte_copied = byte_copied_at_most;
			}
			memcpy((void*)(user_mem+shift), buff, byte_copied);
		}

		if ( shift + buff_len >= 0x80000 )  // 需要写第二个接口对象. 
		{
			unsigned char obj_index = ite->second[1]._obj_index;
			unsigned char prop_id = ite->second[1]._prop_id;
			DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
			uchar* user_mem = ite2iobj->second.address() + sizeof(PropertyFields);
			int user_mem_len = ite2iobj->second.get_size() - sizeof(PropertyFields);
			assert(user_mem_len==0x80000);

			uint start_pos = shift;
			int  byte_copied = buff_len;
			uchar* mem = buff;
			if ( start_pos < 0x80000 )
			{
				start_pos = 0x80000;
				byte_copied -= (0x80000 - shift);
				mem += (0x80000 - shift);
			}
			memcpy(user_mem, mem, byte_copied);
		}
	}

	retcode = 0;

_out:
	return retcode;
}

bool DeviceImpl_TXT::IsIndividualAddressReadEnabled()
{
	Mediator::iterator ite = MapKeyword2DuplexesIte(INDIVIDUAL_ADDR_INDI);
	if ( ite == m_mediator.end() ) // 没有这个映射. 
	{
		return false;  // 默认不响应. 
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	uchar* mem = ite2iobj->second.address() + sizeof(PropertyFields);
	bool enable = mem[0];

	return enable;
}

bool DeviceImpl_TXT::IsIndividualAddressWriteEnabled()
{
	// 有没有映射是否充许响应 IndividualAddressWrite 到接口对象. 
	Mediator::iterator ite = MapKeyword2DuplexesIte(INDIVIDUAL_ADDR_INDI);
	if ( ite == m_mediator.end() ) // 没有这个映射. 
	{
		return false;  // 默认不响应. 
	}

	unsigned char obj_index = ite->second[0]._obj_index;
	unsigned char prop_id = ite->second[0]._prop_id;
	DataContainer::iterator ite2iobj = MapID2FairyIte(obj_index, prop_id);
	uchar* mem = ite2iobj->second.address() + sizeof(PropertyFields);
	bool enable = mem[1];

	return enable;
}

int DeviceImpl_TXT::ReadInFile(const char* file_path)
{
	int retcode = -1;
	int index = 0;
	char* save_ptr;
	char* segment;
	const char* seps = SEPARATOR_STR; // 空格和逗号作分隔符.

	const int huge_buff_len = 4096*512;
	const int huge_buff2_len = 4096*512;
	char* huge_buff = new char[huge_buff_len];
	char* huge_buff2 = new char[huge_buff2_len]; 

	PropertyFields fields;
	int tmp;
	FILE* stream;

	if ( NULL == file_path || m_inited )
	{
		retcode = -1;
		goto _out;
	}

	m_inited = FALSE;
	m_dev_desc_path = file_path;

	m_data.clear();

	// make sure this file's already exist.  
	stream = fopen(file_path, "a");
	if ( NULL == stream )
	{
		std::string err_info = std::string(file_path) + " must exist.";
		report_err_immed(err_info.c_str(), 0xa);
	}
	fclose(stream);
	m_file_stream = fopen(file_path, "r");
	if ( NULL == m_file_stream )
	{
		retcode = errno;
		goto _out;
	}
	while ( fgets(huge_buff2, huge_buff2_len, m_file_stream) )
	{
		trim_str(huge_buff2, " \r\n");
		// '#'起头表示注释 .  
		if ( '\r' == huge_buff2[0] || '\n' == huge_buff2[0] || '#' == huge_buff2[0] || 0 == huge_buff2[0] )
		{
			continue;
		}

		index++;  
		tmp = huge_buff_len;

		if ( '$' == huge_buff2[0] ) // 碰到关键字了. 
		{
			int idx = ExistThisKeyWord(huge_buff2);
			if ( 0 > idx )  // 但没有找到这个关键字. 
			{
				sprintf(huge_buff, "$s: at the line %u, an unrecognized keyword has been detected.\n", 
					file_path, index);
				report_err_immed(huge_buff2, 0xf);
			}

			// 有这个关键字. 
			const char* keyword = key_words[idx];
			std::vector<Duplex> duplexes;
			segment = strtok_s(huge_buff2, seps, &save_ptr); // 略过之前的关键字部分. 
			while ( segment = strtok_s(NULL, seps, &save_ptr) )
			{
				Duplex duplex;
				char* ptr_guard;
				const char* sep_str = " ,\t()";
				char* obj_idx_ptr = strtok_s(segment, sep_str, &ptr_guard);
				char* prop_id_ptr = strtok_s(NULL, sep_str, &ptr_guard);
				
				tmp = huge_buff_len;
				hexs2bins(obj_idx_ptr, (uchar*)huge_buff, &tmp);
				if ( 1 != tmp ) goto _err;
				duplex._obj_index = *(uchar*)huge_buff;

				tmp = huge_buff_len;
				hexs2bins(prop_id_ptr, (uchar*)huge_buff, &tmp);
				if ( 1 != tmp ) goto _err;
				duplex._prop_id = *(uchar*)huge_buff;

				duplexes.push_back(duplex);
			}

			m_mediator[keyword] = duplexes;

			continue;
		}

		// 清掉增强可读性的逗号们. 
		erase_str(huge_buff2, ",");

		// 下面得到属性描述. 
		segment = strtok_s(huge_buff2, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._object_index = *huge_buff;

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._property_id = *huge_buff;

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._elem_type = *huge_buff;

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._perm = *huge_buff;

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._storage_manage = *huge_buff;

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._pid = *huge_buff;

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		memcpy(&fields._data_length, huge_buff, 2);

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		memcpy(&fields._elem_count, huge_buff, 2);

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		fields._w_tag[0] = huge_buff[0];
		fields._w_tag[1] = huge_buff[1];

		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL == segment ){ retcode = index; goto _err; }
		tmp = huge_buff_len;
		hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		memcpy(fields._data_addr, huge_buff, 4);

		// 下面得到元素值. 
		segment = strtok_s(NULL, seps, &save_ptr);
		if ( NULL != segment )
		{
			tmp = huge_buff_len;
			hexs2bins(segment, (unsigned char*)huge_buff, &tmp);
		}
		else
		{
			tmp = 0;
		}

		std::pair<unsigned char, unsigned char> key(fields._object_index, fields._property_id);
		mem_t m( sizeof(PropertyFields) + tmp );
		m.cp((unsigned char*)&fields, sizeof(PropertyFields));
		m.append((unsigned char*)huge_buff, tmp);

		uint total_elem_len_in_byte;
		propfields_parser(fields, NULL, NULL, NULL, NULL, &total_elem_len_in_byte);
		m.resize(sizeof(PropertyFields) + total_elem_len_in_byte);

		if ( m_data.end() == m_data.find(key) )
		{
			m_data[key] = m;
		}
		else
		{
			sprintf(huge_buff, "%s 在第 %u 行重复定义了接口对象(%c, %c)", file_path, index, key.first, key.second);
			report_err_immed(huge_buff, 0xd);
		}
	}

	retcode = 0;
	m_inited = TRUE;
	goto _out;

_err:
	if ( 0 != retcode )
	{
		sprintf(huge_buff2, "设备描述文件 %s 在第 %d 行格式错误.", file_path, index);
		report_err_immed(huge_buff2, 1);
	}

_out:
	if ( NULL != m_file_stream )
	{
		fclose(m_file_stream);
	}

	delete []huge_buff;
	delete []huge_buff2;

	return retcode;
}

int DeviceImpl_TXT::WriteBackFile(const char* file_path)
{
	if ( !m_inited )
	{
		return -1;
	}

	int retcode = 0;
	const int huge_buff_len = 1024*1025;
	const int huge_buff2_len = 1024*1025;
	char* huge_buff = new char[huge_buff_len];
	char* huge_buff2 = new char[huge_buff2_len];

	m_file_stream = fopen(m_dev_desc_path.c_str(), "w");
	if ( NULL == m_file_stream )
	{
		retcode = errno;
		goto _out;
	}

	{  // gcc 抱怨前面的goto跳过了下面变量的初始化.所以这里加个扩号. 
		// 写回到文本文件. 
		int mem_len = 0;
		int line_num = 0;
		for ( DataContainer::iterator ite = m_data.begin();
			ite != m_data.end();
			++ite )
		{
			mem_len += ite->second.get_size();
			line_num++;
		}
		int char_num_needed = mem_len * 2 + line_num * ( 10 + 1 ); // 每个BYTE要用两行字符表示.每行有 10 个分格符和 1 个换行符. 
		char* the_new_txt_mem = new char[char_num_needed];  // 分配内存空间. 
		char* ptr2txt_mem = the_new_txt_mem;
		for ( DataContainer::iterator ite = m_data.begin();
			ite != m_data.end();
			++ite )
		{
			unsigned char* m = ite->second.address();
			// 把你属性描述翻译成可读十六进制字符串形式.每个域用:间隔. 
			for ( int i = 0; i < sizeof(propertyFieldsShiftNum)/sizeof(int); i++ )
			{
				int occupied = propertyFieldsShiftNum[i];
				bins2hexs(m, occupied, ptr2txt_mem, occupied * 2);
				m += occupied;
				ptr2txt_mem += occupied * 2;
				*ptr2txt_mem = SEPARATOR_CHAR;
				ptr2txt_mem++;
			}
			int totalValuesLen = ite->second.get_size()-sizeof(PropertyFields);
			bins2hexs(m, totalValuesLen, ptr2txt_mem, totalValuesLen*2);
			ptr2txt_mem += totalValuesLen * 2;
			*ptr2txt_mem = '\n';
			ptr2txt_mem ++;
		}

		fseek(m_file_stream, 0, SEEK_SET);
		// 把接口对象写入. 
		fwrite(the_new_txt_mem, 1, char_num_needed, m_file_stream);

		fprintf(m_file_stream, "\n");  // 打印一个空行. 

		for ( Mediator::iterator ite = m_mediator.begin();
			  ite != m_mediator.end();
			  ++ite )
		{
			fprintf(m_file_stream, ite->first.c_str());
			std::vector<Duplex>& duplexes = ite->second;
			for ( int i = 0; i < duplexes.size(); i++ )
			{
				bins2hexs(&duplexes[i]._obj_index, 1, huge_buff, huge_buff_len);
				bins2hexs(&duplexes[i]._prop_id, 1, huge_buff2, huge_buff2_len);
				fprintf(m_file_stream, "%c(%s,%s)", SEPARATOR_CHAR, huge_buff, huge_buff2);
			}
			fprintf(m_file_stream, "\n");
		}
			
		// 清理该释放的资源. 
		fclose(m_file_stream);
		m_file_stream = NULL;
		delete []the_new_txt_mem;
		
		//for ( int i = 0; i < m_vec_gobjs.size(); i++ )
		//{
		//	delete [](uchar*)m_vec_gobjs[i];
		//}
		//for ( int i = 0; i < m_vec_gaddrs.size(); i++ )
		//{
		//	delete []m_vec_gaddrs[i];
		//}

		retcode = 0;
	}

_out:
	if ( NULL != m_file_stream )
	{
		fclose(m_file_stream);
	}

	delete []huge_buff;
	delete []huge_buff2;

	return retcode;
}

bool DeviceImpl_TXT::CheckValidity(std::string* info)
{
	char buff[256];

	bool retvalue = false;
	std::string err_info;

	bool gobj_exist = false;
	int  gobj_count = -1;
	// 组对象的值存在另一个接口对象.该接口对象的元素类型是bit,这个变量将指定这个接口对象的元素最长度至少需要多少. 
	int  gobj_val_totallen_atleast_inbit = -1;

	bool gobj_addr_exist = false;
	int  gobj_addr_count = -1;

	// 用来检查SoftUnit的映射. 
	struct {
		const char* _indi;
		uchar _type;
	} vanillas[] = {  // 香草. 
		SOFTUNIT_X_INDI, 0x01,
		SOFTUNIT_Y_INDI, 0x01,
		SOFTUNIT_M_INDI, 0x01,
		SOFTUNIT_S_INDI, 0x01,
		SOFTUNIT_C_INDI, 0x01,
		SOFTUNIT_T_INDI, 0x01,
		SOFTUNIT_D_INDI, 0x22
	};

	// 检查GROUP_OBJ_HEAD的映射
	Mediator::iterator ite2duplex = MapKeyword2DuplexesIte(GROUP_OBJ_HEAD_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在组对象HEAD的映射. 
	{
		gobj_exist = true;
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 1 )
		{
			err_info = GROUP_OBJ_HEAD_INDI" 必须指定一个且只一个接口对象的映射.";
			goto _out;
		}

		uchar obj_idx = duplexes[0]._obj_index;
		uchar prop_id = duplexes[0]._prop_id;
		DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
		if ( ite2iobj == m_data.end() )  // 没有找到这个接口对象. 
		{
			err_info = GROUP_OBJ_HEAD_INDI" 映射的接口对象不存在.";
			goto _out;
		}

		const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
		if ( fields->_elem_type != 0x24 )
		{
			err_info = GROUP_OBJ_HEAD_INDI" 映射的接口对象的元素类型必须是0x24.";
			goto _out;
		}
		gobj_count = fields->_elem_count;
		const uchar* ptr2elems = ite2iobj->second.address() + sizeof(PropertyFields);
		// int the_previous_at_least = 0;
		for ( int i = 0; i < gobj_count; i++ )  // 计算gobj_val_totallen_atleast_inbit
		{
			const uchar* ptr2elem = ptr2elems + i*4;
			ushort start_pos = *(ushort*)(ptr2elem+2);
			GroupObjectHead goh(ptr2elem[0], ptr2elem[1], start_pos);
			int at_least = start_pos + goh.GetLenInBit() - 1;
			if ( at_least > gobj_val_totallen_atleast_inbit )
			{
				gobj_val_totallen_atleast_inbit = at_least;
			}
		}
		// 检查是否元素重叠. 
		uchar* farm = new uchar[gobj_val_totallen_atleast_inbit]; 
		memset(farm, 0, gobj_val_totallen_atleast_inbit);
		for ( int i = 0; i < gobj_count; i++ )
		{
			const uchar* ptr2elem = ptr2elems + i*4;
			ushort start_pos = *(ushort*)(ptr2elem+2);
			GroupObjectHead goh(ptr2elem[0], ptr2elem[1], start_pos);
			int len_in_bit = goh.GetLenInBit();
			if ( start_pos == 0 )
			{
				err_info = GROUP_OBJ_HEAD_INDI" 映射的接口对象指定的组对象值的起始位置不能等于0.";
				delete farm;
				goto _out;
			}
			for ( int j = 0; j < len_in_bit; j++ )
			{
				if ( 0 != farm[start_pos-1+j] )
				{
					err_info = GROUP_OBJ_HEAD_INDI" 映射的接口对象指定的组对象值的起始位置重叠.\r\n"\
						"或者该接口对象的属性描述中指定的元素个数和实际元素个数不一致"\
						"注意组对象值的起始位置从1开始计算.\r\n且配置文件中的数字用小端表示法.";
					delete farm;
					goto _out;
				}
			}
			memset(farm+start_pos-1, 1, len_in_bit);
		}
		delete []farm;
	}

	// 检查组对象值的映射. 
	ite2duplex = MapKeyword2DuplexesIte(GROUP_OBJ_VALUE_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在存放组对象值的映射。 
	{
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 1 )
		{
			err_info = GROUP_OBJ_VALUE_INDI" 必须指定一个且只一个接口对象的映射.";
			goto _out;
		}

		uchar obj_idx = duplexes[0]._obj_index;
		uchar prop_id = duplexes[0]._prop_id;
		DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
		if ( ite2iobj == m_data.end() )  // 没有找到这个接口对象. 
		{
			err_info = GROUP_OBJ_VALUE_INDI" 映射的接口对象不存在.";
			goto _out;
		}

		const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
		if ( fields->_elem_type != 0x01 )
		{
			err_info = GROUP_OBJ_VALUE_INDI" 映射的接口对象的元素类型必须是0x01.";
			goto _out;
		}
		ushort count  = fields->_elem_count;
		if ( count < gobj_val_totallen_atleast_inbit )
		{
			err_info = GROUP_OBJ_VALUE_INDI" 映射的接口对象的元素个数不够.参加 "GROUP_OBJ_HEAD_INDI" 指定的元素值起始位置和元素长度.";
			goto _out;
		}
	}
	else  // 不存在存在组对象值的映射. 
	{
		if ( gobj_exist ) // 但已经存在GroupObjectHead的接口对象了. 
		{
			err_info = "指定了 "GROUP_OBJ_HEAD_INDI" 则必须也指定"GROUP_OBJ_VALUE_INDI;
			goto _out;
		}
	}

	// 检查组地址的映射. 
	ite2duplex = MapKeyword2DuplexesIte(GROUP_ADDRS_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在存放组地址的映射. 
	{
		gobj_addr_exist = true;
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 1 )
		{
			err_info = GROUP_ADDRS_INDI" 必须指定一个且只一个接口对象的映射.";
			goto _out;
		}

		uchar obj_idx = duplexes[0]._obj_index;
		uchar prop_id = duplexes[0]._prop_id;
		DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
		if ( ite2iobj == m_data.end() )  // 没有找到这个接口对象. 
		{
			err_info = GROUP_ADDRS_INDI" 映射的接口对象不存在.";
			goto _out;
		}

		const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
		if ( fields->_elem_type != 0x22 )
		{
			err_info = GROUP_ADDRS_INDI" 映射的接口对象的元素类型必须是0x22.";
			goto _out;
		}
		const uchar* ptr2elems = ite2iobj->second.address() + sizeof(PropertyFields);
		ushort count = fields->_elem_count;
		gobj_addr_count = count;
		// 检查有无重复组地址.  
		std::set<ushort> apocalypse;
		for ( int i = 0; i < count; i++ )
		{
			const uchar* ptr2elem = ptr2elems + i*2;
			uchar gobj_idx = ptr2elem[0];
			uchar gaddr_idx = ptr2elem[1];

			apocalypse.insert( *(ushort*)ptr2elem );
			if ( apocalypse.size() != i + 1 )
			{
				err_info = GROUP_LINKS_INDI" 映射的接口对象指定的组地址有重复."\
					"\r\n或者属性描述中的元素数量和实际指定的元素数量不符";
				goto _out;
			}
		}
	}
	else  // 不存在组地址的映射。 
	{
		if ( gobj_exist ) // 但已经存在GroupObjectHead的接口对象了.就要报错. 
		{
			err_info = "指定了 "GROUP_OBJ_HEAD_INDI" 则必须也指定"GROUP_ADDRS_INDI;
			goto _out;
		}
	}

	// 检查Link的映射. 
	ite2duplex = MapKeyword2DuplexesIte(GROUP_LINKS_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在存放组地址的映射. 
	{
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 1 )
		{
			err_info = GROUP_LINKS_INDI" 必须指定一个且只一个接口对象的映射.";
			goto _out;
		}

		uchar obj_idx = duplexes[0]._obj_index;
		uchar prop_id = duplexes[0]._prop_id;
		DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
		if ( ite2iobj == m_data.end() )  // 没有找到这个接口对象. 
		{
			err_info = GROUP_LINKS_INDI" 映射的接口对象不存在.";
			goto _out;
		}

		const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
		if ( fields->_elem_type != 0x22 )
		{
			err_info = GROUP_LINKS_INDI" 映射的接口对象的元素类型必须是0x22.";
			goto _out;
		}
		ushort count = fields->_elem_count;
		const uchar* ptr2elems = ite2iobj->second.address() + sizeof(PropertyFields);
		// 检查Link有效性以及无重复Link. 
		std::set<ushort> apocalypse;
		for ( int i = 0; i < count; i++ )
		{
			const uchar* ptr2elem = ptr2elems + i*2;
			uchar gobj_idx = ptr2elem[0];
			uchar gaddr_idx = ptr2elem[1];
			
			if ( gobj_idx >= gobj_count || gaddr_idx >= gobj_addr_count )
			{
				err_info = GROUP_LINKS_INDI" 映射的接口对象指定的Link有无效下标.";
				goto _out;
			}

			apocalypse.insert( *(ushort*)ptr2elem );
			if ( apocalypse.size() != i + 1 )
			{
				err_info = GROUP_LINKS_INDI" 映射的接口对象指定的Link有重复.";
				goto _out;
			}
		}
	}
	else  // 不存在Link
	{
		if ( gobj_exist ) // 但已经存在GroupObjectHead的接口对象了.就要报错. 
		{
			err_info = "指定了 "GROUP_OBJ_HEAD_INDI" 则必须也指定"GROUP_LINKS_INDI;
			goto _out;
		}
	}

	// 检查User_Memory的映射. 
	ite2duplex = MapKeyword2DuplexesIte(USER_MEMORY_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在存放组地址的映射. 
	{
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 2 )
		{
			err_info = USER_MEMORY_INDI" 必须指定两个且只有两个映射.";
			goto _out;
		}

		for ( int i = 0; i < 2; i++ )
		{
			uchar obj_idx = duplexes[i]._obj_index;
			uchar prop_id = duplexes[i]._prop_id;
			DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
			if ( ite2iobj == m_data.end() )  // 没有找到这个接口对象. 
			{
				err_info = USER_MEMORY_INDI" 映射的接口对象不存在.";
				goto _out;
			}

			const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
			if ( fields->_elem_type != 0x30 )
			{
				err_info = USER_MEMORY_INDI" 映射的接口对象的元素类型必须是0x30.";
				goto _out;
			}
			if ( fields->_elem_count != 0x8000 )
			{
				err_info = USER_MEMORY_INDI" 映射的接口对象的元素数量必须是0x8000(小端记作0x0080).";
				goto _out;
			}
		}
	}
	else  // 不存在UserMemory的映射. 
	{
		// do nothing...
	}

	// 检查Host_Addr的映射. 
	ite2duplex = MapKeyword2DuplexesIte(HOST_ADDR_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在存放组地址的映射. 
	{
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 1 )
		{
			err_info = HOST_ADDR_INDI" 必须指定一个且只有一个映射.";
			goto _out;
		}

		uchar obj_idx = duplexes[0]._obj_index;
		uchar prop_id = duplexes[0]._prop_id;
		DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
		if ( ite2iobj == m_data.end() )  // 没有找到这个接口对象. 
		{
			err_info = HOST_ADDR_INDI" 映射的接口对象不存在.";
			goto _out;
		}

		const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
		if ( fields->_elem_type != 0x22 )
		{
			err_info = HOST_ADDR_INDI" 映射的接口对象的元素类型必须是0x22.";
			goto _out;
		}
		if ( fields->_elem_count == 0 )
		{
			err_info = HOST_ADDR_INDI" 映射的接口对象指定的元素数量不能为0.";
			goto _out;
		}
	}
	else  // 不存在指定本机地址的映射. 
	{
		// do nothing...
	}

	// 检查 INDIVIDUAL_ADDR 的映射. 
	ite2duplex = MapKeyword2DuplexesIte(INDIVIDUAL_ADDR_INDI);
	if ( ite2duplex != m_mediator.end() )  // 存在存放组地址的映射. 
	{
		std::vector<Duplex>& duplexes = ite2duplex->second;
		if ( duplexes.size() != 1 )
		{
			err_info = INDIVIDUAL_ADDR_INDI" 必须指定一个且只有一个映射.";
			goto _out;
		}

		uchar obj_idx = duplexes[0]._obj_index;
		uchar prop_id = duplexes[0]._prop_id;
		DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
		if ( ite2iobj != m_data.end() )  // 有找到这个接口对象. 
		{
			const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
			if ( fields->_elem_type != 0x21 )
			{
				err_info = INDIVIDUAL_ADDR_INDI" 映射的接口对象的元素类型必须是0x21.";
				goto _out;
			}
			if ( fields->_elem_count != 2 )
			{
				err_info = INDIVIDUAL_ADDR_INDI" 映射的接口对象指定的元素数量必须为2.";
				goto _out;
			}
		}
		else
		{
			err_info = INDIVIDUAL_ADDR_INDI" 映射到的接口对象不存在.";
			goto _out;
		}
	}
	else  // 不存在 INDIVIDUAL_ADDR 的映射. 
	{
		// do nothing...
	}

	// 检查 SOFTUNIT 的映射.
	for ( int i = 0; i < sizeof(vanillas)/sizeof(vanillas[0]); i++ )
	{
		const char* indi = vanillas[i]._indi;
		const uchar data_type = vanillas[i]._type;

		ite2duplex = MapKeyword2DuplexesIte(indi);
		if ( ite2duplex != m_mediator.end() )  // 存在这个软单元的映射.  
		{
			std::vector<Duplex>& duplexes = ite2duplex->second;
			if ( duplexes.size() != 1 )
			{
				err_info = std::string(indi) + " 必须指定一个且只有一个映射.";
				goto _out;
			}

			uchar obj_idx = duplexes[0]._obj_index;
			uchar prop_id = duplexes[0]._prop_id;
			DataContainer::iterator ite2iobj = MapID2FairyIte(obj_idx, prop_id);
			if ( ite2iobj != m_data.end() )  // 有找到这个接口对象. 
			{
				const PropertyFields* fields = (PropertyFields*)ite2iobj->second.address();
				if ( fields->_elem_type != data_type )
				{
					sprintf(buff, "%s 映射的接口对象的元素类型必须是 %u.", indi, data_type);
					err_info = buff;
					goto _out;
				}
			}
			else
			{
				err_info = std::string(indi) + " 映射到的接口对象不存在.";
				goto _out;
			}
		}
	} // end of for loop. 

	retvalue = true;

_out:
	if ( !retvalue && info )
	{
		*info = err_info;
	}   
	return retvalue;
}

int DeviceImpl_TXT::ExistThisKeyWord(const char* keyword)
{
	int count = sizeof(key_words)/sizeof(key_words[0]);

	if ( NULL == keyword )
	{
		return -1;
	}

	for ( int i = 0; i < count; i++ )
	{
		// 找到了. 
		if ( 0 == strncmp(keyword, key_words[i], strlen(key_words[i])) )
		{
			return i;		
		}
	}

	return -1;
}
