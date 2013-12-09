#ifndef _H_APDU_
#define _H_APDU_

#include <string>
#include <vector>
#include "prot_structs.h"
#include "MyConfig.h"

/** @brief �����˼���APDU�����������һ��ʵ��. 
*/
class APDU;
class Frame;
class DeviceManager;
extern const APDU* apdu_ary[];

/** @brief ����APDU���̳����������.APDU stands for APPLICATION PROTOCAL DATA UNIT.
*/
class APDU
{
public:
	/// ����һ��Frame�����Frame�е�APDU�ҳ���.ʧ��ʱ���ؿ�. 
	static APDU* BuildAPDUFromFrame(const Frame& frame);

	/// �����µ�ָ���豸��������ָ��. 
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
		return m_priority & 0x03;  // ֻ��Ҫ�������bits.
	}

	/** @brief check the address type.
	* @return ������ַʱ���� 0. ��׼�����ʱ���� 1.
	*/
	UCHAR GetAddrType()const
	{
		return m_addrtype;
	}

	/** @brief ��apciֵ.�����ACK֮���򷵻�NULL. apci_l_p �� apci_h_p �����᷵��ֵ.
	*/
	const UCHAR* GetAPCI(UCHAR* apci_l_p, UCHAR* apci_h_p)const;

	/** APDU��ÿ�����඼�ܷ����������Ӧ��APDU��.�������apdu_mem����.
	* ���apdu_memָ����ڴ���һ����ȷAPDU���Ļ���������һ����԰���APDU����.
	* ������ȷ�򷵻�NULL. 
	*/
	virtual APDU* Spawn(const UCHAR* apdu_mem, int len)const;

	/** @brief �Ƿ��Ӧ��apdu����һ���ظ�apdu���߶��apdus(�ಥ). 
	* �����Ĭ��ʵ��Ϊ����-1. apdu��ΪNULL, apdusû��Ԫ��.  
	* @param apdu ��Ӧ�ڵ�Ե�ͨѶ�����ɵĻظ�apdu.��ʧ���򷵻�NULL. 
	* @param apdus ��Ӧ�ڶಥ���᷵�ض��APDU����apdus������.  
	* @return 0 on success, non-zero on failure.
	*/
	virtual int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	/** @brief �õ�APDU������. 
	*/
	virtual APDUType GetAPDUType()const
	{
		return m_type;
	}

	/*���������ظ��İ�����
	 */
	virtual int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

protected:
	/** ���⹹�캯��. 
	*/
	virtual APDU* VConstruct()const = 0;

	/** @brief �ı�APDU���ڴ�ռ�.�ú���ͬʱ��apci���Ƶ�ǰ�����ֽ�. 
	* @param size ���ڴ�ռ��С.
	* @return �ɹ�����0.�������ط�0.
	*/
	int Resize(int size);

	/** @brief APDU�Ļ�������.asap�ǿ�ѡ��. 
	*/
	void BasicInfo(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap = NULL);

	/** @brief ����apci,��鴫���apci�Ƿ�ͬ��APDU��apciһ��. 
	*/
	bool ApciMatch(const UCHAR* apci)const;

public:
	BOOL    m_recved_apdu; ///< ��ʾ��apdu�Ǳ������ɵĻ��ǽ��յ���.����ǽ��յ���apdu��������host��ַ��peer�˵�ַ. 

protected:
	/// APDU������.Ĭ��������� AT_STANDARD. 
	// enum { AT_ACK, AT_NACK, AT_BUSY, AT_NACKBUSY, AT_STANDARD } m_type;
	APDUType m_type;

	UCHAR   m_ack_request;
	UCHAR   m_priority;
	UCHAR   m_hop_count_type;

	UCHAR   m_peeraddr[2];
	UCHAR   m_hostaddr[2];
	UCHAR   m_addrtype;  // ������ַ������Ϊ0, ��׼���ַ��Ϊ1. Ĭ��Ϊ0.

	UCHAR*  m_apdu_mem;  // ���ʵ�ʵ�APDU��������. 
	UCHAR   m_apdu_len;  // ʵ�ʵ�APDU���ĳ���. 

	UCHAR   m_apci_l;  ///< ��λ������. 
	UCHAR   m_apci_h;  ///< ��λ������. 

	UCHAR   m_apci_l_mask;  ///< ��λ�����ֵ�����. 
	UCHAR   m_apci_h_mask;  ///< ��λ�����ֵ�����. 

	static DeviceManager* ms_dev_manager;

protected:
	APDU(void);

public:
	virtual ~APDU(void);
};

/** @brief ȷ��.
*/
class APDU_ACK : public APDU
{
public:
	APDU_ACK()
	{
		m_type = AT_ACK;

		m_apci_l = 0xff;   // ����ȷ��֡��apci��bitλ������λ. 
		m_apci_h = 0xff;
	}

