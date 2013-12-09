/********************************************//**
* @file 
* ��lua�����е���lua����������lua����
* written by zhouxiao@kingsoft.com Dec 05, 2008
************************************************/

#pragma once

/** ����lua������lua����(δʵ�ִ���������õ����鷵��ֵ���߼�). 
 * @param L pointer to the lua state
 * @param mod module��
 * @param fun ������
 * @param param �Ǹ��ַ�������lua�����Ĳ�����d��ʾ˫������,i��ʾ����,s��ʾstring, b��ʾboolean
 * D��ʾ˫��������,I��ʾ��������,(�������ʵ��) ��"dsbI" ���ʾ��lua���������ĸ�����������Ϊ˫���ȣ��ַ��ͣ���������
 * @param retv ��lua�����ķ���ֵ���������Ĳ����� d: double i: int, b:bool(С����) s: std::wstring , I: int* D:double*
 * ��ɱ䳤�Ȳ�������Ϊd: double* i: int*, b:bool* s: std::wstring*,I: int* D:double*
 * @return  ���ú����ɹ�ʱ����0, ��0���ǳ�����.
 */
int luat_call(lua_State* L, LPCSTR mod, LPCSTR fun, LPCSTR param, LPCSTR retv, ...);

/** �Ƿ����ĳ������. 
 * @param L pointer to the lua state
 * @param mod mouldue name
 * @param varname variable name
 * @param exist [OUT]�����Ƿ���������������.
 * @return �ɹ�ʱ����0,��0�ǳ�����. 
 */
int luat_existvar(lua_State* L, LPCSTR mod, LPCSTR varname, BOOL& exist);

/** �õ�lua������ĳ��������ֵ. 
 * @param L pointer to the lua state
 * @param mod module��
 * @param varname ������
 * @param type ����������,d��ʾ˫������,i��ʾ����,s��ʾstring, b��ʾboolean.�ɱ䳤�Ȳ���Ϊ��Ӧ���͵�ָ��.
 * @return �ɹ��õ�������ֵ�򷵻�0,���򷵻س�����.
 */
#define THE_VARIABLE_IS_NIL 0xDF
int luat_getvar(lua_State* L, LPCWSTR mod, LPCWSTR varname, wchar_t type, ...);
int luat_getvar(lua_State* L, LPCSTR mod, LPCSTR varname, char type, ...);

/** �õ�Lua�����ж���Ľṹ��.
 * @param L pointer to the lua state.
 * @param mod ģ����. 
 * @param tbname ָ��modģ�����ĸ�������Ҫȡ�Ľṹ��. 
 * @param mem. ָ��һ���ڴ�.�������lua������varnameָ���ı���ת��c�ṹ����ڴ沼�ִ���memָ����ڴ�. 
 * @param mem_len. ��Ϊ�ǿ�.���ں�������ʱ����������ڴ��memʵ��ʹ�õĳ���. 
 * @return �ɹ�ʱ����0. ���򷵻س�����. 
 * @NOTICE ��������Ƚ�Σ�ա������ڴ����ҪС��ʹ��.
 */
int luat_getstruct(lua_State* L, LPCSTR mod, LPCSTR tbname, void* mem, int* mem_len);

/**
 * ͬǰ���luat_getstruct��ɵ�����һ���������ڵ������luat_getstructʱ��Ҫ������lua table�Ѿ�ѹ��luaջ����������.
 */
int luat_getstruct(lua_State* L, void* mem, int* mem_len);

/** �õ�Lua�����ж����һ������ṹ�壬�ýṹ��ĳ�Աȫ����std::string.
* @param L pointer to the lua state.
* @param mod ģ����. 
* @param tbname ָ��modģ�����ĸ�������Ҫȡ�Ľṹ��. 
* @param mem. ������һ���ڴ�.�������lua������varnameָ���ı���ת��c�ṹ����ڴ沼�ִ�������ڴ���. 
* @param mem_len �������ڴ��ĳ���. 
* @return �ɹ�ʱ����0. ���򷵻س�����. 
* @NOTICE Ҫע��luat_free�ͷŴ������ڴ��. ���ϸ�����һ���������Ҳ��Σ��.
*/
int luat_getstruct_filledwithstr(lua_State* L, LPCSTR mod, LPCSTR tbname, void** mem, int* mem_len);
void luat_freestruct_filledwithstr(void* mem);

/** �õ�Lua������ĳ��table�е�Ԫ�ظ���. 
 * @param L lua������ָ��. 
 * @param mod ģ����. 
 * @param tbname ĳ��table������. 
 * @param ʧ��ʱ���ظ���,�������Ǳ�tbname�е�Ԫ�ظ���. 
 */
int luat_gettablenum(lua_State* L, LPCSTR mod, LPCSTR tbname);

/** ��һ���ڴ���Ϊlua�ű����ı����ͺͶ��������Ͷ���,�����һ��lua�������������������
 * @param L pointer to the lua state.
 * @param mem һ�ΰ���lua�ű����ڴ�.
 * @param sz �ö��ڴ�Ĵ�С.
 * @param param �������������.��luat_call������param����.
 * @param retv ����ֵ����������.��luat_call��retv����.
 * @return �ɹ�ִ���򷵻�0,���򷵻س�����.
 */
int luat_run(lua_State* L, const char* mem, size_t sz, LPCSTR param, LPCSTR retv, ...);
