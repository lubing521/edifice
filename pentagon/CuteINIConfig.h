#ifndef _H_CUTEINICONFIG_
#define _H_CUTEINICONFIG_

#include <string>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include "ICuteINIConfig.h"

class CuteINIConfig : public ICuteINIConfig
{
public:
	// 设好IniFile的路径.
	// 如果打开文件失败或格式错会返回非0.否则返回0.
	int SetIniFilePath(const char* ini_file_path);
	
	// sect是SECTION的名字，key就是KEY的名字.
	// s_value以字符串形式返回key对应的value.
	// n 是以整型的形式返回key对应的value.如果s_value可以转换成数. 
	// 如果 s_value 或 n_value 为 NULL. 则不会拷入. 
	// 成功返回0, 失败返回非0.
	int GetValueByKey(const char* sect, const char* key, char* s_value, int* n_value);

	CuteINIConfig(void);

protected:
	virtual void Free();

private:
	~CuteINIConfig(void);

private:
	FILE*       m_inifile;
	std::string m_filepath;

	typedef std::pair<std::string, std::string> KeyValueT;  // 用来存一个键值对. 
	typedef std::vector<KeyValueT> KeyValueVec;             // 用来存很多个键值对. 
	std::map<std::string, KeyValueVec> m_keyvalues;         // 存ini文件里读到的所有键和值对.而map的key则是section name. 
};

#endif