	/// ��֡ʵ����û��apdu.��ʵ��Spawn. 
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

/** @brief ��ȷ��. 
*/
class APDU_NACK : public APDU
{
public:
	APDU_NACK() 
	{ 
		m_type = AT_NACK; 

		m_apci_l = 0xff;  // ����ȷ��֡��apci��bitλ������λ. 
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

/** @brief �ظ�æ. 
*/
class APDU_BUSY : public APDU
{
public:
	APDU_BUSY() 
	{ 
		m_type = AT_BUSY; 

		m_apci_l = 0xff;  // ����ȷ��֡��apci��bitλ������λ. 
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

/** @brief �ظ�æ��. 
*/
class APDU_NACKBUSY : public APDU
{
public:
	APDU_NACKBUSY() 
	{ 
		m_type = AT_NACKBUSY;

		m_apci_l = 0xff;  // ����ȷ��֡��apci��bitλ������λ. 
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

/** @brief ��ֵ. 
*/
class APDU_PropertyValue_Read : public APDU
{
public:
	APDU_PropertyValue_Read()
	{
		m_type = AT_PropertyValue_Read;
		m_apci_l = 0x03;
		m_apci_h = 0xD5;
		if ( g_myconfig.UseFaultyAPCI() )  // ���ݾɵĴ��� apci
		{
			m_apci_h = 0xB5;
		}
	}

	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, UCHAR object_index,
		UCHAR property_id, UCHAR no_of_elem, USHORT start_index);

	/** @brief �Ӹ�APDU���л�ö�ֵapdu�Ĳ���.
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

/** @brief �ظ���ֵ. 
*/
class APDU_PropertyValue_Response : public APDU
{
public:
	APDU_PropertyValue_Response()
	{
		m_type = AT_PropertyValue_Response;
		m_apci_l = 0x03;
		m_apci_h = 0xD6;
		if ( g_myconfig.UseFaultyAPCI() )  // ���ݾɵĴ��� apci
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

	/** @brief �Ӹ�APDU���л�ö�ֵapdu�Ļظ��Ĳ���. 
	* @param len ������������Ȳ���.�ͻ��len����-1.�����ķ���ֵҲ�᷵�ط�0. 
	*/
	int GetParams(UCHAR* object_index, UCHAR* property_id, UCHAR* no_of_elem, 
		USHORT* start_index, UCHAR* buff, int* len)const;

private:
	// testing code.
	// friend class APDU_PropertyValue_Read;
};

/** @brief дֵ. 
*/
class APDU_PropertyValue_Write : public APDU
{
public:
	APDU_PropertyValue_Write()
	{
		m_type = AT_PropertyValue_Write;
		m_apci_l = 0x03;
		m_apci_h = 0xD7;
		if ( g_myconfig.UseFaultyAPCI() )  // ���ݾɵĴ��� apci
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

/** @brief ������. 
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

/** @brief �����ԵĻظ�. 
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

/** @brief д����. 
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

/** @brief ����ֵ. 
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

/** @brief ����ֵ�Ļظ�. 
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

	/// dev_id�����豸ID��.grp_addr�������ַ.������豸��Ҫ�ظ�������ַ��ֵ.��������0.���򷵻ط�0. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* grp_addr, UINT dev_id);

	/// �������GroupValue_Response APDU.�Ѹ�APDU����GroupValueͨ�����巵��.buff_len����ʱ�ǻ���ĳ���.������
	/// �ǻ��屻ʹ�õĳ���. 
	int GetParams(UCHAR* buff, int& buff_len)const;

private:
};

/** @brief д��ֵ. 
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

/** @brief ��������.
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

	// start_index ֻȡ����λ.�߼�λ���ص�. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		UCHAR group_object_number, UCHAR start_index);

	int GetParams(UCHAR* group_object_number, UCHAR* start_index)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;

private:
};

/** @brief ��������Ļظ�. 
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

	// group_object_number ָ���������ı�������. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* asap, 
		UCHAR group_object_number, UCHAR start_index);

	// ���links_buff�ﴫ��links
	int GetParams(UCHAR* group_object_number, UCHAR* start_index, UCHAR* links_buff, int* buff_len)const;

private:
};

/** @brief д������. 
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

	/** @brief ����һ�� A_Link_Write �� APDU. 
	* @param flags ��ͱ���λ�Ƿ��ͱ��.�ڶ�������λ��ɾ��/��ӱ��.0ɾ��,1���.  
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

	/** @param key: ��keyȡ�ĸ�byte��Ϊkey. 
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

	/// ���ڵ�ַ address ֻ��ȡ��20λ.numberֻ��ȡ��4λ. 
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

	// ���к�������octecs. 
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

	// ���к�������octecs. 
	int BuildMe(UCHAR ack_request, UCHAR priority, UCHAR hop_count_type, const UCHAR* serial_num, const UCHAR* new_address);

	int GetParams(UCHAR* serial_num, UCHAR* new_address)const;

	int Response(APDU*& apdu, std::vector<APDU*>& apdus)const;

	int GetExpectedAPDUTypes(std::vector<APDUType>& expected_bytes)const;
};

// ά����֪��APDU��. 
class APDUContainer
{
public:
	// �õ�һ��vector,���vector��װ���������͵�APDU��һ��ʵ��ָ��. 
	const std::vector<APDU*>& GetAPDUVec();

	// �õ����е�Listen�������ĵ�APDU����. 
	const std::vector<APDUType>& GetExpectedAPDUTypesForListen();

	// ֻ��ͨ�������������class. 
	static APDUContainer& GetInstance();

private:
	APDUContainer();

	const std::vector<APDU*>& GetAPDUVec_nolock();

public:
	~APDUContainer();

private:
	// Ŀǰ֧�ֵ�����APDU�������vector�����һ��ʵ��.��д��APDU������������. 
	std::vector<APDU*>     m_vec_apdu;
	// Listen�����ز��������е�APDU����.��Ϊ��ЩAPDU�ǻظ�����������.Listenֻ���������APDU.  
	std::vector<APDUType>  m_vec_expected_apdutypes4listen;

	// ������. 
	HANDLE m_mutex;
};


#endif