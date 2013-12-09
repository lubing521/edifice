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
	/// ��������ļ�.
	/// �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	/// IEIBEmulatorָ������豸ID���������Խӵ��ӿڶ��������.��emulatorΪNULL,�򲻶Խӵ��ӿڶ���.  
	/// ��ʧ�ܣ��ɵ��� fxvm_get_lasterr �õ�һ������������Ϣ���ַ���. 
	_EXPORT_OR_IMPORT_FXVM_ int fxvm_load_file(const char* file_path, IEIBEmulator* emulator, unsigned int dev_id);

	/// ��ͣ�ص�������������ͻᵥ��ִ�г���.�ɹ�ִ�����һ�����ͷ���1,�ڶ����ͷ���2.�Դ�����.ȫ��ִ����ͻ᷵��0.
	/// ����0���ٵ�������������ֻ�ӵ�һ��ִ�г���. 
	_EXPORT_OR_IMPORT_FXVM_ int fxvm_run_step();

	/// �õ��ϴγ���ĳ�����Ϣ�������ַ���. 
	_EXPORT_OR_IMPORT_FXVM_ const char* fxvm_get_lasterr();

	/// �õ���Ԫ��ֵ.�����type���Ϳ�����"X", "Y", "M", "S", "C", "T", "D". �����ΪNULL,�򷵻����е�SoftUnit��ֵ.
	/// unit_data ָʾ�õ���ֵ��д�뵽�����ַ. len����ʱָʾ�����������������ٸ�SoftUnitData�ṹ��.
	/// �����ɹ�ʱ len������ֵ��С�ڻ����len�����ֵ����ʾʵ�������˶��ٸ�SoftUnitData�ṹ��.�������ʧ��,�򴫳�len
	/// ��ֵ����������������ܹ�������������෴��. 
	/// �����ɹ�ʱ����0��ʧ��ʱ���ط�0.
	struct SoftUnitData
	{
		char _id[8];   // ��Ԫ�ı��.  
		unsigned short _value;  // ��Ԫ��ֵ. 
	};
	_EXPORT_OR_IMPORT_FXVM_ int fxvm_read_softunits(const char* type, SoftUnitData* unit_data, int& len);

	// ################################################ testing functions #####################
	// Ignore the following testing functions. 
	_EXPORT_OR_IMPORT_FXVM_ void _fxvm_test(const char* str, const char* pattern);
	_EXPORT_OR_IMPORT_FXVM_ void _fxvm_parse(const char* filepath);
}

#endif