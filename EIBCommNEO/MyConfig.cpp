#include "stdafx.h"
#include "MyConfig.h"
#include "generic_helper.h"

MyConfig g_myconfig;

MyConfig::MyConfig(void)
{
}

MyConfig::~MyConfig(void)
{
}

void MyConfig::Init(const char* ini_file)
{
	int  okay = 0;
	int  tmp = 0;
	char buff[512] = { 0 };
	
	if ( m_inited )
	{
		return;
	}

	ICuteINIConfig* myconfig;
	myconfig = GetICuteINIConfig();
	
	if ( ini_file )
	{
		set_log_ini_file(ini_file);    // 曾经奇怪的BUG.如果把这两行代码的顺序交换.就打不全日志了喔.开日志的句柄会突然失效. 
		okay = myconfig->SetIniFilePath(ini_file); // 暂不知道为什么交换这两行代码,日志句柄会突然失效. 
	}
	else
	{
		set_log_ini_file(MY_CONFIG_INI_FILE);
		okay = myconfig->SetIniFilePath(MY_CONFIG_INI_FILE);
	}

	if ( 0 != okay )
	{
		report_err_immed("Failed to read in "MY_CONFIG_INI_FILE, 0xd);
	}

	okay = myconfig->GetValueByKey("CLIENT", "host", buff, NULL);
	if ( 0 == okay ) m_cli_hostaddr = buff;
	else m_cli_hostaddr = "0001";

	okay = myconfig->GetValueByKey("COMPATIBLE", "faulty_apci", NULL, &m_use_faulty_apci);
	if ( 0 != okay )  // 若没有设置兼容模式则默认不使用兼容模式. 
		m_use_faulty_apci = false;

	okay = myconfig->GetValueByKey("COMPATIBLE", "faulty_authorize", NULL, &m_use_faulty_authorize);
	if ( 0 != okay )
	{
		m_use_faulty_authorize = false;
	}

	okay = myconfig->GetValueByKey("DEVICE", "addr_obj_index", NULL, &tmp);
	if ( 0 != okay )
	{
		m_addr_obj_index = 1;
	}
	else
	{
		m_addr_obj_index = tmp;
	}
	
	okay = myconfig->GetValueByKey("DEVICE", "addr_prop_id", NULL, &tmp);
	if ( 0 != okay )
	{
		m_addr_prop_id = 5;
	}
	else
	{
		m_addr_prop_id = tmp;
	}

	okay = myconfig->GetValueByKey("DEVICE", "devices_dir", buff, NULL);
	if ( 0 != okay )
	{
		m_devices_dir = "devices";
	}
	else
	{
		int len = strlen(buff)-1;
		char tail = buff[len];
		if ( tail == '\\' || tail == '/' )
		{
			buff[len-1] = 0;
		}
		m_devices_dir = buff;
	}

#ifndef __GNUC__  // windows
	m_devices_dir += "\\";
#else
	m_devices_dir += "/";
#endif

	CreateDirectory(m_devices_dir.c_str(), NULL);

	okay = myconfig->GetValueByKey("DEVICE", "broadcast_addr", buff, NULL);
	if ( 0 != okay )
	{
		m_broadcast_addr[0] = '\x00';
		m_broadcast_addr[1] = '\xff';
	}
	else
	{
		int addr_len = sizeof(m_broadcast_addr);
		okay = hexs2bins(buff, m_broadcast_addr, &addr_len);
		if ( 0 != okay )
		{
			m_broadcast_addr[0] = '\x00';
			m_broadcast_addr[1] = '\xff';
		}
	}

	okay = myconfig->GetValueByKey("DEVICE", "serialnum_obj_index", NULL, &tmp);
	if ( 0 != okay )
	{
		m_serialnum_obj_index = 1;
	}
	else
	{
		m_serialnum_obj_index = tmp;
	}

	okay = myconfig->GetValueByKey("DEVICE", "serialnum_prop_id", NULL, &tmp);
	if ( 0 != okay )
	{
		m_serialnum_prop_id = 4;
	}
	else
	{
		m_serialnum_prop_id = tmp;
	}

	okay = myconfig->GetValueByKey("TX", "INTERVAL_TIME", NULL, &m_tx_interval_time);
	if ( 0 != okay )
	{
		m_tx_interval_time = 100;
	}

	okay = myconfig->GetValueByKey("RX", "MOST_EMPTY_IN_ROW", NULL, &m_most_empty_in_row);
	if ( 0 != okay )
	{
		m_most_empty_in_row = 4;
	}

	okay = myconfig->GetValueByKey("RX", "FRAME_LIFE", NULL, &m_frame_life_time);
	if ( 0 != okay )
	{
		m_frame_life_time = 60;
	}

	okay = myconfig->GetValueByKey("RX", "TIMEOUT", NULL, &m_rx_timeout);
	if ( 0 != okay )
	{
		m_rx_timeout = 1000;
	}

	okay = myconfig->GetValueByKey("RX", "RESPONSE_TIME", NULL, &m_rx_response_time);
	if ( 0 != okay )
	{
		m_rx_response_time = 100;
	}

	okay = myconfig->GetValueByKey("RX", "CHECK_ADDRESS", NULL, &m_rx_check_address);
	if ( 0 != okay )
	{
		m_rx_check_address = 1;
	}

	okay = myconfig->GetValueByKey("UDP", "TMP_DEST_PORT", NULL, &tmp);
	if ( 0 != okay )
	{
		m_tmp_dest_port = 1215;
	}
	else
	{
		m_tmp_dest_port = (ushort)tmp;
	}

	myconfig->Free();

	m_inited = TRUE;
}

bool MyConfig::UseFaultyAPCI()
{
	return !!m_use_faulty_apci;
}

bool MyConfig::UseFaultyAuthorize()
{
	return !!m_use_faulty_authorize;
}

