// pentagon.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Pentagon.h"
#include "Logger.h"
#include "prot_structs.h"
#include "CuteINIConfig.h"
#include "generic_helper.h"

//#ifdef _MANAGED
//#pragma managed(push, off)
//#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//#ifdef _MANAGED
//#pragma managed(pop)
//#endif

//////////////////////////////////////////////////////////////////////////////////////////
///// 操作MYSQL的一些函数
///////////////////////////////////////////////////////////////////////////////////

//int config_mysql(const char* svr_addr, const char* user, const char* passwd, const char* db_name)
//{
//	int retcode = db_manager_t::instance().config_mysql(svr_addr, user, passwd, db_name);
//
//	return retcode;
//	
////	return -1;
//}

//int commit_sql(const char* sql, SQL_Result_Handler* record_handler)
//{
//	int retcode = db_manager_t::instance().commit_sql(sql, record_handler);
//	
//	return retcode;
//}

//unsigned char* safe_binary_tsafe(unsigned char* dest, const unsigned char* str, int len)
//{
//	unsigned char* ret = db_manager_t::instance().safe_binary(dest, str, len);
//	
//	return ret;
////	return NULL;
//}
//
//unsigned char* safe_binary(const unsigned char* str, int len)
//{
//	unsigned char* ret = db_manager_t::instance().safe_binary(str, len);
//
//	return ret;
////	return NULL;
//}

char* trim_str(char* str, const char* rub)
{
	if ( NULL != str && NULL != rub )
	{
		int len = strlen(str);
		int rubl = strlen(rub);
		int i = 0;
		bool skip = false;

		for ( i = 0; i < len; i ++ )
		{
			skip = false;
			for ( int j = 0; j < rubl; j++ )
			{
				if ( rub[j] == str[i] )
				{
					skip = true;
					continue;
				}
			}
			if ( skip )
			{
				continue;
			}

			break;
		}
 
		if ( 0 != i )
		{
			memmove(str, str+i, len+1-i);
		}

		len -= i;

		for ( i = len - 1; i >= 0; i -- )
		{
			skip = false;
			for ( int j = 0; j < rubl; j++ )
			{
				if ( rub[j] == str[i] )
				{
					skip = true;
				}
			}

			if ( skip )
			{
				continue;
			}

			str[i + 1] = 0;

			break;
		}
	}

	return str;
}

char* erase_str(char* str, const char* rub)
{
	int  write_it_here;
	int  str_len;
	int  rub_len;
	bool erase_me;

	if ( NULL == str || NULL == rub )
	{
		return str;
	}

	write_it_here = -1;
	str_len = strlen(str);
	rub_len = strlen(rub);
	for ( int i = 0; i < str_len; i++ )
	{
		erase_me = false;
		for ( int j = 0; j < rub_len; j++ )
		{
			if ( rub[j] == str[i] )
			{
				erase_me = true;
				break;
			}
		}

		if ( !erase_me )
		{
			write_it_here++;
		}
		else
		{
			continue;
		}

		if ( write_it_here != i )
		{
			str[write_it_here] = str[i];
		}
	} // end of for
	str[write_it_here+1] = 0;

	return str;
}

char* to_uppercase(char* str)
{
	if ( NULL == str )
	{
		return NULL;
	}
	int len = strlen(str);
	for ( int i = 0; i < len; i++ )
	{
		str[i] = toupper(str[i]);
	}
	return str;
}

char* to_lowercase(char* str)
{
	if ( NULL == str )
	{
		return NULL;
	}
	int len = strlen(str);
	for ( int i = 0; i < len; i++ )
	{
		str[i] = tolower(str[i]);
	}
	return str;
}

