#ifndef _H_FX_VM_
#define _H_FX_VM_

#include "IEIBEmulator.h"

#ifdef __cplusplus
extern "C" {
#endif

	// according to the control macro, deciding whether export or import functions
#undef _EXPORT_OR_IMPORT_FXVM_
#ifdef _IM_FX_VM_DLL_
#define _EXPORT_OR_IMPORT_FXVM_ __declspec(dllexport)
#else
#define _EXPORT_OR_IMPORT_FXVM_ __declspec(dllimport)
#endif
	/// 载入代码文件.
	/// 成功时返回0,失败时返回非0. 
	/// IEIBEmulator指针和软设备ID号是用来对接到接口对象的数据.若emulator为NULL,则不对接到接口对象.  
	/// 若失败，可调用 fxvm_get_lasterr 得到一个描述出错信息的字符串. 
	_EXPORT_OR_IMPORT_FXVM_ int fxvm_load_file(const char* file_path, IEIBEmulator* emulator, unsigned int dev_id);

	/// 不停地调用这个函数，就会单步执行程序.成功执行完第一条语句就返回1,第二条就返回2.以此类推.全部执行完就会返回0.
	/// 返回0后再调用这个函数就又会从第一步执行程序. 
	_EXPORT_OR_IMPORT_FXVM_ int fxvm_run_step();

	/// 得到上次出错的出错信息的描述字符串. 
	_EXPORT_OR_IMPORT_FXVM_ const char* fxvm_get_lasterr();

	/// 得到软单元的值.传入的type类型可以是"X", "Y", "M", "S", "C", "T", "D". 但如果为NULL,则返回所有的SoftUnit的值.
	/// unit_data 指示得到的值将写入到这个地址. len传入时指示这个区域最多可填入多少个SoftUnitData结构体.
	/// 函数成功时 len传出的值将小于或等于len传入的值，表示实际填入了多少个SoftUnitData结构体.如果函数失败,则传出len
	/// 的值将是期望这个区域能够保存的数量的相反数. 
	/// 函数成功时返回0，失败时返回非0.
	struct SoftUnitData
	{
		char _id[8];   // 软单元的编号.  
		unsigned short _value;  // 软单元的值. 
	};
	_EXPORT_OR_IMPORT_FXVM_ int fxvm_read_softunits(const char* type, SoftUnitData* unit_data, int& len);

	// ################################################ testing functions #####################
	// Ignore the following testing functions. 
	_EXPORT_OR_IMPORT_FXVM_ void _fxvm_test(const char* str, const char* pattern);
	_EXPORT_OR_IMPORT_FXVM_ void _fxvm_parse(const char* filepath);
}

#endif