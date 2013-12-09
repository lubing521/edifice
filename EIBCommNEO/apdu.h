#ifndef _H_APDU_
#define _H_APDU_

#include <string>
#include <vector>
#include "prot_structs.h"
#include "MyConfig.h"

/** @brief 放置了己有APDU的所有子类的一个实例. 
*/
class APDU;
class Frame;
class DeviceManager;
extern const APDU* apdu_ary[];

/** @brief 所有APDU都继承于这个父类.APDU stands for APPLICATION PROTOCAL DATA UNIT.
*/
class APDU
{
public:
	/// 传入一个Frame，则把Frame中的APDU找出来.失败时返回空. 
	static APDU* BuildAPDUFromFrame(const Frame& frame);

	/// 设置新的指向设备管理对象的指针. 
	static DeviceManager* SetDeviceManager(DeviceManager* manager);

	bool IsAck()const{ return m_type == AT_ACK; }
	bool IsNack()const { return m_type == AT_NACK; }
	bool IsBusy()const { return m_type == AT_BUSY; }
	bool IsNackbusy()const { return m_type == AT_NACKBUSY; }
	bool IsStandard()const 
	{ 
		bool b = true;;
		if ( IsAck() ) b = false;
		else if ( IsNack() ) b = false;
		else if ( IsBusy() ) b = false;
		else if ( IsNackbusy() ) b = false;
		return b;
	} 

	UCHAR GetApduLen()const { return m_apdu_len; }
	const UCHAR* GetApduMem()const { return m_apdu_mem; }

	void SetHostAddr(const UCHAR* hostaddr)
	{
		if ( NULL != hostaddr )
		{
			m_hostaddr[0] = hostaddr[0];
			m_hostaddr[1] = hostaddr[1];
		}
	}

	void SetPeerAddr(const UCHAR* peeraddr)
	{
		if ( NULL != peeraddr )
		{
			m_peeraddr[0] = peeraddr[0];
			m_peeraddr[1] = peeraddr[1];
		}
	}

	const UCHAR* GetHostAddr()const
	{
		return m_hostaddr;
	}

	const UCHAR* GetPeerAddr()const
	{
		return m_peeraddr;
	}

	const UCHAR GetPriority()const
	{
		return m_priority & 0x03;  // 只需要最低两个bits.
	}

	/** @brief check the address type.
	* @return 单个地址时返回 0. 标准组地组时返回 1.
	*/
	UCHAR GetAddrType()const
	{
		return m_addrtype;
	}

	/** @brief 拿apci值.如果是ACK之类则返回NULL. apci_l_p 和 apci_h_p 都不会返回值.
	*/
	const UCHAR* GetAPCI(UCHAR* apci_l_p, UCHAR* apci_h_p)const;

	/** APDU的每个子类都能分析与它相对应的APDU包.这个包由apdu_mem传入.
	* 如果apdu_mem指向的内存是一个正确APDU包的话。就生成一个相对包的APDU对象.
	* 若不正确则返回NULL. 
	*/
	virtual APDU* Spawn(const UCHAR* apdu_mem, int len)const;

	/** @brief 是否对应该apdu生成一个回复apdu或者多个apdus(多播). 
	* 基类的默认实现为返回-1. apdu置为NULL, apdus没有元素.  
	* @param apdu 对应于点对点通讯。生成的回复apdu.若失败则返回NULL. 
	* @param apdus 对应于多播。会返回多个APDU存在apdus容器中.  
	* @return 0 on success, non-zero on failure.
	*/
	virtual int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	/** @brief 得到APDU的类型. 
	*/
	virtual APDUType GetAPDUType()const
	{
		return m_type;
	}

	/*设置期望回复的包类型
	 */
	virtual int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

protected:
	/** 虚拟构造函数. 
	*/
	virtual APDU* VConstruct()const = 0;

	/** @brief 改变APDU的内存空间.该函数同时把apci复制到前两个字节. 
	* @param size 新内存空间大小.
	* @return 成功返回0.参数错返回非0.
	*/
	int Resize(int size);

	/** @brief APDU的基本设置.asap是可选的. 
	*/
	void BasicInfo(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap = NULL);

