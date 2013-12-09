////////////////////////////////////////////////////////////////////////
///  该类执行实际的代码.
//////////////////////////////////////////////////////////////////////

#ifndef _H_EXECUTOR_
#define _H_EXECUTOR_

#include <stack>
#include <map>
#include "syntaxparser.h"
#include "SoftUnit.h"
#include "IEIBEmulator.h"

struct _SoftUnitData
{
	char _id[8];   // 软单元的编号.  
	unsigned short _value;  // 软单元的值. 
};

class Executor
{
public:
	Executor(void);
	~Executor(void);

	int Run(const std::vector<WordParsed>& words_parsed);

	int Run_LD(const std::vector<WordParsed>& words_parsed);
	int Run_LDI(const std::vector<WordParsed>& words_parsed);
	int Run_LDP(const std::vector<WordParsed>& words_parsed);
	int Run_LDF(const std::vector<WordParsed>& words_parsed);
	
	int Run_AND(const std::vector<WordParsed>& words_parsed);
	int Run_ANI(const std::vector<WordParsed>& words_parsed);
	int Run_ANDP(const std::vector<WordParsed>& words_parsed);
	int Run_ANDF(const std::vector<WordParsed>& words_parsed);

	int Run_OR(const std::vector<WordParsed>& words_parsed);
	int Run_ORI(const std::vector<WordParsed>& words_parsed);
	int Run_ORP(const std::vector<WordParsed>& words_parsed);
	int Run_ORF(const std::vector<WordParsed>& words_parsed);

	int Run_ANB(const std::vector<WordParsed>& words_parsed);
	int Run_ORB(const std::vector<WordParsed>& words_parsed);

	int Run_OUT(const std::vector<WordParsed>& words_parsed);

	int Run_SET(const std::vector<WordParsed>& words_parsed);
	int Run_RST(const std::vector<WordParsed>& words_parsed);

	int Run_MPS(const std::vector<WordParsed>& words_parsed);
	int Run_MRD(const std::vector<WordParsed>& words_parsed);
	int Run_MPP(const std::vector<WordParsed>& words_parsed);

	int Run_INV(const std::vector<WordParsed>& words_parsed);
	int Run_NOP(const std::vector<WordParsed>& words_parsed);
	int Run_END(const std::vector<WordParsed>& words_parsed);

	int Run_MOV(const std::vector<WordParsed>& words_parsed);
	int Run_DMOV(const std::vector<WordParsed>& words_parsed);
	int Run_MOVP(const std::vector<WordParsed>& words_parsed);
	int Run_DMOVP(const std::vector<WordParsed>& words_parsed);

	int Run_CMP(const std::vector<WordParsed>& words_parsed);
	int Run_CMPP(const std::vector<WordParsed>& words_parsed);
	int Run_DCMP(const std::vector<WordParsed>& words_parsed);
	int Run_DCMPP(const std::vector<WordParsed>& words_parsed);

	/// 读出所有软单元的状态.成功时返回个数.
	/// type 标志要得到的软元件的类型. 可以是"X", "Y", "M", "S", "C", "T", "D". 如果为NULL,则返回所有的SoftUnit的值. 
	void ReadSoftUnitData(const char* type, std::vector<_SoftUnitData>& sunits);

	void Clear();

	void Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id);

private:
	// 传入软元件的类型和编号,返回SoftUnit.如X001.失败则返回NULL. 
	SoftUnit* GetSoftUnit(const char* code);

	// width 取 1, 2. 分别表示两个byte宽和四个byte宽. 
	// lvlORpulse 取 1 为电平触发，取 2 为脉冲触发. 
	int _Run_MOV(const std::vector<WordParsed>& words_parsed, int width, int lvlORpulse);
	int _Run_CMP(const std::vector<WordParsed>& words_parsed, int width, int lvlORpulse);

private:
	std::stack<UINT>  m_stack;
	std::stack<UINT>  m_stack4m;

	std::map<std::string, SoftUnit_X> m_map_x;
	std::map<std::string, SoftUnit_Y> m_map_y;
	std::map<std::string, SoftUnit_M> m_map_m;
	std::map<std::string, SoftUnit_S> m_map_s;
	std::map<std::string, SoftUnit_C> m_map_c;
	std::map<std::string, SoftUnit_T> m_map_t;
	std::map<std::string, SoftUnit_D> m_map_d;

	std::map<std::string, SoftUnit_G> m_map_g; // 组对象. 

	IEIBEmulator* m_emulator; // 用来对接到接口对象的数据. 
	UINT  m_dev_id;
};

#endif