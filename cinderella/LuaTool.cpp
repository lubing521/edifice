/********************************************//**
*  从lua环境中调用lua函数，检索lua变量
*  written by zhouxiao@kingsoft.com Dec 05, 2008
************************************************/
#include "stdafx.h"
#include "LuaTool.h"

// the following 2 global variables r used to make output log info more redable.
const char* g_current_mod;
const char* g_current_fun;

// 辅助函数，调用lua环境中的函数，该lua函数已经压入lua的栈. lf 是 lua function 的缩写
static int luat_call_lf(lua_State* L, LPCSTR param, LPCSTR retv,  va_list argptr)
{
	int retcode = 0;
	int narg = ( param ? strlen(param) : 0 );   // 参数的个数
	int nret = ( retv ? strlen(retv) : 0 );   // 返回值的个数

	// 把所有参数压入lua stack
	luaL_checkstack(L, narg, "too many arguments.");
	for ( int i=0; i < narg; i++ )
	{
		switch( *(param+i) )
		{
		case L'd':
			{
				double d = va_arg(argptr, double);
				lua_pushnumber(L, d);
				break;
			}

		case L'i':
			{
				int n = va_arg(argptr, int);
				lua_pushinteger(L, n);
				break;
			}

		case L's':
			{
				std::wstring& str = va_arg(argptr, std::wstring);
				lua_pushstring(L, CW2A(str.c_str()));

			}
			break;

		case L'b':
			{
				bool b = va_arg(argptr, bool);
				lua_pushboolean(L, b);
			}

		case L'D':
		case L'I':
			{
				// lua_createtable(L, ,);  繁琐，暂时不需要这个功能就先不实现啦！
				retcode = 10;
				goto Exit0;
			}

		default:
			retcode = 7;
			goto Exit0;
		}
	}

	// 调用lua的函数失败
	if ( 0 != lua_pcall(L, narg, nret, 0) )
	{
		retcode = 4;
		LPCSTR errstr = lua_tostring(L, -1);
//		LOG.wprintf(L"### Mod: %s Fun: %s Info: %s\n", g_current_mod, g_current_fun, errstr);
		POPMSGBOX("Mod: %s Fun: %s Info: %s\n", g_current_mod, g_current_fun, errstr);
		goto Exit0;
	}

	// 取返回值
	LPCSTR _retv = retv;
	for ( int i = -nret; i < 0; i++ )
	{
		switch( *_retv++ )
		{
		case L'd':
			{
				double d = luaL_checknumber(L, i);
				*va_arg(argptr, double*) = d;
				break;
			}

		case L'i':
			{
				int n = luaL_checknumber(L, i);
				*va_arg(argptr, int*) = n;
				break;
			}

		case L's':
			{
				const char* s = luaL_checkstring(L, i);
				*va_arg(argptr, std::wstring*) = CA2W(s);
				break;
			}
			break;

		case L'b':
			{
				bool b = !!lua_toboolean(L, i);
				*va_arg(argptr, bool*) = b;
				break;
			}

		case L'D':
		case L'I':
			{
				// 麻烦，暂时不需要这个功能.未实现
				retcode = 10;
				break;
			}

		default:
			retcode = 7;
			break;
		}
	}

Exit0:
	return retcode;
}

