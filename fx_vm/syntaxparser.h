////////////////////////////////////////////////////////////
/// 这个类是作语法分析的
//////////////////////////////////////////////////

#ifndef _H_SyntaxParser_
#define _H_SyntaxParser_

#include <vector>
#include "lexicalparser.h"

// 一个函数指针.该类型的函数接收一组Word，然后执行它. 
typedef int (*RunCode)(const std::vector<WordParsed>& words_parsed);

// 描述语法规则.每条规则由OT_FAKE隔开.  
extern WordType syntax_rules[];
// 描述语法规则.每条规则由NULL隔开,其它的string是正则式. 
extern const char* syntax_rules_re[];

class SyntaxParser
{
public:
	SyntaxParser(void);
	~SyntaxParser(void);

	int CheckSyntax(const std::vector<WordParsed>& words_parsed, RunCode runcode, std::string& err_info);

private:
	// 这个vector里存放了分析syntax_rules后每条rule在syntax_rules里的下标号以及所占的长度. 
	std::vector<std::pair<int, int>> m_rule_map;
	std::vector<std::pair<int, int>> m_rule_map_re;
};

#endif