int memcmp_inbit(const void* left, const void* right, unsigned int len_inbit)
{
	int retcode;
	int len_in_byte = ceil_divide(len_inbit, 8);
	int brood = len_in_byte * 8 - len_inbit;

	unsigned char* _left = new unsigned char[len_in_byte];
	unsigned char* _right = new unsigned char[len_in_byte];

	memcpy(_left, left, len_in_byte);
	memcpy(_right, right, len_in_byte);

	if ( brood != 0 )
	{
//		unsigned char mask_code = build_mask_code(8-brood);
		unsigned char mask_code = ~build_mask_code(brood);
		((unsigned char*)_left)[len_in_byte-1] &= mask_code;
		((unsigned char*)_right)[len_in_byte-1] &= mask_code;
	}
	
	retcode = memcmp(_left, _right, len_in_byte);
	
	delete []_left;
	delete []_right;

	return retcode;
}

unsigned short beXle_s(unsigned short us)
{
	unsigned short rt;
	*((unsigned char*)&rt + 1) = *(unsigned char*)&us;
	*(unsigned char*)&rt = *((unsigned char*)&us + 1);
	return rt;
}

unsigned int mask_num(const unsigned char* mem, const char* mask)
{
	int  mask_len = strlen(mask);
	char mask_padding[1024] = { 0 };
	strcpy(mask_padding, mask);
	// 去掉mask后面的 '0000'
	int i;
	for ( i = mask_len - 1; i >= 0; i-- )
	{
		if ( 0x30 == mask_padding[i] ) 
			mask_padding[i] = 0;
		else
			break;
	}
	mask_len = i + 1;
	unsigned char byte_mask_str[] = {'0', 'x', '?', '?', '\0'};
	unsigned char byte_mask_char;
	unsigned char byte_this_time;
	unsigned int  i_got_u = 0;

	int loop_count = mask_len / 2;
	for ( int i = 0; i < loop_count; i++ )
	{
		byte_mask_str[2] = mask_padding[i*2];
		byte_mask_str[3] = mask_padding[i*2+1];

		byte_mask_char = strtol((const char*)byte_mask_str, NULL, 0);

		byte_this_time = (mem[i] & byte_mask_char);

		i_got_u = ( i_got_u << 8 ) + byte_this_time;
	}
	if ( 1 == mask_len % 2 )  // mask去掉结尾0后的长度是奇数. 就要取理后四个bits
	{
		byte_mask_str[2] = mask_padding[mask_len-1];
		byte_mask_str[3] = 0;

		byte_mask_char = strtol((const char*)byte_mask_str, NULL, 0);

		byte_this_time = ((mem[mask_len/2]>>4) & byte_mask_char);
		i_got_u = ( i_got_u << 4 ) + byte_this_time;
	}
	// 有可能还要右移.下面是判断要右移多少bit的逻辑.
	unsigned char laft_mask_code = strtol(&mask_padding[mask_len-1], NULL, 16);
	for ( ; 0 == laft_mask_code%2; laft_mask_code/=2 )
	{
		i_got_u >>= 1;
	}

	return i_got_u;
}

unsigned char xor_check_sum(const unsigned char* buff, int len)
{
	if ( !buff )
	{
		assert("NULL pointer.");
		return 0;
	}
	unsigned char checksum = *buff;
	for ( int i = 1; i < len; i ++ )
	{
		checksum ^= buff[i];
	}
	return checksum;
}

int fuzzy_memcmp(const unsigned char* mem, const unsigned char* pattern, int len)
{
	for ( int i = 0; i < len; i ++ )
	{
		if ( '?' == pattern[i] )
		{
			continue;
		}

		if ( mem[i] == pattern[i] )
		{
			continue;
		}

		return 1; // 不匹配.
	}

	return 0; // 匹配.
}

