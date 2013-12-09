#ifndef _H_IDEVICE_
#define _H_IDEVICE_

#include <vector>
#include "prot_structs.h"
#include "group_object.h"

#define MAX_USER_MEMORY_SIZE 0x100000 

class mem_t;

/** @brief ���ǲ����豸���ݵĽӿ�. 
 */
class IDevice
{
public:
	/** @brief ���ظ��豸�ĵ�ַ���׵�ַ.��ַռ����BYTES. 
	 * @param h ��� h ��ΪNULL����ͨ��h����һ���ɶ�ʮ�������ַ�����ʽ�ĵ�ַ. 
	 */
	virtual const unsigned char* GetHostAddr(std::string* h) = 0;

	/** @brief ���ظ��豸��SerialNum���׵�ַ.��ַռ6��bytes. 
	 * @param h ��� h ��ΪNULL����ͨ��h����һ���ɶ�ʮ�������ַ�����ʽ�ĵ�ַ. 
	 */
	virtual const unsigned char* GetSerialNum(std::string* h) = 0;

	/** @brief ���ñ�����ַ. 
	 * @return �ɹ�����0,���򷵻ط�0. 
	 */
	virtual int SetHostAddr(const unsigned char* addr) = 0;

	/** @brief �õ�һ��Ԫ����ռ�ĳ���.��λΪbit.
	 * @return �ɹ�����������ΪԪ����ռ����,��λΪbit.ʧ�ܷ��ط�0.����û�����(����,����)��. 
	 */
	virtual int GetPerElemLen_inbit(unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief �õ�Ԫ�صĸ���. 
	 * @return ����Ԫ�صĸ���.������������(����,����)���򷵻ظ���. 
	 */
	virtual int GetElemCount(unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief ɾ��һ���ӿڶ���. 
	 * @return �ɹ�ɾ������0.�������������ӿڶ����򷵻ط�0. 
	 */
	virtual int RemoveIObject(unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief ͨ��������ź�����ID�ŵõ�һ�����������Ľṹ��.
	 * @return �ɹ�����0.ʧ�ܷ��ط�0. 
	 */
	virtual int GetPropFields(unsigned char object_index, unsigned char prop_id, PropertyFields& fields) = 0;

	/** @brief ͨ��������ź�����ID����һ����������. 
	 * @return �ɹ�����0.ʧ�ܷ��ط�0.��������ڸö���index,����id��᷵�ط�0.
	 */
	virtual int SetPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields) = 0;

	/** @brief ����һ����������. 
	 * @return �ɹ�����0.ʧ�ܷ��ط�0.������ڸö���index,����id����᷵�ط�0. 
	 */
	virtual int AddPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields) = 0;

	/** @brief �õ�һЩԪ�ص�ֵ.���뻺����. 
	 * ���ڳ����Ǽ���bit��Ԫ��.Ԫ��ֵ����buff������׵�ַ���롣 
	 * @start_index Ԫ�صĿ�ʼ���.��1��ʼ����.  
	 * @count Ҫ�õ���Ԫ�ظ���. 
	 * @buff �������׵�ַ.
	 * @buff_len ����ʱ��ʾ�������ĳ��ȣ�����ʱ��ʾ������ʹ���˶೤. 
	 * @return �ɹ�ʱ����0.ʧ��ʱ���ط�0.  
	 */
	virtual int GetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
						unsigned char count, unsigned char* buff, int& buff_len) = 0;

