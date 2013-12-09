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
	// ���IniFile��·��.
	// ������ļ�ʧ�ܻ��ʽ��᷵�ط�0.���򷵻�0.
	int SetIniFilePath(const char* ini_file_path);
	
	// sect��SECTION�����֣�key����KEY������.
	// s_value���ַ�����ʽ����key��Ӧ��value.
	// n �������͵���ʽ����key��Ӧ��value.���s_value����ת������. 
	// ��� s_value �� n_value Ϊ NULL. �򲻻´��. 
	// �ɹ�����0, ʧ�ܷ��ط�0.
	int GetValueByKey(const char* sect, const char* key, char* s_value, int* n_value);

	CuteINIConfig(void);

protected:
	virtual void Free();

private:
	~CuteINIConfig(void);

private:
	FILE*       m_inifile;
	std::string m_filepath;

	typedef std::pair<std::string, std::string> KeyValueT;  // ������һ����ֵ��. 
	typedef std::vector<KeyValueT> KeyValueVec;             // ������ܶ����ֵ��. 
	std::map<std::string, KeyValueVec> m_keyvalues;         // ��ini�ļ�����������м���ֵ��.��map��key����section name. 
};

#endif