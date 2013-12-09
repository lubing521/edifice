#include "StdAfx.h"
#include "TheDeviceObj.h"
#include "Pentagon.h"
#include "EIBCommNEO.h"

#define REPORT_FILE_NAME   "testing_report.txt"
#define MAGIC_TRING_TIMES  6   // 如果读写失败时最多重复多少次. 

extern std::auto_ptr<TheDeviceObj> g_devobj;

TheDeviceObj::TheDeviceObj(const char* host, const char* peer, int port, int baundrate, 
						   const std::vector<std::pair<int, int>>& spec):m_specs(spec)
{
	int retcode = -1;
	unsigned char host_addr[2];
	unsigned char peer_addr[2];

	if ( 4 < strlen(host) || 4 < strlen(peer) )
	{
		MessageBoxA(NULL, "FATAL ERROR", "Arguments host or peer cannot exceeds length 4.", MB_OK);
		exit(1);
	}

	int len = 2;
	hexs2bins(host, host_addr, &len);
	hexs2bins(peer, peer_addr, &len);
	// retcode = m_serial_comm->Init(port, baundrate, host_addr, peer_addr);
	retcode = Init(port, baundrate, host_addr);

	if ( 0 != retcode )
	{
		MessageBoxA(NULL, "FATAL ERROR", "Init ISerialPort error.", MB_OK);
		exit(1);
	}

	m_report_file = fopen(REPORT_FILE_NAME, "w");
	if ( NULL == m_report_file )
	{
		MessageBoxA(NULL, REPORT_FILE_NAME, "Failed to open file", MB_OK);
		exit(1);
	}

	m_port = port;
	memcpy(m_host_addr, host_addr , sizeof(m_host_addr));
	memcpy(m_peer_addr, peer_addr , sizeof(m_peer_addr));
}

TheDeviceObj::~TheDeviceObj(void)
{
	//m_serial_comm->Uninit();
	//m_serial_comm->Free();
	Uninit();

	if ( NULL != m_report_file )
	{
		fclose(m_report_file);
	}
}

int TheDeviceObj::RecordMe(const char* format, ...)
{
	int retcode = 0;
	va_list ap;
	va_start(ap, format);
	retcode = vfprintf(m_report_file, format, ap);
	vfprintf(stdout, format, ap);
	va_end(ap);

	return retcode;
}

bool TheDeviceObj::HasHugeNumOfValues(int obj_id, int attrib_id)
{
	if ( 2 == obj_id )
	{
		if ( 8 == attrib_id || 9 == attrib_id )
		{
			return true;
		}
	}

	return false;
}

int TheDeviceObj::ReadDevice()
{
	//int retcode = 0;
	RecordMe("初始化工作.先读入所有对象以及它对应的所有属性的描述.\n");
	for ( int i = 0; i < m_specs.size(); i ++ )
	{
		int obj_id = m_specs[i].first;
		int max_att_id = m_specs[i].second;

		for ( int j = 1; j <= max_att_id; j ++ )
		{
			PropertyFields fields0 ,fields1;
			std::pair<int, int> index(obj_id, j);
			int okay0, okay1;
			//retcode = m_serial_comm->Property_Read(obj_id, j, fields, MAGIC_TRING_TIMES);
			okay0 = Property_Read(m_peer_addr,0,0,0,obj_id, j, fields0);
			okay1 = Property_Read(m_peer_addr,0,0,0,obj_id,j,fields1);
				if ( 0 == okay0 && 0 == okay1 ) // 读两次属性描述都成功. 
				{
					// 且两次读出来的数据都一样.
					if ( 0 == memcmp(&fields0, &fields1, sizeof(PropertyFields)) )
					{
						m_properties[index] = fields0;
					}
					else
					{
						RecordMe("初始化工作碰到严重错误.读到对象ID %d 属性ID %d 的属性描述两次"
							"但访回的属性描述不一致.\n", obj_id, j);
						report_err_immed("初始化工作必须成功.", 1);
					}
				}
				else if ( 0 == (okay0 & okay1) )  // 两次读属性描述，其中一次成功了. 
				{
					RecordMe("初始化工作碰到严重错误.读到对象ID %d 属性ID %d 的属性描述两次"
						"但只成功一次.\n", obj_id, j);
					report_err_immed("初始化工作必须成功.", 1);
				}
				else
				{ 
					RecordMe("读到对象ID %d 属性ID %d 的属性两次均失败.可能不存在该(对象,属性)对.也可能是个错误.\n", obj_id, j);
				}
		}
	}

	return 0;
}

