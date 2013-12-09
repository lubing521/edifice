#include "stdafx.h"
#include "group_object.h"
#include "generic_helper.h"

//int GroupObjectHead::ms_obj_idx;
//int GroupObjectHead::ms_prop_id;

//void GroupObjectHead::SetObjIndexAndPropID(unsigned char obj_index, unsigned char prop_id)
//{
//	ms_obj_idx = obj_index;
//	ms_prop_id = prop_id;
//}

//GroupObjectHead* GroupObjectHead::BuildGroupObjectHeadFromMem(unsigned char* mem)
//{
//	GroupObjectHead* obj = NULL;
//	if ( NULL == mem )
//	{
//		goto _out;
//	}
//
//	ElemTypeField elem_type(m_type);
//	if ( elem_type.get_unit_code() > 0x02 )  // 非法.只支持bit,和byte.
//	{
//		goto _out;
//	}
//
//	obj = new GroupObjectHead;
//	memcpy(obj, mem, sizeof(GroupObjectHead));
//
//_out:
//	return obj;
//}

/// 通过设置参数来创建一个组对象. 
//GroupObjectHead* GroupObjectHead::BuildGroupObjectHeadFromParams(unsigned char type, unsigned char state, unsigned short start)
//{
//	GroupObjectHead* obj = new GroupObjectHead;
//	
//	obj->m_type = type;
//	obj->m_state;
//	obj->m_start_pos = start;
//
//	return obj;
//}

GroupObjectHead::GroupObjectHead()
{
	m_inited = false;
}

GroupObjectHead::~GroupObjectHead()
{
}

GroupObjectHead::GroupObjectHead(unsigned char type, unsigned char state, unsigned short start)
{
	m_type = type;
	m_state = state;
	m_start_pos = start;
	m_inited = true;
}

void GroupObjectHead::Init(unsigned char type, unsigned char state, unsigned short start)
{
	m_type = type;
	m_state = state;
	m_start_pos = start;
	m_inited = true;
}

int GroupObjectHead::GetLenInBit()
{
	if ( !m_inited )
	{
		report_err_immed("GroupObjectHead未初始化就被使用.", 0xe);
	}

	uint value_len_in_bit;

	ElemTypeField elem_type(m_type);
	value_len_in_bit = elem_type.get_len_in_bit();

	return value_len_in_bit;
}

int GroupObjectHead::GetLenInByte()
{
	if ( !m_inited )
	{
		report_err_immed("GroupObjectHead未初始化就被使用.", 0xe);
	}

	uint value_len_in_bit = GetLenInBit();
	uint value_len_in_byte = ceil_divide(value_len_in_bit, 8);

	return value_len_in_byte;
}
 
//int GroupObjectHead::GetValue(unsigned char* val, int& len)
//{
//	uint value_len_in_bit;
//	ElemTypeField elem_type(m_type);
//	value_len_in_bit = elem_type.get_len_in_bit();
//	int len_atleast_inbyte = CeilDivide(value_len_in_bit, 8);
//
//	if ( 0 == ms_obj_idx || 0 == ms_prop_id )
//	{
//		report_err_immed("must call GroupObjectHead::SetObjIndexAndPropID first.", 0xa);
//	}
//	
//	if ( len < len_atleast_inbyte )
//	{
//		len = -len_atleast_inbyte;
//		return len;
//	}
//
//	
//	
//	return 0;
//}
//
//int GroupObjectHead::SetValue(unsigned char* val)
//{
//	if ( NULL == buff )
//	{
//		return -1;
//	}
//
//	int len_in_byte = MatchType2Len(_type);
//	memcpy(_value, buff, len_in_byte);
//
//	return 0;
//}
