#ifndef _H_MM_UDPCOMM_
#define _H_MM_UDPCOMM_

#include "commbase.h"

class UDPComm_MM : public CommBase
{
public:
	UDPComm_MM(void);
	~UDPComm_MM(void);

	// ������ʼ��UDPͨѶ��Ҫ����Դ.����UDPͨѶ�࣬����Ҫ�󶨵�IP��ַ�Ͷ˿ں�. 
	int Init(const InitData& initdata);
	/// ����ʼ��. 
	void Uninit();

protected:
	// ��һ��֡����. 
	virtual int _send_frame(const Frame* frame);
	// buff�ǻ����׵�ַ,buff_len����ʱ�ǻ���ĳ���,����ʱ�ǻ���ʹ�õĳ���.�ɹ�ʱ����0.ʧ�ܷ��ط�0.  
	virtual int _recv_bytes(mem_t& mem);

	// ��peer�˵�ַӳ�䵽IP��ַ��(ip��ַ�Ͷ˿ں�). 
	// ����ɹ���������0, ip��ӳ�䵽��ip��ַ, port��ӳ�䵽�Ķ˿ں�.ip��port�����Ѿ��������ֽ����õ�. 
	// ���ʧ����������non-zero. 
	int map_peer_addr2ip_pair(const uchar* peer_addr, uint& ip, ushort& port);

private:
	BOOL m_inited;

	uint   m_bound_ip;
	ushort m_port;
	SOCKET m_sockfd;
};

#endif
