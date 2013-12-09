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

	// ��һ��UDP socket.
	m_sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sockfd == INVALID_SOCKET)
	{
		retcode = WSAGetLastError();
		goto _out_clean;
	}
	
	// �����UDP socket��Ϊ��������ʽ.����Ͳ���������_recv_bytes������.
	nonblock = 1;
	if (  SOCKET_ERROR == ioctlsocket(m_sockfd, FIONBIO, (unsigned long*)&nonblock) )  //��socketΪ��������ʽ.
	{
		retcode = WSAGetLastError();
		goto _out_clean;
	}

	
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = m_bound_ip;
	addr.sin_port        = m_port;
	retcode = bind(m_sockfd, (struct sockaddr*) &addr, sizeof(addr));  // ���UDP��ַ.
	if ( retcode != 0 )
	{
		retcode = WSAGetLastError();
		goto _out_clean;
	}
	
	// �����ʼ��. 
	retcode = CommBase::Init(initdata);
	if ( retcode != 0 )
	{
		goto _out_clean;
	}

	// ���ˣ���ʼ���ɹ�. 
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

	// ��frame���õ�Ҫ���ŵĵ�ַ.�������豸��ַ��Ҳ���������ַ. 
	const uchar* peer_addr = frame->GetPeerAddr();
	retcode = map_peer_addr2ip_pair(peer_addr, ip, port);  // ip �� port �Ѿ��������ֽ���. 
	if ( retcode != 0 )
	{
		return retcode;
	}

	frame->GetFrameMem(frame_mem, frame_len);

	SOCKADDR_IN  addr; // Ҫ�����ĵ�ַ. 
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = ip; // network order already.
	addr.sin_port        = port;
	for ( int pos = 0; pos < frame_len; )
	{
		// ��,winsocket��sendto��֧��MSG_WAITALL.
		int ret = sendto(m_sockfd, (char*)frame_mem + pos,  frame_len - pos, /*MSG_WAITALL*/0, (sockaddr*)&addr, sizeof(addr));
		if (ret == SOCKET_ERROR)
		{
			assert(0);
			log_me("sendto error occured.");
			retcode = WSAGetLastError();
			goto _out;
		}
		// frame��С��256���ǿ϶�С��һ��MTU��������ֻ��Ҫһ��UDP��ѽ�����Բ����ܻ���Ҫ���Ͷ�Ρ�
		assert( ret + pos == frame_len );
		pos += ret; // Ӧ�ò��ᵽ������. 
	}

	/// �ɹ���UDP�������ں�buffer�ˡ�
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
	
	// winsocket��recvfrom��֧��MSG_DONTWAIT!!! ����m_sockfd�Ѿ���ɷ�����ģʽ��.
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
	// UNDO. ���豸��ַӳ�䵽IP��ַ���߼������������.
	// UNDO. ������д�����ر���loop��ַ�ʹ������ļ�����һ����ʱ�õ�Ŀ�Ķ˿ں�. 
	// UNDO. Ҫ���ݲ�ͬ��ӳ������д����. 
	
	ip = inet_addr("127.0.0.1");
	port = htons(g_myconfig.GetTmpDestPort());

	return 0;
}