	/** @brief 传入apci,检查传入的apci是否同该APDU的apci一致. 
	*/
	bool ApciMatch(const UCHAR* apci)const;

public:
	BOOL    m_recved_apdu; ///< 表示该apdu是本机生成的还是接收到的.如果是接收到的apdu，则它的host地址是peer端地址. 

protected:
	/// APDU的类型.默认情况下是 AT_STANDARD. 
	// enum { AT_ACK, AT_NACK, AT_BUSY, AT_NACKBUSY, AT_STANDARD } m_type;
	APDUType m_type;

	UCHAR   m_ack_request;
	UCHAR   m_priority;
	UCHAR   m_hop_count_type;

	UCHAR   m_peeraddr[2];
	UCHAR   m_hostaddr[2];
	UCHAR   m_addrtype;  // 单个地址类型则为0, 标准组地址则为1. 默认为0.

	UCHAR*  m_apdu_mem;  // 存放实际的APDU包的内容. 
	UCHAR   m_apdu_len;  // 实际的APDU包的长度. 

	UCHAR   m_apci_l;  ///< 低位控制字. 
	UCHAR   m_apci_h;  ///< 高位控制字. 

	UCHAR   m_apci_l_mask;  ///< 低位控制字的掩码. 
	UCHAR   m_apci_h_mask;  ///< 高位控制字的掩码. 

	static DeviceManager* ms_dev_manager;

protected:
	APDU(void);

public:
	virtual ~APDU(void);
};

/** @brief 确认.
*/
class APDU_ACK : public APDU
{
public:
	APDU_ACK()
	{
		m_type = AT_ACK;

		m_apci_l = 0xff;   // 假设确认帧的apci的bit位都被置位. 
		m_apci_h = 0xff;
	}

	/// 短帧实际上没有apdu.不实现Spawn. 
	APDU* Spawn(const UCHAR*, int)const
	{
		return NULL;
	}

protected:
	APDU* VConstruct()const
	{
		return new APDU_ACK;
	}

};

/** @brief 否定确定. 
*/
class APDU_NACK : public APDU
{
public:
	APDU_NACK() 
	{ 
		m_type = AT_NACK; 

		m_apci_l = 0xff;  // 假设确认帧的apci的bit位都被置位. 
		m_apci_h = 0xff;
	}

	APDU* Spawn(const UCHAR*, int)const
	{
		return NULL;
	}

protected:
	APDU* VConstruct()const
	{
		return new APDU_NACK;
	}
};

/** @brief 回复忙. 
*/
class APDU_BUSY : public APDU
{
public:
	APDU_BUSY() 
	{ 
		m_type = AT_BUSY; 

		m_apci_l = 0xff;  // 假设确认帧的apci的bit位都被置位. 
		m_apci_h = 0xff;
	}

	APDU* Spawn(const UCHAR*, int)const
	{
		return NULL;
	}

protected:
	APDU* VConstruct()const
	{
		return new APDU_BUSY;
	}
};

/** @brief 回复忙否定. 
*/
class APDU_NACKBUSY : public APDU
{
public:
	APDU_NACKBUSY() 
	{ 
		m_type = AT_NACKBUSY;

		m_apci_l = 0xff;  // 假设确认帧的apci的bit位都被置位. 
		m_apci_h = 0xff;
	}

	APDU* Spawn(const UCHAR*, int)const
	{
		return NULL;
	}

protected:
	APDU* VConstruct()const
	{
		return new APDU_NACKBUSY;
	}
};

/** @brief 读值. 
*/
class APDU_PropertyValue_Read : public APDU
{
public:
	APDU_PropertyValue_Read()
	{
		m_type = AT_PropertyValue_Read;
		m_apci_l = 0x03;
		m_apci_h = 0xD5;
		if ( g_myconfig.UseFaultyAPCI() )  // 兼容旧的错误 apci
		{
			m_apci_h = 0xB5;
		}
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR object_index,
		UCHAR property_id, UCHAR no_of_elem, USHORT start_index);

	/** @brief 从该APDU包中获得读值apdu的参数.
	*/
	int GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, USHORT* start_index)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

protected:
	APDU* VConstruct()const
	{
		return new APDU_PropertyValue_Read;
	}
};

