#include "stdafx.h"
#include "pentagon.h"
#include "utilities.h"

#ifndef __GNUC__   // windows平台下的实现. 

int EnumFiles(const char* dir, const char* pattern, std::list<std::string>& results, bool insentive)
{
	int retcode = 0;

	WIN32_FIND_DATAA FindFileData;
	HANDLE   hFind = INVALID_HANDLE_VALUE;
	char     DirSpec[MAX_PATH]= { 0 };  // directory specification
	DWORD    dwError = 0;
	DWORD    dw1stSlash = 0;

	char _dir[MAX_PATH] = {};
	strcpy(_dir, dir);            // 保证 _dir 以 "\\" 结尾
	if( _dir[strlen(_dir)-1] != '\\' )
	{
		strcat(_dir, "\\"); 
	}

	strcpy(DirSpec, _dir);
	strcat(DirSpec, "*");

	// 这里用的是ATL的正则式的ANSI版本，这个版本的正则式遇到中文时有可能崩溃.所以不能用来找中文.
	// 得闲把这个正则式库改成public/目录下 deelx.h 里的正则式库. 
	CAtlRegExp<CAtlRECharTraitsA> re;         // The regular expression object
	CAtlREMatchContext<CAtlRECharTraitsA> mc; // The match context object
	re.Parse( pattern, insentive );                  // construct the matching pszPattern   

	hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		retcode = -1;
	} 
	else 
	{
		char buff[MAX_PATH] = {};
		do
		{
			memset(buff, 0, MAX_PATH);
			if ( 0 == stricmp(FindFileData.cFileName, ".")
				|| 0 == stricmp(FindFileData.cFileName, "..") )
			{
				continue;
			}

			if ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				continue;
			}

			if ( re.Match(FindFileData.cFileName, &mc) )
			{
				//wcscpy(buff, _dir);
				//wcscat(buff, FindFileData.cFileName);
				results.push_back(FindFileData.cFileName);
			}
		}while(FindNextFileA(hFind, &FindFileData) != 0);

		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			retcode = -2;
		}
	}

	return retcode;
}

#else

int EnumFiles(const char* dir, const char* pattern, std::list<std::string>&results, bool insentive)
{
	char cmd_line[512] = { 0 };
	char buff[512] = { 0 };
	char _dir[512] = { 0 };
	const char* format = "ls %s | grep -E%c \"%s\"";

	int dir_len = strlen(dir);
	strcpy(_dir, dir);
	if ( _dir[dir_len-1] == '\\' ) // 把windows路径转成unix风格路径. 
	{
		_dir[dir_len-1] = '/';
	}
	if ( _dir[dir_len-1] !=  '/' )
	{
		strcat(_dir, "/");
	}

	results.clear();

	if ( insentive )
		sprintf(cmd_line, format, _dir, ' ', pattern);
	else
		sprintf(cmd_line, format, _dir, 'i', pattern);

	FILE* stream = popen(cmd_line, "r");
	
	if ( NULL == stream )
	{
		return -1;
	}

	while ( fgets(buff, sizeof(buff), stream) )
	{
		trim_str(buff, " \r\n");
		results.push_back(buff);
	}
	pclose(stream);

	return 0;
}

#endif