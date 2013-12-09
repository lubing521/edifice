////////////////////////////////////////////////////////////
/// ����������﷨������
//////////////////////////////////////////////////

#ifndef _H_SyntaxParser_
#define _H_SyntaxParser_

#include <vector>
#include "lexicalparser.h"

// һ������ָ��.�����͵ĺ�������һ��Word��Ȼ��ִ����. 
typedef int (*RunCode)(const std::vector<WordParsed>& words_parsed);

// �����﷨����.ÿ��������OT_FAKE����.  
extern WordType syntax_rules[];
// �����﷨����.ÿ��������NULL����,������string������ʽ. 
extern const char* syntax_rules_re[];

class SyntaxParser
{
public:
	SyntaxParser(void);
	~SyntaxParser(void);

	int CheckSyntax(const std::vector<WordParsed>& words_parsed, RunCode runcode, std::string& err_info);

private:
	// ���vector�����˷���syntax_rules��ÿ��rule��syntax_rules����±���Լ���ռ�ĳ���. 
	std::vector<std::pair<int, int>> m_rule_map;
	std::vector<std::pair<int, int>> m_rule_map_re;
};

#endif