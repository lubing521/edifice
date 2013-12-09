#pragma once
#include <vector>
#include <utility>
#include <map>
#include "prot_structs.h"

// �����궨������ RecordMe ��ӡ��־����. 
#define NORMAL_MSG_LEVEL
#define WARNING_MSG_LEVEL  "WARNING: "
#define ERROR_MSG_LEVEL    "ERROR: "

// ����ṹ��������ô��д����. 
struct WritingTestData 
{
	std::string _ops;  // ��β���
	int _obj_id;   // ...
	int _att_id;   // ...
	int _count;    // д���ٸ�Ԫ��
	int _start_pos;  // ���ĸ�λ�ÿ�ʼ
	std::string _data_str;  // дʲô.�ɶ�ʮ�����Ʊ�ʾ. 
};

class TheDeviceObj
{
public:
	// specָ����pair<int,int>�Ե�һ��vector.ָ�����ж��ٶ���ID,ÿ������ID���������ֵ. 
	TheDeviceObj(const char* host, const char* peer, int port, int baundrate,
					const std::vector<std::pair<int, int>>& spec);	
	~TheDeviceObj(void);

	// ���豸. 
	int ReadDevice();

	// ͨ������ID������ID�õ�Property
	PropertyFields GetPropertie(int o_id, int a_id)
	{
		std::pair<int, int> index(o_id, a_id);
		return m_properties[index];
	}

	// ��ͨ�Ż�鷢����Կ.��Կ��ȷ����0.key���㳤���ַ����Ͳ�������ԿҲ����0.���򷵻ط�0. 
	int Sendkey(const std::string& key);

	// ��ԭʼ����.�����ÿɶ�ʮ��������ʽ��ʾ��ԭʼ����. 
	int eSend_Rawdate(const std::string& rawdata_h);
	// ������������ֵ�Ķ���ID������ID�Ƿ�ƥ��. 
	// ���ƥ�䷵�� 0. ��ƥ�䷵�ط� 0 .
	int CheckPropIDMatching();

	// ���Զ�RAM...
	int CheckRAM_r(const std::vector<std::pair<int, int>>& ram_obj_att, int testing_count);

	// ����˳�������.���밴˳�������ʱ���ж�������¼. s means serial.
	int CheckProperty_rs(int total_record, int testing_count);

	// ������ID����ID����������.
	int CheckProperty_ri(const std::vector<std::pair<int, int>>& obj_att_range, int testing_count);

	// ���԰�˳���ֵ.���밴˳�������ֵʱ���ж�������¼. 
	int CheckValue_rs(int total_record, int testing_count);

	// ���԰�ID����ID������ֵ. 
	int CheckValue_ri(const std::vector<std::pair<int, int>>& obj_att_range, int testing_count);

	// ����дֵ. 
	int CheckValue_w(const WritingTestData& wtd);
    
	// ��ѯȨ��. 
	int eAuthorize_Query(const unsigned char* key, unsigned char* level);

private:
	// ��ӡ��־. 
	int  RecordMe(const char* format, ...);
	
	// ���������Ļ�.��д����ֵ��Ҫ�ö�д����2.
	bool HasHugeNumOfValues(int obj_id, int attrib_id);

	// �����ĸ�������ӦͨѶ�ӿڵķ�Ex�汾.������Ex�汾�շ�������������15������.
	// ��ȫ�ɹ��ŷ���0.
	// �����0.
	// VERY IMPORTANT NOTICE. CAVEAT. ����ĺ�������֧�ֳ�������bit�����Ԫ��.
	int PropertyValue_Read_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned short& count, 
						       unsigned short start, unsigned char* buff, int& buff_len);
	int PropertyValue_Write_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned char count,
							   unsigned short start, const unsigned char* buff, int buff_len);
	int PropertyValue_Read2_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned short& count,
							   unsigned short start, unsigned char* buff, int& buff_len);
	int PropertyValue_Write2_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned short count,
							   unsigned short start, const unsigned char* buff, int buff_len);
	///////////////////////////////////////////////////////////////////////////////////////////////

	static void Lurker(const UCHAR* buff, int len, const void* arg);

private:
	int    m_port;
	uchar   m_peer_addr[2];
	uchar   m_host_addr[2];

	// ���map����device�����е�����. 
	std::vector<std::pair<int, int>> m_specs;   // ����ID �� ���� ID ��. 
	std::map<std::pair<int, int>, PropertyFields> m_properties;
	std::map<int, PropertyFields> m_properties_s; // ����ŶԴ�����������ֵ. 


	// ���vector����device�е�ֵ.
	// std::vector<mem_t>  m_values;

	FILE*   m_report_file;

	// �������յ�������. 
	struct Response
	{
		int  _obj_id;
		int  _attrib_id;
		int  _start_pos;
		int  _count;
		std::string _resp_str;   // �ɶ�ʮ�������ַ���. 

		bool operator < (const Response& another)const //���ֵ���ȽϷ�ʽ
		{
			bool b = false;
			if ( _obj_id < another._obj_id )
			{
				b = true;
				goto _out;
			}
			else if ( _obj_id > another._obj_id )
			{
				b = false;
				goto _out;
			}

			if ( _attrib_id < another._attrib_id )
			{
				b = true;
				goto _out;
			}
			else if ( _attrib_id > another._attrib_id )
			{
				b = false;
				goto _out;
			}

			if ( _count < another._count )
			{
				b = true;
				goto _out;
			}
			else if ( _count == another._count )
			{
				b = false;
				goto _out;
			}

			if ( _resp_str < another._resp_str )
			{
				b = true;
				goto _out;
			}
			else if ( _resp_str > another._resp_str )
			{
				b = false;
				goto _out;
			}
	
_out:
			return b;
		}
	};
	struct ResponseAttached
	{
		int _hit_times;   // ���д���. 

		ResponseAttached()
		{
			_hit_times = 0;
		}
	};
	// map �� key �Ǹ�Reponse�ṹ. value�Ƕ�Ӧ�����Response�Ĵ���. 
	std::map<Response, ResponseAttached>  m_responses;

	unsigned char m_big_buff[2048];
	unsigned char m_big_buff2[2048];
};
