#ifndef _H_MM_UDPCOMM_
#define _H_MM_UDPCOMM_

#include "commbase.h"

class UDPComm_MM : public CommBase
{
public:
	UDPComm_MM(void);
	~UDPComm_MM(void);

	// 用来初始化UDP通讯需要的资源.对于UDP通讯类，传入要绑定的IP地址和端口号. 
	int Init(const InitData& initdata);
	/// 反初始化. 
	void Uninit();

protected:
	// 把一个帧发走. 
	virtual int _send_frame(const Frame* frame);
	// buff是缓冲首地址,buff_len传入时是缓冲的长度,传出时是缓冲使用的长度.成功时返回0.失败返回非0.  
	virtual int _recv_bytes(mem_t& mem);

	// 把peer端地址映射到IP地址对(ip地址和端口号). 
	// 如果成功函数返回0, ip记映射到的ip地址, port记映射到的端口号.ip和port都是已经用网络字节序存好的. 
	// 如果失败则函数返回non-zero. 
	int map_peer_addr2ip_pair(const uchar* peer_addr, uint& ip, ushort& port);

private:
	BOOL m_inited;

	uint   m_bound_ip;
	ushort m_port;
	SOCKET m_sockfd;
};

#endif
