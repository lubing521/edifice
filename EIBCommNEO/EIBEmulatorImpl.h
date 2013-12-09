#ifndef _H_EIBEmulatorImpl_
#define _H_EIBEmulatorImpl_

#include "CommBase.h"
#include "DeviceManager.h"
#include "Pentagon.h"
#include "IEIBEmulator.h"
#include "SerialComm_MM.h"
#include "global.h"

/** @brief ģ�����Ĺ��ܵ�ʵ�����������. 
 * ���淢�ֵ�������������ʵ��û��Ҫ����ӿ�.δ��������ֻ��������һ��ʵ��.����Ϊ�˷���
 * ����ģ������ӿ�.�������� IEIBEmulator. 
 */
// class EIBEmulatorImpl : public IEIBEmulator
class EIBEmulatorImpl : public IBase
{
public:
	virtual int Init(const InitData& initdata);
	virtual int Uninit();

	/// �õ����ն����ﹲ�ж��������͵�APDU.
	/// ����ʱszָʾtypes�����������ܴ���ٸ�enum.����ʱ���bytesָʾ�����鹻�ã���szָʾʵ�ʴ��˶��ٸ�.��ʱ��������0. 
	/// ������鲻���ã��򷵻غ�����0,��sz����Ϊ���������������෴��. 
	int QueryAPDUTypesInRxQueue(APDUType types[], int& sz);

