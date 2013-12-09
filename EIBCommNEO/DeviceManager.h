#ifndef _H_DEVICEMANAGER_
#define _H_DEVICEMANAGER_

#include <map>
#include <string>
#include <set>
#include "generic_helper.h"
#include "IDevice.h"

/** @brief �������е��豸. 
  */
class DeviceManager
{
public:
	int  Init();
	int  Uninit();

	/// ����һ���µ��豸.��ʧ���򷵻�0xffffffff. 
	/// template_file ��ָ�������ӿڶ�����������ļ�. 
	uint CreateNewDev(const char* template_file);

	/// ɾ��һ���豸.�����豸��ID��. 
	/// �ɹ�����0,���򷵻ط�0. 
	int RemoveDev(uint dev_id);

	/// ͨ���豸ID�ŵõ��豸.������������ID�򷵻�NULL. 
	IDevice* GetDeviceByID(uint dev_id);

	/// �����Ե��豸ID�Ŵ���buff����.
	/// ��buffΪ�����buff��������buff_len����Ϊ�������ȵ��෴��. 
	/// ��buff�㹻����buff_len��Ϊʹ�õĳ�����.���ȵĵ�λ���ĸ���λ��. 
	/// ��������ֵΪ�ܹ��ж����豸. 
	uint GetAllDevices(unsigned int* buff, int& buff_len);

	/// �����������е��豸.ÿ����һ�����η���һ��.һֱ������NULL��ʾ����.
	/// ���restart��Ϊ��,�����¿�ʼ,�õ���һ��. 
	/// ����������û�б���ʼ��.��Ҳ�᷵��NULL. 
	/// �ú�����Ȼ�����̰߳�ȫ���. 
	IDevice* NextDevice(BOOL restart = FALSE);

	/// ͨ���豸��ַ�ҵ��豸. 
	IDevice* GetDeviceByDAddr(const UCHAR* addr);

	/// ���е��豸���Ƿ����һ��grp_addr�����ַ. 
	BOOL ExistThisGroupAddr(const UCHAR* grp_addr);

public:
	DeviceManager(void);
	~DeviceManager(void);

private:
	/// ��һ�������豸ID. 
	uint NextAvaID();

private:
	bool m_inited;
	typedef std::map<uint, IDevice*> DeviceMap;  ///< �豸ID�ź��豸ָ��. 
	DeviceMap::iterator m_ite4nextdev;  ///< ��NextDevice��. 
	DeviceMap    m_devs; ///< key���豸��ID��.value�����豸�ӿڵ�ָ��. 
	uint  m_next_ava_id; ///< ����һ����ЧID. 
	std::string m_dir;  ///< ����.dev�ļ���Ŀ¼. 
};

#endif