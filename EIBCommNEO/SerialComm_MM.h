#ifndef _H_SERIALCOMM_MM_
#define _H_SERIALCOMM_MM_

#include "CommBase.h"

/** @brief 实现了CommBase.支持多主的串口通迅类. 
 */
class SerialComm_MM : public CommBase
{
public:
	SerialComm_MM(void);
	~SerialComm_MM(void);

	/// 打开串口等初始化工作. 
	int Init(const InitData& initdata);
	/// 反初始化. 
	void Uninit();

protected:

#ifdef __GNUC__
	// 这个函数给linux用.传入一个波特率，返回一个最接近的波特率的常量.用来设置串口设备. 
	uint GetBaundRateConst(uint baund_rate);
#endif

protected:
	virtual int _send_frame(const Frame* frame);
	// buff是缓冲首地址,buff_len传入时是缓冲的长度,传出时是缓冲使用的长度.成功时返回0.失败返回非0.  
	virtual int _recv_bytes(mem_t& mem);

private:
	volatile bool  m_inited;
	uint      m_time_cost_per_byte;  ///< 发送接收每个字节需要的大概时间. 

#ifndef __GNUC__
	HANDLE    m_hcom;       ///< COM端口的句柄.windows用. 
#else
	int       m_fd_comm;    ///< 打开COM端口后的文件描述符.linux用. 
#endif
};

#endif