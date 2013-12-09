#include "stdafx.h"
#include "SerialPort4Lua.h"
#include "LuaTool.h"
#include "Pentagon.h"
#include "TheDeviceObj.h"
#include "EIBCommNEO.h"
#include <memory>

int test(void* arg);

extern std::auto_ptr<TheDeviceObj> g_devobj;

// COM�˿ں�. 
int g_com_port;
// ������. 
int g_baud_rate;
// ������ַ. 
std::string g_host;
// Ŀ�����ַ. 
std::string g_peer;
// ʹ������. 
std::string g_key;
// ��ԭʼ֡.
std::vector<std::string> g_rawdatas;
// ��ʼ����.ָ���˹��ж��ٸ�����,ÿ��������������ID��. 
std::vector<std::pair<int, int>> g_obj_prop_range;
// �Ƿ�򿪶�����. 
int g_reading_test;
// �Ƿ��д����. 
int g_writing_test;
// 485���Դ���. 
int g_ram_count;
//��˳�����������ʱ�����Զ��ٴ�. 
int g_prop_ps_count;
// ����������ID�Զ���������ʱ�����Զ��ٴ�. 
int g_prop_pi_count;
// ��˳���ֵʱ�����Զ��ٴ�. 
int g_prop_vs_count;
// ����������ID�Զ�ֵʱ�����Զ��ٴ�. 
int g_prop_vi_count;
// ���˳���д����������������,��˳���дʱ���ж�������¼. 
int g_prop_s_max_index;
// ��дRAM��Ӧ�Ķ���ID������ID���. 
std::vector<std::pair<int, int>> g_ram_obj_att;
//��д����ʱ�����Զ��ٴ�. 
// int g_prop_count;
//˳���дʱ���ж�������¼. 
//int g_prop_total_record;
// ...UNDO
std::vector<std::pair<int, int>> g_prop_obj_att_range;
// �����ִ�����Դ���. 
int g_frame_control_code_wrong_count;
// Ŀ���ַ������Դ���. 
int g_frame_dest_addr_wrong_count;
// ���ȴ�����Դ���. 
int g_frame_length_wrong_count;
// ���������Դ���. 
int g_frame_command_wrong_count;
// ID������Դ���. 
int g_frame_id_wrong_count;
// ��ʼλ�ô�����Դ���. 
int g_frame_start_pos_wrong_count;
// �±���������Դ���. 
int g_frame_suffix_pos_wrong_count;

int run_script(const char* path, lua_State* L)
{
	int retcode = 0;

	//const char * dir = working_dir();
	//std::string _dir = dir;
	//_dir += path;

	const HANDLE hFile = CreateFileA(path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		retcode = GetLastError();
		goto _out;
	}

	DWORD dwLength = GetFileSize(hFile, NULL);
	const HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, dwLength, NULL);
	char * pFile = (char *)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

	retcode = luat_run(L, pFile, dwLength, NULL, NULL);

	UnmapViewOfFile(pFile);
	CloseHandle(hFileMap);

_out:
	return retcode;
}

// �����ýű�.
// ����ĵ�һ�����������ýű���·��. 
static int run(lua_State* L)
{
	const char* config_file = luaL_checkstring(L, 1);

	int retcode = run_script(config_file, L);

	lua_pushnumber(L, retcode);

	return 1;
}

// ��һ��������һ���ַ���.
// �ڶ���������һ��lua table.����ջ��. 
static int check_struct_attop(lua_State* L)
{
	const char* t = luaL_checkstring(L, 1);
	WritingTestData	wtd;
	int len = sizeof(wtd);
	int retcode = luat_getstruct(L, &wtd, &len);
	if ( 0 == retcode )
	{
		if ( NULL != g_devobj.get() )
		{
			g_devobj->CheckValue_w(wtd);
		}
		else
		{
			POPMSGBOX("The global object TheDeviceObj has not yet been instantiated.");
		}
	}
	else
	{
		POPMSGBOX("check_struct_attop has detected illegal data.");
	}

//	static unsigned int num;
//	printf("%u:  (%d,%d) %s %s\n", ++num, td._obj_id, td._att_id, td._ops.c_str(), td._str.c_str());

	return 0;
}

// ת�ɿɶ�ʮ��������ʽ. 
// ����һ���ַ�������һ���ַ���. 
static int bins2hexs(lua_State* L)
{
	char buff[512] = { 0 };
	int buff_len = sizeof(buff);
	const char* str = luaL_checkstring(L, 1);

	bins2hexs((const unsigned char*)str, strlen(str), buff, buff_len);

	lua_pushstring(L, buff);

	return 1;
}

