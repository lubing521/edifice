#pragma once
#include <vector>
#include <utility>
#include <map>
#include "prot_structs.h"

// 三个宏定义用在 RecordMe 打印日志函数. 
#define NORMAL_MSG_LEVEL
#define WARNING_MSG_LEVEL  "WARNING: "
#define ERROR_MSG_LEVEL    "ERROR: "

// 这个结构体描述怎么做写测试. 
struct WritingTestData 
{
	std::string _ops;  // 如何操作
	int _obj_id;   // ...
	int _att_id;   // ...
	int _count;    // 写多少个元素
	int _start_pos;  // 从哪个位置开始
	std::string _data_str;  // 写什么.可读十六进制表示. 
};

class TheDeviceObj
{
public:
	// spec指定了pair<int,int>对的一个vector.指明了有多少对象ID,每个对象ID的最大属性值. 
	TheDeviceObj(const char* host, const char* peer, int port, int baundrate,
					const std::vector<std::pair<int, int>>& spec);	
	~TheDeviceObj(void);

	// 读设备. 
	int ReadDevice();

	// 通过对象ID和属性ID得到Property
	PropertyFields GetPropertie(int o_id, int a_id)
	{
		std::pair<int, int> index(o_id, a_id);
		return m_properties[index];
	}

	// 给通信伙伴发送密钥.密钥正确返回0.key是零长度字符串就不发送密钥也返回0.否则返回非0. 
	int Sendkey(const std::string& key);

	// 发原始数据.传入用可读十六进制形式表示的原始数据. 
	int eSend_Rawdate(const std::string& rawdata_h);
	// 检查读到的属性值的对象ID和属性ID是否匹配. 
	// 如果匹配返回 0. 不匹配返回非 0 .
	int CheckPropIDMatching();

	// 测试读RAM...
	int CheckRAM_r(const std::vector<std::pair<int, int>>& ram_obj_att, int testing_count);

	// 测试顺序读属性.传入按顺序读属性时共有多少条记录. s means serial.
	int CheckProperty_rs(int total_record, int testing_count);

	// 按对象ID属性ID对来读属性.
	int CheckProperty_ri(const std::vector<std::pair<int, int>>& obj_att_range, int testing_count);

	// 测试按顺序读值.传入按顺序读属性值时共有多少条记录. 
	int CheckValue_rs(int total_record, int testing_count);

	// 测试按ID属性ID对来读值. 
	int CheckValue_ri(const std::vector<std::pair<int, int>>& obj_att_range, int testing_count);

	// 测试写值. 
	int CheckValue_w(const WritingTestData& wtd);
    
	// 查询权限. 
	int eAuthorize_Query(const unsigned char* key, unsigned char* level);

private:
	// 打印日志. 
	int  RecordMe(const char* format, ...);
	
	// 如果返回真的话.读写它的值就要用读写方法2.
	bool HasHugeNumOfValues(int obj_id, int attrib_id);

	// 下面四个函数对应通讯接口的非Ex版本.区别在Ex版本收发的数量超过了15的限制.
	// 完全成功才返回0.
	// 否则非0.
	// VERY IMPORTANT NOTICE. CAVEAT. 下面的函数都不支持长度是以bit计算的元素.
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

	// 这个map记下device中所有的属性. 
	std::vector<std::pair<int, int>> m_specs;   // 对象ID 和 属性 ID 对. 
	std::map<std::pair<int, int>, PropertyFields> m_properties;
	std::map<int, PropertyFields> m_properties_s; // 按序号对存入所有属性值. 


	// 这个vector记下device中的值.
	// std::vector<mem_t>  m_values;

	FILE*   m_report_file;

	// 用来存收到的数据. 
	struct Response
	{
		int  _obj_id;
		int  _attrib_id;
		int  _start_pos;
		int  _count;
		std::string _resp_str;   // 可读十六进制字符串. 

		bool operator < (const Response& another)const //类字典序比较方式
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
		int _hit_times;   // 命中次数. 

		ResponseAttached()
		{
			_hit_times = 0;
		}
	};
	// map 的 key 是个Reponse结构. value是对应的这个Response的次数. 
	std::map<Response, ResponseAttached>  m_responses;

	unsigned char m_big_buff[2048];
	unsigned char m_big_buff2[2048];
};
