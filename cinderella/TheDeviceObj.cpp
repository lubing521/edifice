#include "StdAfx.h"
#include "TheDeviceObj.h"
#include "Pentagon.h"
#include "EIBCommNEO.h"

#define REPORT_FILE_NAME   "testing_report.txt"
#define MAGIC_TRING_TIMES  6   // �����дʧ��ʱ����ظ����ٴ�. 

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
	RecordMe("��ʼ������.�ȶ������ж����Լ�����Ӧ���������Ե�����.\n");
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
				if ( 0 == okay0 && 0 == okay1 ) // �����������������ɹ�. 
				{
					// �����ζ����������ݶ�һ��.
					if ( 0 == memcmp(&fields0, &fields1, sizeof(PropertyFields)) )
					{
						m_properties[index] = fields0;
					}
					else
					{
						RecordMe("��ʼ�������������ش���.��������ID %d ����ID %d ��������������"
							"���ûص�����������һ��.\n", obj_id, j);
						report_err_immed("��ʼ����������ɹ�.", 1);
					}
				}
				else if ( 0 == (okay0 & okay1) )  // ���ζ���������������һ�γɹ���. 
				{
					RecordMe("��ʼ�������������ش���.��������ID %d ����ID %d ��������������"
						"��ֻ�ɹ�һ��.\n", obj_id, j);
					report_err_immed("��ʼ����������ɹ�.", 1);
				}
				else
				{ 
					RecordMe("��������ID %d ����ID %d ���������ξ�ʧ��.���ܲ����ڸ�(����,����)��.Ҳ�����Ǹ�����.\n", obj_id, j);
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
			RecordMe(WARNING_MSG_LEVEL"��Կ %s ��ʽ������Կ�ĳ���Ϊ���ֽ�,��Ҫ�ð˸�ʮ�������ַ���ʾ.\n", key.c_str());
			return -1;
		}
		if ( 0 == eAuthorize_Query(m_big_buff, &level) )
		{
			RecordMe(NORMAL_MSG_LEVEL"��Կ %s ������,�õ�Ȩ�޵ȼ� %u\n", key.c_str(), level);
			return 0;
		}
		else
		{
			RecordMe(WARNING_MSG_LEVEL"��Կ %s ���ܾ�����������\n", key.c_str());
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
		g_devobj->RecordMe("######## ���ͽű�ָ����֡.\n");
	}

	const char* ptr = (const char*)arg;
	if ( *ptr == 't' )
		g_devobj->RecordMe(NORMAL_MSG_LEVEL"����֡ %s \n", buxx);
	else if ( *ptr == 'r' )
		g_devobj->RecordMe(NORMAL_MSG_LEVEL"�յ�֡ %s \n\n", buxx);
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
		RecordMe(WARNING_MSG_LEVEL"�ⲻ��һ���Ϸ���֡ %s \n", rawdata_h.c_str());
	}
	else if ( SEND_RAWDATA_NO_LEGAL_RESPONSE == retcode )
	{
		RecordMe(WARNING_MSG_LEVEL"û���յ��ظ����ߺϷ��Ļظ�. \n");
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

		RecordMe("��������ID: %d ����ID: %d ʱȴ���� ���� ID: %d ���� ID: %d ��ƥ��.\n", obj_id, attr_id, obj_id_rcved, attr_id_rcved);

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
	RecordMe("\n###### ��ʼRAM����\n");
	RecordMe("������RAM�ڴ�� %d ��\n", testing_count);

	for ( std::vector<std::pair<int, int>>::const_iterator ite = ram_obj_att.begin();
		  ite != ram_obj_att.end(); 
		  ++ ite )
	{
		int object_id = ite->first;
		int attrib_id = ite->second;

		RecordMe("��ʼ������ID %d ���� ID %d ��RAM.\n", object_id, attrib_id);

		std::map<std::pair<int, int>, PropertyFields>::const_iterator ite2prop = m_properties.find(*ite);
		if ( ite2prop == m_properties.end() )  // û���ҵ���Ӧ�������ID����ID. 
		{
			RecordMe("��ʼ��ʱδ�ܶ�������ID %d ���� ID %d �ļ�¼\n", object_id, attrib_id);
			continue;
		}

		/////////////////////////////////////////////////////////

		PropertyFields fields = ite2prop->second;

		int elem_count =  fields._elem_count;   // �����ٸ�Ԫ��.
		int elem_count_read_this_time = elem_count;
		int up_bound = 1;   // ÿ�ζ�15��Ԫ��.���RAM�����15��Ԫ�أ���Ҫ�����2,3,4...��ʼ��.
		if ( elem_count >= 16 )
		{
			up_bound = elem_count - 15;
			elem_count_read_this_time = 15;
		}
		else
		{
			elem_count_read_this_time = elem_count;
		}
		int indices_num = up_bound - 0;  // ��ʼ��Ź��ж��ٸ�. 
		for ( int i = 0; i < testing_count; i++ )
		{
			int start_pos = ( i % up_bound ) + 1;
			unsigned char count = elem_count_read_this_time;
			unsigned char count_save = count;
			int buff_len = sizeof(m_big_buff);
			//int okay = m_serial_comm->PropertyValue_Read(object_id, attrib_id, count, start_pos, m_big_buff, buff_len, 1);
			int okay =PropertyValue_Read(m_peer_addr,0,0,0,object_id, attrib_id, count, start_pos, m_big_buff, buff_len);
			if ( 0 == okay )  // �ɹ�����ֵ. 
			{
				// ����ת��ʮ������readable string.
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
				// һ�ζ�ʧ��. 
				RecordMe("������ID %d ����ID %d ��ʼλ�� %d Ԫ�ظ��� %d ʧ��.\n", object_id, attrib_id, start_pos, count_save);
			}
		} // ���������еĲ��Դ���.

		for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
			  ite != m_responses.end(); 
			  ++ ite )
		{
			RecordMe("������ID %d ����ID %d ��ֵ����ʼλ��Ϊ %d,Ԫ�� %d ��. ���յ��ظ� %s %d ��\n", 
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

	record_me && RecordMe("\n###### ��ʼ��˳�����������.\n");
	m_responses.clear();

	for ( int i = 1; i <= total_record; i++ )
	{
		int object_id = 0;
		int attrib_id = i;

		//RecordMe("��˳�������.˳��ID�� %d \n", i);

		for ( int j = 0; j < loop_times; j++ )
		{
			PropertyFields fields;
			//int okay = m_serial_comm->Property_Read(object_id, attrib_id, fields, 1);
			int okay = Property_Read(m_peer_addr,0,0,0,object_id, attrib_id, fields);
			if ( 0 == okay )
			{
				// ����ת��ʮ������readable string.
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
				record_me && RecordMe("��˳�����������ʱʧ��.˳��ID�� %d \n", i);
			}
		}
	}

	for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
		  ite != m_responses.end(); 
		  ++ ite )
	{
		record_me && RecordMe("��˳�������������˳��� %d ���յ��ظ� %s %d ��\n", ite->first._attrib_id,
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

	RecordMe("\n###### ��ʼ��������ź�����ID�ŶԶ���������.\n");

	for ( std::vector<std::pair<int, int>>::const_iterator ite = obj_att_range.begin();
		  ite != obj_att_range.end();
		  ++ ite )
	{
		int object_id = ite->first;
		
		for ( int j = 1; j <= ite->second; j ++ )
		{
			int attrib_id = j;

			RecordMe("��ʼ������ID %d ���� ID %d ������.\n", object_id, attrib_id);

			for ( int i = 0; i < testing_count; i++ )
			{
				PropertyFields fields;
				//int okay = m_serial_comm->Property_Read(object_id, attrib_id, fields, 1);
				int okay = Property_Read(m_peer_addr,0,0,0,object_id, attrib_id, fields);
				if ( 0 != okay )
				{
					RecordMe("������ID %d ���� ID %d ������ʧ��.\n", object_id, attrib_id);
				}
				else
				{
					// ����ת��ʮ������readable string.
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
			RecordMe("������ID %d ����ID %d �����ԣ����յ��ظ� %s %d ��\n", 
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

	RecordMe("\n###### ��ʼ��˳��Ŷ�ֵ.\n");
	m_responses.clear();

	for ( int i = 1; i <= total_record; i++ )
	{
		int attrib_id = i;

		std::map<int, PropertyFields>::const_iterator ite2prop = m_properties_s.find(i);
		if ( ite2prop == m_properties_s.end() )  // û���ҵ���Ӧ�������ID����ID. 
		{
			RecordMe("û�����Ϊ %d �����Լ�¼\n", i);
			continue;
		}

		PropertyFields fields = ite2prop->second;
		int elem_count =  fields._elem_count;   // �����ٸ�Ԫ��. 
		int elem_count_read_this_time = elem_count;
		int up_bound = 1;   // ÿ�ζ�15��Ԫ��.���RAM�����15��Ԫ�أ���Ҫ�����2,3,4...��ʼ��.
		if ( elem_count >= 16 )
		{
			up_bound = elem_count - 15;
			elem_count_read_this_time = 15;
		}
		else
		{
			elem_count_read_this_time = elem_count;
		}
		int indices_num = up_bound - 0;  // ��ʼ��Ź��ж��ٸ�. 

		for ( int j = 0; j < testing_count; j ++ )
		{
			int start_pos = ( j % up_bound ) + 1;
			unsigned char count = elem_count_read_this_time;
			unsigned char count_save = count;
			int buff_len = sizeof(m_big_buff);
			//int okay = m_serial_comm->PropertyValue_Read(object_id, attrib_id, count, start_pos, m_big_buff, buff_len, 1);
			int okay = PropertyValue_Read(m_peer_addr,0,0,0,object_id, attrib_id, count, start_pos, m_big_buff, buff_len);
			if ( 0 == okay )  // �ɹ�����ֵ. 
			{
				// ����ת��ʮ������readable string.
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
				// һ�ζ�ʧ��. 
				RecordMe("����� %d ��ֵ ��ʼλ�� %d Ԫ�ظ��� %d ʧ��.\n", i, start_pos, count_save);
			}
		} // ���������еĲ��Դ���.

		for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
			  ite != m_responses.end(); 
			  ++ ite )
		{
				RecordMe("����� %d ��ֵ ��ʼλ�� %d Ԫ�� %d ��. ���յ��ظ� %s %d ��\n", 
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

	RecordMe("\n###### ��ʼ���ն�����ź�����ID�Բ��Զ�ֵ.\n");

	for ( std::vector<std::pair<int, int>>::const_iterator ite = obj_att_range.begin();
		  ite != obj_att_range.end();
		  ++ ite )
	{
		int object_id = ite->first;
		
		for ( int j = 1; j <= ite->second; j ++ )
		{
			int attrib_id = j;

			RecordMe("��ʼ����Ӧ����ID %d ���� ID %d ��ֵ.\n", object_id, attrib_id);

			std::pair<int, int> the_key(object_id, attrib_id);
			std::map<std::pair<int, int>, PropertyFields>::const_iterator ite2prop = m_properties.find(the_key);
			if ( ite2prop == m_properties.end() )  // û���ҵ���Ӧ�������ID����ID. 
			{
				RecordMe("û�ж���ID %d ���� ID %d �ļ�¼\n", object_id, attrib_id);
				continue;
			}

			PropertyFields fields = ite2prop->second;
			int elem_count =  fields._elem_count;   // �����ٸ�Ԫ��. 
			int elem_count_read_this_time = elem_count;
			int up_bound = 1;   // ÿ�ζ�15��Ԫ��.���RAM�����15��Ԫ�أ���Ҫ�����2,3,4...��ʼ��.
			if ( elem_count >= 16 )
			{
				up_bound = elem_count - 15;
				elem_count_read_this_time = 15;
			}
			else
			{
				elem_count_read_this_time = elem_count;
			}
			int indices_num = up_bound - 0;  // ��ʼ��Ź��ж��ٸ�. 

			for ( int i = 0; i < testing_count; i++ )
			{
				int start_pos = ( i % up_bound ) + 1;
				unsigned char count = elem_count_read_this_time;
				int buff_len = sizeof(m_big_buff);
				//int okay = m_serial_comm->PropertyValue_Read(object_id, attrib_id, count, start_pos, m_big_buff, buff_len, 1);
                int okay = PropertyValue_Read(m_peer_addr,0,0,0,object_id, attrib_id, count, start_pos, m_big_buff, buff_len);
				if ( 0 == okay )  // �ɹ�����ֵ. 
				{
					// ����ת��ʮ������readable string.
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
					// һ�ζ�ʧ��. 
					RecordMe("������ID %d ����ID %d ��ʼλ�� %d Ԫ�ظ��� %d ʧ��.\n", object_id, attrib_id, start_pos, count);
				}
			} // ���������еĲ��Դ���.

			for ( std::map<Response, ResponseAttached>::iterator ite = m_responses.begin();
				ite != m_responses.end(); 
				++ ite )
			{

				RecordMe("����Ӧ����ID %d ����ID %d ��ֵ ��ʼλ�� %d Ԫ�� %d ��. ���յ��ظ� %s %d ��\n", 
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
	
	bool read_out_okay[8] = { 0 }; // �������n���Ƿ���ɹ�. 
	bool write_in_okay[8] = { 0 }; // �������n���Ƿ�д�ɹ�. 
	std::string read_out_str[8];  // �����������������.�ɶ�ʮ�����Ʊ�ʾ. 
	std::string write_in_str[8];  // ������д. 
	int  idx_of_readoutstr = 0;   // �����±���. 
	int  idx_of_writeinstr = 0;   // ...
	BOOL has_huge_numof_values = HasHugeNumOfValues(wtd._obj_id, wtd._att_id);

	// ִ�����еĶ���. 
	for ( int i = 0; i < ops.size(); i ++ )
	{
		int okay = 0;
		int buff_len = sizeof(m_big_buff);
		int buff_len2 = sizeof(m_big_buff2);
		
		// once a bug arised here. 'cos some of the IO functions is likely to change the 'count ' parameter passed to it.
		// so 'count' is reset here at the beginning of every loop.
		count = wtd._count;

		if ( 'r' == ops[i] || 'R' == ops[i] )  // ������. 
		{			
			// ��Ҫ�ö�����2. 
			if ( has_huge_numof_values )
			{
				okay = PropertyValue_Read2_Ex(obj_id, att_id, count, start_pos, m_big_buff, buff_len);
			}
			else  // �ö�����1.
			{
				okay = PropertyValue_Read_Ex(obj_id, att_id, count, start_pos, m_big_buff, buff_len);
			}
			
			// ����okay����ֵ.�������������.
			if ( NULL != bins2hexs(m_big_buff, buff_len, (char*)m_big_buff2, buff_len2) && 0 != buff_len )
			{
				RecordMe(NORMAL_MSG_LEVEL"������ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u ʵ�ʶ��� %u ��Ԫ��. ֵΪ %s\n", 
					obj_id, att_id, start_pos, wtd._count, count, m_big_buff2);
				read_out_okay[idx_of_readoutstr] = true;
				read_out_str[idx_of_readoutstr] = (char*)m_big_buff2;
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"������ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u ��ȡ����!\n", 
					obj_id, att_id, start_pos, wtd._count);
				read_out_okay[idx_of_readoutstr] = false;
			}
			idx_of_readoutstr ++;
		}
		else if ( 'w' == ops[i] ||  'W' == ops[i] )  // д����. 
		{
			// ��Ҫд�����ݴ��� m_big_buff.
			// Ҫ�ѿɶ�ʮ��������ʽ��ת��. 
			if ( data_str.size() >= 2 && '0' == data_str[0] && ( 'x' == data_str[1] || 'X' == data_str[1] ) )
			{
				hexs2bins(data_str.c_str() + 2, m_big_buff, &buff_len);
			}
			else // ������ת��. 
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
				RecordMe(NORMAL_MSG_LEVEL"д��ɹ�.д����ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u д���ֵΪ %s\n", 
					obj_id, att_id, start_pos, count, m_big_buff2 );
				write_in_str[idx_of_writeinstr] = (char*)m_big_buff2;
				write_in_okay[idx_of_writeinstr] = true;
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"д��ʧ��.д����ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u д���ֵΪ %s\n", 
					obj_id, att_id, start_pos, count, m_big_buff2 );
				write_in_okay[idx_of_writeinstr] = false;
			}
			
			idx_of_writeinstr ++;
		}
		else
		{
			MessageBoxA(NULL, "only r or w is accepted.", "Script Format Error", MB_OK);
		}
	} // end of for loop. // ִ�����еĶ���. 

	// ���� "��д��" �����Ľ��.
	if ( 0 == stricmp(ops.c_str(), "rwr") )
	{
		if ( read_out_okay[0] && read_out_okay[1] && write_in_okay[0] )  // ǰ��Ķ������ɹ�.
		{
			if ( read_out_str[0] == read_out_str[1] )
			{
				RecordMe(NORMAL_MSG_LEVEL"����ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u����д��������д��֮ǰ����ֵ��д��֮�����ֵһ�£�����: %s\n", 
					obj_id, att_id, start_pos, count, read_out_str[0].c_str() );
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"����ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u����д��������д��֮ǰ����ֵ��д��֮�����ֵ��һ�£�д��ǰ: %s д���: %s\n", 
					obj_id, att_id, start_pos, count, read_out_str[0].c_str(), read_out_str[1].c_str() );
			}
		}
	}
	else if ( 0 == stricmp(ops.c_str(), "wr") )  // ���� "д��" �����Ľ��.
	{
		if ( write_in_okay[0] && read_out_okay[0] )  // ǰ���д���������ɹ�.
		{
			if ( write_in_str[0] == read_out_str[0] )
			{
				RecordMe(NORMAL_MSG_LEVEL"����ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u����д���������д���ֵ�Ͷ�����ֵһ�¡�����: %s\n", 
					obj_id, att_id, start_pos, count, read_out_str[0].c_str() );
			}
			else
			{
				RecordMe(ERROR_MSG_LEVEL"����ID %u, ����ID %u ��Ԫ��ֵ����ʼλ�� %u, ���� %u����д���������д���ֵ�Ͷ�����ֵ��һ�£�д��: %s ����: %s\n", 
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

			int new_bufflen = buff_len - another_bufflen;  // �����������. 
			unsigned short new_count = count - another_count;   // ��Ҫ��������Ҳ�����. 
			
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

	// ����Ӧ����Ԫ��������ż����. it doesn't make any sense if count equals zero.
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
			int new_bufflen = buff_len - another_bufflen;  // �����������. 
			int new_count = count - another_count;   // ��Ҫ��������Ҳ�����. 
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

	if ( 0 != buff_len % count )   // ����Ӧ����Ԫ��������ż����. 
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