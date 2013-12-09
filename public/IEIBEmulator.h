#ifndef _H_IEIBEMULATOR_
#define _H_IEIBEMULATOR_

#include "global.h"

class GroupObjectHead;
struct PropertyFields;

class IEIBEmulator : public IBase
{
public:
	virtual int Init(unsigned short port, unsigned int baudrate) = 0;
	virtual int Uninit() = 0;

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
		unsigned char property_id, unsigned char& count, unsigned short start, unsigned char* buff, int& buff_len) = 0;

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
		unsigned char property_id, unsigned char count, unsigned short start, const unsigned char* buff, int buff_len) = 0;

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
		unsigned char property_id, unsigned short start, unsigned char* buff, int& buff_len) = 0;

	/** @brief дֵ2. 
	 * �ο� PropertyValue_Read2ͬPropertyValue_Write.  
	 */
	virtual int PropertyValue_Write2(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned short start, const unsigned char* buff, int buff_len) = 0;

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
		unsigned char property_id, PropertyFields& prop_fields) = 0;

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
		unsigned char property_id, const PropertyFields& prop_fields) = 0;

	/** @brief ����Կ����ͨ�Ż��,�����Ӧ��Ȩ�޵ȼ�. 
	 * @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	 * @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	 * @param priority ���ȼ�. 
 	 * @param key �Ӹõ�ַȡ�ĸ�byte��Ϊ��Կ. 
	 * @param level ��������ɹ���ѵõ���Ȩ�޵ȼ�д��õ�ַָ���byte.
	 * @return �ɹ�ʱ����0,���򷵻ط�0. 
	 */
	virtual int Authorize_Query(const unsigned char* host, const unsigned char* peer, unsigned char priority, const unsigned char* key, 
		unsigned char* level) = 0;

	/** @brief ͨ�����ַ��ֵ. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ���������ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������.������ֵ�������buff. 
	* @param buff_len ��������.�����Ȳ��������᷵�ط�0�����������ȵ��෴������buff_len. 
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	virtual int GroupValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char* buff, 
		int& buff_len ) = 0;

	/** @brief ͨ�����ַдֵ. 
	* @param host �Ӹõ�ַȡ����byte��������Ϊ���ͻ���ַ. 
	* @param peer Ҫ���������ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������.Ҫд��ֵ�����buff. 
	* @param buff_len_in_bit ������ʹ�õĳ���.ǧ��ע������ĳ��ȵ�λ��Bit. 
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	virtual int GroupValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, const unsigned char* buff, 
		int buff_len_in_bit) = 0;

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
		unsigned char start_index, unsigned char* buff, int& buff_len) = 0;

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
		unsigned char flags, const unsigned char* group_addr) = 0;

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
		unsigned char number, unsigned char* buff) = 0;

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
		unsigned char number, unsigned char* buff) = 0;

	/** @brief �㲥.��ͨ�Ż��ĵ�ַ. 
	* ��Ҫ�ֶ������豸�Ƿ���Ӧ�ø�֡.Ĭ�ϲ���Ӧ. 
	* @param host ����֡�ı�����ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������ �����ĵ�ַ��������˴˻���.
	* @param buff_len ����ʱ�ǻ������Ĵ�С������ʱָʾ������ʹ�õĴ�С.���������������,�򷵻�������С���෴��. 
	* @return �ɹ�����0,ʧ�ܷ��ط�0. 
	*/
	virtual int IndividualAddress_Read(const unsigned char* host, unsigned char priority,  unsigned char* buff, int& buff_len) = 0;

	/** @brief �㲥.�޸�ͨ�Ż��ĵ�ַ. 
	* ��Ҫ�ֶ������豸�Ƿ���Ӧ��֡.Ĭ�ϲ���Ӧ��֡. 
	* @param host ������ַ. 
	* @param priority ���ȼ�. 
	* @param new_address ͨ�Ż����µ�ַ. 
	* @return �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	*/
	virtual int IndividualAddress_Write(const unsigned char* host, unsigned char priority, const unsigned char* new_address) = 0;

	/** @brief �㲥.ͨ��SerialNumber�ҵ�ַ. 
	* @param host ����֡�ı�����ַ. 
	* @param priority ���ȼ�. 
	* @param serial_num �������ַȡ����byte��Ϊserial number. 
	* @param buff ������ �����ĵ�ַ��������˻�����. 
	* @param buff_len ����ʱ�ǻ������Ĵ�С������ʱָʾ������ʵ��ʹ�õĴ�С.��������������ã��򷵻�������С���෴��. 
	* @return �ɹ�����0��ʧ�ܷ��ط�0. 
	*/
	virtual int IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char priority, unsigned char* serial_num,
		unsigned char* buff, int& buff_len) = 0;

	/** @brief �㲥.ͨ��SerialNumberд��ַ. 
	* @param host ����֡�ı�����ַ. 
	* @param priority ���ȼ�. 
	* @param new_addr �Ӹõ�ַȡ����bytes��ΪҪ���õĵ�ַ. 
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	*/
	virtual int IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char priority, 
		const unsigned char* serial_num, const unsigned char* new_addr) = 0;

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
	virtual int Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, int& recv_buff_len) = 0;

	/** @brief ���ü����շ�֡�Ļص�����. 
	*/
	virtual Reporter SetMonitor(Reporter r) = 0;

	/** @brief �����ŵ�,�յ�����֡�󽻸�ÿ���豸.����豸��Ҫ�ظ��ͷ�����Ҫ�ظ���֡����ȥ. 
	* ���Ǹ��첽�汾.���ں�̨���߳�.������������.��Ҫ�����յ���������Щ����.��Ҫ���ûص�����SetMonitor.
	*/
	virtual int Listen_Async() = 0;

	/** @brief �����ŵ����յ�����֡�󽻸�ÿ���豸.����豸��Ҫ�ظ��ͻ᷵����Ҫ�ظ���֡.�����ͬ���汾.����������. 
	* @param recv_buff ����յ���֡����������.�������������д.�Ͳ���.�μ�recv_buff_len����. 
	* @param recv_buff_len ����ʱָʾ�������ĳ��ȣ�����ʱ.�����������д����д�˵ĳ���.�������д���������ĳ��ȵ��෴��. 
	* @param send_buff ��ѷ��͵�֡����������.�������������д.�Ͳ���.�μ�send_buff_len����.
	* @param send_buff_len ����ʱָʾ�������ĳ���,����ʱ.�����������д����д���˶����ֽ�.�������д���������ĳ��ȵ��෴���� 
	* @return ����յ��ᷨ��֡�����ͷ���0.���򷵻ط�0. 
	*/
	virtual int Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len) = 0;

	/** @brief ����һ���µ����豸. 
	* @param template_file ��һ��ģ���ļ�.ָ����ô�������豸.�����NULL,������һ��û���κνӿڶ�������������豸. 
	* @return ���������������ID��.������� 0xffffffff ��ʧ��.
	*/