int luat_call(lua_State* L, LPCSTR mod, LPCSTR fun, LPCSTR param, LPCSTR retv, ...)
{
	ATLASSERT( fun );  // 参数检查，这三个查数不能为NULL

//	LOG.wputfs(L"=== Begin to call lua function %s::%s", mod, fun);

	int retcode = 0;
	int stacktop = lua_gettop(L);
	
	g_current_mod = mod;  // for log to record error msg.
	g_current_fun = fun;
	
	if ( NULL == fun )
	{
		retcode = ERROR_INVALID_PARAMETER;
		goto Exit0;
	}

	try
	{	// 得到lua函数.
		if ( mod )   // mod不为NULL
		{
			lua_getfield(L, LUA_GLOBALSINDEX, mod);  // 把模块table压入栈顶
			lua_getfield(L, -1, fun);                // 把模块table中的函数压入栈顶
		}
		else  // 检索全局函数
		{
			lua_getfield(L, LUA_GLOBALSINDEX, fun);
		}

		va_list ap;       // points to each unnamed arg in turn
		int narg = ( param ? strlen(param) : 0 );   // 参数的个数
		int nret = ( retv ? strlen(retv) : 0 );     // 返回值的个数
		
		va_start(ap, retv);

		retcode = luat_call_lf(L, param, retv, ap);  // call lua functions		

		
		// 结束遍历...
		va_end(ap);

	}
	catch (std::string& errinfo)  // lua环境抛出的异常被截获
	{
//		LOG.wputfs(L"### MOD: %s FUN: %s Info: %s", mod, fun, errinfo.c_str());
		POPMSGBOX("MOD: %s FUN: %s Info: %s", mod, fun, errinfo.c_str());

		goto Exit0;
	}

Exit0:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前

	g_current_mod = NULL;  // for log info output
	g_current_fun = NULL;

//	LOG.wputfs(L"=== after call lua function %s::%s, luat_call returns:%d", mod, fun, retcode);

	return retcode;
}

int luat_existvar(lua_State* L, LPCSTR mod, LPCSTR varname, BOOL& exist)
{
	int retcode = 0;
	int stacktop = lua_gettop(L);

	ATLASSERT( varname );  // 参数检查
	if ( varname == NULL )
	{
		retcode = -7;
		goto Exit0;
	}

	exist = TRUE;
	try
	{
		// 把要检索的变量放入栈
		if ( mod )   // mod不为NULL
		{
			lua_getfield(L, LUA_GLOBALSINDEX, mod);  // 把模块table压入栈顶
			lua_getfield(L, -1, varname);
			if ( lua_isnil(L, -1) ) // 如果这个值是nil也算不存在这个变量
			{
				exist = FALSE;
			}
		}
		else  // 检索全局函数
		{
			lua_getfield(L, LUA_GLOBALSINDEX, varname);
		}
	}
	catch (...)
	{
		exist = FALSE;
	}

Exit0:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前

	return retcode;
}

int luat_getvar(lua_State* L, LPCWSTR mod, LPCWSTR varname, wchar_t type, ...)
{
	int retcode = 0;
	int stacktop = lua_gettop(L);

	ATLASSERT( varname && type );  // 参数检查
	if ( !(varname && type) )
	{
		retcode = 7;
		goto Exit0;
	}

	try
	{
		// 把要检索的变量放入栈
		if ( mod )   // mod不为NULL
		{
			lua_getfield(L, LUA_GLOBALSINDEX, CW2A(mod));  // 把模块table压入栈顶
			lua_getfield(L, -1, CW2A(varname));             
		}
		else  // 检索全局函数
		{
			lua_getfield(L, LUA_GLOBALSINDEX, CW2A(varname));
		}

		va_list ap;       // points to unnamed argument
		va_start(ap, type);

		// 把lua栈中的变量取出
		switch( type )
		{
		case L'd':
			{
				int d = luaL_checknumber(L, -1);
				*va_arg(ap, double*) = d;
				break;
			}

		case L'i':
			{
				int n = luaL_checknumber(L, -1);
				*va_arg(ap, int*) = n;
				break;
			}

		case L's':
			{
				const char* s = luaL_checkstring(L, -1);
				*va_arg(ap, std::wstring*) = CA2W(s);
				break;
			}

		case L'b':
			{
				bool  b = !!lua_toboolean(L, -1);
				*va_arg(ap, bool*) = b;
				break;
			}

		case L'D':
		case L'I':
			{
				// lua_createtable(L, ,);  繁琐，暂时不需要这个功能就先不实现啦！
				retcode = 10;
				goto Exit0;
			}

		default:
			retcode = 7;
			goto Exit0;
		}

		va_end(ap);
	}
	catch (std::wstring& errinfo)
	{
		retcode = -1;

		//LOG.wputfs(L"### MOD: %s VAR: %s Info: %s", mod, varname, errinfo.c_str());
		//POPMSGBOX(L"### MOD: %s VAR: %s Info: %s", mod, varname, errinfo.c_str());
		// continue...
	}

Exit0:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前

	return retcode;
}