	/// ����������������֡.����һ��FramePeek�ṹ�������.
	/// filter ������ APDUType ������ָ������Ҫ����֡.���filter�� AT_UNAVAILABLE�򷵻����е�֡.
	/// ��Ҫ��� CleanFramePeek �ͷŵ�����κ���������ڴ�. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ش�����.
#define FRAME_PEEKER_BUSY -1      // ��һС��ʱ����û�о���������������ΪBUSY.
	int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count);

	/// �� FramePeeker �������ص� frames �ĵ�ַ����.�ͷ���Դ. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	int ClearFramePeek(void* addr);

	/// �ӽ��ն�����ɾ��ĳ��֡.���� FramePeeker �еõ���FramePeek�ṹ���־Ҫɾ���Ľ��ն����е�֡. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ط�0.
	int RemoveFrameFromRxQueue(const FramePeek& peek);

	/** @brief ��ֵ. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�.0--3.    
	* @param object_index �����index����. 
	* @param property_id ���Ե�ID��. 
	* @param count ����ǩ��Ϊ����C++����.ָʾ�����õ����ٸ�ֵ.��������ʱ����Ϊʵ��ȡ�õ�ֵ. 
	* ����ô�C����.�������unsigned char& ���� unsigned char*.��ָ���޷���char��ָ��. 
	* @param start �ӵڼ���Ԫ�ؿ�ʼȡֵ.���ȡֵ 2^14-1. 
	* @param buff ��������ʱ,��ȡ����ֵ�����������. 
	* @param buff_len ����ʱָʾ�������ĳ���.����ʱbuff_len����Ϊ������ʵ��ʹ�õĳ���. 
	* @return �ɹ�����0.�����0. 
	*/
	virtual int PropertyValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned char& count, unsigned short start, unsigned char* buff, int& buff_len);


	/** @brief дֵ. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�. 
	* @param object_index �����index����. 
	* @param property_id ���Ե�ID��. 
	* @param count д���ٸ�Ԫ�ص�ֵ. 
	* @param start �ӵڼ���Ԫ�ؿ�ʼдֵ. 
	* @param buff Ҫд������ݵ��׵�ַ. 
	* @param buff_len Ҫд������ݵĳ���. 
	* @return �ɹ�����0.�����0. 
	*/	 
	virtual int PropertyValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned char count, unsigned short start, const unsigned char* buff, int buff_len);

	/** @brief ��ֵ2. 
	 * ������PropertyValue_Read �������ÿ��ֻ��һ��ֵ.����start��ֵ�ɱ�ʾ2^16-1.��PropertyValue_Read�е�startֻ��ȡ��2^12-1. 
	 * @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	 * @param priority ���ȼ�. 
	 * @param object_index �����index����. 
	 * @param property_id ���Ե�ID��. 
	 * @param start �ӵڼ���Ԫ�ؿ�ʼȡֵ. 
	 * @param buff ��������ʱ,��ȡ����ֵ�����������. 
	 * @param buff_len ����ʱָʾ�������ĳ���.����ʱbuff_len����Ϊ������ʵ��ʹ�õĳ���. 
	 * @return �ɹ�����0.�����0. 
	 */
	virtual int PropertyValue_Read2(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned short start, unsigned char* buff, int& buff_len);

	/** @brief дֵ2. 
	 * �ο� PropertyValue_Read2ͬPropertyValue_Write.  
	 */
	virtual int PropertyValue_Write2(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned short start, const unsigned char* buff, int buff_len);

	/** @brief ����������.  
	 * @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	 * @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	 * @param priority ���ȼ�. 
	 * @param object_index �����index����. 
	 * @param property_id ���Ե�ID��. 
	 * @param prop_fields ����һ��PropertyFields�ṹ�������.����ʱ���������Ե�����. 
	 * @return �ɹ�����0.�����0. 
	 */
	virtual int Property_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, PropertyFields& prop_fields);

	/** @brief д��������. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�. 
	* @param object_index �����index����. 
	* @param property_id ���Ե�ID��. 
	* @param prop_fields ����һ��PropertyFields�ṹ�������.�ṹ�����ֵ����д�뵽��������.  
	* @return �ɹ�����0.�����0. 
	*/
	virtual int Property_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, const PropertyFields& prop_fields);

	/** @brief ����Կ����ͨ�Ż��,�����Ӧ��Ȩ�޵ȼ�. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�. 
	* @param key �Ӹõ�ַȡ�ĸ�byte��Ϊ��Կ. 
	* @param level ��������ɹ���ѵõ���Ȩ�޵ȼ�д��õ�ַָ���byte.
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	virtual int Authorize_Query(const unsigned char* host, const unsigned char* peer, unsigned char priority, const unsigned char* key, 
		unsigned char* level);

	/** @brief ͨ�����ַ��ֵ. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ���������ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������.������ֵ�������buff. 
	* @param buff_len ��������.�����Ȳ��������᷵�ط�0�����������ȵ��෴������buff_len. 
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	virtual int GroupValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char* buff, 
		int& buff_len );

	/** @brief ͨ�����ַдֵ. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ���������ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������.Ҫд��ֵ�����buff. 
	* @param buff_len_in_bit ������ʹ�õĳ���.ǧ��ע������ĳ��ȵ�λ��Bit. 
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	virtual int GroupValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, const unsigned char* buff, 
		int buff_len_in_bit);

	/** @brief ��ͨѶ����һЩ����. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ�����ĵ�ַ(����ַ). 
	* @param priority ���ȼ�. 
	* @param group_obj_num ָ�������ı�������. ??
	* @param start_index �����������������ַ���е�һ������������ַ������. 
	* @param buff �������׵�ַ.���ɹ����Ѱѵõ���������ַ�б����û���.ÿ�����ַ����byte. 
	* @param buff_len ����ʱ��ʾ�������Ĵ�С.�����ɹ�ʱ,buff_len����Ϊ������ʹ�õĳ���.���������ַ�ĸ������Զ���
	* ���������̫С�治�����ַ���������ط�0.���������Ļ��������ȵ��෴������buff_len. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/
	virtual int Link_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char group_obj_num, 
		unsigned char start_index, unsigned char* buff, int& buff_len);
 
	/** @brief ���ӻ�ɾ��ͨѶ��������. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ�����ĵ�ַ(����ַ). 
	* @param priorify ���ȼ�. 
	* @param group_obj_num ָ�������ı�������. 
	* @param flag ָ�����ӻ���ɾ��. 
	* @param group_addr ���ַ. 
	* @return �ɹ�ʱ����0.���򷵻س�����. 
	*/
	virtual int Link_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char group_obj_num,
		unsigned char flags, const unsigned char* group_addr);

	/** @brief ���û��ڴ�����. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ�����ĵ�ַ(��Ե㵥��ַ). 
	* @param priority ���ȼ�. 
	* @param address �ڴ��ַ.��Ч��Χ 0x00000000 -- 0x000FFFFF.����20λ��Ч.��Ѱַһ���ڴ�. 
	* @param number Ҫȡ���ٸ���λ��.��Ч��Χ 1 - 11. 
	* @param buff ȡ���İ�λ������buff��Ҫȷ��buff��ʼ��number���ֽ��ǿ�д��ȫд���.  
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0.  
	*/
	virtual int UserMemory_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned int address, 
		unsigned char number, unsigned char* buff);

	/** @brief д�û��ڴ�����. 
	 * @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	 * @param peer Ҫ�����ĵ�ַ(��Ե㵥��ַ). 
	 * @param priority ���ȼ�. 
	 * @param address �ڴ��ַ.��Ч��Χ 0x00000000 -- 0x000FFFFF.����20λ��Ч.��Ѱַһ���ڴ�. 
	 * @param number Ҫȡ���ٸ���λ��.��Ч��Χ 1 - 11. 
	 * @param buff ��buff��ʼ��number���ֽڱ�д��.   
	 * @return �ɹ�ʱ����0,ʧ��ʱ���ط�0.  
	 */
	virtual int UserMemory_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned int address,
			unsigned char number, unsigned char* buff);

	/** @brief �㲥.��ͨ�Ż��ĵ�ַ. 
	 * ��Ҫ�ֶ������豸�Ƿ���Ӧ�ø�֡.Ĭ�ϲ���Ӧ. 
	 * @param host ����֡�ı�����ַ. 
	 * @param priority ���ȼ�. 
	 * @param buff ������ �����ĵ�ַ��������˴˻���.
	 * @param buff_len ����ʱ�ǻ������Ĵ�С������ʱָʾ������ʹ�õĴ�С.���������������,�򷵻�������С���෴��. 
	 * @return �ɹ�����0,ʧ�ܷ��ط�0. 
	 */
	virtual int IndividualAddress_Read(const unsigned char* host, unsigned char priority,  unsigned char* buff, int& buff_len);

	/** @brief �㲥.�޸�ͨ�Ż��ĵ�ַ. 
	* ��Ҫ�ֶ������豸�Ƿ���Ӧ��֡.Ĭ�ϲ���Ӧ��֡. 
	* @param host ������ַ. 
	* @param priority ���ȼ�. 
	* @param new_address ͨ�Ż����µ�ַ. 
	* @return �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	*/
	virtual int IndividualAddress_Write(const unsigned char* host, unsigned char priority, const unsigned char* new_address);


	/** @brief �㲥.ͨ��SerialNumber�ҵ�ַ. 
	 * @param host ����֡�ı�����ַ. 
	 * @param priority ���ȼ�. 
	 * @param serial_num �������ַȡ����byte��Ϊserial number. 
	 * @param buff ������ �����ĵ�ַ��������˻�����. 
	 * @param buff_len ����ʱ�ǻ������Ĵ�С������ʱָʾ������ʵ��ʹ�õĴ�С.��������������ã��򷵻�������С���෴��. 
	 * @return �ɹ�����0��ʧ�ܷ��ط�0. 
	 */
	virtual int IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char priority, unsigned char* serial_num,
			unsigned char* buff, int& buff_len);

	/** @brief �㲥.ͨ��SerialNumberд��ַ. 
	 * @param host ����֡�ı�����ַ. 
	 * @param priority ���ȼ�. 
	 * @param new_addr �Ӹõ�ַȡ����bytes��ΪҪ���õĵ�ַ. 
	 * @return �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	 */
	virtual int IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char priority, 
			const unsigned char* serial_num, const unsigned char* new_addr);

	/** @brief ����ԭʼ֡����(��EIBЭ�鶨��Ӧ�ò����).  
	 * @param raw_data ָ��Ҫ���͵����ݵ��׵�ַ. 
	 * @param raw_data_len Ҫ�������ݵĳ���. 
	 * @param recv_buff ���յ��ظ�ʱ�ѻظ�����˻���. 
	 * @param recv_buff_len �������ĳ���.����յ��ظ�ʱ�����������Ͳ��´�����ݣ�������ȱ���Ϊ�������ȵ��෴��. 
	 * @return �ɹ�ʱ����0.�����0. ��������ֵ SEND_RAWDATA_ILLEGAL_PARAMS(������)  
	 * SEND_RAWDATA_NOT_A_FRAME(��������ݲ���һ����δ�ܵ�֡) SEND_RAWDAAT_NO_LEGAL_RESPONSE(���ջظ�ʱ��ʱ)
	 */
