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

//////////////////////////////// 下面导出C接口.
#ifdef __cplusplus
extern "C" {
#endif

#undef _EXPORT_OR_IMPORT_
#ifndef __GNUC__   // VC编译器. 
	// according to the control macro, deciding whether export or import functions
	#ifdef _I_AM_DLL_PENTAGON_
		#define _EXPORT_OR_IMPORT_  __declspec(dllexport)
	#else
		#define _EXPORT_OR_IMPORT_  __declspec(dllimport)
	#endif
#endif

#ifdef __GNUC__   // GCC编译器
	#ifdef _I_AM_DLL_PENTAGON_
		#define _EXPORT_OR_IMPORT_  __attribute__((visibility("default")))
	#else
	    // linux下如果不是DLL本身，则把这个宏定为空. 
		#define _EXPORT_OR_IMPORT_  
	#endif
#endif

	//////////////////////////////////////////////////////////////////
	///// 操作数据库的帮助函数. 
	////////////////////////////////////////////////////////////////////////

	//// 配置mysql服务器的一些参数. 
	//_EXPORT_OR_IMPORT_ int config_mysql(const char* svr_addr, const char* user, const char* passwd, const char* db_name);

	//// 执行某个sql语句.
	//_EXPORT_OR_IMPORT_ int commit_sql(const char* sql, SQL_Result_Handler* record_handler);

	//// 线程安全的. 把一段内存转成可以写入数据库的转义后的字符串. 
	//_EXPORT_OR_IMPORT_ unsigned char* safe_binary_tsafe(unsigned char* dest, const unsigned char* str, int len);

	//// 同上. 但非线程安全. 
	//_EXPORT_OR_IMPORT_ unsigned char* safe_binary(const unsigned char* str, int len);

	//////////////////////////////////////////////////////////////////////
	//// 操作字符串的一些函数.
	//////////////////////////////////////////////////////////////////////////
	
	// 去掉字符串前后的某些字符.这些字符由rub字符串指定. 
	_EXPORT_OR_IMPORT_ char* trim_str(char* str, const char* rub);

	// 把字符串中的某些字符全部擦除.这些字符由rub字符串指字. 
	_EXPORT_OR_IMPORT_ char* erase_str(char* str, const char* rub);

	// 转大写.  
	_EXPORT_OR_IMPORT_ char* to_uppercase(char* str);

	// 转小写. 
	_EXPORT_OR_IMPORT_ char* to_lowercase(char* str);

	// bit流比较. 
	// len_inbit 表示要比较的bit位位数. 
	// NOTICE.这个函数同所有标准C库函数一样不检查指针参数是否为NULL，如果为NULL，立即崩溃. 
	_EXPORT_OR_IMPORT_ int memcmp_inbit(const void* left, const void* right, unsigned int len_inbit); 

	/////////////////////////////////////////////////////////////////////////////////
	///// 操作位的一些帮助函数. 
	////////////////////////////////////////////////////////////////////

	// 大端的ushort和小端的ushort互换.即传入大端即返回小端，传入小端即返回大端。
	_EXPORT_OR_IMPORT_ unsigned short beXle_s(unsigned short us);

	// 传入内存块的低地址. 再传入一个掩码字符串. 如"FF00"，则取该内存块起始地址开始的一个字节.
	// 数值以Big-endian的形式解析.最大只能解析一个int大小. 
	// BUG NOTICE：掩码是以两个一组进行处理的.意味着掩码 "0FF0" 将不会返回一个BYTE,而是返回一个SHORT大小,
	// 第一次用"0F"运算,然后左移8位再加上用"F0"取到的. 
	_EXPORT_OR_IMPORT_ unsigned int mask_num(const unsigned char* mem, const char* mask);

	// 辅助函数.
	// 算一小段内存的xor checksum值.
	_EXPORT_OR_IMPORT_ unsigned char xor_check_sum(const unsigned char* buff, int len);

	// 模糊内存匹配. 支持 ?  匹配时返回 0. 不匹配时返回 1.
	_EXPORT_OR_IMPORT_ int fuzzy_memcmp(const unsigned char* mem, const unsigned char* pattern, int len);

	// 向左移位shift位内存.返回值是移出的高bit位们.向左移byte的高位bit. 
	_EXPORT_OR_IMPORT_ unsigned char left_shift_mem(unsigned char* mem, int len, int shift);

	// 向右移位shift位内存.返回值是移出的低bit位们.向右移byte的低位bit. 
	_EXPORT_OR_IMPORT_ unsigned char right_shift_mem(unsigned char* mem, int len, int shift);

	// 向左移位shift位比特流.返回值是移出的低bit位们.向左移byte的低位bit. 
	_EXPORT_OR_IMPORT_ unsigned char left_shift_bitstream(unsigned char* bitstream, int len, int shift);

	// 向右移位shift位比特流.返回值是移出的高bit位们.向右移byte的高位bit. 
	_EXPORT_OR_IMPORT_ unsigned char right_shift_bitstream(unsigned char* bitstream, int len, int shift);

	// 向左移位shift位内存.返回值是移出的低位bit位.向左移byte的低位bit. 

	// 构建一个掩码.高位开始的left_bit_num位比特设为 1 .
	// 比如 build_mask_code(2) 将返回 0xc0 即 1100 0000
	_EXPORT_OR_IMPORT_ unsigned char build_mask_code(int left_bit_num);

	// 把一块内存转成十六进制human readable字符串.需要在第三个参数提供缓冲区.返回值就是这个缓冲区。 
	_EXPORT_OR_IMPORT_ char* bins2hexs(const unsigned char* mem, int mem_len, char* buff, int buff_len);

	// 把十六进制human readable字符串转成内存块.如果标识不是十六进制符号则buff会被填上?.并返回出错误码.成功返回0. 
	_EXPORT_OR_IMPORT_ int hexs2bins(const char* hexstr, unsigned char* buff, int* lenp);

	// 把用0,1表示的bit流字符串转成内存块.如果标识不是0,或者1则该8位组的不可识别字符后的字符都被忽略. 
	// 比如 bits 可以是 "10111010110011"
	// 如果参数正确，包括缓存buff足够则函数返回0,否则返回非0. 
	_EXPORT_OR_IMPORT_ int bits2bins(const char* bits, unsigned char* buff, int* lenp);

	//// 字符串中的每一个char转换。比如 '1' 转成 1, 'f'转成 15.其它非十六进制字符不转换.
	// 一个错误的函数. 实际上用 hexs2bins 就可以了. 
	//_EXPORT_OR_IMPORT_ int readable_num2unreadable(const char* org_str, char* dest_str, int* dest_len);

	/////////////////////////////////////////////////////////////
	/// 记日志用函数.log_me和log_me_nn是线程安全的. 
	////////////////////////////////////////////////////////////////
	// 设置log的参数从哪个配置文件中读入.这个函数必须在 log_me 和 log_me_nn 之前调用才有效. 
	_EXPORT_OR_IMPORT_ void set_log_ini_file(const char* inifile);
	// 会自动添加 '\n'.
	_EXPORT_OR_IMPORT_ int log_me(const char* format, ...);
	// 不会自动添加 '\n'
	_EXPORT_OR_IMPORT_ int log_me_nn(const char* format, ...);

	////////////////////////////////////////////////////////////////
	/// 及时报告致命错误并退出. 
	_EXPORT_OR_IMPORT_ void report_err_immed(const char* err_info, int exit_code);

	///////////////////////////////////////
	/// OS类函数
	/////////////////////////////////////////////////

	// 传入一个路径.返回basename.同BASH里的basename. 
	_EXPORT_OR_IMPORT_ const char* get_basename(const char* path);

	// 返回当前工作目录.存在static变量中.非线程安全函数. 
	// 目录字符串以 \\ 结尾. 
	_EXPORT_OR_IMPORT_ const char* working_dir();

	//// 下面的几个时间相关函数的实现没有实现移植版本.另外这几个函数暂时不需要了.于是就被注释了. 
	//// 得到mysql数据库中的DATATIME类型字符串表示当前的时间. 格式如 'YYYY-MM-DD HH:MM:SS'.
	//// 参数buff传入一个缓冲区至少长 strlen('YYYY-MM-DD HH:MM:SS') + 1 = 20;
	//// 返回值也是缓入的缓冲区地址. 
	//_EXPORT_OR_IMPORT_ char* get_datatime(char* buff);

	//// 分析一个 'YYYY-MM-DD HH:MM:SS' 格式的字符串，返回一个datatime_t结构体.
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

	//// 把datatime_t结构体表示成 'YYYY-MM-DD HH:MM:SS' 形式的字符串. buff为要传入的缓冲区.len为缓冲区长度. 
	//// 缓回 buff 的地址. 但 dt 或 buff 有一个为NULL，则返回 NULL.
	//_EXPORT_OR_IMPORT_ const char* datatime_t2str(const datatime_t* dt, char* buff, int buff_len);

	//// 把用七个字节表示的时间转成 datatime_t 结构.
	//// 七个字节为 年月日时分秒星期. 
	//_EXPORT_OR_IMPORT_ datatime_t sevenbytetime2datatime_t(unsigned char* sevenbytetime);

	//// 把datatime_t表示的时间转成七字节形式写入buff地址.暂时忽略星期.
	//_EXPORT_OR_IMPORT_ void datatime_t2sevenbytetime(const datatime_t& dt, unsigned char* buff);

	//// 把当前的时间写7字节形式表示写到buff的地址.
	//_EXPORT_OR_IMPORT_ void current_time_in7byte(unsigned char* buff);

	////////////////////////////////////////////////////////////////////
	//// 分析属性描述的一个帮助函数.得到属性描述里记载的元素个数，一个元素的长度(都转成单位bit)，这个元素的长度是不是
	//// 以BIT单位定义的.所有元素的长度(转成单位bit),所有元素的长度(转成单位BYTE.)
	_EXPORT_OR_IMPORT_ void propfields_parser(const PropertyFields& fields, ushort* elem_count, bool* is_elem_unit_in_bit,
									ushort* elem_len_in_bit, uint* total_elem_len_in_bit, uint* total_elem_len_in_byte);

	////////////////////////////////////////// 其它辅助函数 ///////////////////////
	// 和 / 除法不一样,该函数向上取整.
	_EXPORT_OR_IMPORT_ int ceil_divide(uint dividend, uint divisor);
	
	//////////////////////////////////////////////////////////////////
	///////// 面向接口.工厂模式的两个函数. (废弃.不用工厂.直接拿接口.)

	//// 工厂函数.根据名字创建一个StemCell对象.
	//_EXPORT_OR_IMPORT_ StemCell* BuildStemCell(const char* name);
	//// 用来销毁这个对象.
	//_EXPORT_OR_IMPORT_ void  DestroyStemCell(StemCell* cell);

	// 得到 ICuteINIConfig 接口. 
	_EXPORT_OR_IMPORT_ ICuteINIConfig* GetICuteINIConfig();

#ifdef __GNUC__
	_EXPORT_OR_IMPORT_ int stricmp(const char* left, const char* right);
#endif

	//////////////////////////////////////
	///// 测试用函数 
	//////////////////////////////////////////
	_EXPORT_OR_IMPORT_ void test_me(const char* arg);

#ifdef __cplusplus
}
#endif

#endif