int luat_getvar(lua_State* L, LPCSTR mod, LPCSTR varname, char type, ...)
{
	int retcode = 0;
	int stacktop = lua_gettop(L);

	ATLASSERT( varname && type );  // 参数检查
	if ( !(varname && type) )
	{
		retcode = 7;
		goto Exit0;
	}

	try
	{
		// 把要检索的变量放入栈
		if ( mod )   // mod不为NULL
		{
			lua_getfield(L, LUA_GLOBALSINDEX, mod);  // 把模块table压入栈顶
			lua_getfield(L, -1, varname);             
		}
		else  // 检索全局函数
		{
			lua_getfield(L, LUA_GLOBALSINDEX, varname);
		}

		va_list ap;       // points to unnamed argument.
		va_start(ap, type);

		// 把lua栈中的变量取出
		switch( type )
		{
		case L'd':
			{
				int d = luaL_checknumber(L, -1);
				*va_arg(ap, double*) = d;
				break;
			}

		case L'i':
			{
				int n = luaL_checknumber(L, -1);
				*va_arg(ap, int*) = n;
				break;
			}

		case L's':
			{
				const char* s = luaL_checkstring(L, -1);
				*va_arg(ap, std::string*) = s;
				break;
			}

		case L'b':
			{
				bool  b = !!lua_toboolean(L, -1);
				*va_arg(ap, bool*) = b;
				break;
			}

		case L'D':
		case L'I':
			{
				// lua_createtable(L, ,);  繁琐，暂时不需要这个功能就先不实现啦！
				retcode = 10;
				goto Exit0;
			}

		default:
			retcode = 7;
			goto Exit0;
		}

		va_end(ap);
	}
	catch (std::wstring& errinfo)
	{
		retcode = -1;

		//LOG.wputfs(L"### MOD: %s VAR: %s Info: %s", mod, varname, errinfo.c_str());
		//POPMSGBOX(L"### MOD: %s VAR: %s Info: %s", mod, varname, errinfo.c_str());
		// continue...
	}

Exit0:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前

	return retcode;
}

int luat_getstruct(lua_State* L, LPCSTR mod, LPCSTR tbname, void* mem, int* mem_len)
{
	int retcode = 0;
	int stacktop = lua_gettop(L);
	int count = luat_gettablenum(L, mod, tbname);
	unsigned char* ptr = (unsigned char*)mem;
	int mem_used_length = 0;

	if ( count < 0 )  // 得到table的元素数量出错. 
	{
		retcode = count;
		goto _out;
	}

	ATLASSERT( mod && tbname && mem );  // 参数检查
	if ( !(mod && tbname && mem) )
	{
		retcode = 7;
		goto _out;
	}

	try
	{
		for ( int i = 1; i <= count; i++ )
		{
			if ( NULL != mod )
			{
				lua_getfield(L, LUA_GLOBALSINDEX, mod);
				lua_getfield(L, -1, tbname);
				lua_remove(L, -2);
			}
			else
			{
				lua_getfield(L, LUA_GLOBALSINDEX, tbname);
			}
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			lua_remove(L, -2);

			int t = lua_type(L, -1);
			if ( LUA_TNUMBER == t )
			{
				*(int*)ptr = luaL_checknumber(L, -1);
				ptr += sizeof(int);
				mem_used_length += sizeof(int);
			}
			else if ( LUA_TSTRING ==t )
			{
				memset(ptr, 0, sizeof(std::string));
				
				*(std::string*)ptr = luaL_checkstring(L, -1);
				ptr += sizeof(std::string);
				mem_used_length += sizeof(std::string);
			}
			else
			{
				retcode = 5;
				goto _out;
			}
		}  // end of for loop
	}
	catch (std::wstring& errinfo)
	{
		retcode = -1;
		POPMSGBOX("### MOD: %s TABLENAME: %s Info: %s", mod, tbname, errinfo.c_str());
	}

	if ( NULL != mem_len )
	{
		*mem_len = mem_used_length;
	}

_out:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前

	return retcode;
}