int TheDeviceObj::Sendkey(const std::string& key)
{
	if ( 0 != key.length() )
	{
		unsigned char level;
		int buff_len = sizeof(m_big_buff);
		hexs2bins(key.c_str(), m_big_buff, &buff_len);
		if ( 4 != buff_len )
		{
			RecordMe(WARNING_MSG_LEVEL"密钥 %s 格式错误。密钥的长度为四字节,需要用八个十六进制字符表示.\n", key.c_str());
			return -1;
		}
		if ( 0 == eAuthorize_Query(m_big_buff, &level) )
		{
			RecordMe(NORMAL_MSG_LEVEL"密钥 %s 被接受,得到权限等级 %u\n", key.c_str(), level);
			return 0;
		}
		else
		{
			RecordMe(WARNING_MSG_LEVEL"密钥 %s 被拒绝！！！！！\n", key.c_str());
			return -2;
		}
	}

	return 0;
}

void TheDeviceObj::Lurker(const UCHAR* buff, int len, const void* arg)
{
	char buxx[1024];
	int  buxx_len = sizeof(buxx);
	bins2hexs(buff, len, buxx, buxx_len);

	static unsigned long lurk;

	if ( ! lurk++ )
	{
		g_devobj->RecordMe("######## 发送脚本指定的帧.\n");
	}

	const char* ptr = (const char*)arg;
	if ( *ptr == 't' )
		g_devobj->RecordMe(NORMAL_MSG_LEVEL"发出帧 %s \n", buxx);
	else if ( *ptr == 'r' )
		g_devobj->RecordMe(NORMAL_MSG_LEVEL"收到帧 %s \n\n", buxx);
}

int TheDeviceObj::eSend_Rawdate(const std::string& rawdata_h)
{
	int retcode = 0;
	int buff_len = sizeof(m_big_buff);
	int buff_len2 = sizeof(m_big_buff2);

	Reporter save = SetMonitor(Lurker);

	hexs2bins(rawdata_h.c_str(), m_big_buff, &buff_len);
	retcode = Send_Rawdate(m_big_buff, buff_len, m_big_buff2, buff_len2);

	if ( SEND_RAWDATA_NOT_A_FRAME == retcode )
	{
		RecordMe(WARNING_MSG_LEVEL"这不是一个合法的帧 %s \n", rawdata_h.c_str());
	}
	else if ( SEND_RAWDATA_NO_LEGAL_RESPONSE == retcode )
	{
		RecordMe(WARNING_MSG_LEVEL"没有收到回复或者合法的回复. \n");
	}

	SetMonitor(save);

	return retcode;
}

int TheDeviceObj::CheckPropIDMatching()
{
	int retcode = 0;

	for ( std::map<std::pair<int, int>, PropertyFields>::iterator ite = m_properties.begin();
		  ite != m_properties.end();
		  ++ite )
	{
		int obj_id = ite->first.first;
		int attr_id = ite->first.second;
		int obj_id_rcved = ite->second._object_index;
		int attr_id_rcved = ite->second._property_id;

		if ( obj_id == obj_id_rcved && attr_id == attr_id_rcved )
		{
			continue;
		}

		RecordMe("检索对象ID: %d 属性ID: %d 时却返回 对象 ID: %d 属性 ID: %d 不匹配.\n", obj_id, attr_id, obj_id_rcved, attr_id_rcved);

		retcode ++;
	}

	return retcode;
}

