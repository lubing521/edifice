#include "stdafx.h"
#include "DeviceManager.h"
#include "utilities.h"
#include "DeviceImpl_TXT.h"
#include "Frame.h"
#include "APDU.h"

DeviceManager::DeviceManager(void)
{
	m_inited = false;
}

DeviceManager::~DeviceManager(void)
{
	Uninit();
}

int DeviceManager::Init()
{
	int retcode = 0;
	if ( m_inited )  // 不可初始化多次. 
	{
		return -1;
	}

	m_next_ava_id = 1; 

	// 用文本描述的设备文件初始化DeviceManager. 
	DeviceImpl_TXT* dev = NULL;
	uint key;
	std::string file_path;
	m_dir = g_myconfig.GetDevicesDir();
	
	// ffffffff 这个特殊的设备号是给 ePropertyValue_Read 之类的协议API用的. 
	std::string ffffffff_dev_path = m_dir + "ffffffff.dev"; 
	// 确保存在这个文件. 
	FILE* stream = fopen(ffffffff_dev_path.c_str(), "a");
	if ( NULL == stream )
	{
		std::string err_info = ffffffff_dev_path + " must exist.";
		report_err_immed(err_info.c_str(), 0xa);
	}
	fclose(stream);

	std::list<std::string> dev_files;
	EnumFiles(m_dir.c_str(), "^[0-9a-fA-F]+\\.dev$", dev_files, false);
	for ( std::list<std::string>::iterator ite = dev_files.begin();
		  ite != dev_files.end();
		  ++ite )
	{
		file_path = m_dir + *ite;
		dev = new DeviceImpl_TXT;
		retcode = dev->Init(file_path.c_str(), NULL);
		if ( 0 != retcode )
		{
			char err[256] = { 0 };
			sprintf(err, "未能读入设备文件 %s", file_path.c_str());
			report_err_immed(err, 0xf);
		}
		key = dev->GetDevID();
		m_devs[key] = dev;
	}

	m_inited = true;
	m_ite4nextdev = m_devs.begin();

	// 通知两个通讯层.一共有哪些设备. 
	APDU::SetDeviceManager(this);
	Frame::SetDeviceManager(this);

	return retcode;
}

int DeviceManager::Uninit()
{
	int retcode = 0;

	if ( !m_inited )
	{
		return -1;
	}

	for ( DeviceMap::iterator ite = m_devs.begin();
		  ite != m_devs.end();
		  ++ite )
	{
		delete ite->second; // 会在析构函数里调用Uninit(). 
	}

	m_inited = FALSE;

	return retcode;
}

uint DeviceManager::CreateNewDev(const char* template_file)
{
	int retcode = 0;
	char buff[1024] = { 0 };

	DeviceImpl_TXT* dev = new DeviceImpl_TXT;
	uint ava_id = NextAvaID();
	sprintf(buff, "%s%x.dev", m_dir.c_str(), ava_id);

	if ( template_file )
	{
		retcode = dev->Init(template_file, &ava_id);
		dev->ResetDevDescFilePath(buff);
	}
	else
	{
		retcode = dev->Init(buff, &ava_id);
	}
	
	if ( 0 != retcode )
	{
		return 0xffffffff;
	}

	m_devs[ava_id] = dev;

	return ava_id;
}

int DeviceManager::RemoveDev(uint dev_id)
{
	DeviceMap::iterator ite = m_devs.find(dev_id);
	if ( m_devs.end() == ite )
	{
		return -1;
	}
	else
	{
		IDevice* dev = ite->second;
		m_devs.erase(ite);
		dev->CommitSuicide();
		delete dev;
		return 0;
	}
}

IDevice* DeviceManager::GetDeviceByID(uint dev_id)
{
	DeviceMap::iterator ite = m_devs.find(dev_id);
	if ( m_devs.end() == ite )
	{
		return NULL;
	}
	else
	{
		IDevice* dev = ite->second;
		return dev;
	}
}

uint DeviceManager::GetAllDevices(unsigned int* buff, int& buff_len)
{
	uint count = m_devs.size();
	if ( NULL != buff && buff_len >= count ) // 足够长. 
	{
		buff_len = 0;
		for ( DeviceMap::iterator ite = m_devs.begin();
			 ite != m_devs.end();
			 ++ite )
		{
			if ( ite->first == ~0 )
			{
				count--; // 不用向函数调用者报告这个特殊的设备号. 
				continue;
			}
			memcpy( buff + buff_len, &ite->first, sizeof(uint) );
			buff_len ++;
		}	
	}
	else
	{
		buff_len = count;
		buff_len = -buff_len;
	}

	return count;
}

uint DeviceManager::NextAvaID()
{
	while ( true )
	{
		DeviceMap::iterator ite = m_devs.find(m_next_ava_id);
		if ( m_devs.end() == ite )
		{
			return m_next_ava_id;
		}
		m_next_ava_id++;
	}

	return -1;
}

//int DeviceManager::GetDeviceCount()
//{
//	if ( !m_inited )
//	{
//		return -1;
//	}
//
//	int count = m_devs.size();
//
//	return count;
//}

IDevice* DeviceManager::NextDevice(BOOL restart)
{
	if ( !m_inited )
	{
		return NULL;
	}

	if ( restart )
	{
		m_ite4nextdev = m_devs.begin();
		return m_ite4nextdev++->second;
	}

	if ( m_devs.end() == m_ite4nextdev )
	{
		m_ite4nextdev = m_devs.begin();
		return NULL;
	}

	return m_ite4nextdev++->second;
}

IDevice* DeviceManager::GetDeviceByDAddr(const UCHAR* addr)
{
	if ( NULL == addr )
	{
		return NULL;
	}

	const unsigned char* ptr2addr;
	for ( DeviceMap::iterator ite = m_devs.begin();
		ite != m_devs.end();
		++ite )
	{
		ptr2addr = ite->second->GetHostAddr(NULL);
		if ( NULL != ptr2addr && 0 == memcmp(addr, ptr2addr, 2) ) // 设备已经设置地址并且地址匹配. 
		{
			return ite->second;
		}
	}	
	
	return NULL;
}

BOOL DeviceManager::ExistThisGroupAddr(const UCHAR *grp_addr)
{
	BOOL exist;
	for ( DeviceMap::iterator ite = m_devs.begin();
		  ite != m_devs.end();
		  ++ite )
	{
		exist = ite->second->ExistThisGroupAddr(grp_addr);
		if ( exist )
		{
			return TRUE;
		}
	}

	return FALSE;
}