#define SEND_RAWDATA_ILLEGAL_PARAMS 7
#define SEND_RAWDATA_NOT_A_FRAME 2
#define SEND_RAWDATA_NO_LEGAL_RESPONSE  3
	virtual int Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, 
		int& recv_buff_len);

	/** @brief ���ü����շ�֡�Ļص�����. 
	*/
	virtual Reporter SetMonitor(Reporter r);

	/** @brief �����ŵ�,�յ�����֡�󽻸�ÿ���豸.����豸��Ҫ�ظ��ͷ�����Ҫ�ظ���֡����ȥ. 
	* ���Ǹ��첽�汾.���ں�̨���߳�.������������.��Ҫ�����յ���������Щ����.��Ҫ���ûص�����SetMonitor.
	*/
	virtual int Listen_Async();

	/** @brief �����ŵ����յ�����֡�󽻸�ÿ���豸.����豸��Ҫ�ظ��ͻ᷵����Ҫ�ظ���֡.�����ͬ���汾.����������. 
	* @param recv_buff ����յ���֡����������.�������������д.�Ͳ���.�μ�recv_buff_len����. 
	* @param recv_buff_len ����ʱָʾ�������ĳ��ȣ�����ʱ.�����������д����д�˵ĳ���.�������д���������ĳ��ȵ��෴��. 
	* @param send_buff ��ѷ��͵�֡����������.�������������д.�Ͳ���.�μ�send_buff_len����.
	* @param send_buff_len ����ʱָʾ�������ĳ���,����ʱ.�����������д����д���˶����ֽ�.�������д���������ĳ��ȵ��෴���� 
	* @param waiting_time ���ȶ��ٺ��뺯���ͷ���.���Ϊ-1,�ͻ����յ����ݻ�������. 
	* @return ����յ��ᷨ��֡�����ͷ���0.�����ʱ����74(��ʱг��). ���򷵻ط�0.  
	*/
	virtual int Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len, uint waiting_time);

	/** @brief ����һ���µ����豸. 
	* @param template_file ��һ��ģ���ļ�.ָ����ô�������豸.�����NULL,������һ��û���κνӿڶ�������������豸. 
	* @return ���������������ID��.������� 0xffffffff ��ʧ��.
	*/
