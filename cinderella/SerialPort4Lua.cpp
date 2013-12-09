#include "stdafx.h"
#include "SerialPort4Lua.h"
#include "LuaTool.h"
#include "Pentagon.h"
#include "TheDeviceObj.h"
#include "EIBCommNEO.h"
#include <memory>

int test(void* arg);

extern std::auto_ptr<TheDeviceObj> g_devobj;

// COM端口号. 
int g_com_port;
// 波特率. 
int g_baud_rate;
// 本机地址. 
std::string g_host;
// 目标机地址. 
std::string g_peer;
// 使用密码. 
std::string g_key;
// 发原始帧.
std::vector<std::string> g_rawdatas;
// 初始化用.指明了共有多少个对象,每个对象的最大属性ID号. 
std::vector<std::pair<int, int>> g_obj_prop_range;
// 是否打开读测试. 
int g_reading_test;
// 是否打开写测试. 
int g_writing_test;
// 485测试次数. 
int g_ram_count;
//按顺序读属性描述时共测试多少次. 
int g_prop_ps_count;
// 按对象属性ID对读属性描述时共测试多少次. 
int g_prop_pi_count;
// 按顺序读值时共测试多少次. 
int g_prop_vs_count;
// 按对象属性ID对读值时共测试多少次. 
int g_prop_vi_count;
// 最大顺序读写属性描述的索引号,即顺序读写时共有多少条记录. 
int g_prop_s_max_index;
// 读写RAM对应的对象ID和属性ID组合. 
std::vector<std::pair<int, int>> g_ram_obj_att;
//读写属性时共测试多少次. 
// int g_prop_count;
//顺序读写时共有多少条记录. 
//int g_prop_total_record;
// ...UNDO
std::vector<std::pair<int, int>> g_prop_obj_att_range;
// 控制字错误测试次数. 
int g_frame_control_code_wrong_count;
// 目标地址错误测试次数. 
int g_frame_dest_addr_wrong_count;
// 长度错误测试次数. 
int g_frame_length_wrong_count;
// 命令错误测试次数. 
int g_frame_command_wrong_count;
// ID错误测试次数. 
int g_frame_id_wrong_count;
// 起始位置错误测试次数. 
int g_frame_start_pos_wrong_count;
// 下标量错误测试次数. 
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

// 跑配置脚本.
// 传入的第一个参数是配置脚本的路径. 
static int run(lua_State* L)
{
	const char* config_file = luaL_checkstring(L, 1);

	int retcode = run_script(config_file, L);

	lua_pushnumber(L, retcode);

	return 1;
}

// 第一个参数是一个字符串.
// 第二个参数是一个lua table.它在栈顶. 
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

// 转成可读十六进制形式. 
// 传入一个字符串返回一个字符串. 
static int bins2hexs(lua_State* L)
{
	char buff[512] = { 0 };
	int buff_len = sizeof(buff);
	const char* str = luaL_checkstring(L, 1);

	bins2hexs((const unsigned char*)str, strlen(str), buff, buff_len);

	lua_pushstring(L, buff);

	return 1;
}

// 可读十六进制形式转成bins......
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

// lua环境测试用函数.
static int call_test_from_lua(lua_State* L)
{
	const char* str = luaL_checkstring(L, 1);
	int retcode = test((void*)str);
	lua_pushnumber(L, retcode);
	
	return 1;
}

///////////////////////////////////////////////////////////////////
/// 导出C函数给Lua环境使用. 
static const struct luaL_Reg serial_port_testing_funs[] = {
	{"test", call_test_from_lua},
	{"run", run},
	{"chsat", check_struct_attop},
	{"bins2hexs", bins2hexs},
	{"hexs2bins", hexs2bins},
	{NULL, NULL}
};

// 用这个函数替换lua环境默认的出错处理函数
static int lua_panic_handler(lua_State* L)throw(std::string)
{
	const char* errinfo = lua_tostring(L, -1);

	char buff[MAX_PATH] = { 0 };
	_snprintf(buff, MAX_PATH-1, "Errinfo: %s", errinfo);

	throw std::string(buff);  // raise a exception
  
	return 0;
}

// 注册函数给Lua环境使用. 
void register_funs(lua_State* L)
{
	luaL_register(L, "t", serial_port_testing_funs);

	lua_atpanic(L, lua_panic_handler);  // 设置捕获lua环境环重出错时的函数
}


// 把配置脚本里的参数读入. 
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

	if ( THE_VARIABLE_IS_NIL == retcode ) //  key 没有被定义.
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
		report_err_immed("prop_total_record 必须大于0.", 0);
	}
}

// DLL导出了这个函数.测试的入口函数.  
int test(void* arg)
{
	UCHAR buff[4096] = { 0 };
	lua_State* L = luaL_newstate();  // start lua virtual machine. 
	if ( NULL == L )
	{
		return -1;
	}
	luaL_openlibs(L);   // Opens all standard Lua libraries into the given state.  

	// 运行配置文件. 
	int retcode = run_script((const char*)arg, L);
	if ( 0 != retcode )
	{
		printf("运行脚本 %s 出错.", (const char*)arg);
		goto _out;
	}

	// 读入配置文件. 
	retcode = init_config_params(L);
	if ( 0 != retcode )
	{
		printf("读配置文件 %s 出错.", (const char*)arg);
		goto _out;
	}

	// 检查参数的合理性.如果有参数不合理则会在这个函数里直接中止程序。 
	CheckReasonablity();
	// 构建测试用对象. 

	g_devobj.reset(new TheDeviceObj(g_host.c_str(), g_peer.c_str(), g_com_port, g_baud_rate, g_prop_obj_att_range));

	g_devobj->Sendkey(g_key.c_str());


	for ( int i = 0; i < g_rawdatas.size(); i++ )
	{
		g_devobj->eSend_Rawdate(g_rawdatas[i]);
	}

	// 读测试. 
	if ( 0 != g_reading_test )
	{
		retcode = g_devobj->ReadDevice();
		CHECK_RETCODE(retcode);
		retcode = g_devobj->CheckPropIDMatching();
		CHECK_RETCODE(retcode);
		retcode = g_devobj->CheckRAM_r(g_ram_obj_att, g_ram_count);  // p 表示属性property,v表示值,s表示顺序,i表示按id对.
		retcode = g_devobj->CheckProperty_rs(g_prop_s_max_index , g_prop_ps_count);
		retcode = g_devobj->CheckProperty_ri(g_prop_obj_att_range,g_prop_pi_count );
		retcode = g_devobj->CheckValue_rs(g_prop_s_max_index, g_prop_vs_count);
		retcode = g_devobj->CheckValue_ri(g_prop_obj_att_range, g_prop_vi_count);
	}
	
	// 写测试. 
	if ( 0 != g_writing_test )
	{
		luat_call(L, "TEST", "start2test", NULL, NULL);
	}

_out:	
	return retcode;
}