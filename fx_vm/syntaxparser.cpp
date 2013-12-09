#include "StdAfx.h"
#include "SyntaxParser.h"

WordType syntax_rules[] = {
	OP_LD, SU_X, OT_FAKE,
	OP_LD, SU_Y, OT_FAKE,
	OP_LD, SU_M, OT_FAKE,
	OP_LD, SU_S, OT_FAKE,
	OP_LD, SU_C, OT_FAKE,
	OP_LD, SU_T, OT_FAKE,
	OP_LD, SU_G, OT_FAKE,

	OP_LDI, SU_X, OT_FAKE,
	OP_LDI, SU_Y, OT_FAKE,
	OP_LDI, SU_M, OT_FAKE,
	OP_LDI, SU_S, OT_FAKE,
	OP_LDI, SU_C, OT_FAKE,
	OP_LDI, SU_T, OT_FAKE,
	OP_LDI, SU_G, OT_FAKE,

	OP_LDP, SU_X, OT_FAKE,
	OP_LDP, SU_Y, OT_FAKE,
	OP_LDP, SU_M, OT_FAKE,
	OP_LDP, SU_S, OT_FAKE,
	OP_LDP, SU_C, OT_FAKE,
	OP_LDP, SU_T, OT_FAKE,
	OP_LDP, SU_G, OT_FAKE,

	OP_LDF, SU_X, OT_FAKE,
	OP_LDF, SU_Y, OT_FAKE,
	OP_LDF, SU_M, OT_FAKE,
	OP_LDF, SU_S, OT_FAKE,
	OP_LDF, SU_C, OT_FAKE,
	OP_LDF, SU_T, OT_FAKE,
	OP_LDF, SU_G, OT_FAKE,

	OP_AND, SU_X, OT_FAKE,
	OP_AND, SU_Y, OT_FAKE,
	OP_AND, SU_M, OT_FAKE,
	OP_AND, SU_S, OT_FAKE,
	OP_AND, SU_C, OT_FAKE,
	OP_AND, SU_T, OT_FAKE,
	OP_AND, SU_G, OT_FAKE,

	OP_ANI, SU_X, OT_FAKE,
	OP_ANI, SU_Y, OT_FAKE,
	OP_ANI, SU_M, OT_FAKE,
	OP_ANI, SU_S, OT_FAKE,
	OP_ANI, SU_C, OT_FAKE,
	OP_ANI, SU_T, OT_FAKE,
	OP_ANI, SU_G, OT_FAKE,

	OP_ANDP, SU_X, OT_FAKE,
	OP_ANDP, SU_Y, OT_FAKE,
	OP_ANDP, SU_M, OT_FAKE,
	OP_ANDP, SU_S, OT_FAKE,
	OP_ANDP, SU_C, OT_FAKE,
	OP_ANDP, SU_T, OT_FAKE,
	OP_ANDP, SU_G, OT_FAKE,

	OP_ANDF, SU_X, OT_FAKE,
	OP_ANDF, SU_Y, OT_FAKE,
	OP_ANDF, SU_M, OT_FAKE,
	OP_ANDF, SU_S, OT_FAKE,
	OP_ANDF, SU_C, OT_FAKE,
	OP_ANDF, SU_T, OT_FAKE,
	OP_ANDF, SU_G, OT_FAKE,

	OP_OR, SU_X, OT_FAKE,
	OP_OR, SU_Y, OT_FAKE,
	OP_OR, SU_M, OT_FAKE,
	OP_OR, SU_S, OT_FAKE,
	OP_OR, SU_C, OT_FAKE,
	OP_OR, SU_T, OT_FAKE,
	OP_OR, SU_G, OT_FAKE,

	OP_ORI, SU_X, OT_FAKE,
	OP_ORI, SU_Y, OT_FAKE,
	OP_ORI, SU_M, OT_FAKE,
	OP_ORI, SU_S, OT_FAKE,
	OP_ORI, SU_C, OT_FAKE,
	OP_ORI, SU_T, OT_FAKE,
	OP_ORI, SU_G, OT_FAKE,

	OP_ORP, SU_X, OT_FAKE,
	OP_ORP, SU_Y, OT_FAKE,
	OP_ORP, SU_M, OT_FAKE,
	OP_ORP, SU_S, OT_FAKE,
	OP_ORP, SU_C, OT_FAKE,
	OP_ORP, SU_T, OT_FAKE,
	OP_ORP, SU_G, OT_FAKE,

	OP_ORF, SU_X, OT_FAKE,
	OP_ORF, SU_Y, OT_FAKE,
	OP_ORF, SU_M, OT_FAKE,
	OP_ORF, SU_S, OT_FAKE,
	OP_ORF, SU_C, OT_FAKE,
	OP_ORF, SU_T, OT_FAKE,
	OP_ORF, SU_G, OT_FAKE,

	OP_ANB, OT_FAKE,
	OP_ORB, OT_FAKE,

	OP_OUT, SU_Y, OT_FAKE,
	OP_OUT, SU_M, OT_FAKE,
	OP_OUT, SU_S, OT_FAKE,
	OP_OUT, SU_G, OT_FAKE,
	OP_OUT, SU_C, OT_H_NUM, OT_FAKE, 
	OP_OUT, SU_T, OT_H_NUM, OT_FAKE,
	OP_OUT, SU_C, OT_K_NUM, OT_FAKE,
	OP_OUT, SU_T, OT_K_NUM, OT_FAKE,

	OP_SET, SU_Y, OT_FAKE,
	OP_SET, SU_M, OT_FAKE,
	OP_SET, SU_S, OT_FAKE,

	OP_RST, SU_Y, OT_FAKE,
	OP_RST, SU_M, OT_FAKE,
	OP_RST, SU_S, OT_FAKE,
	OP_RST, SU_C, OT_FAKE,
	OP_RST, SU_T, OT_FAKE,
	OP_RST, SU_D, OT_FAKE,

	OP_MPS, OT_FAKE,
	OP_MRD, OT_FAKE,
	OP_MPP, OT_FAKE,

	OP_INV, OT_FAKE,
	OP_NOP, OT_FAKE
};

