/********************************************//**
* @file 
* 从lua环境中调用lua函数，检索lua变量
* written by zhouxiao@kingsoft.com Dec 05, 2008
************************************************/

#pragma once

/** 调用lua环境的lua函数(未实现传入数组和拿到数组返回值的逻辑). 
 * @param L pointer to the lua state
 * @param mod module名
 * @param fun 函数名
 * @param param 是个字符串描述lua函数的参数，d表示双精度型,i表示整型,s表示string, b表示boolean
 * D表示双精度数组,I表示整型数组,(传数组待实现) 如"dsbI" 则表示该lua函数接受四个参数，依次为双精度，字符型，整型数组
 * @param retv 拿lua函数的返回值，如果传入的参数是 d: double i: int, b:bool(小布尔) s: std::wstring , I: int* D:double*
 * 则可变长度参数部分为d: double* i: int*, b:bool* s: std::wstring*,I: int* D:double*
 * @return  调用函数成功时返回0, 非0则是出错码.
 */
int luat_call(lua_State* L, LPCSTR mod, LPCSTR fun, LPCSTR param, LPCSTR retv, ...);

/** 是否存在某个变量. 
 * @param L pointer to the lua state
 * @param mod mouldue name
 * @param varname variable name
 * @param exist [OUT]传出是否存在这个环境变量.
 * @return 成功时返回0,非0是出错码. 
 */
int luat_existvar(lua_State* L, LPCSTR mod, LPCSTR varname, BOOL& exist);

/** 得到lua环境中某个变量的值. 
 * @param L pointer to the lua state
 * @param mod module名
 * @param varname 变量名
 * @param type 变量的类型,d表示双精度型,i表示整型,s表示string, b表示boolean.可变长度部分为相应类型的指针.
 * @return 成功得到变量的值则返回0,否则返回出错码.
 */
#define THE_VARIABLE_IS_NIL 0xDF
int luat_getvar(lua_State* L, LPCWSTR mod, LPCWSTR varname, wchar_t type, ...);
int luat_getvar(lua_State* L, LPCSTR mod, LPCSTR varname, char type, ...);

/** 得到Lua环境中定义的结构体.
 * @param L pointer to the lua state.
 * @param mod 模块名. 
 * @param tbname 指名mod模块中哪个变量是要取的结构体. 
 * @param mem. 指向一块内存.函数会把lua环境中varname指定的变量转成c结构体的内存布局存入mem指向的内存. 
 * @param mem_len. 若为非空.则在函数返回时在这里记下内存块mem实际使用的长度. 
 * @return 成功时返回0. 否则返回出错码. 
 * @NOTICE 这个函数比较危险。容易内存出错。要小心使用.
 */
int luat_getstruct(lua_State* L, LPCSTR mod, LPCSTR tbname, void* mem, int* mem_len);

/**
 * 同前面的luat_getstruct完成的任务一样。区别在调用这个luat_getstruct时，要检索的lua table已经压在lua栈的最上面了.
 */
int luat_getstruct(lua_State* L, void* mem, int* mem_len);

/** 得到Lua环境中定义的一个特殊结构体，该结构体的成员全部是std::string.
* @param L pointer to the lua state.
* @param mod 模块名. 
* @param tbname 指名mod模块中哪个变量是要取的结构体. 
* @param mem. 将传出一块内存.函数会把lua环境中varname指定的变量转成c结构体的内存布局存入这块内存中. 
* @param mem_len 传出的内存块的长度. 
* @return 成功时返回0. 否则返回出错码. 
* @NOTICE 要注意luat_free释放传出的内存块. 和上个函数一样这个函数也很危险.
*/
int luat_getstruct_filledwithstr(lua_State* L, LPCSTR mod, LPCSTR tbname, void** mem, int* mem_len);
void luat_freestruct_filledwithstr(void* mem);

/** 得到Lua环境中某个table中的元素个数. 
 * @param L lua环境的指针. 
 * @param mod 模块名. 
 * @param tbname 某个table的名字. 
 * @param 失败时返回负数,否则则是表tbname中的元素个数. 
 */
int luat_gettablenum(lua_State* L, LPCSTR mod, LPCSTR tbname);

/** 把一段内存视为lua脚本，文本类型和二进制类型都得,载入成一个lua函数并传入参数调用它
 * @param L pointer to the lua state.
 * @param mem 一段包含lua脚本的内存.
 * @param sz 该段内存的大小.
 * @param param 传入参数的类型.见luat_call函数的param参数.
 * @param retv 返回值的类型设置.见luat_call的retv参数.
 * @return 成功执行则返回0,否则返回出错码.
 */
int luat_run(lua_State* L, const char* mem, size_t sz, LPCSTR param, LPCSTR retv, ...);