	/** @brief ����һЩԪ�ص�ֵ.���뻺����. 
	 * @return �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	 */
	virtual int SetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
						unsigned char count, const unsigned char* buff, int buff_len) = 0;

	/** @brief ���ظ��豸���еĶ���INDEX������ID��. 
	 * @param buff ���������idnex������id�ԣ�������byte.
	 * @buff_len ����ʱ�ǻ������ĳ��ȣ�����ʱ��ʹ�õĳ���.����������������򲻻�д�����ݣ���ʱbuff_len����Ϊ�������ȵ��෴��.  
	 * @return ���ض���index������ID�Եĸ���.��ʱbuff_lenΪ��������ֵ������.����ʧ��ʱ�������ظ���.  
	 */
	virtual int GetAllIDPairs(unsigned char* buff, int& buff_len) = 0;

	/** @brief ���ع��ж��ٸ������. 
	 */
	virtual int GetGroupObjectCount() = 0;

	/** @brief �������ŷ���GroupObjectHead.
	 * �����Ŵ�0��ʼ����.���û����������žͷ���-1,���򷵻�0.����gobj_head��д��.  
	 */
	virtual int GetGroupObjectHeadByIndex(unsigned char index, GroupObjectHead& gobj_head) = 0;

	/** @brief ����������ֵ. 
     * @param index ������������. 
	 * @param mem �������ַ�õ�ֵ. 
	 * @return �ɹ�ʱ����0,ʧ�ܷ��ط�0. 
	 */
	virtual int SetGroupObjectValue(unsigned char index, const unsigned char* val) = 0;

	/** @brief �õ�������ֵ. 
	 * @param index ������������. 
	 * @param mem �ѵõ���ֵ����mem. 
	 * @param len ����ʱlen��mem���Ĵ�С��λΪbyte������ʱlenָʾд��mem��bit������λ��bit. 
	 * @return �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	 */
	virtual int GetGroupObjectValue(unsigned char index, unsigned char* val, int* len) = 0;

	/// ����ɾ�������. 
	/** @brief �ṩ������������ɾ��һ�������. 
	 * �����Ŵ�0��ʼ����. 
	 */