// �ɶ�ʮ��������ʽת��bins......
static int hexs2bins(lua_State* L)
{
	const char* hexs = luaL_checkstring(L, 1);
	int hexs_len = strlen(hexs);
	int buff_len = hexs_len / 2 + 2;
	unsigned char* buff = new unsigned char[buff_len];
	memset(buff, 0, buff_len);
	hexs2bins(hexs, buff, &buff_len);
	lua_pushlstring(L, (const char*)buff, buff_len);
	delete []buff;

	return 1;
}

// lua���������ú���.
static int call_test_from_lua(lua_State* L)
{
	const char* str = luaL_checkstring(L, 1);
	int retcode = test((void*)str);
	lua_pushnumber(L, retcode);
	
	return 1;
}

///////////////////////////////////////////////////////////////////
/// ����C������Lua����ʹ��. 
static const struct luaL_Reg serial_port_testing_funs[] = {
	{"test", call_test_from_lua},
	{"run", run},
	{"chsat", check_struct_attop},
	{"bins2hexs", bins2hexs},
	{"hexs2bins", hexs2bins},
	{NULL, NULL}
};

// ����������滻lua����Ĭ�ϵĳ�������
static int lua_panic_handler(lua_State* L)throw(std::string)
{
	const char* errinfo = lua_tostring(L, -1);

	char buff[MAX_PATH] = { 0 };
	_snprintf(buff, MAX_PATH-1, "Errinfo: %s", errinfo);

	throw std::string(buff);  // raise a exception
  
	return 0;
}

// ע�ắ����Lua����ʹ��. 
void register_funs(lua_State* L)
{
	luaL_register(L, "t", serial_port_testing_funs);

	lua_atpanic(L, lua_panic_handler);  // ���ò���lua�������س���ʱ�ĺ���
}


// �����ýű���Ĳ�������. 
int init_config_params(lua_State* L)
{
#define CHECK_RETCODE(x) if(0!=x) goto _out

	int   retcode = 0;
	int   mem_len = 0;
	unsigned char* mem = NULL;

	retcode = luat_getvar(L, "TEST", "com_port", 'i', &g_com_port);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "baud_rate", 'i', &g_baud_rate);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "host", 's', &g_host);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "peer", 's', &g_peer);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "key", 's', &g_key);

	if ( THE_VARIABLE_IS_NIL == retcode ) //  key û�б�����.
	{
		g_key = "";
	}
	else
	{
		CHECK_RETCODE(retcode);
	}

	retcode = luat_getvar(L, "TEST", "reading_test", 'i', &g_reading_test);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "writing_test", 'i', &g_writing_test);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "ram_count", 'i', &g_ram_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getstruct_filledwithstr(L, "TEST", "ram_obj_att", (void**)&mem, &mem_len);
	CHECK_RETCODE(retcode);

	retcode = luat_getstruct_filledwithstr(L, "TEST", "raw_frames", (void**)&mem, &mem_len);
	for ( int i = 0; i < mem_len/sizeof(std::string); i++ )
	{
		g_rawdatas.push_back(*(std::string*)(mem + i * sizeof(std::string)));
	}
	luat_freestruct_filledwithstr(mem);

	retcode = luat_getstruct_filledwithstr(L, "TEST", "obj_prop_range", (void**)&mem, &mem_len);
	CHECK_RETCODE(retcode);
	for ( int i = 0; i < mem_len/sizeof(std::string); i++ )
	{
		char* context = NULL;
		char buff[256] = { 0 };
		std::string s = *(std::string*)(mem + i * sizeof(std::string));
		strcpy(buff, s.c_str());
		std::pair<int, int> the_pair;
		the_pair.first = atoi( strtok_s(buff, ":", &context) );
		the_pair.second = atoi(strtok_s(NULL, ":", &context));
		g_obj_prop_range.push_back(the_pair);
	}
	luat_freestruct_filledwithstr(mem);

	retcode = luat_getstruct_filledwithstr(L, "TEST", "ram_obj_att", (void**)&mem, &mem_len);
	CHECK_RETCODE(retcode);
	for ( int i = 0; i < mem_len/sizeof(std::string); i++)
	{
		char* context = NULL;
		char buff[256] = { 0 };
		std::string s = *(std::string*)(mem + i * sizeof(std::string));
		strcpy(buff, s.c_str());
		std::pair<int, int> the_pair;
		the_pair.first = atoi( strtok_s(buff, ",", &context) );
		the_pair.second = atoi(strtok_s(NULL, ",", &context));
		g_ram_obj_att.push_back(the_pair);
	}
	luat_freestruct_filledwithstr(mem);

	//retcode = luat_getvar(L, "TEST", "prop_count", 'i', &g_prop_count);

	//retcode = luat_getvar(L, "TEST", "prop_total_record", 'i', &g_prop_total_record);
	//CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "prop_ps_count", 'i', &g_prop_ps_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "prop_pi_count", 'i', &g_prop_pi_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "prop_vs_count", 'i', &g_prop_vs_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "prop_vi_count", 'i', &g_prop_vi_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "prop_s_max_index", 'i', &g_prop_s_max_index);
	CHECK_RETCODE(retcode);

	retcode = luat_getstruct_filledwithstr(L, "TEST", "prop_obj_att_range", (void**)&mem, &mem_len);
	CHECK_RETCODE(retcode);

	for ( int i = 0; i < mem_len/sizeof(std::string); i++)
	{
		char* context = NULL;
		char buff[256] = { 0 };
		std::string s = *(std::string*)(mem + i * sizeof(std::string));
		strcpy(buff, s.c_str());
		std::pair<int, int> the_pair;
		the_pair.first = atoi( strtok_s(buff, ":", &context) );
		the_pair.second = atoi(strtok_s(NULL, ":", &context));
		g_prop_obj_att_range.push_back(the_pair);
	}
	luat_freestruct_filledwithstr(mem);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_control_code_wrong_count", 'i', &g_frame_control_code_wrong_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_dest_addr_wrong_count", 'i', &g_frame_dest_addr_wrong_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_length_wrong_count", 'i', &g_frame_length_wrong_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_command_wrong_count", 'i', &g_frame_command_wrong_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_id_wrong_count", 'i', &g_frame_id_wrong_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_start_pos_wrong_count", 'i', &g_frame_start_pos_wrong_count);
	CHECK_RETCODE(retcode);

	retcode = luat_getvar(L, "TEST", "frame_suffix_pos_wrong_count", 'i', &g_frame_suffix_pos_wrong_count);
	CHECK_RETCODE(retcode);

