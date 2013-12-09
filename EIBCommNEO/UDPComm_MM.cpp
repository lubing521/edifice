#include "stdafx.h"
#include "UDPComm_MM.h"

UDPComm_MM::UDPComm_MM(void)
{
	m_inited = FALSE;
	m_sockfd = INVALID_SOCKET;
}

UDPComm_MM::~UDPComm_MM(void)
{
	Uninit();
}

int UDPComm_MM::Init(const InitData& initdata)
{
	if (m_inited)
	{
		return -1;
	}
	
	int retcode;
	WSADATA wsaData;
	SOCKADDR_IN  addr;
	int nonblock;

	retcode = WSAStartup(MAKEWORD(2,2), &wsaData);
	if ( 0 != retcode )
	{
		goto _out;
	}

	m_bound_ip = initdata._udp_comm_initdata._bound_ip;
	m_port = initdata._udp_comm_initdata._port;

	// 开一个UDP socket.
	m_sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sockfd == INVALID_SOCKET)
	{
		retcode = WSAGetLastError();
		goto _out_clean;
	}
	
	// 把这个UDP socket设为非阻塞方式.这里就不会阻塞在_recv_bytes函数里.
	nonblock = 1;
	if (  SOCKET_ERROR == ioctlsocket(m_sockfd, FIONBIO, (unsigned long*)&nonblock) )  //设socket为非阻塞方式.
	{
		retcode = WSAGetLastError();
		goto _out_clean;
	}

	
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = m_bound_ip;
	addr.sin_port        = m_port;
	retcode = bind(m_sockfd, (struct sockaddr*) &addr, sizeof(addr));  // 绑好UDP地址.
	if ( retcode != 0 )
	{
		retcode = WSAGetLastError();
		goto _out_clean;
	}
	
	// 基类初始化. 
	retcode = CommBase::Init(initdata);
	if ( retcode != 0 )
	{
		goto _out_clean;
	}

	// 至此，初始化成功. 
	m_inited = TRUE;

_out:
	return retcode;

_out_clean:
	if ( m_sockfd != INVALID_SOCKET )
	{
		closesocket(m_sockfd);
		m_sockfd = INVALID_SOCKET;
	}
	WSACleanup();

	return retcode;
}

void UDPComm_MM::Uninit()
{
	CommBase::Uninit();

	if ( !m_inited )
	{
		return;
	}

	if ( m_sockfd != INVALID_SOCKET )
	{
		closesocket(m_sockfd);
		m_sockfd = INVALID_SOCKET;
	}

	WSACleanup();
}

int UDPComm_MM::_send_frame(const Frame* frame)
{
	int retcode;
	uint ip;
	ushort port;
	const uchar* frame_mem;
	uchar frame_len;

	// 从frame里拿到要发放的地址.可能是设备地址，也可能是组地址. 
	const uchar* peer_addr = frame->GetPeerAddr();
	retcode = map_peer_addr2ip_pair(peer_addr, ip, port);  // ip 和 port 已经是网络字节序. 
	if ( retcode != 0 )
	{
		return retcode;
	}

	frame->GetFrameMem(frame_mem, frame_len);

	SOCKADDR_IN  addr; // 要发往的地址. 
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = ip; // network order already.
	addr.sin_port        = port;
	for ( int pos = 0; pos < frame_len; )
	{
		// 晕,winsocket的sendto不支持MSG_WAITALL.
		int ret = sendto(m_sockfd, (char*)frame_mem + pos,  frame_len - pos, /*MSG_WAITALL*/0, (sockaddr*)&addr, sizeof(addr));
		if (ret == SOCKET_ERROR)
		{
			assert(0);
			log_me("sendto error occured.");
			retcode = WSAGetLastError();
			goto _out;
		}
		// frame都小于256，那肯定小于一个MTU啦。所以只需要一个UDP包呀！所以不可能会需要发送多次。
		assert( ret + pos == frame_len );
		pos += ret; // 应该不会到达这里. 
	}

	/// 成功把UDP包交给内核buffer了。
	retcode = 0;

_out:
	return retcode;
}

int UDPComm_MM::_recv_bytes(mem_t& mem)
{
	int retcode;
	int bytes_num;
	char buff[1024];

	mem.clear();
	
	// winsocket的recvfrom不支持MSG_DONTWAIT!!! 不过m_sockfd已经设成非阻塞模式了.
	retcode = recvfrom(m_sockfd, buff, sizeof(buff), /*MSG_DONTWAIT*/0, NULL, NULL);
	if (retcode == SOCKET_ERROR)
	{
		retcode -1;
	}

	bytes_num = retcode;
	if ( bytes_num > 0 )
	{
		mem.cp((uchar*)buff, bytes_num);
	}

	return 0;
}

int UDPComm_MM::map_peer_addr2ip_pair(const uchar* peer_addr, uint& ip, ushort& port)
{
	// UNDO. 把设备地址映射到IP地址的逻辑更改这个函数.
	// UNDO. 这里暂写死返回本机loop地址和从配置文件里用一个临时用的目的端口号. 
	// UNDO. 要根据不同的映射规则改写这里. 
	
	ip = inet_addr("127.0.0.1");
	port = htons(g_myconfig.GetTmpDestPort());

	return 0;
}