int TheDeviceObj::CheckRAM_r(const std::vector<std::pair<int, int>>& ram_obj_att, int testing_count)
{
	int retcode = 0;
	m_responses.clear();

	if ( 0 == testing_count )
	{
		return 0;
	}

	RecordMe("\n############################\n\n");
	RecordMe("\n###### 开始RAM测试\n");
	RecordMe("读所有RAM内存各 %d 次\n", testing_count);

	for ( std::vector<std::pair<int, int>>::const_iterator ite = ram_obj_att.begin();
		  ite != ram_obj_att.end(); 
		  ++ ite )
	{
		int object_id = ite->first;
		int attrib_id = ite->second;

		RecordMe("开始读对象ID %d 属性 ID %d 的RAM.\n", object_id, attrib_id);

		std::map<std::pair<int, int>, PropertyFields>::const_iterator ite2prop = m_properties.find(*ite);
		if ( ite2prop == m_properties.end() )  // 没有找到对应这个对象ID属性ID. 
		{
			RecordMe("初始化时未能读到对象ID %d 属性 ID %d 的记录\n", object_id, attrib_id);
			continue;
		}

		/////////////////////////////////////////////////////////

		PropertyFields fields = ite2prop->second;

		int elem_count =  fields._elem_count;   // 共多少个元素.
		int elem_count_read_this_time = elem_count;
		int up_bound = 1;   // 每次读15个元素.如果RAM里多于15个元素，就要从序号2,3,4...开始读.
		if ( elem_count >= 16 )
		{
			up_bound = elem_count - 15;
			elem_count_read_this_time = 15;
		}
		else
		{
			elem_count_read_this_time = elem_count;
		}
		int indices_num = up_bound - 0;  // 开始序号共有多少个. 
		for ( int i = 0; i < testing_count; i++ )
		{
			int start_pos = ( i % up_bound ) + 1;
			unsigned char count = elem_count_read_this_time;
			unsigned char count_save = count;
			int buff_len = sizeof(m_big_buff);
			//int okay = m_serial_comm->PropertyValue_Read(object_id, attrib_id, count, start_pos, m_big_buff, buff_len, 1);
			int okay =PropertyValue_Read(m_peer_addr,0,0,0,object_id, attrib_id, count, start_pos, m_big_buff, buff_len);
			if ( 0 == okay )  // 成功读到值. 
			{
				// 把它转成十六进制readable string.
				bins2hexs(m_big_buff, buff_len, (char*)m_big_buff2, sizeof(m_big_buff2));
				Response resp;
				resp._obj_id = object_id;
				resp._attrib_id = attrib_id;
				resp._start_pos = start_pos;
				resp._count = count;
				resp._resp_str = (char*)m_big_buff2;
				ResponseAttached& ra = m_responses[resp];
				ra._hit_times ++;
			}
			else
			{
				// 一次读失败. 
				RecordMe("读对象ID %d 属性ID %d 起始位置 %d 元素个数 %d 失败.\n", object_id, attrib_id, start_pos, count_save);
			}
		} // 进行完所有的测试次数.

		for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
			  ite != m_responses.end(); 
			  ++ ite )
		{
			RecordMe("读对象ID %d 属性ID %d 的值，起始位置为 %d,元素 %d 个. 共收到回复 %s %d 次\n", 
						ite->first._obj_id,  ite->first._attrib_id,
						ite->first._start_pos, ite->first._count,
						ite->first._resp_str.c_str(), ite->second._hit_times );
		}
		m_responses.clear();
	} // end of the main for loop. 

	RecordMe("\n\n");

	return retcode;
}

int TheDeviceObj::CheckProperty_rs(int total_record, int testing_count)
{
	int  retcode = 0;
	BOOL record_me = TRUE;
	int  loop_times = testing_count;

	if ( 0 == testing_count )
	{
		record_me = FALSE;
		loop_times = 1;
	}

	record_me && RecordMe("\n###### 开始按顺序读属性描述.\n");
	m_responses.clear();

	for ( int i = 1; i <= total_record; i++ )
	{
		int object_id = 0;
		int attrib_id = i;

		//RecordMe("按顺序读属性.顺序ID号 %d \n", i);

		for ( int j = 0; j < loop_times; j++ )
		{
			PropertyFields fields;
			//int okay = m_serial_comm->Property_Read(object_id, attrib_id, fields, 1);
			int okay = Property_Read(m_peer_addr,0,0,0,object_id, attrib_id, fields);
			if ( 0 == okay )
			{
				// 把它转成十六进制readable string.
				bins2hexs((const unsigned char*)&fields, sizeof(fields), (char*)m_big_buff2, sizeof(m_big_buff2));
				Response resp;
				resp._obj_id = object_id;
				resp._attrib_id = attrib_id;
				resp._resp_str = (char*)m_big_buff2;
				ResponseAttached& ra = m_responses[resp];
				ra._hit_times ++;
				
				m_properties_s[i] = fields;
			}
			else
			{
				record_me && RecordMe("按顺序读属性描述时失败.顺序ID号 %d \n", i);
			}
		}
	}

	for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
		  ite != m_responses.end(); 
		  ++ ite )
	{
		record_me && RecordMe("按顺序读属性描述，顺序号 %d 共收到回复 %s %d 次\n", ite->first._attrib_id,
			ite->first._resp_str.c_str(), ite->second._hit_times );
	}

	m_responses.clear();