int luat_getstruct(lua_State* L, void* mem, int* mem_len)
{
	int count = 0;
	int retcode = 0;
	int stacktop = lua_gettop(L);
	lua_getfield(L, LUA_GLOBALSINDEX, "table");
	lua_getfield(L, -1, "getn");
	lua_remove(L, -2);
	lua_pushvalue(L, -2);  // 复制一份之前的table在栈顶. 
	lua_call(L, 1, 1);
	count = luaL_checknumber(L, -1);  // 得到table里元素的个数.

	unsigned char* ptr = (unsigned char*)mem;
	int mem_used_length = 0;

	if ( count < 0 )  // 得到table的元素数量出错. 
	{
		retcode = count;
		goto _out;
	}

	try
	{
		for ( int i = 1; i <= count; i++ )
		{
			int save = lua_gettop(L);

			lua_pushvalue(L, -2);  // 再复制一份之前的table在栈顶. 
			lua_pushnumber(L, i);
			lua_gettable(L, -2);

			int t = lua_type(L, -1);
			if ( LUA_TNUMBER == t )
			{
				*(int*)ptr = luaL_checknumber(L, -1);
				ptr += sizeof(int);
				mem_used_length += sizeof(int);
			}
			else if ( LUA_TSTRING ==t )
			{
				memset(ptr, 0, sizeof(std::string));

				*(std::string*)ptr = luaL_checkstring(L, -1);
				ptr += sizeof(std::string);
				mem_used_length += sizeof(std::string);
			}
			else
			{
				retcode = 5;
				goto _out;
			}

			lua_settop(L, save);
		}  // end of for loop
	}
	catch (std::string& errinfo)
	{
		retcode = -1;
		POPMSGBOX("### luat_getstruct error: %s", errinfo.c_str());
	}

	if ( NULL != mem_len )
	{
		*mem_len = mem_used_length;
	}

_out:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前

	return retcode;
}

int luat_getstruct_filledwithstr(lua_State* L, LPCSTR mod, LPCSTR tbname, void** mem, int* mem_len)
{
	if ( NULL == mem || NULL == mem_len )
	{
		return -1;
	}

	unsigned char* block_of_mem = (unsigned char*)new std::string[128];
	int len_used = 0;
	int retcode = luat_getstruct(L, mod, tbname, block_of_mem, &len_used);
	if ( 0 == retcode )
	{
		*mem = block_of_mem;
		*mem_len = len_used;
	}
	else
	{
		*mem = NULL;
		*mem_len = 0;
	}

	return retcode;
}

void luat_freestruct_filledwithstr(void* mem)
{
	std::string* ptr = (std::string*)mem;
	delete []ptr;
}

int luat_gettablenum(lua_State* L, LPCSTR mod, LPCSTR tbname)
{
	int stacktop = lua_gettop(L);
	int count = 0;

	try
	{
		// 压入函数. 
		lua_getfield(L, LUA_GLOBALSINDEX, "table");
		lua_getfield(L, -1, "getn");
		lua_remove(L, -2);

		// 压入第一个且仅有的一个参数. 
		if ( mod == NULL )
		{
			lua_getfield(L, LUA_GLOBALSINDEX, tbname);
		}
		else
		{
			lua_getfield(L, LUA_GLOBALSINDEX, mod);
			lua_getfield(L, -1, tbname);
			lua_remove(L, -2);
		}
		lua_call(L, 1, 1);     /* call 'table.getn' with 1 arguments and 1 result */
		count = luaL_checknumber(L, -1);
	}
	catch (std::wstring& errinfo)
	{
		POPMSGBOX("### MOD: %s TABLENAME: %s Info: %s", mod, tbname, errinfo.c_str())
		count = -1;
	}

	lua_settop(L, stacktop);

	return count;
}

int luat_run(lua_State* L, const char* mem, size_t sz, LPCSTR param, LPCSTR retv, ...)
{
	int retcode = 0;
	int stacktop = lua_gettop(L);

	if ( 0 != luaL_loadbuffer(L, (const char*)mem, sz, NULL ) )	
	{
//		POPMSGBOX( L"MOD: %s, \r\nFun: %s \r\n%s\r\n", g_current_mod, g_current_fun, lua_tostring(L, -1) );
		retcode = -2;
		goto _out;
	}

	va_list ap;       // points to each unnamed arg in turn
	
	va_start(ap, retv);

	retcode = luat_call_lf(L, param, retv, ap);

	// 结束遍历...
	va_end(ap);

_out:
	lua_settop(L, stacktop);   // 恢复lua栈到Call函数调用前
	
	return retcode;
}