/** @brief 回复读值. 
*/
class APDU_PropertyValue_Response : public APDU
{
public:
	APDU_PropertyValue_Response()
	{
		m_type = AT_PropertyValue_Response;
		m_apci_l = 0x03;
		m_apci_h = 0xD6;
		if ( g_myconfig.UseFaultyAPCI() )  // 兼容旧的错误 apci
		{
			m_apci_h = 0xB6;
		}
	}

	APDU* VConstruct()const
	{
		return new APDU_PropertyValue_Response;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR object_index,
		UCHAR property_id, UCHAR no_of_elem, USHORT start_index);

	/** @brief 从该APDU包中获得读值apdu的回复的参数. 
	* @param len 如果缓冲区长度不够.就会从len返回-1.函数的返回值也会返回非0. 
	*/
	int GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, 
		USHORT* start_index, UCHAR* buff, int* len)const;

private:
	// testing code.
	// friend class APDU_PropertyValue_Read;
};

/** @brief 写值. 
*/
class APDU_PropertyValue_Write : public APDU
{
public:
	APDU_PropertyValue_Write()
	{
		m_type = AT_PropertyValue_Write;
		m_apci_l = 0x03;
		m_apci_h = 0xD7;
		if ( g_myconfig.UseFaultyAPCI() )  // 兼容旧的错误 apci
		{
			m_apci_h = 0xB7;
		}
	}

	APDU* VConstruct()const
	{
		return new APDU_PropertyValue_Write;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR object_index,
		UCHAR property_id, UCHAR no_of_elem, USHORT start_index, const UCHAR* buff, UCHAR buff_len);

	int GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, 
		USHORT* start_index, UCHAR* buff, int* len)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;


private:

};

/** @brief 读属性. 
*/
class APDU_Property_Read : public APDU
{
public:
	APDU_Property_Read()
	{
		m_type = AT_Property_Read;
		m_apci_l = 0x02;
		m_apci_h = 0xc7;
	}

	APDU* VConstruct()const
	{
		return new APDU_Property_Read;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		UCHAR object_index, UCHAR property_id);

	int GetParams(UCHAR* object_index, UCHAR* property_id)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

/** @brief 读属性的回复. 
*/
class APDU_Property_Response : public APDU
{
public:
	APDU_Property_Response()
	{
		m_type = AT_Property_Response;
		m_apci_l = 0x02;
		m_apci_h = 0xc8;
	}

	APDU* VConstruct()const
	{
		return new APDU_Property_Response;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR object_index, UCHAR property_id);

	int GetParams(UCHAR* object_index, UCHAR* property_id, PropertyFields* fields)const;

private:

};

/** @brief 写属性. 
*/
class APDU_Property_Write : public APDU
{
public:
	APDU_Property_Write()
	{
		m_type = AT_Property_Write;
		m_apci_l = 0x02;
		m_apci_h = 0xc9;
	}

	APDU* VConstruct()const
	{
		return new APDU_Property_Write;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR object_index, UCHAR property_id,
		const PropertyFields& fields);

	int GetParams(UCHAR* object_index, UCHAR* property_id, PropertyFields* fields)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

/** @brief 读组值. 
*/
class APDU_GroupValue_Read : public APDU
{
public:
	APDU_GroupValue_Read()
	{
		m_type = AT_GroupValue_Read;
		m_apci_l = 0x00;
		m_apci_h = 0x00;
		m_addrtype = 1;
	}

	APDU* VConstruct()const
	{
		return new APDU_GroupValue_Read;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap);

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:

};

/** @brief 读组值的回复. 
*/
class APDU_GroupValue_Response : public APDU
{
public:
	APDU_GroupValue_Response()
	{
		m_type = AT_GroupValue_Response;
		m_apci_l = 0x00;
		m_apci_h = 0x40;

		m_apci_l_mask = 0x03;
		m_apci_h_mask = 0xc0;

		m_addrtype = 1; 
	}

	APDU* VConstruct()const
	{
		return new APDU_GroupValue_Response;
	}

	/// dev_id传入设备ID号.grp_addr传入组地址.如果该设备需要回复这个组地址读值.则函数返回0.否则返回非0. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* grp_addr, UINT dev_id);