#define INVALID_DEVICE_ID -1
	virtual unsigned int make_dev(const char* template_file);

	/** @brief ɾ��һ�����豸. 
	* @param dev_id Ҫɾ�������豸��ID��. 
	* @return �ɹ�ʱ����0.���ɹ����س�����. 
	*/
	virtual int remove_dev(unsigned int dev_id);

	/** @brief �������е����豸. 
	* @param buff �������豸ID�ŵĻ�����,ÿ�����ֽ�һ��ID��. 
	* @param ����ʱ��ʾ�������Ĵ�С.����ʱ��ʾ��������ʹ�õĳ���.����������������ĳ��ȵ��෴��.���ȵĵ�λ���ĸ�octec. 
	* @return �ɹ�ʱ���ع��ж������豸ID�ű����Ƶ�������.����������������򷵻ظ���. 
	*/
	virtual int get_all_devs(unsigned int* buff, int& buff_len);

	/** @brief �õ����豸�����У��������,����ID����.���뻺��. 
	* @param dev_id ��־���豸���豸ID��. 
	* @param buff ������(�������,����ID)�ԵĻ�����.�������ռһ��byte,����IDҲռһ��byte. 
	* @param buff_len ����ʱ��ʾ�������Ĵ�С.����ʱ��ʾ��������ʹ�õĳ���.����������������ĳ��ȵ��෴��. 
	* @return �ɹ�ʱ���ع��ж���(�������,����ID)�Ա����Ƶ�������.����������������򷵻ظ���. 
	*/
	virtual int get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len);

	/** @brief �����豸�ı�����ַ. 
	 */
	virtual int set_host_addr(unsigned int dev_id, const unsigned char* hostaddr);

	/** @brief �õ�������ַ. 
	 */
	virtual int get_host_addr(unsigned int dev_id, char* addr);

	/** @brief �õ��豸��ӳ����Ϣ. 
	* @param dev_id �豸ID��. �����-1 ���� 0xffffffff ���ʾ�������豸.���豸�ܴ���.�Ҳ��ᱻget_all_devs�õ�.�����������
	* ���豸��������Ϣ����������Ƿ����֡����.
	* @param map_info ӳ����Ϣ�������ṹ����.��DevMapInfo�Ķ���.��publicĿ¼�µ� prot_structs.h �ļ��ж���. 
	* @return ���������������豸�ͷ��ط�0.���򷵻�0. 
	*/
	virtual int get_map_info(unsigned int dev_id, DevMapInfo& map_info);

	/** @brief �����豸��ӳ����Ϣ. 
	* @param dev_id �� GetMapInfo ������ע��.
	* @param map_info �� GetMapInfo ������ע��.
	* @return ���������������豸�ͷ��ط�0,���򷵻�0. 
	*/
	virtual int set_map_info(unsigned int dev_id, const DevMapInfo& map_info);

	/** @brief �õ�һ��Ԫ����ռ�ĳ���.��λΪbit. 
	* @param dev_id ��־���豸���豸ID��. 
	* @param object_index �������. 
	* @param prop_id ����ID. 
	* @return �ɹ�����������ΪԪ����ռ����,��λΪbit.ʧ�ܷ��ط�0.����û�����(����,����)��. 
	*/
	virtual int get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief �õ�Ԫ�صĸ���. 
	* @param dev_id ��־���豸���豸ID��. 
	* @return ����Ԫ�صĸ���.������������(����,����)���򷵻ظ���. 
	*/
	virtual int get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief ͨ��������ź�����ID�ŵõ�һ�����������Ľṹ��. 
	* @param dev_id ��־���豸���豸ID��. 
	* @return �ɹ�����0.ʧ�ܷ��ط�0. 
	*/
	virtual int get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, PropertyFields& fields);

	/** @brief ͨ��������ź�����ID����һ����������. 
	* @return û��û��������������ᱨ��. 
	*/
	virtual int set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief ����һ����������. 
	* @return ����Ѿ������������������ᱨ��. 
	*/
	virtual int add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief �õ�һЩԪ�ص�ֵ.���뻺����. 
	 * @brief dev_id ��־���豸��ID��. 
	 * @param object_index �������. 
	 * @param prop_id ����ID��. 
	 * @param start_index Ԫ�صĿ�ʼ���. 
	 * @param count Ҫ�õ���Ԫ�ظ���. 
	 * @param buff �������׵�ַ. 
	 * @param buff_len ����ʱ��ʾ�������ĳ��ȣ�����ʱ��ʾ������ʹ���˶೤. 
	 * @return �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	 */
	virtual int get_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int& buff_len);

	/** @brief ����һЩԪ�ص�ֵ.���뻺����. 
	* �μ�get_elem_values.�����. 
	*/
	virtual int set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int buff_len);

	/** @brief �õ�ĳ�豸���ж��ٸ������. 
	* @return ���ع��ж��ٸ������. 
	*/
	virtual int get_gobj_count(unsigned int dev_id);

	/** @brief �ṩ������������ɾ��һ�������.  
	* �����豸ID�ź������ţ������Ŵ�0��ʼ����. 
	* @return �ɹ�ʱ����0��ʧ��ʱ���س�����. 
	*/
	// virtual int rm_gobj_byidx(unsigned int dev_id, unsigned char index);

	/** @brief �õ�ĳ�������GroupObjectHead. 
	* �����豸ID�ź�������������. 
	* @gobj_head �����ɹ�ʱ��gobj_head��������.  
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	*/
	virtual int get_gobjhead_byidx(unsigned int dev_id, unsigned char index, GroupObjectHead& gobj_head);

	/** @brief �õ�ĳ��������ֵ.���뻺����. 
	* @param dev_id �豸ID. 
	* @param index ������INDEX. 
	* @param buff ����. 
	* @param len ����ʱ��ʾ�������ĳ���.��������ʱ��ֵ��ʾ���屻ʹ���˶���byte. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/ 
	virtual int get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len);

	/** @brief ����ĳ��������ֵ. 
	* @param dev_id �豸ID. 
	* @param index ���������.    
	* @param buff �����׵�ַ. 
	* @param only_send_when_changed Ϊtrueʱ,ֻ������ֵ�ı�ʱ�������Ϸ���д��ֵ��֡.�����Ϊfalse��
	* ��ʹ��ֵû�б仯Ҳ���������Ϸ���д��ֵ��֡. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/
	virtual int set_gobj_value_byindex(unsigned int dev_id, unsigned char index, unsigned char* buff, bool only_send_when_changed);

	/** @brief ͨ�����ַ����������ֵ  */
	virtual int set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, unsigned char* buff, bool only_send_when_changed);

	// �������������Խӵ�ILogic4Dev.h�ӿ�.�߼����й�.�߼�����fx_vm.dllʵ��. 
	virtual bool DoesThisSoftUnitExist(unsigned int dev_id, const char* softunit_name);
	virtual const uchar* ReadSoftUnit(unsigned int dev_id, const char* softunit_name, uchar* buff);
	virtual int WriteSoftUnit(unsigned int dev_id, const char* softunit_name, const uchar* value);

