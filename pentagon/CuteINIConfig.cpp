#include "stdafx.h"
#include "CuteINIConfig.h"
#include "Pentagon.h"

using namespace std;

CuteINIConfig::CuteINIConfig(void)
{
	m_inifile = NULL;
}

CuteINIConfig::~CuteINIConfig(void)
{
	if ( NULL != m_inifile )
	{
		fclose(m_inifile);
	}
}

int CuteINIConfig::SetIniFilePath(const char* ini_file_path)
{
	int retcode = 0;
	char buff[512] = { 0 };
	string current_section;
	string current_key;
	string current_value;

	if ( NULL == m_inifile )
	{
		m_filepath = ini_file_path;
		m_inifile = fopen(ini_file_path, "r+");
		const char* dir = working_dir();
		if ( NULL == m_inifile )
		{
			retcode = -2;
			goto _out;
		}
		while ( fgets(buff, sizeof(buff), m_inifile) )
		{
			int len = strlen( trim_str(buff, " \r\n") );
			char* equal_sign = strchr(buff, '=');
			if ( NULL != equal_sign )
			{
				*equal_sign = 0;
				trim_str(buff, " ");
				trim_str(equal_sign + 1, " ");
				current_key = buff;
				current_value = equal_sign + 1;

				KeyValueVec& kvv = m_keyvalues[current_section];
				kvv.push_back(KeyValueT(current_key, current_value));
			}
			else if ( '[' == buff[0] && ']' == buff[len-1])
			{
				current_section.assign(buff + 1, len - 2);
			}
			else
			{
				// ... 不能识别的行. 忽略了 ...........
			}
		}
		fclose(m_inifile);
		m_inifile = NULL;
		retcode = 0;
	}
	else
	{
		// 已经打开过文件啦.
		retcode = -1;
	}

_out:
	return retcode;
}

int CuteINIConfig::GetValueByKey(const char* sect, const char* key, char* s_value, int* n_value)
{
	int retcode = -1;
	KeyValueVec& kvv = m_keyvalues[std::string(sect)];

	if ( NULL == sect || NULL == key )
	{
		retcode = -1;
	}
	else
	{
		for ( int i = 0; i < kvv.size(); i++ )
		{
			if ( 0 == stricmp( kvv[i].first.c_str(), key ) ) // 找到了这个key.
			{
				if ( NULL != s_value )
				{
					strcpy(s_value, kvv[i].second.c_str());  // 把value拷入. 
				}
				if ( NULL != n_value )
				{
					*n_value = strtol(kvv[i].second.c_str(), NULL, 10);
				}
				retcode = 0;
			}
		}
	}

	return retcode;
}

void CuteINIConfig::Free()
{
	delete this;
}