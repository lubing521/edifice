//////////////////////////////////////////////////////////////
///  Designed to record log output. 
///  Comments on correctness and clarity are always welcome; 
///  However, comments on coding style will be tolerated.
///  written by zhouxiao@kingsoft.com Dec. 10 2008
////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include "Logger.h"
#include "pentagon.h"
#include "ICuteINIConfig.h"
#include "generic_helper.h"

std::auto_ptr<Logger> Logger::m_instance;

static const bool DEFAULT_TXT_LOG_ON     =   true;
static const bool DEFAULT_TXT_LOG_APPEND =   false;
static const char* DEFAULT_TXT_LOGNAME    =   "my.log";
static const int DEFAULT_TXT_MAXSZ      =   4096;

Logger::Logger(const char* cfgname) : m_txt_on(FALSE), m_txt_append(TRUE), m_txt_logfile(NULL)
{
	int  okay = 0;
	char buff[512] = { 0 };
	const char* config_name = NULL;

	if ( NULL == cfgname )
	{
		config_name = "log.ini";    // Ĭ�ϵ������ļ���
	}
	else
	{
		config_name = cfgname;
	}

	m_mutex = CreateMutex(NULL, FALSE, NULL);
	if ( NULL == m_mutex )
	{
		std::string failed_str = "CreateMutex failed in Logger::Logger.";
		report_err_immed(failed_str.c_str(), 0xf);
	}

//	ICuteINIConfig* m_config = dynamic_cast<ICuteINIConfig*>(BuildStemCell("ICuteINIConfig")); //�ɵĹ�������. 
	ICuteINIConfig* m_config = GetICuteINIConfig();
	okay = m_config->SetIniFilePath(config_name);
	if ( 0 != okay )
	{
		std::string err = std::string("Failed to read in inifile: ") + config_name; 
		report_err_immed(err.c_str(), 0xa);
	}

	okay = m_config->GetValueByKey("LOG", "on", NULL, &m_txt_on);
	if ( 0 != okay ) m_txt_on = DEFAULT_TXT_LOG_ON;

	okay = m_config->GetValueByKey("LOG", "quick_flush", NULL, &m_quick_flush);
	if ( 0 != okay ) m_quick_flush = 0;

	okay = m_config->GetValueByKey("LOG", "append", NULL, &m_txt_append);
	if ( 0 != okay ) m_txt_append = DEFAULT_TXT_LOG_APPEND;


	okay = m_config->GetValueByKey("LOG", "name", buff, NULL);
	if ( 0 == okay ) m_txt_logname = buff; else m_txt_logname = DEFAULT_TXT_LOGNAME;

	int maxsz = DEFAULT_TXT_MAXSZ;
	okay = m_config->GetValueByKey("LOG", "maxsz", NULL, &maxsz);

	m_config->Free();

	//// �Ƿ���xml��ʽ��־
	//GetPrivateProfileStringA( "XML_LOG", "on", "0", buff, sizeof(buff), cfgdir);
	//m_xml_on = (*buff != L'0');
	//// �õ�xml��ʽ��־�ļ����ļ���. 
	//GetPrivateProfileStringA( "XML_LOG", "filename", "sp.xml", buff, sizeof(buff), cfgdir);

	if ( m_txt_on )
	{
		m_txt_logfile = fopen( m_txt_logname.c_str(), "r" );  // �ȴ�ȡ�ļ���С
		if(m_txt_logfile) // ��Ϊ�Ѿ����ڸ��ļ�.�򿪳ɹ�.
		{
			fseek(m_txt_logfile, 0, SEEK_END);
			long filesz = ftell(m_txt_logfile);  // �õ��ļ��Ĵ�С
			if ( filesz>= maxsz )  // ��־�ļ���С��������
			{
				m_txt_append = FALSE;
			}
			fclose(m_txt_logfile);
			m_txt_logfile = NULL;
		}

//		const char* openmode = m_txt_append ?  "a,ccs=UTF-8" : "w,ccs=UTF-8";
		m_txt_logfile = NULL;
		const char* openmode = m_txt_append ?  "a" : "w";
		m_txt_logfile = fopen( m_txt_logname.c_str(), openmode );
		if ( NULL==m_txt_logfile )
		{
			std::string failed_str("Failed to open log file: ");
			failed_str += m_txt_logname;
			report_err_immed(failed_str.c_str(), 0xf);
		}
		else  // ��ӡʱ���
		{
			fprintf(m_txt_logfile, "TOTEM\n");
			memset(buff, 0, sizeof(buff));
			time_t t = time(NULL);
			fprintf(m_txt_logfile, "TIME STAMP: %s\n", ctime(&t));
		}
	}
	// �ͼ�¼ ���룬�˳����� ����־�йص��߼�
	//m_tab_count = 8;
	//m_tabs = new WCHAR[m_tab_count+1];
	//memset(m_tabs, L'\t', m_tab_count);
	//m_tabs[m_tab_count] = 0;
	//m_logfun_tc = 0;

	//if ( m_xml_on )  // ����xml��־����. 
	//{
	//	// �õ���ǰʱ��. 
	//	time_t calendar_time = time(NULL);
	//	char* time_str = ctime(&calendar_time);
	//	time_str && ( time_str[strlen(time_str)-1] = 0 );  // ȥ�� \n �ַ�

		// I was forbidden to use the MSXML way to parse xml.
		//CoInitialize(NULL);
		//HRESULT hr = m_xml_document.CreateInstance(CLSID_DOMDocument);
		//if ( FAILED(hr) || m_xml_document == NULL )
		//	throw 0;

		//// ��ӡ��ǰʱ�䵽xml��ͷ��. 
		//IXMLDOMCommentPtr time_stamp;
		//hr = m_xml_document->createComment(CA2W(time_str), &time_stamp);
		//m_xml_document->appendChild(time_stamp, NULL);

		//// create the root element(also known as the document element).
		//hr = m_xml_document->createElement(L"XML_LOG", &m_xml_root_elem);
		//if ( FAILED(hr) || m_xml_root_elem == NULL )
		//	throw 0;

		//hr = m_xml_document->appendChild(m_xml_root_elem, NULL);

		// The following code failed to work !
		/*KXMLManager m_xml_manager;*/
		//m_xml_manager.LoadFile(m_xml_logname.c_str());
		//m_xml_manager.GetRootNode();
		//m_xml_manager.SetAttribute(L"operation", L"AVQuery:Product-Name");
		//KXMLNode* tmpnode = NULL;
		//m_xml_manager.AppendNode(L"node", &tmpnode);
		//m_xml_manager.SetText(L"text", tmpnode);
		//m_xml_manager.SaveFile(m_xml_logname.c_str());

		// ʱ���. 
		//TiXmlComment * timestamp = new TiXmlComment(time_str); 
		//m_xml_doc.LinkEndChild(timestamp);

		//// ���������. 
		//TiXmlElement * root_elem = new TiXmlElement("MyNameIsRoot");
	//	m_xml_doc.LinkEndChild(root_elem);

		// testin code...
		//append_elem("one", NULL, "%d%s", 100, "text");
		//append_elem("two", NULL, "%d%s", 200, "TEXT");
		//append_elem("three", NULL, NULL);
		//append_elem(L"four", NULL, NULL);
//	}
}

