#ifndef _H_SERIALCOMM_MM_
#define _H_SERIALCOMM_MM_

#include "CommBase.h"

/** @brief ʵ����CommBase.֧�ֶ����Ĵ���ͨѸ��. 
 */
class SerialComm_MM : public CommBase
{
public:
	SerialComm_MM(void);
	~SerialComm_MM(void);

	/// �򿪴��ڵȳ�ʼ������. 
	int Init(const InitData& initdata);
	/// ����ʼ��. 
	void Uninit();

protected:

#ifdef __GNUC__
	// ���������linux��.����һ�������ʣ�����һ����ӽ��Ĳ����ʵĳ���.�������ô����豸. 
	uint GetBaundRateConst(uint baund_rate);
#endif

protected:
	virtual int _send_frame(const Frame* frame);
	// buff�ǻ����׵�ַ,buff_len����ʱ�ǻ���ĳ���,����ʱ�ǻ���ʹ�õĳ���.�ɹ�ʱ����0.ʧ�ܷ��ط�0.  
	virtual int _recv_bytes(mem_t& mem);

private:
	volatile bool  m_inited;
	uint      m_time_cost_per_byte;  ///< ���ͽ���ÿ���ֽ���Ҫ�Ĵ��ʱ��. 

#ifndef __GNUC__
	HANDLE    m_hcom;       ///< COM�˿ڵľ��.windows��. 
#else
	int       m_fd_comm;    ///< ��COM�˿ں���ļ�������.linux��. 
#endif
};

#endif