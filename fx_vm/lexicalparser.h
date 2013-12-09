////////////////////////////////////////////////////////////
/// 这个类是作词法分析的
//////////////////////////////////////////////////

#ifndef _H_LEXICALPARSER_
#define _H_LEXICALPARSER_

#include <string>
#include <vector>
#include "IEIBEmulator.h"
#include "deelx.h"

/// OP 表示 OPerator, SU 表示 SoftUnit， OT 表示 OThers, ER 表示 ERror. 
enum WordType
{
	OP_LD = 100,   /// 取. 
	OP_LDI,        /// 取反转. 
	OP_LDP,        /// 取脉冲上升沿. 
	OP_LDF,        /// 取脉冲下降沿. 
	OP_AND,        /// 与. 
	OP_ANI,        /// 与反转. 
	OP_ANDP,       /// 与脉冲上升沿. 
	OP_ANDF,       /// 与脉冲下降沿. 
	OP_OR,         /// 或. 
	OP_ORI,        /// 或反转. 
	OP_ORP,        /// 或脉冲上升沿. 
	OP_ORF,        /// 或脉冲下降沿. 
	OP_ANB,        /// 回路块与.         
	OP_ORB,        /// 回路块或.  
	OP_OUT,        /// 输出. 
	OP_SET,        /// 置位. 
	OP_RST,        /// 复位. 
	// OP_PLS,        /// 上升沿脉冲. 
	// OP_PLF,        /// 下降沿脉冲. 
	// OP_MC,         /// 主控. 
	// OP_MCR,        /// 主控复位. 
	OP_MPS,        /// 进栈. 
	OP_MRD,        /// 读栈. 
	OP_MPP,        /// 出栈. 
	OP_INV,        /// 反转. 
	OP_NOP,        /// 空操作. 
	OP_END,        /// 结束. 

	OP_MOV,
	OP_MOVP,
	OP_DMOV,
	OP_DMOVP,      /// 传输操作. 

	OP_CMP,
	OP_CMPP,
	OP_DCMP,
	OP_DCMPP,      /// 比较操作. 

	SU_X,          /// 输入软元件. 
	SU_Y,          /// 输出软元件. 
	SU_M,          /// 继电器软元件. 
	SU_S,          /// 状态软元件. 
	SU_C,          /// 计数器软元件. 
	SU_T,          /// 计时器软元件. 
	SU_D,          /// 数据寄存器软元件. 
	SU_G,          /// 组对象寄存器的软元件. 

	OT_K_NUM,      /// 指示期望下一个元素是十进制常数. 
	OT_H_NUM,      /// 指示期望下一个元素是十六进制常数. 
	OT_NUMBER,     /// 数字类型. 
	OT_OTHERS,     /// 乱七八糟的其它组合. 

	OT_FAKE,       /// 不是一个真正的类型.目前用来在syntaxparser.h中用来描述语法的间隔用. 

	ER_UNKNOWN     /// 无法识别. 
};

// 定义了各种WORD的类型的正则表达式. 
struct WordSpecif
{
	WordType    _wordtype;
	std::string _regex;
};

struct WordParsed 
{
	int         _line; ///< WORD出现在哪一行. 
	WordType    _wt;   ///< WORD的类型. 
	std::string _wd;   ///< WORD的真正值. 
};

class LexicalParser
{
public:
	LexicalParser(void);
	~LexicalParser(void);

	/// 把文本由参数txt传入,txt的每个元素表示一行. 
	/// 作词法分析，把分析到的每个词存下.函数返回0.  
	/// 若有词则从参数 err_info 传出错误信息.函数返回非0. 
	int ParseText(const std::vector<std::string>& txt, std::vector<WordParsed>& words, std::string& err_info);

	// 设置与软设备接口对象对接的接口指针. 
	void Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id);	

	WordType MapStr2WordType(const char* str);

private:
	IEIBEmulator* m_emulator;
	UINT  m_dev_id;
	BOOL  m_connected;
};

#endif