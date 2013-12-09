#ifndef _H_GROUP_OBJECT_
#define _H_GROUP_OBJECT_

#include "prot_structs.h"

#pragma pack(1)  // ���߱�������Ҫ�ڽṹ��������������. 
class GroupObjectHead
{
public:
	/// ��ö���index������ID.���IDָ���˴�����õ��ӿڶ����ֵ. 
	/// static void SetObjIndexAndPropID(unsigned char obj_index, unsigned char prop_id);
	
	/// ����һ���ڴ淵��һ��GroupObject. 
	/// �����ʽ����ȷ��������NULL.
	//static GroupObject* BuildGroupObjectFromMem(unsigned char* mem);

	///// ͨ�����ò���������һ�������. 
	///// type ���������.  
	///// state ��̬���. 
	///// start ��ʾ��̬��ǵ�ֵ�ڽӿڶ���ֵ��λ��.��1��ʼ����. 
	//static GroupObject* BuildGroupObjectFromParams(unsigned char type, unsigned char state, unsigned short start);
	GroupObjectHead();
	GroupObjectHead(unsigned char type, unsigned char state, unsigned short start);
	~GroupObjectHead();

	void Init(unsigned char type, unsigned char state, unsigned short start);

	/// �������������ֵ�ĳ���,��λ��bit. 
	int GetLenInBit();

	/// �������������ֵ�ĳ��ȣ���λ��byte. 
	int GetLenInByte();

	///// ��������ֵȡ����.�ŵ�val��,len����Ϊ���صĳ��ȵ�λbit. 
	///// len�ڴ���ʱΪval����ĳ���,��λbyte.
	///// �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	//int GetValue(unsigned char* val, int& len)const;

	///// ����ֵ.
	///// valָ����׵�ַ����ȡ��Ҫ���õ���ֵ. 
	//int SetValue(unsigned char* val)const;

public:
    /// ͬ�ӿڶ���Ķ������͵�Լ��һ��.
	unsigned char m_type; 
	/// bitλ. bit7��,д,����,����,ͨѶ,��¼,bit1��bit0���ȼ�. 
	unsigned char m_state;
	/// ʵ��Ԫ�����ڽӿڶ���ֵ��λ��.��1��ʼ����.  
	unsigned short m_start_pos;

private:
	bool m_inited;

//	static unsigned char ms_obj_idx; ///< ������ֵ�������(�ӿڶ���index������id)��.  
//	static unsigned char ms_prop_id; ///< ������ֵ�������(�ӿڶ���index������id)��. 

private:
	//GroupObject();  ///< ������Ĭ�Ϲ��캯��. 
	//GroupObject(const GroupObject& obj);  ///< �������캯��Ҳ������. 
	//const GroupObject& GroupObject::operator =(const GroupObject& obj);  ///< ��ֵ������Ҳ����ֹ. 
};
#pragma pack()

#endif


// ��ǰ�������ֵ������ʵ�ֵľɴ���. 

//#ifndef _H_GROUP_OBJECT_
//#define _H_GROUP_OBJECT_
//
//#include "prot_structs.h"
//
//#pragma pack(1)  // ���߱�������Ҫ�ڽṹ��������������. 
//struct GroupObject
//{
//public:
//	/// ����һ���ڴ淵��һ��GroupObject.mem_len������ʵ��ʹ�õĳ���. 
//	/// �����ʽ����ȷ��������NULL.
//	/// ����ڴ泤�Ȳ���Ҳ����NULL,mem_len����Ϊʵ����Ҫ�ĳ���. 
//	/// loose����Ӱ���ж��ڴ泤���Ƿ��㹻.���looseΪ��,�������ֵ���ֿ���Ϊ��.ûָ���������ֵ����Ϊ0ֵ.
//	/// ���looseΪ��.�����е������ֵ�����뱻ָ��. 
//	static GroupObject* BuildGroupObjectFromMem(unsigned char* mem, int& mem_len, BOOL loose = TRUE);
//	
//	/// ͨ�����ò���������һ�������. 
//	/// st_tags����̬��ǡ�����Go_Stateö�ٵ����. 
//	/// comm_tags��ͨѶ��ǡ�����Go_Commö�ٵ����.  
//	static GroupObject* BuildGroupObjectFromParams(Go_Type type, EIB_Priority priority, unsigned char st_tags, 
//				unsigned char comm_tags, unsigned char* buff, int& buff_len);
//
//	/// ����δ֪����ʱ����0���򷵻ش������ֵ����Ҫ�ֽ���.�ڶ�������len_in_bit�����NULL,�򷵻���bit�����ĳ���. 
//	static unsigned char MatchType2Len(unsigned char type, uint* len_in_bit = NULL);
//
//	/// �õ���������ṹ����ܳ���,��λbyte. 
//	int GetLength()const;
//
//	/// ���������ֵ�ĳ���.��λBIT. 
//	int GetValueLenInBit()const;
//
//	/// ��������ֵȡ����. 
//	int GetValue(unsigned char* buff, int& len)const;
//
//	/// ����ֵ. 
//	int SetValue(unsigned char* buff);
//
//private:
//	/// De-constructor. 
//	~GroupObject();
//
//public:
//	/// 15�ֶ�������.��3������ȡ1��7�ֱ��ʾ���ٸ�bits.������������ض���0.������ĸ�bit.
//	/// ���ĸ�bits��ʾ1, 2, 3, 4, 6, 8, 10, 14ʱ��Ч.
//	/// ���� Go_Type. 
//	unsigned char _type; 
//	/// ����byte��16��bit.�����������λ15��14��ʾ���ȼ�. 
//	/// 0 - 4 ����λ��ʾͨ�ű��.ͨ�ű�����û���ϵ�. 
//	/// 5 - 9 ����λ��ʾ��̬���.��̬������û���ϵ�.  
//	unsigned short _neuron;
//	/// ʵ��Ԫ������.������. 
//	unsigned char _value[];
//
//private:
//	GroupObject();  ///< ������Ĭ�Ϲ��캯��. 
//	GroupObject(const GroupObject& obj);  ///< �������캯��Ҳ������. 
//	const GroupObject& GroupObject::operator =(const GroupObject& obj);  ///< ��ֵ������Ҳ����ֹ. 
//};
//#pragma pack()
//
//#endif