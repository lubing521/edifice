////////////////////////////////////////////////////////////
/// ����������ʷ�������
//////////////////////////////////////////////////

#ifndef _H_LEXICALPARSER_
#define _H_LEXICALPARSER_

#include <string>
#include <vector>
#include "IEIBEmulator.h"
#include "deelx.h"

/// OP ��ʾ OPerator, SU ��ʾ SoftUnit�� OT ��ʾ OThers, ER ��ʾ ERror. 
enum WordType
{
	OP_LD = 100,   /// ȡ. 
	OP_LDI,        /// ȡ��ת. 
	OP_LDP,        /// ȡ����������. 
	OP_LDF,        /// ȡ�����½���. 
	OP_AND,        /// ��. 
	OP_ANI,        /// �뷴ת. 
	OP_ANDP,       /// ������������. 
	OP_ANDF,       /// �������½���. 
	OP_OR,         /// ��. 
	OP_ORI,        /// ��ת. 
	OP_ORP,        /// ������������. 
	OP_ORF,        /// �������½���. 
	OP_ANB,        /// ��·����.         
	OP_ORB,        /// ��·���.  
	OP_OUT,        /// ���. 
	OP_SET,        /// ��λ. 
	OP_RST,        /// ��λ. 
	// OP_PLS,        /// ����������. 
	// OP_PLF,        /// �½�������. 
	// OP_MC,         /// ����. 
	// OP_MCR,        /// ���ظ�λ. 
	OP_MPS,        /// ��ջ. 
	OP_MRD,        /// ��ջ. 
	OP_MPP,        /// ��ջ. 
	OP_INV,        /// ��ת. 
	OP_NOP,        /// �ղ���. 
	OP_END,        /// ����. 

	OP_MOV,
	OP_MOVP,
	OP_DMOV,
	OP_DMOVP,      /// �������. 

	OP_CMP,
	OP_CMPP,
	OP_DCMP,
	OP_DCMPP,      /// �Ƚϲ���. 

	SU_X,          /// ������Ԫ��. 
	SU_Y,          /// �����Ԫ��. 
	SU_M,          /// �̵�����Ԫ��. 
	SU_S,          /// ״̬��Ԫ��. 
	SU_C,          /// ��������Ԫ��. 
	SU_T,          /// ��ʱ����Ԫ��. 
	SU_D,          /// ���ݼĴ�����Ԫ��. 
	SU_G,          /// �����Ĵ�������Ԫ��. 

	OT_K_NUM,      /// ָʾ������һ��Ԫ����ʮ���Ƴ���. 
	OT_H_NUM,      /// ָʾ������һ��Ԫ����ʮ�����Ƴ���. 
	OT_NUMBER,     /// ��������. 
	OT_OTHERS,     /// ���߰�����������. 

	OT_FAKE,       /// ����һ������������.Ŀǰ������syntaxparser.h�����������﷨�ļ����. 

	ER_UNKNOWN     /// �޷�ʶ��. 
};

// �����˸���WORD�����͵�������ʽ. 
struct WordSpecif
{
	WordType    _wordtype;
	std::string _regex;
};

struct WordParsed 
{
	int         _line; ///< WORD��������һ��. 
	WordType    _wt;   ///< WORD������. 
	std::string _wd;   ///< WORD������ֵ. 
};

class LexicalParser
{
public:
	LexicalParser(void);
	~LexicalParser(void);

	/// ���ı��ɲ���txt����,txt��ÿ��Ԫ�ر�ʾһ��. 
	/// ���ʷ��������ѷ�������ÿ���ʴ���.��������0.  
	/// ���д���Ӳ��� err_info ����������Ϣ.�������ط�0. 
	int ParseText(const std::vector<std::string>& txt, std::vector<WordParsed>& words, std::string& err_info);

	// ���������豸�ӿڶ���ԽӵĽӿ�ָ��. 
	void Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id);	

	WordType MapStr2WordType(const char* str);

private:
	IEIBEmulator* m_emulator;
	UINT  m_dev_id;
	BOOL  m_connected;
};

#endif