_out:
	return retcode;	
}

int TheDeviceObj::CheckProperty_ri(const std::vector<std::pair<int, int>>& obj_att_range, int testing_count)
{
	int retcode = 0;
	m_responses.clear();


	if ( 0 == testing_count || obj_att_range.empty() )
	{
		return 0;
	}

	RecordMe("\n###### 开始按对象序号和属性ID号对读属性描述.\n");

	for ( std::vector<std::pair<int, int>>::const_iterator ite = obj_att_range.begin();
		  ite != obj_att_range.end();
		  ++ ite )
	{
		int object_id = ite->first;
		
		for ( int j = 1; j <= ite->second; j ++ )
		{
			int attrib_id = j;

			RecordMe("开始读对象ID %d 属性 ID %d 的属性.\n", object_id, attrib_id);

			for ( int i = 0; i < testing_count; i++ )
			{
				PropertyFields fields;
				//int okay = m_serial_comm->Property_Read(object_id, attrib_id, fields, 1);
				int okay = Property_Read(m_peer_addr,0,0,0,object_id, attrib_id, fields);
				if ( 0 != okay )
				{
					RecordMe("读对象ID %d 属性 ID %d 的属性失败.\n", object_id, attrib_id);
				}
				else
				{
					// 把它转成十六进制readable string.
					bins2hexs((const unsigned char*)&fields, sizeof(fields), (char*)m_big_buff2, sizeof(m_big_buff2));
					Response resp;
					resp._obj_id = object_id;
					resp._attrib_id = attrib_id;
					resp._resp_str = (char*)m_big_buff2;
					ResponseAttached& ra = m_responses[resp];
					ra._hit_times ++;
				}
			}
		}

		for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
			  ite != m_responses.end(); 
			  ++ ite )
		{
			RecordMe("读对象ID %d 属性ID %d 的属性，共收到回复 %s %d 次\n", 
					ite->first._obj_id,  ite->first._attrib_id,
					ite->first._resp_str.c_str(), ite->second._hit_times );
		}
	
		m_responses.clear();
	}

_out:
	return retcode;
}

