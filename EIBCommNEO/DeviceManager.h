#ifndef _H_DEVICEMANAGER_
#define _H_DEVICEMANAGER_

#include <map>
#include <string>
#include <set>
#include "generic_helper.h"
#include "IDevice.h"

/** @brief 管理所有的设备. 
  */
class DeviceManager
{
public:
	int  Init();
	int  Uninit();

	/// 增加一个新的设备.若失败则返回0xffffffff. 
	/// template_file 是指导创建接口对象的组对象的文件. 
	uint CreateNewDev(const char* template_file);

	/// 删除一个设备.传入设备的ID号. 
	/// 成功返回0,否则返回非0. 
	int RemoveDev(uint dev_id);

	/// 通过设备ID号得到设备.如果不存在这个ID则返回NULL. 
	IDevice* GetDeviceByID(uint dev_id);

	/// 把所以的设备ID号存入buff缓冲.
	/// 若buff为零或者buff不够，则buff_len被置为期望长度的相反数. 
	/// 若buff足够长，buff_len置为使用的长度数.长度的单位是四个八位组. 
	/// 函数返回值为总共有多少设备. 
	uint GetAllDevices(unsigned int* buff, int& buff_len);

	/// 用来编历所有的设备.每调用一次依次返回一个.一直到返回NULL表示结束.
	/// 如果restart设为真,则重新开始,得到第一个. 
	/// 如果该类对象没有被初始化.则也会返回NULL. 
	/// 该函数显然不是线程安全的喔. 
	IDevice* NextDevice(BOOL restart = FALSE);

	/// 通过设备地址找到设备. 
	IDevice* GetDeviceByDAddr(const UCHAR* addr);

	/// 所有的设备里是否存在一个grp_addr的组地址. 
	BOOL ExistThisGroupAddr(const UCHAR* grp_addr);

public:
	DeviceManager(void);
	~DeviceManager(void);

private:
	/// 下一个可用设备ID. 
	uint NextAvaID();

private:
	bool m_inited;
	typedef std::map<uint, IDevice*> DeviceMap;  ///< 设备ID号和设备指针. 
	DeviceMap::iterator m_ite4nextdev;  ///< 给NextDevice用. 
	DeviceMap    m_devs; ///< key是设备的ID号.value就是设备接口的指针. 
	uint  m_next_ava_id; ///< 记下一个有效ID. 
	std::string m_dir;  ///< 放置.dev文件的目录. 
};

#endif