#include "StdAfx.h"
#include "LexicalParser.h"
#include "deelx.h"

WordSpecif WordsSpecs[] = {
	OP_LD,   "^LD$",
	OP_LDI,  "^LDI$", 
	OP_LDP,  "^LDP$",
	OP_LDF,  "^LDF$",
	OP_AND,  "^AND$",
	OP_ANI,  "^ANI$",
	OP_ANDP, "^ANDP$",
	OP_ANDF,  "^ANDF$",
	OP_OR,    "^OR$",
	OP_ORI,   "^ORI$",
	OP_ORP,   "^ORP$",
	OP_ORF,   "^ORF$",
	OP_ANB,   "^ANB$",     
	OP_ORB,   "^ORB$",
	OP_OUT,   "^OUT$",
	OP_SET,   "^SET$",
	OP_RST,   "^RST$",
	// OP_PLS,   "^PLS$",
	// OP_PLF,   "^PLF$",
	// OP_MC,         // 主控. 
	// OP_MCR,        // 主控复位. 
	OP_MPS,   "^MPS$",
	OP_MRD,   "^MRD$",
	OP_MPP,   "^MPP$",
	OP_INV,   "^INV$",
	OP_NOP,   "^NOP$",
	OP_END,   "^END$",
	
	OP_MOV,   "^MOV$",
	OP_MOVP,  "^MOVP$",
	OP_DMOV,  "^DMOV$",
	OP_DMOVP, "^DMOVP$",
	
	OP_CMP,   "^CMP$",
	OP_CMPP,  "^CMPP$",
	OP_DCMP,  "^DCMP$",
	OP_DCMPP, "^DCMPP$",

	SU_X,     "^X[0-9]{1,4}$",   // NOTICE.软单元的编号是否需要支持十六进制呢? 
	SU_Y,     "^Y[0-9]{1,4}$",
	SU_M,     "^M[0-9]{1,4}$",
	SU_S,     "^S[0-9]{1,4}$",
	SU_C,     "^C[0-9]{1,4}$",
	SU_T,     "^T[0-9]{1,4}$",
	SU_D,     "^D[0-9]{1,4}$",
	SU_G,     "^G[0-9]{1,4}$",

	OT_K_NUM, "^K[0-9]+$",
	OT_H_NUM, "^H[0-9A-F]+$",
	OT_NUMBER, "^[0-9A-F]+$",
	OT_OTHERS, "^K[1-9][XYMS][0-9]+|K[1-9]H[0-9A-F]+$"
};

LexicalParser::LexicalParser(void)
{
	m_dev_id = ~0;
	m_emulator = NULL;
	m_connected = FALSE;
}

LexicalParser::~LexicalParser(void)
{
}

void LexicalParser::Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id)
{
	m_emulator = emulator;
	m_dev_id = dev_id;
	
	if ( m_emulator && m_dev_id != ~0 )
		m_connected = TRUE;
	else
		m_connected = FALSE;
}

int LexicalParser::ParseText(const std::vector<std::string>& txt, std::vector<WordParsed>& words, std::string& err_info)
{
	int retcode = 0;
	char buff[512] = { 0 };
	char buff2[265] = { 0 };
	char buxx[64] = { 0 };
	char* sentinel;
	const char* seps = " \t\r\n";
	const char* word;
	WordType wt;
	WordParsed wp;

	words.clear();

	for ( int i = 0; i < txt.size(); i++ )
	{
		strncpy(buff, txt[i].c_str(), sizeof(buff));
		word = strtok_s(buff, seps, &sentinel);
		while ( NULL != word )
		{
			if ( 0 == *(word+1) && (*word == 'K' || *word == 'H'))  // 如果碰到单独的K或H，则合并后面一个.
			{
				char what = *word;
				buxx[0] = *word;
				word = strtok_s(NULL, seps, &sentinel);
				if ( NULL != word )
				{
					strcpy(buxx+1, word);
				}
				else
				{
					sprintf(buff, "第 %u 行 %c 后必须接相应的数字.", i+1, what);
					err_info = buff;
					retcode = -1;
					goto _out;
				}
				word = buxx;
			}

			wt = MapStr2WordType(word);
			BOOL legal_word = FALSE;

			if ( wt != ER_UNKNOWN )
			{
				legal_word = TRUE;
				if ( m_connected && ( SU_X == wt || SU_Y == wt || SU_M == wt || SU_S == wt || SU_C == wt 
					|| SU_T == wt || SU_D == wt || SU_G == wt ) )
				{
					if ( !m_emulator->DoesThisSoftUnitExist(m_dev_id, word) )
						legal_word = FALSE;
				}
				if ( legal_word )
				{
					wp._wd = word;
					wp._wt = wt;
					wp._line = i + 1;
					words.push_back(wp);
				}
			}
			
			if ( !legal_word )
			{
				sprintf(buff2, "第 %u 行碰到无法识别的标示符 %s.", i+1, word);
				err_info = buff2;
				retcode = -1;
				goto _out;
			}

			word = strtok_s(NULL, seps, &sentinel);
		} // end of while.  
	} // end of for. 

_out:
	return retcode;
}

WordType LexicalParser::MapStr2WordType(const char* str)
{
	if ( NULL == str )
	{
		return ER_UNKNOWN;
	}

	int loop_count = sizeof(WordsSpecs)/sizeof(WordsSpecs[0]);
	for ( int i = 0; i < loop_count; i++ )
	{
		CRegexpT <char> regexp(WordsSpecs[i]._regex.c_str());
		MatchResult result = regexp.MatchExact(str);
		if( result.IsMatched() )
		{
			return WordsSpecs[i]._wordtype;
		}
	}

	return ER_UNKNOWN;
}