int TheDeviceObj::CheckValue_rs(int total_record, int testing_count)
{
	int retcode = 0;
	int object_id = 0;

	if ( 0 == testing_count )
	{
		return 0;
	}

	if ( m_properties_s.empty() )
	{
		RecordMe("CheckValue_rs failed. Must call CheckProperty_rs first.\n");
		return -1;
	}

	RecordMe("\n###### 开始按顺序号读值.\n");
	m_responses.clear();

	for ( int i = 1; i <= total_record; i++ )
	{
		int attrib_id = i;

		std::map<int, PropertyFields>::const_iterator ite2prop = m_properties_s.find(i);
		if ( ite2prop == m_properties_s.end() )  // 没有找到对应这个对象ID属性ID. 
		{
			RecordMe("没有序号为 %d 的属性记录\n", i);
			continue;
		}

		PropertyFields fields = ite2prop->second;
		int elem_count =  fields._elem_count;   // 共多少个元素. 
		int elem_count_read_this_time = elem_count;
		int up_bound = 1;   // 每次读15个元素.如果RAM里多于15个元素，就要从序号2,3,4...开始读.
		if ( elem_count >= 16 )
		{
			up_bound = elem_count - 15;
			elem_count_read_this_time = 15;
		}
		else
		{
			elem_count_read_this_time = elem_count;
		}
		int indices_num = up_bound - 0;  // 开始序号共有多少个. 

		for ( int j = 0; j < testing_count; j ++ )
		{
			int start_pos = ( j % up_bound ) + 1;
			unsigned char count = elem_count_read_this_time;
			unsigned char count_save = count;
			int buff_len = sizeof(m_big_buff);
			//int okay = m_serial_comm->PropertyValue_Read(object_id, attrib_id, count, start_pos, m_big_buff, buff_len, 1);
			int okay = PropertyValue_Read(m_peer_addr,0,0,0,object_id, attrib_id, count, start_pos, m_big_buff, buff_len);
			if ( 0 == okay )  // 成功读到值. 
			{
				// 把它转成十六进制readable string.
				bins2hexs(m_big_buff, buff_len, (char*)m_big_buff2, sizeof(m_big_buff2));
				Response resp;
				resp._obj_id = object_id;
				resp._attrib_id = attrib_id;
				resp._start_pos = start_pos;
				resp._count = count_save;
				resp._resp_str = (char*)m_big_buff2;
				ResponseAttached& ra = m_responses[resp];
				ra._hit_times ++;
			}
			else
			{
				// 一次读失败. 
				RecordMe("按序号 %d 读值 起始位置 %d 元素个数 %d 失败.\n", i, start_pos, count_save);
			}
		} // 进行完所有的测试次数.

		for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
			  ite != m_responses.end(); 
			  ++ ite )
		{
				RecordMe("按序号 %d 读值 起始位置 %d 元素 %d 个. 共收到回复 %s %d 次\n", 
				ite->first._attrib_id, ite->first._start_pos, ite->first._count,
				ite->first._resp_str.c_str(), ite->second._hit_times );
		}
		m_responses.clear();
	} // end of the main for loop.

	RecordMe("\n\n\n");

	return 0;
}

int TheDeviceObj::CheckValue_ri(const std::vector<std::pair<int, int>>& obj_att_range, int testing_count)
{
	int retcode = 0;
	m_responses.clear();

	if ( 0 == testing_count )
	{
		return 0;
	}

	RecordMe("\n###### 开始按照对象序号和属性ID对测试读值.\n");

	for ( std::vector<std::pair<int, int>>::const_iterator ite = obj_att_range.begin();
		  ite != obj_att_range.end();
		  ++ ite )
	{
		int object_id = ite->first;
		
		for ( int j = 1; j <= ite->second; j ++ )
		{
			int attrib_id = j;

			RecordMe("开始读对应对象ID %d 属性 ID %d 的值.\n", object_id, attrib_id);

			std::pair<int, int> the_key(object_id, attrib_id);
			std::map<std::pair<int, int>, PropertyFields>::const_iterator ite2prop = m_properties.find(the_key);
			if ( ite2prop == m_properties.end() )  // 没有找到对应这个对象ID属性ID. 
			{
				RecordMe("没有对象ID %d 属性 ID %d 的记录\n", object_id, attrib_id);
				continue;
			}

			PropertyFields fields = ite2prop->second;
			int elem_count =  fields._elem_count;   // 共多少个元素. 
			int elem_count_read_this_time = elem_count;
			int up_bound = 1;   // 每次读15个元素.如果RAM里多于15个元素，就要从序号2,3,4...开始读.
			if ( elem_count >= 16 )
			{
				up_bound = elem_count - 15;
				elem_count_read_this_time = 15;
			}
			else
			{
				elem_count_read_this_time = elem_count;
			}
			int indices_num = up_bound - 0;  // 开始序号共有多少个. 

			for ( int i = 0; i < testing_count; i++ )
			{
				int start_pos = ( i % up_bound ) + 1;
				unsigned char count = elem_count_read_this_time;
				int buff_len = sizeof(m_big_buff);
				//int okay = m_serial_comm->PropertyValue_Read(object_id, attrib_id, count, start_pos, m_big_buff, buff_len, 1);
                int okay = PropertyValue_Read(m_peer_addr,0,0,0,object_id, attrib_id, count, start_pos, m_big_buff, buff_len);
				if ( 0 == okay )  // 成功读到值. 
				{
					// 把它转成十六进制readable string.
					bins2hexs(m_big_buff, buff_len, (char*)m_big_buff2, sizeof(m_big_buff2));
					Response resp;
					resp._obj_id = object_id;
					resp._attrib_id = attrib_id;
					resp._start_pos = start_pos;
					resp._count = count;
					resp._resp_str = (char*)m_big_buff2;
					ResponseAttached& ra = m_responses[resp];
					ra._hit_times ++;
				}
				else
				{
					// 一次读失败. 
					RecordMe("读对象ID %d 属性ID %d 起始位置 %d 元素个数 %d 失败.\n", object_id, attrib_id, start_pos, count);
				}
			} // 进行完所有的测试次数.

			for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
				ite != m_responses.end(); 
				++ ite )
			{

				RecordMe("读对应对象ID %d 属性ID %d 的值 起始位置 %d 元素 %d 个. 共收到回复 %s %d 次\n", 
					ite->first._obj_id,ite->first._attrib_id, ite->first._start_pos, ite->first._count,
					ite->first._resp_str.c_str(), ite->second._hit_times );
			}
		}

		m_responses.clear();
	}

	RecordMe("\n\n\n");