_out:
	return retcode;
}

void CheckReasonablity()
{
	if ( g_prop_s_max_index <= 0 )
	{
		report_err_immed("prop_total_record �������0.", 0);
	}
}

// DLL�������������.���Ե���ں���.  
int test(void* arg)
{
	UCHAR buff[4096] = { 0 };
	lua_State* L = luaL_newstate();  // start lua virtual machine. 
	if ( NULL == L )
	{
		return -1;
	}
	luaL_openlibs(L);   // Opens all standard Lua libraries into the given state.  

	// ���������ļ�. 
	int retcode = run_script((const char*)arg, L);
	if ( 0 != retcode )
	{
		printf("���нű� %s ����.", (const char*)arg);
		goto _out;
	}

	// ���������ļ�. 
	retcode = init_config_params(L);
	if ( 0 != retcode )
	{
		printf("�������ļ� %s ����.", (const char*)arg);
		goto _out;
	}

	// �������ĺ�����.����в�����������������������ֱ����ֹ���� 
	CheckReasonablity();
	// ���������ö���. 

	g_devobj.reset(new TheDeviceObj(g_host.c_str(), g_peer.c_str(), g_com_port, g_baud_rate, g_prop_obj_att_range));

	g_devobj->Sendkey(g_key.c_str());


	for ( int i = 0; i < g_rawdatas.size(); i++ )
	{
		g_devobj->eSend_Rawdate(g_rawdatas[i]);
	}

	// ������. 
	if ( 0 != g_reading_test )
	{
		retcode = g_devobj->ReadDevice();
		CHECK_RETCODE(retcode);
		retcode = g_devobj->CheckPropIDMatching();
		CHECK_RETCODE(retcode);
		retcode = g_devobj->CheckRAM_r(g_ram_obj_att, g_ram_count);  // p ��ʾ����property,v��ʾֵ,s��ʾ˳��,i��ʾ��id��.
		retcode = g_devobj->CheckProperty_rs(g_prop_s_max_index , g_prop_ps_count);
		retcode = g_devobj->CheckProperty_ri(g_prop_obj_att_range,g_prop_pi_count );
		retcode = g_devobj->CheckValue_rs(g_prop_s_max_index, g_prop_vs_count);
		retcode = g_devobj->CheckValue_ri(g_prop_obj_att_range, g_prop_vi_count);
	}
	
	// д����. 
	if ( 0 != g_writing_test )
	{
		luat_call(L, "TEST", "start2test", NULL, NULL);
	}

_out:	
	return retcode;
}