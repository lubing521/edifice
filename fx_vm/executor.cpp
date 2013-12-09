#include "StdAfx.h"
#include "Executor.h"
#include "assert.h"
#include "Pentagon.h"

struct MapOp2Executor
{
	WordType _wt;
	int (Executor::*_run)(const std::vector<WordParsed>& words_parsed);
}mapOp2Executors[] = {
	OP_LD,  &Executor::Run_LD,
	OP_LDI, &Executor::Run_LDI,
	OP_LDP, &Executor::Run_LDP,
	OP_LDF, &Executor::Run_LDF,

	OP_AND, &Executor::Run_AND,
	OP_ANI, &Executor::Run_ANI,
	OP_ANDP, &Executor::Run_ANDP,
	OP_ANDF, &Executor::Run_ANDF,

	OP_OR, &Executor::Run_OR,
	OP_ORI, &Executor::Run_ORI,
	OP_ORP, &Executor::Run_ORP,
	OP_ORF, &Executor::Run_ORF,

	OP_ANB, &Executor::Run_ANB,
	OP_ORB, &Executor::Run_ORB,

	OP_OUT, &Executor::Run_OUT,

	OP_SET, &Executor::Run_SET,
	OP_RST, &Executor::Run_RST,

	OP_MPS, &Executor::Run_MPS,
	OP_MRD, &Executor::Run_MRD,
	OP_MPP, &Executor::Run_MPP,

	OP_INV, &Executor::Run_INV,
	OP_NOP, &Executor::Run_NOP,
	OP_END, &Executor::Run_END,

	OP_MOV, &Executor::Run_MOV,
	OP_MOVP, &Executor::Run_MOVP,
	OP_DMOV, &Executor::Run_DMOV,
	OP_DMOVP, &Executor::Run_DMOVP,     /// 传输操作. 

	OP_CMP, &Executor::Run_CMP,
	OP_CMPP, &Executor::Run_CMPP,
	OP_DCMP, &Executor::Run_DCMP,
	OP_DCMPP, &Executor::Run_DCMPP
}; 

SoftUnit* Executor::GetSoftUnit(const char* code)
{
	if ( NULL == code )
	{
		return NULL;
	}

	SoftUnit* retval = NULL;
	char softunit_type = code[0];

	int count_ori;
	int count_now;

	switch ( softunit_type )
	{
	case 'X':
		{
			count_ori = m_map_x.size();
			retval = &m_map_x[code];
			count_now = m_map_x.size();
			break;
		}

	case 'Y':
		{
			count_ori = m_map_y.size();
			retval = &m_map_y[code];
			count_now = m_map_y.size();
			break;
		}

	case 'M':
		{
			count_ori = m_map_m.size();
			retval = &m_map_m[code];
			count_now = m_map_m.size();
			break;
		}

	case 'S':
		{
			count_ori = m_map_s.size();
			retval = &m_map_s[code];
			count_now = m_map_s.size();
			break;
		}

	case 'T':
		{
			count_ori = m_map_t.size();
			retval = &m_map_t[code];
			count_now = m_map_t.size();
			break;
		}

	case 'C':
		{
			count_ori = m_map_c.size();
			retval = &m_map_c[code];
			count_now = m_map_c.size();
			break;
		}

	case 'G':
		{
			count_ori = m_map_g.size();
			retval = &m_map_g[code];
			count_now = m_map_g.size();
			break;
		}
	}

	if ( count_now > count_ori )  // 插入了新的SoftUnit.
	{
		if ( 0 == retval->Connect2SoftDev(m_emulator, m_dev_id) && 0 == retval->SetUnitName(code) )
			return retval;
		else
			return NULL;  // 检查到不能联接到接口对象的数据. 
	}
}

int Run(const std::vector<WordParsed>& words_parsed, Executor& exe)
{
	int retcode;

	retcode = exe.Run(words_parsed);
	
	return retcode;
}

Executor::Executor(void)
{
	m_emulator = NULL;
}

Executor::~Executor(void)
{
}