_out:
	return retcode;
}

int TheDeviceObj::CheckValue_w(const WritingTestData& wtd)
{
	int  retcode = 0;
	
	const std::string& ops = wtd._ops;
	unsigned char obj_id = wtd._obj_id;
	unsigned char att_id = wtd._att_id;
	unsigned short count = wtd._count;
	unsigned short start_pos = wtd._start_pos;
	std::string data_str = wtd._data_str;
	
	bool read_out_okay[8] = { 0 }; // 用来存第n次是否读成功. 
	bool write_in_okay[8] = { 0 }; // 用来存第n次是否写成功. 
	std::string read_out_str[8];  // 用来存读出来的数据.可读十六进制表示. 
	std::string write_in_str[8];  // 用来存写. 
	int  idx_of_readoutstr = 0;   // 两个下标量. 
	int  idx_of_writeinstr = 0;   // ...
	BOOL has_huge_numof_values = HasHugeNumOfValues(wtd._obj_id, wtd._att_id);

	// 执行所有的动作. 
	for ( int i = 0; i < ops.size(); i ++ )
	{
		int okay = 0;
		int buff_len = sizeof(m_big_buff);
		int buff_len2 = sizeof(m_big_buff2);
		
		// once a bug arised here. 'cos some of the IO functions is likely to change the 'count ' parameter passed to it.
		// so 'count' is reset here at the beginning of every loop.
		count = wtd._count;

		if ( 'r' == ops[i] || 'R' == ops[i] )  // 读动作. 
		{			
			// 需要用读方法2. 
			if ( has_huge_numof_values )
			{
				okay = PropertyValue_Read2_Ex(obj_id, att_id, count, start_pos, m_big_buff, buff_len);
			}
			else  // 用读方法1.
			{
				okay = PropertyValue_Read_Ex(obj_id, att_id, count, start_pos, m_big_buff, buff_len);
			}
			
			// 忽略okay返回值.读到多少算多少.
			if ( NULL != bins2hexs(m_big_buff, buff_len, (char*)m_big_buff2, buff_len2) && 0 != buff_len )
			{
				RecordMe(NORMAL_MSG_LEVEL"读对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u 实际读到 %u 个元素. 值为 %s\n", 
					obj_id, att_id, start_pos, wtd._count, count, m_big_buff2);
				read_out_okay[idx_of_readoutstr] = true;
				read_out_str[idx_of_readoutstr] = (char*)m_big_buff2;
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"读对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u 读取错误!\n", 
					obj_id, att_id, start_pos, wtd._count);
				read_out_okay[idx_of_readoutstr] = false;
			}
			idx_of_readoutstr ++;
		}
		else if ( 'w' == ops[i] ||  'W' == ops[i] )  // 写动作. 
		{
			// 把要写的数据存入 m_big_buff.
			// 要把可读十六进制形式做转化. 
			if ( data_str.size() >= 2 && '0' == data_str[0] && ( 'x' == data_str[1] || 'X' == data_str[1] ) )
			{
				hexs2bins(data_str.c_str() + 2, m_big_buff, &buff_len);
			}
			else // 不用做转化. 
			{
				memset(m_big_buff, 0, sizeof(m_big_buff));
				buff_len = data_str.size();
				memcpy(m_big_buff, data_str.c_str(), buff_len);
			}
		
			if ( has_huge_numof_values )
			{
				okay = PropertyValue_Write2_Ex(obj_id, att_id, count, start_pos, m_big_buff, buff_len);
			}
			else
			{
				okay = PropertyValue_Write_Ex(obj_id, att_id, count, start_pos, m_big_buff, buff_len);
			}
			
			bins2hexs(m_big_buff, buff_len, (char*)m_big_buff2, buff_len2);
			if ( 0 == okay )
			{
				RecordMe(NORMAL_MSG_LEVEL"写入成功.写对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u 写入的值为 %s\n", 
					obj_id, att_id, start_pos, count, m_big_buff2 );
				write_in_str[idx_of_writeinstr] = (char*)m_big_buff2;
				write_in_okay[idx_of_writeinstr] = true;
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"写入失败.写对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u 写入的值为 %s\n", 
					obj_id, att_id, start_pos, count, m_big_buff2 );
				write_in_okay[idx_of_writeinstr] = false;
			}
			
			idx_of_writeinstr ++;
		}
		else
		{
			MessageBoxA(NULL, "only r or w is accepted.", "Script Format Error", MB_OK);
		}
	} // end of for loop. // 执行所有的动作. 

	// 处理 "读写读" 操作的结果.
	if ( 0 == stricmp(ops.c_str(), "rwr") )
	{
		if ( read_out_okay[0] && read_out_okay[1] && write_in_okay[0] )  // 前面的动作都成功.
		{
			if ( read_out_str[0] == read_out_str[1] )
			{
				RecordMe(NORMAL_MSG_LEVEL"对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u。读写读操作。写入之前读出值和写入之后读出值一致，都是: %s\n", 
					obj_id, att_id, start_pos, count, read_out_str[0].c_str() );
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u。读写读操作。写入之前读出值和写入之后读出值不一致，写入前: %s 写入后: %s\n", 
					obj_id, att_id, start_pos, count, read_out_str[0].c_str(), read_out_str[1].c_str() );
			}
		}
	}
	else if ( 0 == stricmp(ops.c_str(), "wr") )  // 处理 "写读" 操作的结果.
	{
		if ( write_in_okay[0] && read_out_okay[0] )  // 前面的写读操作都成功.
		{
			if ( write_in_str[0] == read_out_str[0] )
			{
				RecordMe(NORMAL_MSG_LEVEL"对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u。先写后读操作。写入的值和读出的值一致。都是: %s\n", 
					obj_id, att_id, start_pos, count, read_out_str[0].c_str() );
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"对象ID %u, 属性ID %u 的元素值。起始位置 %u, 数量 %u。先写后读操作。写入的值和读出的值不一致，写入: %s 读出: %s\n", 
					obj_id, att_id, start_pos, count, write_in_str[0].c_str(), read_out_str[0].c_str() );
			}
		}
	}


	return retcode;
}