//int Logger::append_elem_v(LPCSTR name, std::map<std::string, std::string>* attrs, LPCSTR content, ...)
//{
//	int retcode = 0;
	/*
	if ( m_xml_on )
	{
		TiXmlElement* root_elem = m_xml_doc.RootElement();
		TiXmlElement* nelem = new TiXmlElement(name);  
		root_elem->LinkEndChild(nelem);

		if ( NULL != attrs )
		{
			for( std::map<std::string, std::string>::iterator ite = attrs->begin(); 
				 ite != attrs->end();  
				 ++ ite )
			{
				nelem->SetAttribute(ite->first.c_str(), ite->second.c_str());
			}

		}

		char buff[2048] = { 0 };
		if ( NULL != content )
		{
			va_list ap;
			va_start(ap, content);

			vsprintf(buff, content, ap);
			//strcpy(buff, content);

			va_end(ap);
		}
		nelem->LinkEndChild(new TiXmlText(buff));
	}
*/
//	return retcode;
//}

//int Logger::append_elem_v(LPCWSTR name, std::map<std::wstring, std::wstring>* attrs, LPCWSTR content, ...)
//{
/*	std::map<std::string, std::string> ansi_attrs;
	
	if ( NULL != attrs )
	{
		for( std::map<std::wstring, std::wstring>::iterator ite = attrs->begin(); 
			ite != attrs->end();  
			++ ite )
		{
			CW2A first(ite->first.c_str());
			CW2A second(ite->second.c_str());
			ansi_attrs.insert( std::pair<std::string, std::string>(first.m_psz, second.m_psz) );
		}
	}

	WCHAR buff[1024] = { 0 };
	if ( NULL != content )
	{
		va_list ap;
		va_start(ap, content);
		_vsnwprintf(buff, sizeof(buff)/sizeof(WCHAR), content, ap);
		va_end(ap);
	}

//	return append_elem(CW2A(name), &ansi_attrs, CW2A(buff));*/
//	return 0;
//}