public:
	EIBEmulatorImpl(void);
	virtual ~EIBEmulatorImpl(void);

private:
	// �ȷ�����. 
	ResponseType tx_rx_sync(const Frame* send_frame, Frame*& recv_frame);

	// �ȷ�����.һֱ�յ�TIME_OUTΪֹ.���յ���֡ȫ��������. 
	ResponseType tx_rx_till_timeout_sync(const Frame* send_frame, std::vector<Frame*>& recv_frames); 
	
	// ���պ�.���waiting_time�������˻�RESPONSE_TIMEOUT,���waiting_timeΪ-1.����Զ�������յ�֡Ϊֹ. 
	ResponseType rx_tx_sync(uint waiting_time);

	// ���ݷ���֡����ظ�֡�������֡����������. 
	void impose_response_restrictions(const Frame* send_frame, std::vector<APDUType>& expected_apdutypes, 
				const uchar*& check_from, const uchar*& check_to);

	// �ɹ�ʱ�ӵõ���frame����Ҫ���͵�frames�ŵ�vector��.
	// ��������0.���ʧ����send_frame����ΪNULL,�������ط�0. 
	int build_send_frames_from_recv_frame(Frame* recv_frame, std::vector<Frame*>& send_frames);

private:
	friend DWORD WINAPI listen_proc(void * arg);  // �첽��Listen_Async�������߳�. 
	uint _listen_proc();

private:
	bool   m_inited;

	CommBase* m_comm; ///< �ṩͨѶ����. 
	struct FramePeekerLocker
	{
		FramePeekerLocker(CommBase* comm)
		{
			m_comm = comm;
			m_comm->LockFramePeeker();
		}

		~FramePeekerLocker()
		{
			m_comm->UnlockFramePeeker();
		}

	private:
		CommBase* m_comm;
	};

	DeviceManager* m_dev_manager; ///< ���������豸������. 

	volatile bool m_quit_listen_thread;  ///< �жϼ����߳�. 
	HANDLE  m_listen_event;  ///< �����ͼ����߳�ͬ��. 
	HANDLE  m_listen_thread;  ///< �����̵߳ľ��. 
	
	HANDLE  m_mutex_objs; ///< �����ӿڶ���������ǰҪ��������. 

	Reporter  m_reporter;
};

#endif