// 正则式描述规则. 
const char* syntax_rules_re[] = {
	// MOV的语法. 
	"^D?MOVP?$", "^K[0-9]+|H[0-9A-F]+|K[1-8]H[0-9A-F]+|K[1-8]Y|K[1-8]M|K[1-8]S|T|C|D$", "^K[1-8][YMS][0-9]+|T[0-9]+|[CD][0-9]+$", NULL,

	// CMP的语法. 
	"D?CMPP?", "^K[0-9]+|H[0-9A-F]+|K[1-8]X|K[1-8]Y|K[1-8]M|K[1-8]S|T|C|D$", "^[KH][0-9]+|K[1-8][XYMS][0-9]+|[TCD][0-9]+$", "^[XYMS][0-9]+$", NULL
};

SyntaxParser::SyntaxParser(void)
{
	std::pair<int, int> pr;
	
	int loop_count = sizeof(syntax_rules)/sizeof(syntax_rules[0]);
	int index = -1;
	
	for ( int i = 0; i < loop_count; i++ )
	{
		if ( index == -1 )
		{
			index = i;
		}
		if ( OT_FAKE == syntax_rules[i] )
		{
			pr.first = index;   // 起始位置. 
			pr.second = i - index;  // 长度. 
			m_rule_map.push_back(pr);
			index = -1;
		}
	}

	loop_count = sizeof(syntax_rules_re)/sizeof(syntax_rules_re[0]);
	index = -1;
	for ( int i = 0; i < loop_count; i++ )
	{
		if ( index == -1 )
		{
			index = i;
		}
		if ( NULL == syntax_rules_re[i] )
		{
			pr.first = index;
			pr.second = i - index;
			m_rule_map_re.push_back(pr);
			index = -1;
		}
	}
}

SyntaxParser::~SyntaxParser(void)
{
}

int SyntaxParser::CheckSyntax(const std::vector<WordParsed>& words_parsed, RunCode runcode, std::string& err_info)
{
	int retcode = 0;
	char buff[512] = { 0 };
	int words_count = words_parsed.size();
	int i = 0;
	while ( i < words_count )
	{
		bool matched;
		int most_got = words_count - i;
		
		for ( int j = 0; j < m_rule_map.size(); j++ ) // 试着匹配每一条语法规则. PART ONE. 指定简单的规则.
		{
			std::pair<int, int> pr = m_rule_map[j];
			if ( pr.second > most_got )  // 没有这么多word可用了. 
			{
				continue;
			}
			
			matched = true;
			for ( int k = 0; k < pr.second; k++ )
			{
				WordType left =  syntax_rules[pr.first+k];
				WordType right = words_parsed[i+k]._wt;
				if ( left != right )
				{
					matched = false;
					break;
				}
			}

			if ( matched )
			{
				if ( runcode )  // 如果设置了回调则向回调函数报告. 
				{
					std::vector<WordParsed> words;
					words.assign(words_parsed.begin()+i, words_parsed.begin()+i+pr.second);
					runcode(words);
				}
				i += pr.second;

				break;
			}
		} // end of for. 

		if ( !matched ) // 没有匹配简单规则.则尝试正则式规则. 
		{
			for ( int j = 0; j < m_rule_map_re.size(); j++ ) // 试着匹配每一条语法规则. PART ONE. 指定正则式指定的规则.
			{
				std::pair<int, int> pr = m_rule_map_re[j];
				if ( pr.second > most_got )  // 没有这么多word可用了. 
				{
					continue;
				}

				matched = true;
				for ( int k = 0; k < pr.second; k++ )
				{
					const char* pattern = syntax_rules_re[pr.first+k];
					const char* word = words_parsed[i+k]._wd.c_str();

					CRegexpT <char> regexp(pattern);
					MatchResult result = regexp.MatchExact(word);

					if( !result.IsMatched() )  // 不匹配软单元的命名约定. 
					{
						matched = false;
						break;
					}
				}

				if ( matched )
				{
					if ( runcode )  // 如果设置了回调则向回调函数报告. 
					{
						std::vector<WordParsed> words;
						words.assign(words_parsed.begin()+i, words_parsed.begin()+i+pr.second);
						runcode(words);
					}
					i += pr.second;

					break;
				}
			} // end of for. 
		}

		if ( !matched ) // 匹配每条规则都没成功.报告语法错误. 
		{
			sprintf(buff, "第 %d 行测试到语法错语", words_parsed[i]._line);
			err_info = buff;
			retcode = -1;
			goto _out;
		}
	} // end of while. 

_out:
	return retcode;
}
