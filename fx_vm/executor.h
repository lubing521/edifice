////////////////////////////////////////////////////////////////////////
///  ����ִ��ʵ�ʵĴ���.
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
	char _id[8];   // ��Ԫ�ı��.  
	unsigned short _value;  // ��Ԫ��ֵ. 
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

	/// ����������Ԫ��״̬.�ɹ�ʱ���ظ���.
	/// type ��־Ҫ�õ�����Ԫ��������. ������"X", "Y", "M", "S", "C", "T", "D". ���ΪNULL,�򷵻����е�SoftUnit��ֵ. 
	void ReadSoftUnitData(const char* type, std::vector<_SoftUnitData>& sunits);

	void Clear();

	void Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id);

private:
	// ������Ԫ�������ͺͱ��,����SoftUnit.��X001.ʧ���򷵻�NULL. 
	SoftUnit* GetSoftUnit(const char* code);

	// width ȡ 1, 2. �ֱ��ʾ����byte����ĸ�byte��. 
	// lvlORpulse ȡ 1 Ϊ��ƽ������ȡ 2 Ϊ���崥��. 
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

	std::map<std::string, SoftUnit_G> m_map_g; // �����. 

	IEIBEmulator* m_emulator; // �����Խӵ��ӿڶ��������. 
	UINT  m_dev_id;
};

#endif