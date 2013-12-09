#ifndef _H_ICUTEINICONFIG_
#define _H_ICUTEINICONFIG_

////////////////////////////////////////////////////////////////////////
/// �������������д�����ļ�
//////////////////////////////////////////////////////////////////////////////

class ICuteINIConfig
{
public:
	// ���IniFile��·��.
	// ������ļ�ʧ�ܻ��ʽ��᷵�ط�0.���򷵻�0.
	virtual int SetIniFilePath(const char* ini_file_path) = 0;

	// sect��SECTION�����֣�key����KEY������.
	// s_value���ַ�����ʽ����key��Ӧ��value.
	// n �������͵���ʽ����key��Ӧ��value.���s_value����ת������. 
	// �ɹ�����0, ʧ�ܷ��ط�0.
	virtual int GetValueByKey(const char* sect, const char* key, char* s_value, int* n_value) = 0;

	virtual void Free() = 0;

protected:
	virtual ~ICuteINIConfig(){};
};

#endif