int Executor::Run(const std::vector<WordParsed>& words_parsed)
{
	int retcode = 0;
	int loop_count = sizeof(mapOp2Executors)/sizeof(MapOp2Executor);

	if ( words_parsed.empty() )
	{
		report_err_immed("Executor::Run are not expected to receive a empty vector.", 0xee);
	}

	for ( int i = 0; i < loop_count; i++ )
	{
		if ( mapOp2Executors[i]._wt == words_parsed[0]._wt )
		{
			retcode = (this->*(mapOp2Executors[i]._run))(words_parsed);
			return retcode;
		}		
	}

	report_err_immed("Executor::Run cannot handle this operation.", 0xee);
	// it's not expected to reach here.
	return retcode;
}

int Executor::Run_LD(const std::vector<WordParsed>& words_parsed)
{
	assert( 2 == words_parsed.size() && OP_LD == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());
	
	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		m_stack.push(state);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_LDI(const std::vector<WordParsed>& words_parsed)
{
	assert( 2 == words_parsed.size() && OP_LDI == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		m_stack.push(!state);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_LDP(const std::vector<WordParsed>& words_parsed)
{
	assert( 2 == words_parsed.size() && OP_LDP == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL previous_state = *soft_unit->GetSaveValue();
		BOOL the_value = FALSE;
		if ( state && !previous_state ) // 上升沿. 
		{
			the_value = TRUE;
		}
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_LDF(const std::vector<WordParsed>& words_parsed)
{
	assert( 2 == words_parsed.size() && OP_LDF == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL previous_state = *soft_unit->GetSaveValue();
		BOOL the_value = FALSE;
		if ( !state && previous_state ) // 下降沿. 
		{
			the_value = TRUE;
		}
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_AND(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_AND == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL top_stack = m_stack.top();
		//m_stack.pop();
		BOOL the_value = state && top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ANI(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_ANI == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL top_stack = m_stack.top();
		// m_stack.pop();
		BOOL the_value = !state && top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ANDP(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_ANDP == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL previous_state = *soft_unit->GetSaveValue();
		BOOL top_stack = m_stack.top();
		//m_stack.pop();
		BOOL the_value = !previous_state && state && top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ANDF(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_ANDP == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL previous_state = *soft_unit->GetSaveValue();
		BOOL top_stack = m_stack.top();
		//m_stack.pop();
		BOOL the_value = previous_state && !state && top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_OR(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_OR == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL top_stack = m_stack.top();
		// m_stack.pop();
		BOOL the_value = state || top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ORI(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_ORI == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL top_stack = m_stack.top();
		//m_stack.pop();
		BOOL the_value = !state || top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ORP(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_ORP == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL previous_state = *soft_unit->GetSaveValue();
		BOOL top_stack = m_stack.top();
		// m_stack.pop();
		BOOL the_value = !previous_state && state || top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ORF(const std::vector<WordParsed> &words_parsed)
{
	assert( 2 == words_parsed.size() && OP_ORF == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 1 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	std::string softunit_id = words_parsed[1]._wd;
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		BOOL state = *soft_unit->GetValue();
		BOOL previous_state = *soft_unit->GetSaveValue();
		BOOL top_stack = m_stack.top();
		// m_stack.pop();
		BOOL the_value = previous_state && !state || top_stack;
		m_stack.push(the_value);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_ANB(const std::vector<WordParsed> &words_parsed)
{
	assert( 1 == words_parsed.size() && OP_ANB == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 2 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	BOOL level_one = m_stack.top();
	m_stack.pop();
	BOOL level_two = m_stack.top();
	m_stack.pop();

	BOOL the_value = level_one && level_two;
	m_stack.push(the_value);

	retcode = 0;

	return retcode;
}

int Executor::Run_ORB(const std::vector<WordParsed> &words_parsed)
{
	assert( 1 == words_parsed.size() && OP_ORB == words_parsed[0]._wt );

	char buff[512];
	int retcode = -1;

	if ( m_stack.size() < 2 )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	BOOL level_one = m_stack.top();
	m_stack.pop();
	BOOL level_two = m_stack.top();
	m_stack.pop();

	BOOL the_value = level_one || level_two;
	m_stack.push(the_value);

	retcode = 0;

	return retcode;
}

int Executor::Run_OUT(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() >= 2 && OP_OUT == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	UCHAR softunit_type = words_parsed[1]._wd.at(0);
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		USHORT trigger_point;
		UCHAR state = m_stack.top();
		m_stack.pop();
		soft_unit->SetValue(&state);

		if ( 'C' == softunit_type || 'T' == softunit_type )
		{
			std::string num = words_parsed[2]._wd.c_str() + 1;
			if ( 'K' == words_parsed[2]._wd[0] )
			{
				trigger_point = strtol(num.c_str(), NULL, 10);
			}
			else if ( 'H' == words_parsed[2]._wd[0] )
			{
				trigger_point = strtol(num.c_str(), NULL, 16);
			}

			if ( 'C' == softunit_type )  // 计数器. 
			{
				SoftUnit_C* su = (SoftUnit_C*)soft_unit;
				su->SetTriggerPoint(trigger_point);
			}
			else // 计时器. 
			{
				SoftUnit_T* su = (SoftUnit_T*)soft_unit;
				su->SetTriggerPoint(trigger_point);
			}

			retcode = 0;
		}
		else
		{
			retcode = 0;
		}
	}

	return retcode;
}

int Executor::Run_SET(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 2 && OP_SET == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	UCHAR softunit_type = words_parsed[1]._wd.at(0);
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());
	SoftUnit_1Bit* soft_unit_1bit = dynamic_cast<SoftUnit_1Bit*>(soft_unit);

	if ( NULL != soft_unit && NULL != soft_unit_1bit )
	{
		soft_unit_1bit->SetState(TRUE);
		retcode = 0;
	}

	return retcode;
}

int Executor::Run_RST(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 2 && OP_RST == words_parsed[0]._wt );

	int retcode = -1;
	std::string softunit_id = words_parsed[1]._wd;
	UCHAR softunit_type = words_parsed[1]._wd.at(0);
	SoftUnit* soft_unit = GetSoftUnit(softunit_id.c_str());

	if ( NULL != soft_unit )
	{
		switch ( softunit_type )
		{
		case 'Y':
		case 'M':
		case 'S':
			{
				SoftUnit_1Bit* su_1bit = dynamic_cast<SoftUnit_1Bit*>(soft_unit);
				su_1bit->SetState(FALSE);
				break;
			}
		
		case 'T':
			{
				SoftUnit_T* su_t = dynamic_cast<SoftUnit_T*>(soft_unit);
				su_t->Reset();
				break;
			}

		case 'C':
			{
				SoftUnit_C* su_c = dynamic_cast<SoftUnit_C*>(soft_unit);
				su_c->Reset();
				break;
			}

		case 'D':
			{
				UCHAR buff[2] = { 0 };
				soft_unit->SetValue(buff);
				break;
			}
		}  // end of switch. 
	}

	return retcode;
}

int Executor::Run_MPS(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 1 && OP_MPS == words_parsed[0]._wt );
	char buff[256];
	if ( 0 == m_stack.size() )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}
	
	UINT at_top = m_stack.top();
	m_stack4m.push(at_top);

	return 0;
}

int Executor::Run_MRD(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 1 && OP_MRD == words_parsed[0]._wt );
	char buff[256];
	if ( 0 == m_stack4m.size() )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	// 实际效果把工作栈栈顶的元素修改.
	UINT at_top = m_stack4m.top();
	m_stack.pop();
	m_stack.push(at_top);

	return 0;
}

int Executor::Run_MPP(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 1 && OP_MPP == words_parsed[0]._wt );
	char buff[256];
	if ( 0 == m_stack.size() )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	UINT at_top = m_stack4m.top();
	m_stack4m.pop();
	m_stack.pop();
	m_stack.push(at_top);

	return 0;
}

int Executor::Run_INV(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 1 && OP_INV == words_parsed[0]._wt );
	char buff[256];
	if ( 0 == m_stack.size() )
	{
		sprintf(buff, "第 %d 行运行时错误.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	UINT at_top = m_stack.top();
	at_top ^= 1;
	//m_stack.pop();
	m_stack.push(at_top);

	return 0;
}

int Executor::Run_CMP(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_CMP(words_parsed, 1, 1); // 2个byte宽，电平触发. 
	return retcode;
}

int Executor::Run_CMPP(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_CMP(words_parsed, 1, 2); // 2个byte宽，脉冲触发. 
	return retcode;
}

int Executor::Run_DCMP(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_CMP(words_parsed, 2, 1); // 4个byte宽，电平触发. 
	return retcode;
}

int Executor::Run_DCMPP(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_CMP(words_parsed, 2, 2); // 4个byte宽，脉冲触发. 
	return retcode;
}

int Executor::_Run_CMP(const std::vector<WordParsed>& words_parsed, int width, int lvlORpulse)
{
	int retcode = 0;
	assert( words_parsed.size() == 4 );
	char buff[256] = { 0 };

	// UNDO. 增加判断何时才执行这个语句的逻辑. 

	CRegexpT<char> regexp;
	MatchResult result;

	char softunit_name[16];
	UINT softunit_num;

	UINT values[2];
	UINT widths[2];  // width 是以四bits为单位的. 
	std::string words[2] = { words_parsed[1]._wd, words_parsed[2]._wd };
	for ( int idx = 0; idx < 2; idx++ ) // false infinite while. just to take advantage of the break clause.
	{
		regexp.Compile("K[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{
			values[idx] = strtol(words[idx].c_str()+1, NULL, 10);
			widths[idx] = 0; // 0表示自适应长度. 
			continue;
		}

		regexp.Compile("H[0-9A-F]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			values[idx] = strtol(words[idx].c_str()+1, NULL, 16);
			widths[idx] = 0;
			continue;
		}

		regexp.Compile("K[1-8]X[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(words[idx].c_str()+3, NULL, 10);
			widths[idx] = *(words[idx].c_str()+1) - '0';
			values[idx] = 0;
			for ( int i = 0; i < widths[idx]*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "X%u", su_num);
				SoftUnit_X* su = dynamic_cast<SoftUnit_X*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 1;
				values[idx] |= su->GetState();
			}
			continue;
		}

		regexp.Compile("K[1-8]Y[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(words[idx].c_str()+3, NULL, 10);
			widths[idx] = *(words[idx].c_str()+1) - '0';
			values[idx] = 0;
			for ( int i = 0; i < widths[idx]*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "Y%u", su_num);
				SoftUnit_Y* su = dynamic_cast<SoftUnit_Y*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 1;
				values[idx] |= su->GetState();
			}
			continue;
		}

		regexp.Compile("K[1-8]M[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(words[idx].c_str()+3, NULL, 10);
			widths[idx] = *(words[idx].c_str()+1) - '0';
			values[idx] = 0;
			for ( int i = 0; i < widths[idx]*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "M%u", su_num);
				SoftUnit_M* su = dynamic_cast<SoftUnit_M*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 1;
				values[idx] |= su->GetState();
			}
			continue;
		}

		regexp.Compile("K[1-8]S[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			softunit_num = strtol(words[idx].c_str()+3, NULL, 10);
			widths[idx] = *(words[idx].c_str()+1) - '0';
			values[idx] = 0;
			for ( int i = 0; i < widths[idx]*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "S%u", su_num);
				SoftUnit_S* su = dynamic_cast<SoftUnit_S*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 1;
				values[idx] |= su->GetState();
			}
			continue;
		}

		regexp.Compile("T[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_T* su = dynamic_cast<SoftUnit_T*>(GetSoftUnit(words[idx].c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words[idx].c_str());
				report_err_immed(buff, 0xaa);
			}
			values[idx] = su->GetCountValue();

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(words[idx].c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "T%u", su_num);
				SoftUnit_T* su = dynamic_cast<SoftUnit_T*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words[idx].c_str());
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 16;
				values[idx] |= su->GetCountValue();
			}
			widths[idx] = width * 4;
			continue;
		}

		regexp.Compile("C[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			SoftUnit_C* su = dynamic_cast<SoftUnit_C*>(GetSoftUnit(words[idx].c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words[idx].c_str());
				report_err_immed(buff, 0xaa);
			}
			values[idx] = su->GetCountValue();

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(words[idx].c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "C%u", su_num);
				SoftUnit_C* su = dynamic_cast<SoftUnit_C*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words[idx].c_str());
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 16;
				values[idx] |= su->GetCountValue();
			}
			widths[idx] = width * 4;
			continue;
		}

		regexp.Compile("D[0-9]+");
		result = regexp.MatchExact(words[idx].c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_D* su = dynamic_cast<SoftUnit_D*>(GetSoftUnit(words[idx].c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words[idx].c_str());
				report_err_immed(buff, 0xaa);
			}
			values[idx] = *(USHORT*)su->GetValue();

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(words[idx].c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "D%u", su_num);
				SoftUnit_D* su = dynamic_cast<SoftUnit_D*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words[idx].c_str());
					report_err_immed(buff, 0xaa);
				}
				values[idx] <<= 16;
				values[idx] |= *(USHORT*)su->GetValue();
			}
			widths[idx] = width * 4;
			continue;
		}

		sprintf(buff, "第%d行运行时错误.没有匹配到任何规则.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}  // end of while.

	while ( true ) // fake infinite loop.
	{
		if ( widths[0] != 0 && widths[1] != 0 && widths[0] != widths[1] )
		{
			sprintf(buff, "第%d行运行时错误.要比较的类型不匹配.", words_parsed[0]._line);
			report_err_immed(buff, 0xaa);
		}

		std::string out_unitname = words_parsed[3]._wd;
		BOOL equal = ( values[0] == values[1] );  // UNDO. 是这样吧。相等时真值写入D软单元。 

		regexp.Compile("Y[0-9]+");
		result = regexp.MatchExact(out_unitname.c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_Y* su = dynamic_cast<SoftUnit_Y*>(GetSoftUnit(softunit_name));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words_parsed[3]._wd.c_str());
				report_err_immed(buff, 0xaa);
			}
			su->SetState(equal);
			break;
		}

		regexp.Compile("M[0-9]+");
		result = regexp.MatchExact(out_unitname.c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_M* su = dynamic_cast<SoftUnit_M*>(GetSoftUnit(softunit_name));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words_parsed[3]._wd.c_str());
				report_err_immed(buff, 0xaa);
			}
			su->SetState(equal);
			break;
		}

		regexp.Compile("S[0-9]+");
		result = regexp.MatchExact(out_unitname.c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_S* su = dynamic_cast<SoftUnit_S*>(GetSoftUnit(softunit_name));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, words_parsed[3]._wd.c_str());
				report_err_immed(buff, 0xaa);
			}
			su->SetState(equal);
			break;
		}

		sprintf(buff, "第%d行运行时错误.没有匹配到任何规则.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}

	return retcode;
}

