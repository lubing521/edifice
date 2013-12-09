#ifndef _H_GROUP_OBJECT_
#define _H_GROUP_OBJECT_

#include "prot_structs.h"

#pragma pack(1)  // 告诉编译器不要在结构体里乱填东西搞对齐. 
class GroupObjectHead
{
public:
	/// 设好对象index和属性ID.这对ID指明了从哪里得到接口对象的值. 
	/// static void SetObjIndexAndPropID(unsigned char obj_index, unsigned char prop_id);
	
	/// 分析一段内存返回一个GroupObject. 
	/// 如果格式不正确则函数返回NULL.
	//static GroupObject* BuildGroupObjectFromMem(unsigned char* mem);

	///// 通过设置参数来创建一个组对象. 
	///// type 组对象类型.  
	///// state 组态标记. 
	///// start 表示组态标记的值在接口对象值的位置.从1开始计数. 
	//static GroupObject* BuildGroupObjectFromParams(unsigned char type, unsigned char state, unsigned short start);
	GroupObjectHead();
	GroupObjectHead(unsigned char type, unsigned char state, unsigned short start);
	~GroupObjectHead();

	void Init(unsigned char type, unsigned char state, unsigned short start);

	/// 返回这个组对象的值的长度,单位是bit. 
	int GetLenInBit();

	/// 返回这个组对象的值的长度，单位是byte. 
	int GetLenInByte();

	///// 把组对象的值取出来.放到val里,len被置为无素的长度单位bit. 
	///// len在传入时为val缓冲的长度,单位byte.
	///// 成功时返回0.失败时返回非0. 
	//int GetValue(unsigned char* val, int& len)const;

	///// 设置值.
	///// val指向的首地址可以取到要设置的新值. 
	//int SetValue(unsigned char* val)const;

public:
    /// 同接口对象的定义类型的约定一致.
	unsigned char m_type; 
	/// bit位. bit7读,写,发送,更新,通讯,记录,bit1和bit0优先级. 
	unsigned char m_state;
	/// 实际元素所在接口对象值的位置.从1开始计数.  
	unsigned short m_start_pos;

private:
	bool m_inited;

//	static unsigned char ms_obj_idx; ///< 组对象的值存在这个(接口对象index，属性id)里.  
//	static unsigned char ms_prop_id; ///< 组对象的值存在这个(接口对象index，属性id)里. 

private:
	//GroupObject();  ///< 不能用默认构造函数. 
	//GroupObject(const GroupObject& obj);  ///< 拷贝构造函数也不可用. 
	//const GroupObject& GroupObject::operator =(const GroupObject& obj);  ///< 赋值操作符也被禁止. 
};
#pragma pack()

#endif


// 以前的组对象值不定长实现的旧代码. 

//#ifndef _H_GROUP_OBJECT_
//#define _H_GROUP_OBJECT_
//
//#include "prot_structs.h"
//
//#pragma pack(1)  // 告诉编译器不要在结构体里乱填东西搞对齐. 
//struct GroupObject
//{
//public:
//	/// 分析一段内存返回一个GroupObject.mem_len被填上实际使用的长度. 
//	/// 如果格式不正确则函数返回NULL.
//	/// 如果内存长度不够也返回NULL,mem_len被置为实际需要的长度. 
//	/// loose变量影响判断内存长度是否足够.如果loose为真,则组对象值部分可以为空.没指定的组对象值被填为0值.
//	/// 如果loose为假.则所有的组对象值都必须被指定. 
//	static GroupObject* BuildGroupObjectFromMem(unsigned char* mem, int& mem_len, BOOL loose = TRUE);
//	
//	/// 通过设置参数来创建一个组对象. 
//	/// st_tags是组态标记。它是Go_State枚举的组合. 
//	/// comm_tags是通讯标记。它是Go_Comm枚举的组合.  
//	static GroupObject* BuildGroupObjectFromParams(Go_Type type, EIB_Priority priority, unsigned char st_tags, 
//				unsigned char comm_tags, unsigned char* buff, int& buff_len);
//
//	/// 传入未知类型时返回0否则返回存组对象值所需要字节数.第二个参数len_in_bit如果非NULL,则返回以bit计数的长度. 
//	static unsigned char MatchType2Len(unsigned char type, uint* len_in_bit = NULL);
//
//	/// 得到这个组对象结构体的总长度,单位byte. 
//	int GetLength()const;
//
//	/// 这个组对象的值的长度.单位BIT. 
//	int GetValueLenInBit()const;
//
//	/// 把组对象的值取出来. 
//	int GetValue(unsigned char* buff, int& len)const;
//
//	/// 设置值. 
//	int SetValue(unsigned char* buff);
//
//private:
//	/// De-constructor. 
//	~GroupObject();
//
//public:
//	/// 15种对象类型.低3个比特取1到7分别表示多少个bits.如果低三个比特都是0.则检查高四个bit.
//	/// 高四个bits表示1, 2, 3, 4, 6, 8, 10, 14时有效.
//	/// 参照 Go_Type. 
//	unsigned char _type; 
//	/// 两个byte共16个bit.最高两个比特位15，14表示优先级. 
//	/// 0 - 4 比特位表示通信标记.通信标记是用或组合的. 
//	/// 5 - 9 比特位表示组态标记.组态标记是用或组合的.  
//	unsigned short _neuron;
//	/// 实际元素所在.不定长. 
//	unsigned char _value[];
//
//private:
//	GroupObject();  ///< 不能用默认构造函数. 
//	GroupObject(const GroupObject& obj);  ///< 拷贝构造函数也不可用. 
//	const GroupObject& GroupObject::operator =(const GroupObject& obj);  ///< 赋值操作符也被禁止. 
//};
//#pragma pack()
//
//#endif