int TheDeviceObj::eAuthorize_Query(const unsigned char* key, unsigned char* level)
{
	int retcode = 0;
	retcode = Authorize_Query(m_peer_addr, 0,0,0, key, level);
	return retcode;
}

int TheDeviceObj::PropertyValue_Read_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned short& count, 
										unsigned short start, unsigned char* buff, int& buff_len)
{
	int retcode = 0;

	if ( count > 15 )
	{
		int total_count_read = 0;
		int total_buff_used = 0;
		int another_bufflen = buff_len;
		unsigned char another_count = 15;

		//retcode = m_serial_comm->PropertyValue_Read(obj_id, attrib_id, another_count, start, buff, another_bufflen);
		retcode = PropertyValue_Read(m_peer_addr,0,0,0,obj_id, attrib_id, another_count, start, buff, another_bufflen);
		if ( 0 == retcode )
		{
			total_count_read += another_count;
			total_buff_used += another_bufflen;

			int new_bufflen = buff_len - another_bufflen;  // 缓冲区变短了. 
			unsigned short new_count = count - another_count;   // 需要读的数量也变短了. 
			
			buff_len = total_buff_used;
			count = total_count_read;

			retcode = PropertyValue_Read_Ex(obj_id, attrib_id, new_count, start + another_count,
					buff + another_bufflen, new_bufflen);
			if ( 0 == retcode )
			{
				total_count_read += new_count;
				total_buff_used += new_bufflen;

				buff_len = total_buff_used;
				count = total_count_read;
			}
		}
		return retcode;
	}
	else
	{
		unsigned char another_count = count;
		//retcode = m_serial_comm->PropertyValue_Read(obj_id, attrib_id, another_count, start, buff, buff_len);
        retcode = PropertyValue_Read(m_peer_addr,0,0,0,obj_id, attrib_id, another_count, start, buff, buff_len);
		count = another_count;
		return retcode;
	}
}