int Executor::Run_MOV(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_MOV(words_parsed, 1, 1); // 2个byte宽，电平触发. 
	return retcode;
}

int Executor::Run_DMOV(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_MOV(words_parsed, 2, 1);
	return retcode;
}

int Executor::Run_MOVP(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_MOV(words_parsed, 1, 2);
	return retcode;
}

int Executor::Run_DMOVP(const std::vector<WordParsed>& words_parsed)
{
	int retcode;
	retcode = _Run_MOV(words_parsed, 2, 2);
	return retcode;
}

int Executor::_Run_MOV(const std::vector<WordParsed>& words_parsed, int width, int lvlORpulse)
{
	int retcode = 0;
	assert( words_parsed.size() == 3 );
	//	const char* _reg_exp_s[] = { , , , , , , , ,  };
	//	const char* _reg_exp_d[] = { "K[1-8]Y[0-9]+", "K[1-8]M[0-9]+", "K[1-8]S[0-9]+", "T[0-9]+", "C[0-9]+", "D[0-9]+" };

	// UNDO. 增加判断何时才执行这个语句的逻辑. 
	char buff[512];
		
	char softunit_name[16];
	UINT softunit_num;

	UINT left_value;
	UINT left_width;  // width 是以四bits为单位的. 
	std::string left_word = words_parsed[1]._wd;
	while ( true ) // false infinite while. just to take advantage of the break clause.
	{
		CRegexpT<char> regexp;
		MatchResult result;

		regexp.Compile("K[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			left_value = strtol(left_word.c_str()+1, NULL, 10);
			left_width = 0; // 0表示自适应长度. 
			break;
		}

		regexp.Compile("H[0-9A-F]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			left_value = strtol(left_word.c_str()+1, NULL, 16);
			left_width = 0;
			break;
		}

		regexp.Compile("K[1-8]H[0-9A-F]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			left_value = strtol(left_word.c_str()+3, NULL, 16);
			left_width = *(left_word.c_str()+1) - '0';
			break;
		}

		regexp.Compile("K[1-8]Y[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(left_word.c_str()+3, NULL, 10);
			left_width = *(left_word.c_str()+1) - '0';
			left_value = 0;
			for ( int i = 0; i < left_width*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "Y%u", su_num);
				SoftUnit_Y* su = dynamic_cast<SoftUnit_Y*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				left_value <<= 1;
				left_value |= su->GetState();
			}
			break;
		}

		regexp.Compile("K[1-8]M[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(left_word.c_str()+3, NULL, 10);
			left_width = *(left_word.c_str()+1) - '0';
			left_value = 0;
			for ( int i = 0; i < left_width*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "M%u", su_num);
				SoftUnit_M* su = dynamic_cast<SoftUnit_M*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				left_value <<= 1;
				left_value |= su->GetState();
			}
			break;
		}

		regexp.Compile("K[1-8]S[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			softunit_num = strtol(left_word.c_str()+3, NULL, 10);
			left_width = *(left_word.c_str()+1) - '0';
			left_value = 0;
			for ( int i = 0; i < left_width*4; i++ )
			{
				UINT su_num = softunit_num + i;
				sprintf(softunit_name, "S%u", su_num);
				SoftUnit_S* su = dynamic_cast<SoftUnit_S*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				left_value <<= 1;
				left_value |= su->GetState();
			}
			break;
		}

		regexp.Compile("T[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_T* su = dynamic_cast<SoftUnit_T*>(GetSoftUnit(left_word.c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
				report_err_immed(buff, 0xaa);
			}
			left_value = su->GetCountValue();

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(left_word.c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "T%u", su_num);
				SoftUnit_T* su = dynamic_cast<SoftUnit_T*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
					report_err_immed(buff, 0xaa);
				}
				left_value <<= 16;
				left_value |= su->GetCountValue();
			}
			left_width = width * 4;
			break;
		}

		regexp.Compile("C[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			SoftUnit_C* su = dynamic_cast<SoftUnit_C*>(GetSoftUnit(left_word.c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
				report_err_immed(buff, 0xaa);
			}
			left_value = su->GetCountValue();

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(left_word.c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "C%u", su_num);
				SoftUnit_C* su = dynamic_cast<SoftUnit_C*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
					report_err_immed(buff, 0xaa);
				}
				left_value <<= 16;
				left_value |= su->GetCountValue();
			}
			left_width = width * 4;
			break;
		}

		regexp.Compile("D[0-9]+");
		result = regexp.MatchExact(left_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			SoftUnit_D* su = dynamic_cast<SoftUnit_D*>(GetSoftUnit(left_word.c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
				report_err_immed(buff, 0xaa);
			}
			left_value = *(USHORT*)su->GetValue();

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(left_word.c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "D%u", su_num);
				SoftUnit_D* su = dynamic_cast<SoftUnit_D*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
					report_err_immed(buff, 0xaa);
				}
				left_value <<= 16;
				left_value |= *(USHORT*)su->GetValue();
			}
			left_width = width * 4;
			break;
		}

		sprintf(buff, "第%d行运行时错误.没有匹配到任何规则.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}  // end of while.

	UINT right_value;
	UINT right_width;  // width 是以四bits为单位的. 
	std::string right_word = words_parsed[2]._wd;
	while ( true ) // false infinite while. just to take advantage of the break clause.
	{
		CRegexpT<char> regexp;
		MatchResult result;

		regexp.Compile("K[1-8]Y[0-9]+");
		result = regexp.MatchExact(right_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(right_word.c_str()+3, NULL, 10);
			right_width = *(right_word.c_str()+1) - '0';
			if ( left_width != 0 || left_width != right_width )
			{
				sprintf(buff, "第 %d 行运行时错误.左右操作数的类型不匹配.", words_parsed[0]._line);
				report_err_immed(buff, 0xaa);				
			}
			for ( int i = 0; i < right_width*4; i++ )
			{
				UINT su_num = softunit_num + ( right_width*4 - i - 1 );
				sprintf(softunit_name, "Y%u", su_num);
				SoftUnit_Y* su = dynamic_cast<SoftUnit_Y*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				right_value = (left_value >> i);
				right_value &= 0x1;
				su->SetState(right_value);
			}
			break;
		}

		regexp.Compile("K[1-8]M[0-9]+");
		result = regexp.MatchExact(right_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			softunit_num = strtol(right_word.c_str()+3, NULL, 10);
			right_width = *(right_word.c_str()+1) - '0';
			if ( left_width != 0 || left_width != right_width )
			{
				sprintf(buff, "第 %d 行运行时错误.左右操作数的类型不匹配.", words_parsed[0]._line);
				report_err_immed(buff, 0xaa);				
			}
			for ( int i = 0; i < right_width*4; i++ )
			{
				UINT su_num = softunit_num + ( right_width*4 - i - 1 );
				sprintf(softunit_name, "M%u", su_num);
				SoftUnit_M* su = dynamic_cast<SoftUnit_M*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				right_value = (left_value >> i);
				right_value &= 0x1;
				su->SetState(right_value);
			}
			break;
		}

		regexp.Compile("K[1-8]S[0-9]+");
		result = regexp.MatchExact(right_word.c_str());
		if( result.IsMatched() ) // do something. 
		{ 
			softunit_num = strtol(right_word.c_str()+3, NULL, 10);
			right_width = *(right_word.c_str()+1) - '0';
			if ( left_width != 0 || left_width != right_width )
			{
				sprintf(buff, "第 %d 行运行时错误.左右操作数的类型不匹配.", words_parsed[0]._line);
				report_err_immed(buff, 0xaa);				
			}
			for ( int i = 0; i < right_width*4; i++ )
			{
				UINT su_num = softunit_num + ( right_width*4 - i - 1 );
				sprintf(softunit_name, "S%u", su_num);
				SoftUnit_S* su = dynamic_cast<SoftUnit_S*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第 %d 行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, softunit_name);
					report_err_immed(buff, 0xaa);
				}
				right_value = (left_value >> i);
				right_value &= 0x1;
				su->SetState(right_value);
			}
			break;
		}

		regexp.Compile("T[0-9]+");
		result = regexp.MatchExact(right_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			right_width = 4 * width;
			if ( left_width != 0 || left_width != right_width )
			{
				sprintf(buff, "第 %d 行运行时错误.左右操作数的类型不匹配.", words_parsed[0]._line);
				report_err_immed(buff, 0xaa);				
			}
			SoftUnit_T* su = dynamic_cast<SoftUnit_T*>(GetSoftUnit(left_word.c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
				report_err_immed(buff, 0xaa);
			}
			USHORT tmp = left_value;
			su->SetCountValue(tmp);

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(left_word.c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "T%u", su_num);
				SoftUnit_T* su = dynamic_cast<SoftUnit_T*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
					report_err_immed(buff, 0xaa);
				}
				tmp = (left_value >> 16);
				su->SetCountValue(tmp);
			}
			break;
		}

		regexp.Compile("C[0-9]+");
		result = regexp.MatchExact(right_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			right_width = 4 * width;
			if ( left_width != 0 || left_width != right_width )
			{
				sprintf(buff, "第 %d 行运行时错误.左右操作数的类型不匹配.", words_parsed[0]._line);
				report_err_immed(buff, 0xaa);				
			}
			SoftUnit_C* su = dynamic_cast<SoftUnit_C*>(GetSoftUnit(left_word.c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
				report_err_immed(buff, 0xaa);
			}
			USHORT tmp = left_value;
			su->SetCountValue(tmp);

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(left_word.c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "C%u", su_num);
				SoftUnit_C* su = dynamic_cast<SoftUnit_C*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
					report_err_immed(buff, 0xaa);
				}
				tmp = (left_value >> 16);
				su->SetCountValue(tmp);
			}
			break;
		}

		regexp.Compile("D[0-9]+");
		result = regexp.MatchExact(right_word.c_str());
		if( result.IsMatched() ) // do something. 
		{
			right_width = 4 * width;
			if ( left_width != 0 || left_width != right_width )
			{
				sprintf(buff, "第 %d 行运行时错误.左右操作数的类型不匹配.", words_parsed[0]._line);
				report_err_immed(buff, 0xaa);				
			}
			SoftUnit_D* su = dynamic_cast<SoftUnit_D*>(GetSoftUnit(left_word.c_str()));
			if ( NULL == su )
			{
				sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
				report_err_immed(buff, 0xaa);
			}
			USHORT tmp = left_value;
			su->SetValue((UCHAR*)&tmp);

			if ( 2 == width )  // 32位的运算.要取两次. 
			{
				softunit_num = strtol(left_word.c_str()+1, NULL, 10);
				UINT su_num = softunit_num + 1;
				sprintf(softunit_name, "C%u", su_num);
				SoftUnit_D* su = dynamic_cast<SoftUnit_D*>(GetSoftUnit(softunit_name));
				if ( NULL == su )
				{
					sprintf(buff, "第%d行运行时错误.软元件%s没有被映射到接口对象.", words_parsed[0]._line, left_word.c_str());
					report_err_immed(buff, 0xaa);
				}
				tmp = (left_value >> 16);
				su->SetValue((UCHAR*)&tmp);
			}
			break;
		}

		sprintf(buff, "第%d行运行时错误.没有匹配到任何规则.", words_parsed[0]._line);
		report_err_immed(buff, 0xaa);
	}  // end of while.

	return retcode;
}