#define INVALID_DEVICE_ID -1
	virtual unsigned int make_dev(const char* template_file) = 0;

	/** @brief ɾ��һ�����豸. 
	* @param dev_id Ҫɾ�������豸��ID��. 
	* @return �ɹ�ʱ����0.���ɹ����س�����. 
	*/
	virtual int remove_dev(unsigned int dev_id) = 0;

	/** @brief �������е����豸. 
	* @param buff �������豸ID�ŵĻ�����,ÿ�����ֽ�һ��ID��. 
	* @param ����ʱ��ʾ�������Ĵ�С.����ʱ��ʾ��������ʹ�õĳ���.����������������ĳ��ȵ��෴��.���ȵĵ�λ���ĸ�octec. 
	* @return �ɹ�ʱ���ع��ж������豸ID�ű����Ƶ�������.����������������򷵻ظ���. 
	*/
	virtual int get_all_devs(unsigned int* buff, int& buff_len) = 0;

	/** @brief �õ����豸�����У��������,����ID����.���뻺��. 
	* @param dev_id ��־���豸���豸ID��. 
	* @param buff ������(�������,����ID)�ԵĻ�����.�������ռһ��byte,����IDҲռһ��byte. 
	* @param buff_len ����ʱ��ʾ�������Ĵ�С.����ʱ��ʾ��������ʹ�õĳ���.����������������ĳ��ȵ��෴��. 
	* @return �ɹ�ʱ���ع��ж���(�������,����ID)�Ա����Ƶ�������.����������������򷵻ظ���. 
	*/
	virtual int get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len) = 0;

	/** @brief �����豸�ı�����ַ. 
	 */
	virtual int set_host_addr(unsigned int dev_id, const unsigned char* hostaddr) = 0;

	/** @brief �õ��豸�ı�����ַ. 
     */
	virtual int get_host_addr(unsigned int dev_id, char* addr) = 0;

	/** @brief �õ�һ��Ԫ����ռ�ĳ���.��λΪbit. 
	* @param dev_id ��־���豸���豸ID��. 
	* @param object_index �������. 
	* @param prop_id ����ID. 
	* @return �ɹ�����������ΪԪ����ռ����,��λΪbit.ʧ�ܷ��ط�0.����û�����(����,����)��. 
	*/
	virtual int get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief �õ�Ԫ�صĸ���. 
	* @param dev_id ��־���豸���豸ID��. 
	* @return ����Ԫ�صĸ���.������������(����,����)���򷵻ظ���. 
	*/
	virtual int get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief ͨ��������ź�����ID�ŵõ�һ�����������Ľṹ��. 
	* @param dev_id ��־���豸���豸ID��. 
	* @return �ɹ�����0.ʧ�ܷ��ط�0. 
	*/
	virtual int get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, PropertyFields& fields) = 0;

	/** @brief ͨ��������ź�����ID����һ����������. 
	* @return û��û��������������ᱨ��. 
	*/
	virtual int set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields) = 0;

	/** @brief ����һ����������. 
	* @return ����Ѿ������������������ᱨ��. 
	*/
	virtual int add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields) = 0;

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
		unsigned char count, unsigned char* buff, int& buff_len) = 0;

	/** @brief ����һЩԪ�ص�ֵ.���뻺����. 
	* �μ�get_elem_values.�����. 
	*/
	virtual int set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int buff_len) = 0;

	/** @brief �õ�ĳ�豸���ж��ٸ������. 
	* @return ���ع��ж��ٸ������. 
	*/
	virtual int get_gobj_count(unsigned int dev_id) = 0;

	/** @brief �ṩ������������ɾ��һ�������.  
	* �����豸ID�ź������ţ������Ŵ�0��ʼ����. 
	* @return �ɹ�ʱ����0��ʧ��ʱ���س�����. 
	*/
	// virtual int rm_gobj_byidx(unsigned int dev_id, unsigned char index) = 0;

	/** @brief �õ�ĳ�������GroupObjectHead. 
	* �����豸ID�ź�������������. 
	* @gobj_head �����ɹ�ʱ��gobj_head��������.  
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	*/
	virtual int get_gobjhead_byidx(unsigned int dev_id, unsigned char index, GroupObjectHead& gobj_head) = 0;

	/** @brief �õ�ĳ��������ֵ.���뻺����. 
	* @param dev_id �豸ID. 
	* @param index ������INDEX. 
	* @param buff ����. 
	* @param len ����ʱ��ʾ�������ĳ���.��������ʱ��ֵ��ʾ���屻ʹ���˶���byte. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/ 
	virtual int get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len) = 0;

	/** @brief ͨ�����������д������ֵ. 
	 * @param dev_id �豸ID. 
	 * @param index ���������.    
	 * @param buff ����ֵ���׵�ַ. 
	 * @param only_send_when_changed ֻ�и�������ֵ������²Ż��������Ϸ��͸�����ֵ��֡. 
	 * @return �ɹ�ʱ����0.���򷵻ط�0. 
	 */
	virtual int set_gobj_value_byindex(unsigned int dev_id, unsigned char index, unsigned char* buff, bool only_send_when_changed) = 0;

	/** @brief ͨ�������ĵ�ַд������ֵ. 
	 * @param dev_id �豸ID.
	 * @param gaddr ���ַ. 
	 * @param buff ����ֵ���׵�ַ. 
	 * @param only_send_when_changed ֻ�и�������ֵ������²Ż��������Ϸ��͸�����ֵ��֡. 
	 * @return �ɹ�ʱ����0,���򷵻ط�0.  
	 */
	virtual int set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, unsigned char* buff, bool only_send_when_changed) = 0;

	/// �������������Խӵ��߼���Ԫ.�μ�IDevice�ӿ��е����ƺ���. 
	virtual bool DoesThisSoftUnitExist(unsigned int dev_id, const char* softunit_name) = 0;
	virtual const unsigned char* ReadSoftUnit(unsigned int dev_id, const char* softunit_name, unsigned char* buff) = 0;
	virtual int WriteSoftUnit(unsigned int dev_id, const char* softunit_name, const unsigned char* value) = 0;
	
	virtual ~IEIBEmulator(){};
};

#endif
