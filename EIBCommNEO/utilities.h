#ifndef _H_UTILITIES_
#define _H_UTILITIES_

#include <string>
#include <list>

/// 遍历文件夹取文件名.insentive表示是匹配字符串时是否大小写敏感.results中放相对路径.
int EnumFiles(const char* dir, const char* pattern, std::list<std::string>& results, bool insentive);

#endif