int Executor::Run_NOP(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 1 && OP_NOP == words_parsed[0]._wt );
	UINT u = 0;
	for(int i = 0; i<0xff; i++)
	{
		// 无意的操作. just to waste some cpu time.
		int tmp = i;
		tmp++;
		tmp <<= 2;
		u += tmp;
	}

	return 0;
}

int Executor::Run_END(const std::vector<WordParsed>& words_parsed)
{
	assert( words_parsed.size() == 1 && OP_END == words_parsed[0]._wt );
	UINT u = 0;
	for(int i = 0; i<0xff; i++)
	{
		// 无意的操作. just to waste some cpu time.
		int tmp = i;
		tmp++;
		tmp <<= 2;
		u += tmp;
	}

	return 0;
}

void Executor::ReadSoftUnitData(const char* type, std::vector<_SoftUnitData>& sunits)
{
	char stype;
	if ( NULL != type )
	{
		stype = toupper(*type);
	}

	_SoftUnitData data;

	if ( NULL == type || stype == 'X' )
	{
		for ( std::map<std::string, SoftUnit_X>::iterator ite = m_map_x.begin();
			  ite != m_map_x.end();
			  ++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 1);
			sunits.push_back(data);
		}
	}

	if ( NULL == type || stype == 'Y' )
	{
		for ( std::map<std::string, SoftUnit_Y>::iterator ite = m_map_y.begin();
			ite != m_map_y.end();
			++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 1);
			sunits.push_back(data);
		}		
	}

	if ( NULL == type || stype == 'M' )
	{
		for ( std::map<std::string, SoftUnit_M>::iterator ite = m_map_m.begin();
			ite != m_map_m.end();
			++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 1);
			sunits.push_back(data);
		}
	}

	if ( NULL == type || stype == 'S' )
	{
		for ( std::map<std::string, SoftUnit_S>::iterator ite = m_map_s.begin();
			ite != m_map_s.end();
			++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 1);
			sunits.push_back(data);
		}
	}

	if ( NULL == type || stype == 'C' )
	{
		for ( std::map<std::string, SoftUnit_C>::iterator ite = m_map_c.begin();
			ite != m_map_c.end();
			++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 2);
			sunits.push_back(data);
		}
	}

	if ( NULL == type || stype == 'T' )
	{
		for ( std::map<std::string, SoftUnit_T>::iterator ite = m_map_t.begin();
			ite != m_map_t.end();
			++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 2);
			sunits.push_back(data);
		}
	}

	if ( NULL == type || stype == 'D' )
	{
		for ( std::map<std::string, SoftUnit_D>::iterator ite = m_map_d.begin();
			ite != m_map_d.end();
			++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 2);
			sunits.push_back(data);
		}
	}

	if ( NULL == type || stype == 'G' )
	{
		for ( std::map<std::string, SoftUnit_G>::iterator ite = m_map_g.begin();
			  ite != m_map_g.end();
			  ++ite )
		{
			strcpy(data._id, ite->first.c_str());
			memcpy(&data._value, ite->second.GetValue(), 2);
			sunits.push_back(data);
		}
	}
}

void Executor::Clear()
{
	while ( !m_stack.empty() )
	{
		m_stack.pop();
	}

	while ( !m_stack4m.empty() )
	{
		m_stack4m.pop();
	}

	m_map_x.clear();
	m_map_y.clear();
	m_map_m.clear();
	m_map_s.clear();
	m_map_c.clear();
	m_map_t.clear();
	m_map_d.clear();
	m_map_g.clear();
}

void Executor::Connect2SoftDev(IEIBEmulator *emulator, UINT dev_id)
{
	m_dev_id = dev_id;
	m_emulator = emulator;
}