unsigned char left_shift_mem(unsigned char* mem, int len, int shift)
{
	if ( shift == 0 )
	{
		return 0;
	}
	if ( NULL == mem || 0 >= len || shift > 8 || shift < 0 )
	{
		assert(0);
		return 0;
	}
	unsigned char shiftout = 0;
	unsigned char masked = 0; 
	unsigned char maskcode = build_mask_code(shift);

	// 特殊处理第一个字节. 
	shiftout = (maskcode & mem[0]);
	shiftout >>= ( 8 - shift );
	mem[0] <<= shift;

	for ( int i = 1; i < len; i++ )
	{
		masked = (maskcode & mem[i]);
		masked >>= ( 8 - shift );
		mem[i-1] |= masked;
		mem[i] <<= shift;
	}

	return shiftout; 
}

unsigned char right_shift_mem(unsigned char* mem, int len, int shift)
{
	if ( shift == 0 )
	{
		return 0;
	}
	if ( NULL == mem || 0 >= len || shift > 8 || shift < 0 )
	{
		assert(0);
		return 0;
	}

	unsigned char shiftout = 0;
	unsigned char masked = 0;
	unsigned char maskcode = ~build_mask_code(shift);
	
	shiftout = (maskcode & mem[len-1]);
	shiftout <<= ( 8 - shift );
	mem[len-1] >>= shift;

	for ( int i = len-2; i >= 0; i-- )
	{
		masked = (maskcode & mem[i]);
		masked <<= ( 8 - shift );
		mem[i+1] |= masked;
		mem[i] <<= shift;
	}

	return shiftout;
}

unsigned char left_shift_bitstream(unsigned char* bitstream, int len, int shift)
{
	if ( shift == 0 )
	{
		return 0;
	}
	if ( NULL == bitstream || 0 >= len || shift > 8 || shift < 0 )
	{
		assert(0);
		return 0;
	}

	unsigned char shiftout = 0;
	unsigned char masked = 0; 
	unsigned char maskcode = ~build_mask_code(8-shift);

	// shiftout是返回值. 
	shiftout = (maskcode & bitstream[0]);
	shiftout <<= ( 8 - shift );
	
	bitstream[0] >>= shift;
	for ( int i = 1; i < len; i++ )
	{
		masked = (maskcode & bitstream[i]);
		masked <<= ( 8 - shift );
		bitstream[i-1] |= masked;
		bitstream[i] >>= shift;
	}

	return shiftout; 
}

unsigned char right_shift_bitstream(unsigned char* bitstream, int len, int shift)
{
	unsigned char* buff = new unsigned char[len + 1];
	memset(buff, 0, len + 1);
	memcpy(buff + 1, bitstream, len);
	unsigned char shiftout = left_shift_bitstream(buff + 1, len, 8 - shift);
	buff[0] = shiftout;
	memcpy(bitstream, buff, len);
	shiftout = buff[len];
	delete buff;
	return shiftout;
}

unsigned char build_mask_code(int left_bit_num)
{
	if ( left_bit_num >8 || left_bit_num < 1 )
		return 0;

	const unsigned char mosque = 0x80;
	unsigned char maskcode = 0;

	for ( int i = 0; i < left_bit_num; i ++ )    // 构建掩码. 
	{
		unsigned char pray = ( mosque >> i );
		maskcode |=  pray;
	}

	return maskcode;
}

char* bins2hexs(const unsigned char* mem, int mem_len, char* buff, int buff_len)
{
	int len_expected;

	if ( NULL == mem || 0 == mem_len || NULL == buff || 0 == buff_len ) 
	{
		if ( NULL != buff && 0 != buff_len )
		{
			memset(buff, 0, buff_len);
			return buff;
		}
		else
		{
			return NULL;
		}
	}

	memset(buff, 0, buff_len);

	len_expected = mem_len * 2;
	char* buff_expected = new char[len_expected];
	char small_buff[3];
	for ( int i = 0; i < mem_len; i ++ )
	{
		int height = ( (mem[i] & 0xf0) >> 4);
		int low = (mem[i] & 0x0f);

		sprintf(small_buff, "%X", height);
		sprintf(small_buff+1, "%X", low);
		
		memcpy(buff_expected + i * 2, small_buff, 2);
	}
	
	if ( buff_len >= len_expected )
	{
		memcpy(buff, buff_expected, len_expected);
	}

	delete []buff_expected;

	return buff;
}