//	virtual int RemoveGroupObjectByIndex(unsigned char index) = 0;

	/** @brief ����һ�������. 
	 * @param gobjh �μ� GroupObjectHead ��. 
	 * @param val ��valȡ
	 * @return �ɹ��Ƿ���0,ʧ�ܷ��ش�����. 
	 */
	virtual int AddGroupObject(GroupObjectHead* gobjh, unsigned char* val) = 0;

	/** @brief ���ع��ж������ַ. 
	 */
	virtual int GetGroupAddrCount() = 0;

	/** @brief �ṩ���ַ�������ŷ���һ�����ַ. 
	 */
	virtual const unsigned char* GetGroupAddrByIndex(unsigned char index) = 0;

	/** @brief �ṩ���ַ��������ɾ�������ַ. 
	 * @return �ɹ�����0,ʧ�ܷ��ش�����. 
	 */
	virtual int RemoveGroupAddrByIndex(unsigned char index) = 0;

	/** @brief ����һ�����ַ.
	 * @return �ɹ�����0,ʧ�ܷ��ش�����. 
	 */
	virtual int AddGroupAddr(const unsigned char* g_addr) = 0;

	/** @brief ���豸�Ƿ����ָ�������ַ. 
	 */
	virtual BOOL ExistThisGroupAddr(const unsigned char* g_addr) = 0;

	/** @brief �õ����ж���Link��. 
	 * @return ������Ŀ. 
	 */
	virtual int GetLinkCount() = 0;

	/** @brief �ṩLink�������ŷ���һ��Link.
	 * @param index Ҫȡ��Link��������. 
	 * @param gobj_idx �������Link��������������. 
	 * @param gaddr_idx �������Link�����ַ��������. 
	 * @return ��������Ų������������ط�0. 
	 */
	virtual int GetLinkByIndex(unsigned char index, unsigned char& gobj_idx, unsigned char& gaddr_idx) = 0;

	/** @brief �ṩLink��������ɾ����Link.  
	 * @return �ɹ�����0,ʧ�ܷ��ش�����. 
	 */
	virtual int RemoveLinkByIndex(unsigned char index) = 0;

	/** @brief ����һ��Link. 
	 * @return �ɹ�����0,ʧ�ܷ��ش�����. 
	 */
	virtual int AddLink(unsigned char gobj_idx, unsigned char gaddr_idx) = 0;

	/** @brief �õ�ĳ���������������������ַ. 
	 * @param gobj_idx ���������. 
	 * @param buff �õ������ַ����˻���. 
	 * @param buff_len ����ʱ��ʾ�������Ĵ�С������ʱ��ʾʹ�õĴ�С.�����������������buff_lenΪ�������ȵ��෴��. 
	 * @return �ɹ����صõ������ַ�ĸ���.ʧ���򷵻ظ���. 
	 */
	virtual int GetGaddrsOfGobjs(unsigned char gobj_idx, unsigned char* buff, int& buff_len) = 0;

	/** @brief ��֤��Կ�õ���Ȩ. 
	 *@return �ɹ�ʱ����0,level����Ϊ��Ȩ�ȼ�.���򷵻ط�0,levelδ����. 
	 */
	virtual int GetAuthorize(const unsigned char* key, unsigned char& level) = 0;

	/** @brief �õ�ģ��������û���ַ�ռ������. 
	 * @return ���ر�ʾ�û���ַ�ռ����ݵ��׵�ַ.implying ����Ϊһ��.  
	 */
	virtual const unsigned char* GetUserMemory() = 0;

	/** @brief ��ģ��������û���ַ�ռ�д����. 
	 * @shift ƫ����. 
	 * @buff Ҫд�Ķ����������������. 
	 * @buff_len Ҫд�೤. 
	 * @return �ɹ�д�뷵��0.���Ҫд�����ݳ�����64K��Ѱַ�ռ�ͱ���.buffΪ��buff_len������Ҳ�ᱨ��. 
	 */
	virtual int WriteUserMemory(unsigned int shift, unsigned char* buff, int buff_len) = 0;

	/** @brief ���豸�Ƿ���Ӧ IndividualAddress_Write.
	 */
	virtual bool IsIndividualAddressWriteEnabled() = 0;

	/** @brief ���豸�Ƿ���Ӧ IndividualAddress_Read. 
	 */
	virtual bool IsIndividualAddressReadEnabled() = 0;

	/** @brief ��ɱ����.�������Լ���ʧ. 
	 * txt�汾��ʵ�־��ǰ��������豸���ļ�ɾ��.���ݿ�汾ʵ��Ӧ��������������豸�ļ�¼���ɾ��. 
	 */
	virtual int CommitSuicide() = 0;

	/** @brief �õ��豸��ID��. 
	 */
	virtual UINT GetDevID() = 0;

    /** @brief ����һ������ӳ����Ϣ�ĺ���. 
	 */
	virtual void GetMapInfo(DevMapInfo& map_info) = 0;

	/** @brief ����ӳ��. 
	 */
	virtual void SetMapInfo(const DevMapInfo& map_info) = 0;

	/** @brief �Ƿ����ĳ����Ԫ. 
	 * �����Խӵ��߼���Ԫ�Ĵ���. 
	 * @param softunit_name �� "X001", "Y0077". 
	 * @return ������������Ԫ�򷵻�true,���򷵻�false. 
	 */
	virtual bool DoesThisSoftUnitExist(const char* softunit_name) = 0;

	/** @brief ����Ԫ����ֵ. 
	 * �����Խӵ��߼���Ԫ�Ĵ���. 
	 * @param softunit_name ��Ԫ��������. 
	 * @param buff Ϊ���̰߳�ȫ.��Ԫ����ֵ��д��û���.���������Σ��.����ȫ�ĺ���. 
	 * @return ������Ԫ����ֵ�ĵ�ַ.���ڲ����ڵ���Ԫ��.����NULL. 
	 */
	virtual const uchar* ReadSoftUnit(const char* softunit_name, uchar* buff) = 0;

	/** @brief д��Ԫ����ֵ. 
	 * �����Խӵ��߼���Ԫ�Ĵ���. 
	 * @param softunit_name ��Ԫ��������. 
	 * @param value Ҫд��ֵ���׵�ַ. 
	 * @return д��ɹ�����0.ʧ�ܷ��ط�0. 
	 */
	virtual int WriteSoftUnit(const char* softunit_name, const uchar* value) = 0;

	virtual ~IDevice(){}
};

#endif