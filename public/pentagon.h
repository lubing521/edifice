#ifndef _H_PENTAGON_
#define _H_PENTAGON_

struct PropertyFields;

typedef int BOOL;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int  UINT;
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int  uint;

class ICuteINIConfig;

//struct	SQL_Result_Handler
//{
//	virtual void operator()(MYSQL_RES* res) = 0;
//};

//////////////////////////////// ���浼��C�ӿ�.
#ifdef __cplusplus
extern "C" {
#endif

#undef _EXPORT_OR_IMPORT_
#ifndef __GNUC__   // VC������. 
	// according to the control macro, deciding whether export or import functions
	#ifdef _I_AM_DLL_PENTAGON_
		#define _EXPORT_OR_IMPORT_  __declspec(dllexport)
	#else
		#define _EXPORT_OR_IMPORT_  __declspec(dllimport)
	#endif
#endif

#ifdef __GNUC__   // GCC������
	#ifdef _I_AM_DLL_PENTAGON_
		#define _EXPORT_OR_IMPORT_  __attribute__((visibility("default")))
	#else
	    // linux���������DLL�����������궨Ϊ��. 
		#define _EXPORT_OR_IMPORT_  
	#endif
#endif

	//////////////////////////////////////////////////////////////////
	///// �������ݿ�İ�������. 
	////////////////////////////////////////////////////////////////////////

	//// ����mysql��������һЩ����. 
	//_EXPORT_OR_IMPORT_ int config_mysql(const char* svr_addr, const char* user, const char* passwd, const char* db_name);

	//// ִ��ĳ��sql���.
	//_EXPORT_OR_IMPORT_ int commit_sql(const char* sql, SQL_Result_Handler* record_handler);

	//// �̰߳�ȫ��. ��һ���ڴ�ת�ɿ���д�����ݿ��ת�����ַ���. 
	//_EXPORT_OR_IMPORT_ unsigned char* safe_binary_tsafe(unsigned char* dest, const unsigned char* str, int len);

	//// ͬ��. �����̰߳�ȫ. 
	//_EXPORT_OR_IMPORT_ unsigned char* safe_binary(const unsigned char* str, int len);

	//////////////////////////////////////////////////////////////////////
	//// �����ַ�����һЩ����.
	//////////////////////////////////////////////////////////////////////////
	
	// ȥ���ַ���ǰ���ĳЩ�ַ�.��Щ�ַ���rub�ַ���ָ��. 
	_EXPORT_OR_IMPORT_ char* trim_str(char* str, const char* rub);

	// ���ַ����е�ĳЩ�ַ�ȫ������.��Щ�ַ���rub�ַ���ָ��. 
	_EXPORT_OR_IMPORT_ char* erase_str(char* str, const char* rub);

	// ת��д.  
	_EXPORT_OR_IMPORT_ char* to_uppercase(char* str);

	// תСд. 
	_EXPORT_OR_IMPORT_ char* to_lowercase(char* str);

	// bit���Ƚ�. 
	// len_inbit ��ʾҪ�Ƚϵ�bitλλ��. 
	// NOTICE.�������ͬ���б�׼C�⺯��һ�������ָ������Ƿ�ΪNULL�����ΪNULL����������. 
	_EXPORT_OR_IMPORT_ int memcmp_inbit(const void* left, const void* right, unsigned int len_inbit); 

	/////////////////////////////////////////////////////////////////////////////////
	///// ����λ��һЩ��������. 
	////////////////////////////////////////////////////////////////////

	// ��˵�ushort��С�˵�ushort����.�������˼�����С�ˣ�����С�˼����ش�ˡ�
	_EXPORT_OR_IMPORT_ unsigned short beXle_s(unsigned short us);

	// �����ڴ��ĵ͵�ַ. �ٴ���һ�������ַ���. ��"FF00"����ȡ���ڴ����ʼ��ַ��ʼ��һ���ֽ�.
	// ��ֵ��Big-endian����ʽ����.���ֻ�ܽ���һ��int��С. 
	// BUG NOTICE��������������һ����д����.��ζ������ "0FF0" �����᷵��һ��BYTE,���Ƿ���һ��SHORT��С,
	// ��һ����"0F"����,Ȼ������8λ�ټ�����"F0"ȡ����. 
	_EXPORT_OR_IMPORT_ unsigned int mask_num(const unsigned char* mem, const char* mask);

	// ��������.
	// ��һС���ڴ��xor checksumֵ.
	_EXPORT_OR_IMPORT_ unsigned char xor_check_sum(const unsigned char* buff, int len);

	// ģ���ڴ�ƥ��. ֧�� ?  ƥ��ʱ���� 0. ��ƥ��ʱ���� 1.
	_EXPORT_OR_IMPORT_ int fuzzy_memcmp(const unsigned char* mem, const unsigned char* pattern, int len);

	// ������λshiftλ�ڴ�.����ֵ���Ƴ��ĸ�bitλ��.������byte�ĸ�λbit. 
	_EXPORT_OR_IMPORT_ unsigned char left_shift_mem(unsigned char* mem, int len, int shift);

	// ������λshiftλ�ڴ�.����ֵ���Ƴ��ĵ�bitλ��.������byte�ĵ�λbit. 
	_EXPORT_OR_IMPORT_ unsigned char right_shift_mem(unsigned char* mem, int len, int shift);

	// ������λshiftλ������.����ֵ���Ƴ��ĵ�bitλ��.������byte�ĵ�λbit. 
	_EXPORT_OR_IMPORT_ unsigned char left_shift_bitstream(unsigned char* bitstream, int len, int shift);

	// ������λshiftλ������.����ֵ���Ƴ��ĸ�bitλ��.������byte�ĸ�λbit. 
	_EXPORT_OR_IMPORT_ unsigned char right_shift_bitstream(unsigned char* bitstream, int len, int shift);

	// ������λshiftλ�ڴ�.����ֵ���Ƴ��ĵ�λbitλ.������byte�ĵ�λbit. 

	// ����һ������.��λ��ʼ��left_bit_numλ������Ϊ 1 .
	// ���� build_mask_code(2) ������ 0xc0 �� 1100 0000
	_EXPORT_OR_IMPORT_ unsigned char build_mask_code(int left_bit_num);

	// ��һ���ڴ�ת��ʮ������human readable�ַ���.��Ҫ�ڵ����������ṩ������.����ֵ��������������� 
	_EXPORT_OR_IMPORT_ char* bins2hexs(const unsigned char* mem, int mem_len, char* buff, int buff_len);

	// ��ʮ������human readable�ַ���ת���ڴ��.�����ʶ����ʮ�����Ʒ�����buff�ᱻ����?.�����س�������.�ɹ�����0. 
	_EXPORT_OR_IMPORT_ int hexs2bins(const char* hexstr, unsigned char* buff, int* lenp);

	// ����0,1��ʾ��bit���ַ���ת���ڴ��.�����ʶ����0,����1���8λ��Ĳ���ʶ���ַ�����ַ���������. 
	// ���� bits ������ "10111010110011"
	// ���������ȷ����������buff�㹻��������0,���򷵻ط�0. 
	_EXPORT_OR_IMPORT_ int bits2bins(const char* bits, unsigned char* buff, int* lenp);

	//// �ַ����е�ÿһ��charת�������� '1' ת�� 1, 'f'ת�� 15.������ʮ�������ַ���ת��.
	// һ������ĺ���. ʵ������ hexs2bins �Ϳ�����. 
	//_EXPORT_OR_IMPORT_ int readable_num2unreadable(const char* org_str, char* dest_str, int* dest_len);

	/////////////////////////////////////////////////////////////
	/// ����־�ú���.log_me��log_me_nn���̰߳�ȫ��. 
	////////////////////////////////////////////////////////////////
	// ����log�Ĳ������ĸ������ļ��ж���.������������� log_me �� log_me_nn ֮ǰ���ò���Ч. 
	_EXPORT_OR_IMPORT_ void set_log_ini_file(const char* inifile);
	// ���Զ���� '\n'.
	_EXPORT_OR_IMPORT_ int log_me(const char* format, ...);
	// �����Զ���� '\n'
	_EXPORT_OR_IMPORT_ int log_me_nn(const char* format, ...);

	////////////////////////////////////////////////////////////////
	/// ��ʱ�������������˳�. 
	_EXPORT_OR_IMPORT_ void report_err_immed(const char* err_info, int exit_code);

	///////////////////////////////////////
	/// OS�ຯ��
	/////////////////////////////////////////////////

	// ����һ��·��.����basename.ͬBASH���basename. 
	_EXPORT_OR_IMPORT_ const char* get_basename(const char* path);

	// ���ص�ǰ����Ŀ¼.����static������.���̰߳�ȫ����. 
	// Ŀ¼�ַ����� \\ ��β. 
	_EXPORT_OR_IMPORT_ const char* working_dir();

	//// ����ļ���ʱ����غ�����ʵ��û��ʵ����ֲ�汾.�����⼸��������ʱ����Ҫ��.���Ǿͱ�ע����. 
	//// �õ�mysql���ݿ��е�DATATIME�����ַ�����ʾ��ǰ��ʱ��. ��ʽ�� 'YYYY-MM-DD HH:MM:SS'.
	//// ����buff����һ�����������ٳ� strlen('YYYY-MM-DD HH:MM:SS') + 1 = 20;
	//// ����ֵҲ�ǻ���Ļ�������ַ. 
	//_EXPORT_OR_IMPORT_ char* get_datatime(char* buff);

	//// ����һ�� 'YYYY-MM-DD HH:MM:SS' ��ʽ���ַ���������һ��datatime_t�ṹ��.
	//struct datatime_t 
	//{
	//	unsigned short _year;
	//	unsigned char _month;
	//	unsigned char _day;
	//	unsigned char _hour;
	//	unsigned char _minute;
	//	unsigned char _second;
	//};
	//_EXPORT_OR_IMPORT_ datatime_t datatime_parser(const char* datatime);

	//// ��datatime_t�ṹ���ʾ�� 'YYYY-MM-DD HH:MM:SS' ��ʽ���ַ���. buffΪҪ����Ļ�����.lenΪ����������. 
	//// ���� buff �ĵ�ַ. �� dt �� buff ��һ��ΪNULL���򷵻� NULL.
	//_EXPORT_OR_IMPORT_ const char* datatime_t2str(const datatime_t* dt, char* buff, int buff_len);

	//// �����߸��ֽڱ�ʾ��ʱ��ת�� datatime_t �ṹ.
	//// �߸��ֽ�Ϊ ������ʱ��������. 
	//_EXPORT_OR_IMPORT_ datatime_t sevenbytetime2datatime_t(unsigned char* sevenbytetime);

	//// ��datatime_t��ʾ��ʱ��ת�����ֽ���ʽд��buff��ַ.��ʱ��������.
	//_EXPORT_OR_IMPORT_ void datatime_t2sevenbytetime(const datatime_t& dt, unsigned char* buff);

	//// �ѵ�ǰ��ʱ��д7�ֽ���ʽ��ʾд��buff�ĵ�ַ.
	//_EXPORT_OR_IMPORT_ void current_time_in7byte(unsigned char* buff);

	////////////////////////////////////////////////////////////////////
	//// ��������������һ����������.�õ�������������ص�Ԫ�ظ�����һ��Ԫ�صĳ���(��ת�ɵ�λbit)�����Ԫ�صĳ����ǲ���
	//// ��BIT��λ�����.����Ԫ�صĳ���(ת�ɵ�λbit),����Ԫ�صĳ���(ת�ɵ�λBYTE.)
	_EXPORT_OR_IMPORT_ void propfields_parser(const PropertyFields& fields, ushort* elem_count, bool* is_elem_unit_in_bit,
									ushort* elem_len_in_bit, uint* total_elem_len_in_bit, uint* total_elem_len_in_byte);

	////////////////////////////////////////// ������������ ///////////////////////
	// �� / ������һ��,�ú�������ȡ��.
	_EXPORT_OR_IMPORT_ int ceil_divide(uint dividend, uint divisor);
	
	//////////////////////////////////////////////////////////////////
	///////// ����ӿ�.����ģʽ����������. (����.���ù���.ֱ���ýӿ�.)

	//// ��������.�������ִ���һ��StemCell����.
	//_EXPORT_OR_IMPORT_ StemCell* BuildStemCell(const char* name);
	//// ���������������.
	//_EXPORT_OR_IMPORT_ void  DestroyStemCell(StemCell* cell);

	// �õ� ICuteINIConfig �ӿ�. 
	_EXPORT_OR_IMPORT_ ICuteINIConfig* GetICuteINIConfig();

#ifdef __GNUC__
	_EXPORT_OR_IMPORT_ int stricmp(const char* left, const char* right);
#endif

	//////////////////////////////////////
	///// �����ú��� 
	//////////////////////////////////////////
	_EXPORT_OR_IMPORT_ void test_me(const char* arg);

#ifdef __cplusplus
}
#endif

#endif