int hexs2bins(const char* hexstr, unsigned char* buff, int* lenp)
{
	if ( NULL == hexstr || NULL == buff || NULL == lenp )
	{
		return -1;
	}

	int retcode = 0;
	int len = *lenp;
	int olen = strlen(hexstr);
	int nlen = olen/2 + olen%2;
	char short_buff[3] = { 0 };
	int max_copied = (( nlen > len ) ? len : nlen );

	for ( int i = 0; i < olen; i ++ )
	{
		char ch = hexstr[i];
		bool err = true;
		if ( isdigit(ch) )
			err = false;
		if ( ! (ch<='F' && ch>='A' ) )
			err = false;
		if ( !(ch<='f' && ch>='a' ) )
			err = false;

		if ( err )  // 碰到的字符不在 [0-9A-F] 区间
		{
			memset(buff, '?', *lenp);
			return -2;
		}
	}

	char* long_buff = new char[nlen];
	for ( int i = 0; i < olen/2; i ++ )
	{
		short_buff[0] = hexstr[i*2];
		short_buff[1] = hexstr[i*2+1];
		long_buff[i] = (unsigned char)strtol(short_buff, NULL, 16);
	}
	if ( olen % 2 )
	{
		int the_last_index = olen - 1;
		short_buff[0] = hexstr[the_last_index];
		short_buff[1] = '0';
		long_buff[nlen-1] = (unsigned char)strtol(short_buff, NULL, 16);
	}

	if ( nlen > len )
	{
		retcode = -1;
	}

	memset(buff, 0, len);
	memcpy(buff, long_buff, max_copied);

	*lenp = nlen;  // len引用被改为新的值. 

	delete []long_buff;

	return retcode;
}

int bits2bins(const char* bits, unsigned char* buff, int* lenp)
{
	if ( NULL == bits || NULL == buff || NULL == lenp )
	{
		return -1;
	}

	int retcode = 0;
	int bits_len = strlen(bits);
	int len_expected = ceil_divide(bits_len, 8);
	if ( len_expected == 0 )
	{
		*lenp = 0;
		return 0;
	}

	char small_buff[16];
	unsigned char* buff_expected = new unsigned char[len_expected];
	char* bits_expected = new char[len_expected*8];
	memset(buff_expected, 0, len_expected);
	memset(bits_expected, 0x30, len_expected*8);

	memcpy(bits_expected, bits, bits_len);

	for ( int i = 0; i < len_expected; i++ )
	{
		strncpy(small_buff, bits_expected + i*8, 8 );
		unsigned char uch = strtol(small_buff, NULL, 2);
		buff_expected[i] = uch;
	}

	if ( *lenp >= len_expected )
	{
		*lenp = len_expected;
		memcpy(buff, buff_expected, len_expected);
		retcode = 0;
	}
	else
	{
		*lenp = -len_expected;
		retcode = -1;
	}

	delete bits_expected;
	delete buff_expected;

	return retcode;
}


// 将被废弃的旧函数. 
//int readable_num2unreadable(const char* org_str, char* dest_str, int* dest_len)
//{
//	if ( NULL == org_str || NULL == dest_str || NULL == dest_len )
//	{
//		return -1;
//	}
//
//	memset(dest_str, 0, *dest_len);
//
//	int org_str_len = strlen(org_str);
//	if ( *dest_len < org_str_len )
//	{
//		*dest_len = org_str_len;
//		return -1;
//	}
//
//	char buff[2] = { 0 };
//	for ( int i = 0; i < org_str_len; i++ )
//	{
//		char ch = org_str[i];
//		// 如果是十六进制字符
//		if ( isdigit(ch) || ( ch >= 'a' && ch <= 'f' ) || ( ch >= 'A' && ch <= 'F' ) )
//		{
//			buff[0] = ch;
//			dest_str[i] = strtol(buff, NULL, 16);
//		}
//		else
//		{
//			dest_str[i] = ch;
//		}
//	}
//
//	return 0;
//}
void set_log_ini_file(const char* inifile)
{
	Logger::Instance(inifile);
}

