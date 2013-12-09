#ifndef _H_MYCONFIG_
#define _H_MYCONFIG_

#include <string>
#include "ICuteINIConfig.h"
#include "Pentagon.h"

#define MY_CONFIG_INI_FILE "EIBCommNEO.ini"

/** @brief 读入配置文件. 
 */
class MyConfig
{
public:
	MyConfig(void);
	~MyConfig(void);

	/// 得到可读十六进制字符串形式的本机地址. 
	const std::string& GetHostAddr()
	{
		return m_cli_hostaddr;
	}

	/// 得到两个字节的地址.需要提供一个缓冲.返回值也是这个缓冲. 
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
	
	unsigned char m_addr_obj_index;  // 配置设备地址在哪个对象index.默认1. 
	unsigned char m_addr_prop_id;    // 配置设备地址在哪个属性id.默认5. 
	std::string   m_devices_dir;     // 软设备的描述文件放在哪个目录.默认是 devices\

	unsigned char m_serialnum_obj_index; // 放序列号的接口对象index,默认1. 
	unsigned char m_serialnum_prop_id;   // 放序列号的接口对象id,默认4. 

	UCHAR m_broadcast_addr[2];

	int m_tx_interval_time;   // 发线程里面的发送间隔时间. 

	int m_most_empty_in_row;  // 收线程里面连续多少次没有读到数据就开始分析缓冲. 

	int m_frame_life_time;    // 收线程里收到的帧.多长时间没有被取出就会自动被丢弃.单位秒.  

	int m_rx_timeout;   // 接收的超时时间.单位毫秒. 

	int m_rx_response_time;  // 一个帧发出去后到对方回复帧的最快响应时间.单位毫秒. 

	// 接收时是否检查帧里面存的地址.如果设备地址没在本地设置，或者是没被加入的本地址就忽略该帧.
	// 默认要检查地址信息. 
	int m_rx_check_address; 

	// UDP通讯时目的机的UDP端口号.这是临时测试用代码.以后要改写UDPComm_MM::map_peer_addr2ip_pair函数使设备EIB地址到
	// IP地址对建立映射关系. 
	ushort m_tmp_dest_port;

	//int m_log_on;
	//int m_log_append;
	//int m_log_quick_flush;
	//std::string	m_log_name;
};

extern MyConfig g_myconfig;

#endif