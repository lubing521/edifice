#ifndef _H_ICUTEINICONFIG_
#define _H_ICUTEINICONFIG_

////////////////////////////////////////////////////////////////////////
/// 该类用来方便读写配置文件
//////////////////////////////////////////////////////////////////////////////

class ICuteINIConfig
{
public:
	// 设好IniFile的路径.
	// 如果打开文件失败或格式错会返回非0.否则返回0.
	virtual int SetIniFilePath(const char* ini_file_path) = 0;

	// sect是SECTION的名字，key就是KEY的名字.
	// s_value以字符串形式返回key对应的value.
	// n 是以整型的形式返回key对应的value.如果s_value可以转换成数. 
	// 成功返回0, 失败返回非0.
	virtual int GetValueByKey(const char* sect, const char* key, char* s_value, int* n_value) = 0;

	virtual void Free() = 0;

protected:
	virtual ~ICuteINIConfig(){};
};

#endif