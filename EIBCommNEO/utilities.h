#ifndef _H_UTILITIES_
#define _H_UTILITIES_

#include <string>
#include <list>

/// �����ļ���ȡ�ļ���.insentive��ʾ��ƥ���ַ���ʱ�Ƿ��Сд����.results�з����·��.
int EnumFiles(const char* dir, const char* pattern, std::list<std::string>& results, bool insentive);

#endif