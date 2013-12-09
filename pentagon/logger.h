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
/// 用###起头的行表示出现了一个错误被记录下来.
/// 用===起头的行表示运行lua脚本相关
/// 进入函数时打印 "<" ，退出时打印 ">"   Deprecated !!! too awkward to use.
//==========================================================================

//#define LOG_ENT_F(...)   Logger::Instance().ent_f( __FUNCTIONW__, __VA_ARGS__)
//#define LOG_EXIT_F(...)  Logger::Instance().exit_f( __FUNCTIONW__, __VA_ARGS__)

/** 管理日志的类
 */
class Logger
{
public:
	/// get singleton. 
	static Logger& Instance(const char* inifile);

	/// the following *put* functions return the number of characters written, 
	/// not including the terminating null character, or a negative value if an
	/// output error occurs. 

	/// 格式化输出到txt日志...WITHOUT trailing new line
//	inline int wprintf(LPCWSTR format, ...);
	int printf(const char* format, ...);

	/// 格式化输出到txt日志... WITH trailing new line
//	inline int wputfs(LPCWSTR format, ...);
	int putfs(const char* format, ...);

//	int _wvprintf(LPCWSTR format, va_list argptr);
	int _vprintf(const char* format, va_list argptr);
	
	/// xml日志.加多一个element到root element，name是这个element的名字，attrs存了这个element的属性.
	/// content是这个新element的内容.
	/// 函数名 _v variable length.可变长度.
	/// 成功时返回0，否则错误码.
	//int append_elem_v(LPCWSTR name, std::map<std::wstring, std::wstring>* attrs, LPCWSTR content, ...);
	//int append_elem_v(LPCSTR name, std::map<std::string, std::string>* attrs, LPCSTR content, ...);
	//int append_elem(LPCWSTR name, std::map<std::wstring, std::wstring>* attrs, LPCWSTR content);
	//int append_elem(LPCSTR name, std::map<std::string, std::string>* attrs, LPCSTR content);


	// 刷新缓冲
	int flush();
	 
private:
	// 只充许用单件的方式访问日志类
	friend class std::auto_ptr<Logger>;
	/// 传入配置文件名.会在当前目录下找这个文件且读取里面的配置信息
//	Logger(LPCWSTR cfgname);
	Logger(const char* cfgname);
	/// de-constructor
	~Logger(void);

	// txt日志
	BOOL           m_txt_on;        // 是否启用log
	BOOL           m_quick_flush;   // 是否快速flush.
	BOOL           m_txt_append;    // 重新打开日志时是否把日志append到已有文本的最后
	std::string    m_txt_logname;   // log文件的名字
	FILE*          m_txt_logfile;   // a FILE pointer to the log file

	// xml日志
//	BOOL                m_xml_on;       // 启用xml日志
//	std::wstring        m_xml_logname;  // xml log文件的名字

//	TiXmlDocument       m_xml_doc;        // 操作xml文件.
//	IXMLDOMDocument*  m_xml_document;   // comes from msxml
//	IXMLDOMElement*   m_xml_root_elem;  // self-explanation

	static std::auto_ptr<Logger>  m_instance; // singleton 
	HANDLE   m_mutex;

private:
	/// 进入和退出函数时的记日志，方便排版的thread-local storage和记了些tab的数组.
//	DWORD   m_tls_logfun_tc;  // 线程specified. has not been used yet.
//	int     m_logfun_tc;   // 目前需要打印几多个tab
//	LPWSTR  m_tabs;        // 用来排版log日志的tabs数组首地址
//	int     m_tab_count;
};