int TheDeviceObj::PropertyValue_Write_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned char count,
										 unsigned short start, const unsigned char* buff, int buff_len)
{
	int retcode = 0;

	// 长度应该是元素数量的偶数倍. it doesn't make any sense if count equals zero.
	if ( 0 == count || 0 != buff_len % count )   
	{
		return -1;
	}

	int len_per_elem = buff_len / count;

	if ( count > 15 )
	{
		unsigned char another_count = 15;
		int another_bufflen = len_per_elem * another_count;
		//retcode = m_serial_comm->PropertyValue_Write(obj_id, attrib_id, another_count, start, buff, another_bufflen);
		retcode =PropertyValue_Write(m_peer_addr,0,0,0,obj_id, attrib_id, another_count, start, buff, another_bufflen);
		if ( 0 == retcode )
		{
			int new_bufflen = buff_len - another_bufflen;  // 缓冲区变短了. 
			int new_count = count - another_count;   // 需要读的数量也变短了. 
			retcode = PropertyValue_Write_Ex(obj_id, attrib_id, new_count, start + another_count,
				buff + another_bufflen, new_bufflen);
		}

		return retcode;
	}
	else
	{
		//retcode = m_serial_comm->PropertyValue_Write(obj_id, attrib_id, count, start, buff, buff_len);
		retcode =PropertyValue_Write(m_peer_addr,0,0,0,obj_id, attrib_id, count, start, buff, buff_len);

	}

	return retcode;
}

int TheDeviceObj::PropertyValue_Read2_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned short& count, 
										 unsigned short start, unsigned char* buff, int& buff_len)
{
	int retcode = 0;
	int buff_len_used = 0;
	count = 0;

	for ( int i = 0; i < count; i ++ )
	{
		int another_buff_len = buff_len - buff_len_used; 
		//retcode = m_serial_comm->PropertyValue_Read2(obj_id, attrib_id, start + i, buff + buff_len_used, another_buff_len);
         retcode = PropertyValue_Read2(m_peer_addr,0,0,0,obj_id, attrib_id, start + i, buff + buff_len_used, another_buff_len);
		if ( 0 == retcode )
		{
			buff_len_used += another_buff_len;
			buff_len = buff_len_used;
			count ++;
		}
		else
		{
			break;
		}
	}

	return retcode;
}

int TheDeviceObj::PropertyValue_Write2_Ex(unsigned char obj_id, unsigned char attrib_id, unsigned short count,
										  unsigned short start, const unsigned char* buff, int buff_len)
{
	int retcode = 0;
	int len_per_elem = buff_len / count;

	if ( 0 != buff_len % count )   // 长度应该是元素数量的偶数倍. 
	{
		return -1;
	}

	for ( int i = 0; i < count; i ++ )
	{
		//retcode = m_serial_comm->PropertyValue_Write2(obj_id, attrib_id, start + i, buff + len_per_elem * i, len_per_elem);
		retcode = PropertyValue_Write2(m_peer_addr, 0, 0, 0, obj_id, attrib_id, start + i, buff + len_per_elem * i, len_per_elem);
		if ( 0 != retcode )
		{
			break;
		}
	}

	return retcode;
}