	/// 解析这个GroupValue_Response APDU.把该APDU里存的GroupValue通过缓冲返回.buff_len传入时是缓冲的长度.返回是
	/// 是缓冲被使用的长度. 
	int GetParams(UCHAR* buff, int& buff_len)const;

private:
};

/** @brief 写组值. 
*/
class APDU_GroupValue_Write : public APDU
{
public:
	APDU_GroupValue_Write()
	{
		m_type = AT_GroupValue_Write;
		m_apci_l = 0x00;
		m_apci_h = 0x80;

		m_apci_l_mask = 0x03;
		m_apci_h_mask = 0xc0;

		m_addrtype = 1;
	}

	APDU* VConstruct()const
	{
		return new APDU_GroupValue_Write;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		const UCHAR* buff, USHORT data_len_in_bit);

	int GetParams(UCHAR* buff, USHORT& data_len_in_bit)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

/** @brief 读关联表.
*/
class APDU_Link_Read : public APDU
{
public:
	APDU_Link_Read()
	{
		m_type = AT_Link_Read;
		m_apci_l = 0x03;
		m_apci_h = 0xe5;
	}

	APDU* VConstruct()const
	{
		return new APDU_Link_Read;
	}

	// start_index 只取低四位.高级位被截掉. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		UCHAR group_object_number, UCHAR start_index);

	int GetParams(UCHAR* group_object_number, UCHAR* start_index)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

/** @brief 读关联表的回复. 
*/
class APDU_Link_Response : public APDU
{
public:
	APDU_Link_Response()
	{
		m_type = AT_Link_Response;
		m_apci_l = 0x03;
		m_apci_h = 0xe6;
	}

	APDU* VConstruct()const
	{
		return new APDU_Link_Response;
	}

	// group_object_number 指定的组对象的本地索引. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		UCHAR group_object_number, UCHAR start_index);

	// 会从links_buff里传回links
	int GetParams(UCHAR* group_object_number, UCHAR* start_index, UCHAR* links_buff, int* buff_len)const;

private:
};

/** @brief 写关联表. 
*/
class APDU_Link_Write : public APDU
{
public:
	APDU_Link_Write()
	{
		m_type = AT_Link_Write;
		m_apci_l = 0x03;
		m_apci_h = 0xe7;
	}

	APDU* VConstruct()const
	{
		return new APDU_Link_Write;
	}

	/** @brief 构建一个 A_Link_Write 的 APDU. 
	* @param flags 最低比特位是发送标记.第二个比特位是删除/添加标记.0删除,1添加.  
	*/
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		UCHAR group_object_number, UCHAR flags, const unsigned char* group_address);

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetParams(UCHAR* group_object_number, UCHAR* flags, const unsigned char** group_address)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

class APDU_Authorize_Request : public APDU
{
public:
	APDU_Authorize_Request()
	{
		m_type = AT_Authorize_Request;
		m_apci_l = 0x03;
		m_apci_h = 0xD1;
	}

	APDU* VConstruct()const
	{
		return new APDU_Authorize_Request;
	}

	/** @param key: 从key取四个byte作为key. 
	*/
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, const UCHAR* key);

	int GetParams(UCHAR* key)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;
};

class APDU_Authorize_Response : public APDU
{
public:
	APDU_Authorize_Response()
	{
		m_type = AT_Authorize_Response;
		m_apci_l = 0x03;
		m_apci_h = 0xD2;
	}

	APDU* VConstruct()const
	{
		return new APDU_Authorize_Response;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, const UCHAR* key);

	int GetParams(UCHAR* level)const;
};

class APDU_UserMemory_Read : public APDU
{
public:
	APDU_UserMemory_Read()
	{
		m_type = AT_UserMemory_Read;
		m_apci_l = 0x02;
		m_apci_h = 0xC0;
	}

	APDU* VConstruct()const
	{
		return new APDU_UserMemory_Read;
	}

	/// 对于地址 address 只会取低20位.number只会取低4位. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR number, UINT address);

	int GetParams(UCHAR* number, UINT* address)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

class APDU_UserMemory_Response : public APDU
{
public:
	APDU_UserMemory_Response()
	{
		m_type = AT_UserMemory_Response;
		m_apci_l = 0x02;
		m_apci_h = 0xC1;
	}

	APDU* VConstruct()const
	{
		return new APDU_UserMemory_Response;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR number, UINT address);

