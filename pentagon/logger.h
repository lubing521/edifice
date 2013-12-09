//////////////////////////////////////////////////////////////
///  @file
///  Designed to record log output. 
///  Comments on correctness and clarity are always welcome; 
///  However, comments on coding style will be tolerated.
///  written by zhouxiao@kingsoft.com Dec. 10 2008
///  
///  Enhancement:
///  Data:          Brief:                                 Author:
///  Mar. 12 2009   Simple XML-type log output was added.  zhouxiao 
////////////////////////////////////////////////////////////////////

#pragma once
#include "generic_helper.h"

// other head files...anything related to Intellectual Property are ignored.
#define XML_USE_TINY


//===============================================+++++++++++
/// ��###��ͷ���б�ʾ������һ�����󱻼�¼����.
/// ��===��ͷ���б�ʾ����lua�ű����
/// ���뺯��ʱ��ӡ "<" ���˳�ʱ��ӡ ">"   Deprecated !!! too awkward to use.
//==========================================================================

//#define LOG_ENT_F(...)   Logger::Instance().ent_f( __FUNCTIONW__, __VA_ARGS__)
//#define LOG_EXIT_F(...)  Logger::Instance().exit_f( __FUNCTIONW__, __VA_ARGS__)

/** ������־����
 */
class Logger
{
public:
	/// get singleton. 
	static Logger& Instance(const char* inifile);

	/// the following *put* functions return the number of characters written, 
	/// not including the terminating null character, or a negative value if an
	/// output error occurs. 

	/// ��ʽ�������txt��־...WITHOUT trailing new line
//	inline int wprintf(LPCWSTR format, ...);
	int printf(const char* format, ...);

	/// ��ʽ�������txt��־... WITH trailing new line
//	inline int wputfs(LPCWSTR format, ...);
	int putfs(const char* format, ...);

//	int _wvprintf(LPCWSTR format, va_list argptr);
	int _vprintf(const char* format, va_list argptr);
	
	/// xml��־.�Ӷ�һ��element��root element��name�����element�����֣�attrs�������element������.
	/// content�������element������.
	/// ������ _v variable length.�ɱ䳤��.
	/// �ɹ�ʱ����0�����������.
	//int append_elem_v(LPCWSTR name, std::map<std::wstring, std::wstring>* attrs, LPCWSTR content, ...);
	//int append_elem_v(LPCSTR name, std::map<std::string, std::string>* attrs, LPCSTR content, ...);
	//int append_elem(LPCWSTR name, std::map<std::wstring, std::wstring>* attrs, LPCWSTR content);
	//int append_elem(LPCSTR name, std::map<std::string, std::string>* attrs, LPCSTR content);


	// ˢ�»���
	int flush();
	 
private:
	// ֻ�����õ����ķ�ʽ������־��
	friend class std::auto_ptr<Logger>;
	/// ���������ļ���.���ڵ�ǰĿ¼��������ļ��Ҷ�ȡ�����������Ϣ
//	Logger(LPCWSTR cfgname);
	Logger(const char* cfgname);
	/// de-constructor
	~Logger(void);

	// txt��־
	BOOL           m_txt_on;        // �Ƿ�����log
	BOOL           m_quick_flush;   // �Ƿ����flush.
	BOOL           m_txt_append;    // ���´���־ʱ�Ƿ����־append�������ı������
	std::string    m_txt_logname;   // log�ļ�������
	FILE*          m_txt_logfile;   // a FILE pointer to the log file

	// xml��־
//	BOOL                m_xml_on;       // ����xml��־
//	std::wstring        m_xml_logname;  // xml log�ļ�������

//	TiXmlDocument       m_xml_doc;        // ����xml�ļ�.
//	IXMLDOMDocument*  m_xml_document;   // comes from msxml
//	IXMLDOMElement*   m_xml_root_elem;  // self-explanation

	static std::auto_ptr<Logger>  m_instance; // singleton 
	HANDLE   m_mutex;

private:
	/// ������˳�����ʱ�ļ���־�������Ű��thread-local storage�ͼ���Щtab������.
//	DWORD   m_tls_logfun_tc;  // �߳�specified. has not been used yet.
//	int     m_logfun_tc;   // Ŀǰ��Ҫ��ӡ�����tab
//	LPWSTR  m_tabs;        // �����Ű�log��־��tabs�����׵�ַ
//	int     m_tab_count;
};