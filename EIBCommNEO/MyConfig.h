#ifndef _H_MYCONFIG_
#define _H_MYCONFIG_

#include <string>
#include "ICuteINIConfig.h"
#include "Pentagon.h"

#define MY_CONFIG_INI_FILE "EIBCommNEO.ini"

/** @brief ���������ļ�. 
 */
class MyConfig
{
public:
	MyConfig(void);
	~MyConfig(void);

	/// �õ��ɶ�ʮ�������ַ�����ʽ�ı�����ַ. 
	const std::string& GetHostAddr()
	{
		return m_cli_hostaddr;
	}

	/// �õ������ֽڵĵ�ַ.��Ҫ�ṩһ������.����ֵҲ���������. 
	UCHAR* GetHostAddr_b(UCHAR* buff, int buff_len)
	{
		if ( NULL != buff && buff_len >= 2 )
		{
			char hexs[5] = { 0 };
			memcpy(hexs, m_cli_hostaddr.c_str(), 4);

			hexs2bins(hexs, buff, &buff_len);
		}

		return buff;
	}

	void SetHostAddr(const std::string& host_addr)
	{
		// std::string save = m_cli_hostaddr;
		m_cli_hostaddr = host_addr;
		// return save;
	}

	void SetHostAddr_b(const UCHAR* hostaddr_b)
	{
		if ( NULL != hostaddr_b )
		{
			char s[5] = { 0 };
			bins2hexs(hostaddr_b, 2, s, sizeof(s));
			m_cli_hostaddr = s;
		}
	}

	const std::string& GetPeerAddr()
	{
		return m_cli_peeraddr;
	}

	UCHAR* GetPeerAddr_b(UCHAR* buff, int buff_len)
	{
		if ( NULL != buff && buff_len >= 2 )
		{
			char hexs[5] = { 0 };
			memcpy(hexs, m_cli_peeraddr.c_str(), 4);

			hexs2bins(hexs, buff, &buff_len);
		}

		return buff;
	}

	void SetPeerAddr(const std::string& peer_addr)
	{
		// std::string save = m_cli_hostaddr;
		m_cli_peeraddr = peer_addr;
		// return save;
	}

	void SetPeerAddr_b(const UCHAR* peeraddr_b)
	{
		if ( NULL != peeraddr_b )
		{
			char s[5] = { 0 };
			bins2hexs(peeraddr_b, 2, s, sizeof(s));
			m_cli_peeraddr = s;
		}
	}

	unsigned char GetAddrObjIndex()
	{
		return m_addr_obj_index;
	}

	unsigned char GetAddrPropID()
	{
		return m_addr_prop_id;
	}

	unsigned char GetSerialNumObjIndex()
	{
		return m_serialnum_obj_index;
	}

	unsigned char GetSerialNumPropID()
	{
		return m_serialnum_prop_id;
	}

	const std::string& GetDevicesDir()
	{
		return m_devices_dir;
	}

	bool UseFaultyAPCI();

	bool UseFaultyAuthorize();

	const uchar* GetBroadcastAddr()
	{
		return m_broadcast_addr;
	}

	int GetTxIntervalTime()
	{
		return m_tx_interval_time;
	}

	int GetMostEmptyInRow()
	{
		return m_most_empty_in_row;
	}

	int GetFrameLifeTime()
	{
		return m_frame_life_time;
	}

	int GetRxTimeOut()
	{
		return m_rx_timeout;
	}

	int GetRxResponseTime()
	{
		return m_rx_response_time;
	}

	bool DoesRxNeedCheckAddress()
	{
		return !!m_rx_check_address;
	}

	ushort GetTmpDestPort()
	{
		return m_tmp_dest_port;
	}

	void Init(const char* ini_file);

	//const std::string& GetPeerAddr()
	//{
	//	return m_cli_peeraddr;
	//}

	//bool IsLogOn()
	//{
	//	return !!m_log_on;
	//}

	//bool DoesLogAppend()
	//{
	//	return !!m_log_append;
	//}

	//bool DoesLogQuickFlush()
	//{
	//	return !!m_log_quick_flush;
	//}

	//const std::string& GetLogName()
	//{
	//	return m_log_name;
	//}

private:
	BOOL  m_inited;

//	ICuteINIConfig* m_config;

	std::string  m_cli_hostaddr;
	std::string  m_cli_peeraddr;

	int m_use_faulty_apci;
	int m_use_faulty_authorize;
	
	unsigned char m_addr_obj_index;  // �����豸��ַ���ĸ�����index.Ĭ��1. 
	unsigned char m_addr_prop_id;    // �����豸��ַ���ĸ�����id.Ĭ��5. 
	std::string   m_devices_dir;     // ���豸�������ļ������ĸ�Ŀ¼.Ĭ���� devices\

	unsigned char m_serialnum_obj_index; // �����кŵĽӿڶ���index,Ĭ��1. 
	unsigned char m_serialnum_prop_id;   // �����кŵĽӿڶ���id,Ĭ��4. 

	UCHAR m_broadcast_addr[2];

	int m_tx_interval_time;   // ���߳�����ķ��ͼ��ʱ��. 

	int m_most_empty_in_row;  // ���߳������������ٴ�û�ж������ݾͿ�ʼ��������. 

	int m_frame_life_time;    // ���߳����յ���֡.�೤ʱ��û�б�ȡ���ͻ��Զ�������.��λ��.  

	int m_rx_timeout;   // ���յĳ�ʱʱ��.��λ����. 

	int m_rx_response_time;  // һ��֡����ȥ�󵽶Է��ظ�֡�������Ӧʱ��.��λ����. 

	// ����ʱ�Ƿ���֡�����ĵ�ַ.����豸��ַû�ڱ������ã�������û������ı���ַ�ͺ��Ը�֡.
	// Ĭ��Ҫ����ַ��Ϣ. 
	int m_rx_check_address; 

	// UDPͨѶʱĿ�Ļ���UDP�˿ں�.������ʱ�����ô���.�Ժ�Ҫ��дUDPComm_MM::map_peer_addr2ip_pair����ʹ�豸EIB��ַ��
	// IP��ַ�Խ���ӳ���ϵ. 
	ushort m_tmp_dest_port;

	//int m_log_on;
	//int m_log_append;
	//int m_log_quick_flush;
	//std::string	m_log_name;
};

extern MyConfig g_myconfig;

#endif