	int GetParams(UCHAR* number, UINT* address, UCHAR* buff, int* buff_len)const;

private:
};

class APDU_UserMemory_Write : public APDU
{
public:
	APDU_UserMemory_Write()
	{
		m_type = AT_UserMemory_Write;
		m_apci_l = 0x02;
		m_apci_h = 0xC2;
	}

	APDU* VConstruct()const
	{
		return new APDU_UserMemory_Write;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR number, UINT address, UCHAR* buff);

	int GetParams(UCHAR* number, UINT* address, UCHAR* buff, int* buff_len)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

class APDU_IndividualAddress_Write : public APDU
{
public:
	APDU_IndividualAddress_Write()
	{
		m_type = AT_IndividualAddress_Write;
		m_apci_l = 0x00;
		m_apci_h = 0xC2;
	}

	APDU* VConstruct()const
	{
		return new APDU_IndividualAddress_Write;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* new_address);

	int GetParams(UCHAR* new_address)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;
};

class APDU_IndividualAddress_Read : public APDU
{
public:
	APDU_IndividualAddress_Read()
	{
		m_type = AT_IndividualAddress_Read;
		m_apci_l = 0x01;
		m_apci_h = 0x00;
	}

	APDU* VConstruct()const
	{
		return new APDU_IndividualAddress_Read;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type);

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;
};

class APDU_IndividualAddress_Response : public APDU
{
public:
	APDU_IndividualAddress_Response()
	{
		m_type = AT_IndividualAddress_Response;
		m_apci_l = 0x01;
		m_apci_h = 0x40;
	}

	APDU* VConstruct()const
	{
		return new APDU_IndividualAddress_Response;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type);
};

class APDU_IndividualAddressSerialNumber_Read : public APDU
{
public:
	APDU_IndividualAddressSerialNumber_Read()
	{
		m_type = AT_IndividualAddressSerialNumber_Read;
		m_apci_l = 0x03;
		m_apci_h = 0xDC;
	}

	APDU* VConstruct()const
	{
		return new APDU_IndividualAddressSerialNumber_Read;
	}

	// 序列号有六个octecs. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* serial_num);

	int GetParams(UCHAR* serial_num)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;
};

class APDU_IndividualAddressSerialNumber_Response : public APDU
{
public:
	APDU_IndividualAddressSerialNumber_Response()
	{
		m_type = AT_IndividualAddressSerialNumber_Response;
		m_apci_l = 0x03;
		m_apci_h = 0xDD;
	}

	APDU* VConstruct()const
	{
		return new APDU_IndividualAddressSerialNumber_Response;
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* serialnum);

	int GetParams(uchar* serial_num)const;
};

class APDU_IndividualAddressSerialNumber_Write : public APDU
{
public:
	APDU_IndividualAddressSerialNumber_Write()
	{
		m_type = AT_IndividualAddressSerialNumber_Write;
		m_apci_l = 0x03;
		m_apci_h = 0xDE;
	}

	APDU* VConstruct()const
	{
		return new APDU_IndividualAddressSerialNumber_Write;
	}

	// 序列号有六个octecs. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* serial_num, const UCHAR* new_address);

	int GetParams(UCHAR* serial_num, UCHAR* new_address)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;
};

// 维护己知的APDU们. 
class APDUContainer
{
public:
	// 得到一个vector,这个vector里装了所有类型的APDU各一个实例指针. 
	const std::vector<APDU*>& GetAPDUVec();

	// 得到所有的Listen函数关心的APDU类型. 
	const std::vector<APDUType>& GetExpectedAPDUTypesForListen();

	// 只能通过单件访问这个class. 
	static APDUContainer& GetInstance();

private:
	APDUContainer();

	const std::vector<APDU*>& GetAPDUVec_nolock();

public:
	~APDUContainer();

private:
	// 目前支持的所以APDU都在这个vector里加入一个实例.新写的APDU勿必在这里加入. 
	std::vector<APDU*>     m_vec_apdu;
	// Listen函数必不并心所有的APDU类型.因为有些APDU是回复而不是请求.Listen只关心请求的APDU.  
	std::vector<APDUType>  m_vec_expected_apdutypes4listen;

	// 不解释. 
	HANDLE m_mutex;
};


#endif