int log_me(const char* format, ...)
{
	int retcode = 0;

	char buff[512] = { 0 };   // alas,..
	strncpy(buff, format, 511);   // strict the magic numbers in a small area. 
	strcat(buff, "\n");

	va_list ap;
	va_start(ap, format);
	retcode = Logger::Instance(NULL)._vprintf(buff, ap);
	va_end(ap);

	return retcode;
}

int log_me_nn(const char* format, ...)
{
	int retcode = 0;

	char buff[512] = { 0 };   // alas,..
	strncpy(buff, format, 511);   // strict the magic numbers in a small area. 

	va_list ap;
	va_start(ap, format);
	retcode = Logger::Instance(NULL)._vprintf(buff, ap);
	va_end(ap);

	return retcode;
}

void report_err_immed(const char* err_info, int exit_code)
{
	if ( NULL != err_info )
	{
#ifdef __GNUC__
	fprintf(stderr, err_info);
	log_me(err_info);
#else
	MessageBoxA(NULL, err_info, "a error occured...", MB_OK|MB_ICONEXCLAMATION);
#endif
	}

	exit(exit_code);
}

const char* get_basename(const char* path)
{
	if ( NULL == path )  return NULL;

	const char* ptr2slash = strrchr(path, '/');
	const char* ptr2backslash = strrchr(path, '\\');
	const char* ptr = ( ( ptr2backslash != NULL ) ? ptr2backslash : ptr2slash );
	if ( NULL != ptr ) ptr++;
	else ptr = path;

	return ptr;
}

const char* working_dir()
{
	static std::string wd;
	char buff[MAX_PATH] = { 0 };
	

#ifdef __GNUC__
	getcwd(buff, sizeof(buff));
#else
	GetCurrentDirectoryA(sizeof(buff), buff);
#endif
	
	wd = buff;
	wd += "\\";

	return wd.c_str();
}

//char* get_datatime(char* buff) // 'YYYY-MM-DD HH:MM:SS'
//{
//	if ( NULL != buff )
//	{
//		SYSTEMTIME time;
//		GetLocalTime(&time);
//		sprintf(buff, "%4u-%02u-%02u %02u:%02u:%02u", time.wYear, time.wMonth, time.wDay,
//							time.wHour, time.wMinute, time.wSecond);
//	}
//
//	return buff;
//}
//
////  0123456789ABCDEF
//// 'YYYY-MM-DD HH:MM:SS'
//datatime_t datatime_parser(const char* datatime)
//{
//	// 没有检查datatime参数的格式正确性.
//	datatime_t dt;
//
//	char year[5] = { 0 };
//	char month[3] = { 0 };
//	char day[3] = { 0 };
//	char hour[3] = { 0 };
//	char minute[3] = { 0 };
//	char second[3] = { 0 };
//
//	memcpy(year, datatime + 0, 4);
//	memcpy(month, datatime + 5, 2);
//	memcpy(day, datatime + 8, 2);
//	memcpy(hour, datatime + 11, 2);
//	memcpy(minute, datatime + 14, 2);
//	memcpy(second, datatime + 17, 2);
//
//	dt._year = atoi(year);
//	dt._month = atoi(month);
//	dt._day = atoi(day);
//	dt._hour = atoi(hour);
//	dt._minute = atoi(minute);
//	dt._second = atoi(second);
//
//	return dt;
//}
//
//const char* datatime_t2str(const datatime_t* dt, char* buff, int buff_len)
//{
//	if ( NULL == dt || NULL == buff )
//	{
//		return NULL;
//	}
//
//	// 'YYYY-MM-DD HH:MM:SS'
//	sprintf_s(buff, buff_len, "%4u-%2u-%2d %2u:%2u:%2u", dt->_year, dt->_month, dt->_day, dt->_hour, dt->_minute, dt->_second);
//
//	return buff;
//}
//
//datatime_t sevenbytetime2datatime_t(unsigned char* sevenbytetime)
//{
//	datatime_t dt;
//	dt._year = 2000 + sevenbytetime[0];
//	dt._month = sevenbytetime[1];
//	dt._day =  sevenbytetime[2];
//	dt._hour =  sevenbytetime[3];
//	dt._minute =  sevenbytetime[4];
//	dt._second =  sevenbytetime[5];
//	// 忽略星期
//	return dt;
//}
//
//void datatime_t2sevenbytetime(const datatime_t& dt, unsigned char* buff)
//{
//	memcpy(buff + 1, &dt._month, 5);
//	buff[0] = dt._year - 2000;
//}
//
//void current_time_in7byte(unsigned char* buff)
//{
//	SYSTEMTIME time;
//	GetLocalTime(&time);
//
//	buff[0] = time.wYear - 2000;
//	buff[1] = time.wMonth;
//	buff[2] = time.wDay;
//	buff[3] = time.wHour;
//	buff[4] = time.wMinute;
//	buff[5] = time.wSecond;
//	buff[6] = time.wDayOfWeek;
//}