//int Logger::append_elem(const char* name, std::map<std::wstring, std::wstring>* attrs, LPCWSTR content)
//{
//	return append_elem_v(name, attrs, L"%s", content );
//}

//int Logger::append_elem(LPCSTR name, std::map<std::string, std::string>* attrs, LPCSTR content)
//{
//	return append_elem_v(name, attrs, "%s", content );
//}

int Logger::_vprintf(const char* format, va_list argptr)
{
	if ( ! m_txt_on )
	{
		return 0;
	}
	
	HoldTheMutex holder(m_mutex);

	int retcode = vfprintf(m_txt_logfile, format, argptr);
	const char* es = strerror(errno);
	if ( m_quick_flush )
	{
		flush();
	}

	return retcode;
}

int Logger::printf(const char* format, ...)
{
	int retcode = 0;

	if ( m_txt_on )
	{
		va_list ap;
		va_start(ap, format);
		retcode = _vprintf(format, ap);
		va_end(ap);
	}

	return retcode;
}

int Logger::putfs(const char* format, ...)
{
	int retcode = 0;

	if ( m_txt_on )
	{
		char buff[512] = { 0 };   // alas, magic number.
		strncpy(buff, format, 511);   // strict the magic numbers in a small area. 
		strcat(buff, "\n");

		va_list ap;
		va_start(ap, format);
		retcode = _vprintf(buff, ap);
		va_end(ap);
	}

	return retcode;
}

int Logger::flush()
{
	int retcode = 0;

	if ( m_txt_on )
	{
		retcode = fflush(m_txt_logfile);
	}

	return retcode;
}

Logger& Logger::Instance(const char* inifile)
{
	if ( NULL == m_instance.get() )
	{
		// �������߳�Ҫ����־.��ʵ�����и��ǳ�΢�����Դй©.�����ʷǳ��ǳ��ǳ��ǳ��ǳ���С.
		// й©��һ���Ҳ������ʲô����Ӱ��.�����˿�.�Ժ���ʱ���ٸ���
		// m_instance = new Logger();    // Can't use auto_ptr this way.
		m_instance.reset( new Logger( inifile ) );
	}

	return *m_instance;
}

Logger::~Logger(void)
{
	if ( m_txt_logfile )
	{
		fclose(m_txt_logfile);
	}

	if ( m_mutex )
	{
		CloseHandle(m_mutex);
	}

	//if ( m_xml_on )
	//{
	//	m_xml_doc.SaveFile(CW2A(m_xml_logname.c_str()));
	//}

//	delete []m_tabs;
}

/// ˢ���ļ����Ļ�����. 
/// �޷���ֵ.
//static int Flush(lua_State* L)
//{
//	LOG.flush();
//	return 0;
//}

/// ��һ����Ϣ��log
/// ����д����ַ�����������������.����ʱ���ظ�������
//static int Puts(lua_State* L)
//{
//	int retcode = 0;
//	ATLASSERT(lua_gettop(L)==1);
//	LPCSTR str = luaL_checkstring(L, 1);
//	CA2W w_str(str);
//	POPMSGBOX(w_str);
//	retcode = LOG.wputfs(w_str);
//	lua_pushnumber(L, retcode);
//	return 0;
//}
//
//static const struct luaL_Reg LogFunctions[] = {
//	{"Puts",  Puts},
//	{"Flush", Flush},
//
//	{NULL, NULL}
//};
//
//// call this function to register tar fcuntions to lua
//void RegisterLogTool(lua_State* L)
//{
//	luaL_register(L, "KLog", LogFunctions);
//}