void propfields_parser(const PropertyFields& fields, ushort* elem_count, bool* is_elem_unit_in_bit,
					   ushort* elem_len_in_bit, uint* total_elem_len_in_bit, uint* total_elem_len_in_byte)
{
	ushort _elem_count = fields._elem_count;
	ElemTypeField _etf(fields._elem_type);
	ushort _elem_len_in_bit = _etf.get_len_in_bit();
	uint _total_elem_len_in_bit = _elem_len_in_bit * _elem_count;
	// uint _total_elem_len_in_byte = _total_elem_len_in_bit / 8 + !!_total_elem_len_in_bit % 8;
	uint _total_elem_len_in_byte = ceil_divide(_total_elem_len_in_bit, 8);

	if ( NULL != elem_count )
	{
		*elem_count = _elem_count;
	}

	if ( NULL != elem_len_in_bit )
	{
		*elem_len_in_bit = _elem_len_in_bit;
	}

	if ( NULL != is_elem_unit_in_bit )
	{
		*is_elem_unit_in_bit = _etf.elem_unit_in_bit();
	}

	if ( NULL != total_elem_len_in_bit )
	{
		*total_elem_len_in_bit = _total_elem_len_in_bit;
	}

	if ( NULL != total_elem_len_in_byte )
	{
		*total_elem_len_in_byte = _total_elem_len_in_byte;
	}
}

int ceil_divide(uint dividend, uint divisor)
{
	int r = dividend / divisor + !!(dividend % divisor);
	return r;
}

#ifdef __GNUC__
int stricmp(const char* left, const char* right)
{
	return strcasecmp(left, right);
}
#endif

///////////////////////// 工厂模式 ///////////////////////// 不用工厂啦。直接申请接口. 
//StemCell* BuildStemCell(const char* name)
//{
//	if ( NULL == name )
//	{
//		return NULL;
//	}
//	else if ( 0 == strcmp(name, "ICuteINIConfig") )
//	{
//		return new CuteINIConfig;
//	}
//
//	return NULL;
//}
//
//void  DestroyStemCell(StemCell* cell)
//{
//	cell->Free();
//}

ICuteINIConfig* GetICuteINIConfig()
{
	return new CuteINIConfig;
}

///////////////////////// 一个测试函数 //////////////////////////////////////////
void test_me(const char* arg)
{
	if ( arg == NULL )
	{
		printf("NULL pointer.\n");
	}
	else
	{
		printf("non-null pointer.\n");
	}	
}