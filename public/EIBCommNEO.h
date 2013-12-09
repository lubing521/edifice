#ifndef _H_EIBCOMMNEO_
#define _H_EIBCOMMNEO_

#include "global.h"
#include "prot_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

	// according to the control macro, deciding whether export or import functions. 
#undef _EXPORT_OR_IMPORT_EIBCOMMNEO_

#ifndef __GNUC__   // VC������. 
	// according to the control macro, deciding whether export or import functions
	#ifdef _I_AM_EIBCOMMNEO_DLL_
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_  __declspec(dllexport)
	#else
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_  __declspec(dllimport)
	#endif
#endif

#ifdef __GNUC__   // GCC������
	#ifdef _I_AM_EIBCOMMNEO_DLL_
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_  __attribute__((visibility("default")))
	#else
	    // linux���������DLL����,�Ͱ�����궨��Ϊ��. 
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_
	#endif
#endif

	/** @brief ���������ļ���·��. 
	 * NOTICE ������������ڵ���������������֮ǰ����Ч. 
	 * ������������Ǳ��뱻���õ�.��������ô˺�����dllҲ��ʹ��Ĭ�ϵ�EIBComm.ini. 
	 * �����Ҫ��������������������ļ���·��.������������������֮ǰ���ò���Ч. 
	 * @param ini_file_path �����ļ���·��.���ΪNULL������Ĭ�ϵ� EIBComm.ini. 
	 * @return û�з���ֵ.��������ļ��޷�������߷Ƿ���ᱨ������,���̻������˳�. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ void SetINIPath(const char* ini_path);

	/** @brief �ɵĳ�ʼ������.�ô���ͨѶ. 
	* @param port ����COM�˿ں�.���翪COM5������Ϊ5. 
	* @param baudrate ���ò�����.���� 115200. 
	* @param hostaddr ���ñ�����ַ. 
	* @return �ɹ�����0.�����0. 
	*/  
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Init(int port, int baudrate, const unsigned char* hostaddr);

	/** @brief ��ʼ������.�ô���ͨѶ.ͬInit. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Init_sp(int port, int baudrate, const unsigned char* hostaddr);

	/** @brief ��ʼ������.��UDPͨѶ. 
	 * @param �������ĸ�IP��ַ���ⷢUDP��.������ָ���ʽ��IP��ַ�ַ���.  
     * @param �󶨵������ĸ��˿�.������˿ڷ���UDP��Ҳ������˿ڽ���UDP��.  
	 * @param ������EIB��ַ. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Init_udp(const char* ip, ushort port, const unsigned char* hostaddr);

	/** @brief ����ʼ��. 
	* ������Ҫ���������г��ĺ���ʱ,��Ӧ��֮ǰ���õ�eInit������eUninit�ͷ���Դ. 
	* @return �ɹ�����0.�����0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Uninit();

	/** @brief ����/�ı䱾���ĵ�ַ. 
	* @param hostaddr �µı�����ַ.���ΪNULLָ�룬�򲻸ı����ڵĵ�ַ.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ void SetHostAddr(const unsigned char* hostaddr);

	/// �õ����ն����ﹲ�ж��������͵�APDU.
	/// ����ʱszָʾtypes�����������ܴ���ٸ�enum.����ʱ���bytesָʾ�����鹻�ã���szָʾʵ�ʴ��˶��ٸ�.��ʱ��������0. 
	/// ������鲻���ã��򷵻غ�����0,��sz����Ϊ���������������෴��. 
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int QueryAPDUTypesInRxQueue(APDUType types[], int& sz);

	/// ����������������֡.����һ��FramePeek�ṹ�������.
	/// filter ������ APDUType ������ָ������Ҫ����֡.���filter�� AT_UNAVAILABLE�򷵻����е�֡.
	/// ��Ҫ��� CleanFramePeek �ͷŵ�����κ���������ڴ�. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ش�����.
#define FRAME_PEEKER_BUSY -1      // ��һС��ʱ����û�о���������������ΪBUSY.
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count);

	/// �� FramePeeker �������ص� FramePeek �ĵ�ַ����.�ͷ���Դ. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int ClearFramePeek(void* addr);

	/// �ӽ��ն�����ɾ��ĳ��֡.���� FramePeeker �еõ���FramePeek�ṹ���־Ҫɾ���Ľ��ն����е�֡. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ط�0.
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int RemoveFrameFromRxQueue(const FramePeek& peek);

	/** @brief ����һ��FramePeek�ṹ��. 
	 * @param peek ����FramePeek�ṹ�������.
	 * @param buff ��������ֵ�Ļ�����.
	 * @param data_len_in_bit ��λbit.����ʱ��ʾ�������ĳ���,ע�ⵥλbit.����ʱdata_len_in_bit��ʾʵ��ʹ�õĳ���,��λҲ��bit.��Ϊ��Щ��ֵ��bitΪ��λ��������. 
	 * @return �ɹ�ʱ����0,ʧ��ʱ���ط�0.���FramePeek����д��ֵ��֡��ʧ��,���buff����Ҳʧ��. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Parse_GroupValue_Write(const FramePeek& peek, UCHAR* buff, USHORT& data_len_in_bit);

	/** @brief ���ü����շ�֡�Ļص�����. 
	*/
	typedef void (*Reporter)(const unsigned char* buff, int len, const void* arg);
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ Reporter SetMonitor(Reporter r);

	/** @brief ��ֵ. 
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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, 
		unsigned char& count, unsigned short start, unsigned char* buff, int& buff_len);

	/** @brief дֵ. 
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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, unsigned char count, 
		unsigned short start, const unsigned char* buff, int buff_len);

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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Read2(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, unsigned short start, 
		unsigned char* buff, int& buff_len);

	/** @brief дֵ2. 
	* �ο� PropertyValue_Read2ͬPropertyValue_Write.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Write2(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, 
		unsigned short start, const unsigned char* buff, int buff_len);

	/** @brief ����������.  
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�. 
	* @param object_index �����index����. 
	* @param property_id ���Ե�ID��. 
	* @param prop_fields ����һ��PropertyFields�ṹ�������.����ʱ���������Ե�����. 
	* @return �ɹ�����0.�����0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Property_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, 
		unsigned char property_id, PropertyFields& prop_fields);

	/** @brief д��������. 
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�. 
	* @param object_index �����index����. 
	* @param property_id ���Ե�ID��. 
	* @param prop_fields ����һ��PropertyFields�ṹ�������.�ṹ�����ֵ����д�뵽��������.  
	* @return �ɹ�����0.�����0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Property_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, 
		unsigned char property_id, const PropertyFields& prop_fields);

	/** @brief ����Կ����ͨ�Ż��,�����Ӧ��Ȩ�޵ȼ�. 
	* @param peer �Ӹõ�ַȡ����byte��������ΪĿ�����ַ. 
	* @param priority ���ȼ�. 
	* @param key �Ӹõ�ַȡ�ĸ�byte��Ϊ��Կ. 
	* @param level ��������ɹ���ѵõ���Ȩ�޵ȼ�д��õ�ַָ���byte.
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Authorize_Query(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, const unsigned char* key, unsigned char* level);

	/** @brief ͨ�����ַ��ֵ. 
	* @param peer Ҫ���������ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������.������ֵ�������buff. 
	* @param buff_len ��������.�����Ȳ��������᷵�ط�0�����������ȵ��෴������buff_len. 
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int GroupValue_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char* buff, int& buff_len );

	/** @brief ͨ�����ַдֵ. 
	* @param peer Ҫ���������ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������.Ҫд��ֵ�����buff. 
	* @param buff_len_in_bit Ҫд�೤.��λ��BIT. NOTICE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
	* @return �ɹ�ʱ����0,���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int GroupValue_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, const unsigned char* buff, int buff_len_in_bit);

	/** @brief ��ͨѶ����һЩ����. 
	* @param peer Ҫ�����ĵ�ַ(����ַ). 
	* @param priority ���ȼ�. 
	* @param group_obj_num ָ�������ı�������. ??
	* @param start_index �����������������ַ���е�һ������������ַ������. 
	* @param buff �������׵�ַ.���ɹ����Ѱѵõ���������ַ�б����û���.ÿ�����ַ����byte. 
	* @param buff_len ����ʱ��ʾ�������Ĵ�С.�����ɹ�ʱ,buff_len����Ϊ������ʹ�õĳ���.���������ַ�ĸ������Զ���
	* ���������̫С�治�����ַ���������ط�0.���������Ļ��������ȵ��෴������buff_len. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Link_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char group_obj_num, unsigned char start_index, unsigned char* buff, int& buff_len);

	/** @brief ���ӻ�ɾ��ͨѶ��������. 
	* @param peer Ҫ�����ĵ�ַ(����ַ). 
	* @param priorify ���ȼ�. 
	* @param group_obj_num ָ�������ı�������. 
	* @param flags ָ�����ӻ���ɾ��..0x00ɾ��,0x01���.   
	* @param group_addr ���ַ. 
	* @return �ɹ�ʱ����0.���򷵻س�����. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Link_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char group_obj_num, unsigned char flags, const unsigned char* group_addr);

	/** @brief ���û��ڴ�����. 
	* @param peer Ҫ�����ĵ�ַ(��Ե㵥��ַ). 
	* @param priority ���ȼ�. 
	* @param address �ڴ��ַ.��Ч��Χ 0x00000000 -- 0x000FFFFF.����20λ��Ч.��Ѱַһ���ڴ�. 
	* @param number Ҫȡ���ٸ���λ��.��Ч��Χ 1 - 11. 
	* @param buff ȡ���İ�λ������buff��Ҫȷ��buff��ʼ��number���ֽ��ǿ�д��ȫд���.  
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int UserMemory_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority,
		unsigned char hop_count_type, unsigned int address, unsigned char number, unsigned char* buff);

	/** @brief д�û��ڴ�����. 
	* @param peer Ҫ�����ĵ�ַ(��Ե㵥��ַ). 
	* @param priority ���ȼ�. 
	* @param address �ڴ��ַ.��Ч��Χ 0x00000000 -- 0x000FFFFF.����20λ��Ч.��Ѱַһ���ڴ�. 
	* @param number Ҫȡ���ٸ���λ��.��Ч��Χ 1 - 11. 
	* @param buff ��buff��ʼ��number���ֽڱ�д��.   
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int UserMemory_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority,
		unsigned char hop_count_type, unsigned int address, unsigned char number, unsigned char* buff);


	/** @brief �㲥.��ͨ�Ż��ĵ�ַ. 
	* ��Ҫ�ֶ������豸�Ƿ���Ӧ�ø�֡.Ĭ�ϲ���Ӧ. 
	* @param host ����֡�ı�����ַ. 
	* @param priority ���ȼ�. 
	* @param buff ������ �����ĵ�ַ��������˴˻���.
	* @param buff_len ����ʱ�ǻ������Ĵ�С������ʱָʾ������ʹ�õĴ�С.���������������,�򷵻�������С���෴��. 
	* @return �ɹ�����0,ʧ�ܷ��ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddress_Read(const unsigned char* host, unsigned char ack_request, 
		unsigned char priority, unsigned char hop_count_type, unsigned char* buff, int& buff_len);

	/** @brief �㲥.�޸�ͨ�Ż��ĵ�ַ. 
	* ��Ҫ�ֶ������豸�Ƿ���Ӧ��֡.Ĭ�ϲ���Ӧ��֡. 
	* @param host ������ַ. 
	* @param priority ���ȼ�. 
	* @param new_address ͨ�Ż����µ�ַ. 
	* @return �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddress_Write(const unsigned char* host, unsigned char ack_request, 
		unsigned char priority, unsigned char hop_count_type, const unsigned char* new_address);


	/** @brief �㲥.ͨ��SerialNumber�ҵ�ַ. 
	* @param host ����֡�ı�����ַ. 
	* @param priority ���ȼ�. 
	* @param serial_num �������ַȡ����byte��Ϊserial number. 
	* @param buff ������ �����ĵ�ַ��������˻�����. 
	* @param buff_len ����ʱ�ǻ������Ĵ�С������ʱָʾ������ʵ��ʹ�õĴ�С.��������������ã��򷵻�������С���෴��. 
	* @return �ɹ�����0��ʧ�ܷ��ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char ack_request, 
		unsigned char priority, unsigned char hop_count_type, unsigned char* serial_num, unsigned char* buff, int& buff_len);

	/** @brief �㲥.ͨ��SerialNumberд��ַ. 
	* @param host ����֡�ı�����ַ. 
	* @param priority ���ȼ�. 
	* @param new_addr �Ӹõ�ַȡ����bytes��ΪҪ���õĵ�ַ. 
	* @return �ɹ�ʱ����0,ʧ��ʱ���ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char ack_request,
		unsigned char priority, unsigned char hop_count_type, const unsigned char* serial_num, const unsigned char* new_addr);

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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, 
		int& recv_buff_len);

	/** @brief (Ҫ�����ľɽӿ�.Ϊ�˼���.�ڲ�ʵ�ֺ;ɰ治һ��)�����ŵ�,�յ�����֡�󽻸�ÿ���豸.����豸��Ҫ�ظ��ͷ�����Ҫ�ظ���֡����ȥ. 
	* ���Ǹ��첽�汾.���ں�̨���߳�.������������.��Ҫ�����յ���������Щ����.��Ҫ���ûص�����SetMonitor.
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Listen_Async();

	/** @brief (Ҫ�����ľɽӿ�.Ϊ�˼���.�ڲ�ʵ�ֺ;ɰ治һ��)�����ŵ����յ�����֡�󽻸�ÿ���豸.����豸��Ҫ�ظ��ͻ᷵����Ҫ�ظ���֡.�����ͬ���汾.����������. 
	* @param recv_buff ����յ���֡����������.�������������д.�Ͳ���.�μ�recv_buff_len����. 
	* @param recv_buff_len ����ʱָʾ�������ĳ��ȣ�����ʱ.�����������д����д�˵ĳ���.�������д���������ĳ��ȵ��෴��. 
	* @param send_buff ��ѷ��͵�֡����������.�������������д.�Ͳ���.�μ�send_buff_len����.
	* @param send_buff_len ����ʱָʾ�������ĳ���,����ʱ.�����������д����д���˶����ֽ�.�������д���������ĳ��ȵ��෴��.
	* @param waiting_time ���ȶ��ٺ��뺯���ͷ���.���Ϊ-1,�ͻ����յ����ݻ�������. 
	* @return ����յ��ᷨ��֡�����ͷ���0.�����ʱ����74(��ʱг��). ���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len, uint waiting_time);
	
	/** @brief (Ҫ�����ľɽӿ�.Ϊ�˼���)����һ��д��ֵ��֡.
	* @param frame ָ��֡���׵�ַ. 
	* @param frame_len ֡�ĳ���. 
	* @param grp_addr �����ɹ������������������ַ. 
	* @param buff �����ɹ�ʱ����ֵд��û���. 
	* @param buff_len ����ʹ�õĳ���. 
	* @return �����һ���Ϸ���д��ֵ��֡���һ������㹻����������ֵ�򷵻�0,���򷵻ط�0.
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int WriteGrpVal_FrameParser(unsigned char* frame, int frame_len, unsigned char* grp_addr,
		unsigned char* buff, unsigned short& buff_len);

	/** @brief (Ҫ�����ľɽӿ�.Ϊ�˼���)����һ��д�ӿڶ����ֵ��֡. 
	* @param frame ֡���׵�ַ.(in) 
	* @param frame_len ֡�ĳ���.(in)
	* @param from ֡���׵�ַ.(out)
	* @param object_index �ӿڶ����������.(out)
	* @param prop_id ����id.(out)
	* @param count д���ٸ�Ԫ��.(out)
	* @param start ���ĸ�Ԫ�ؿ�ʼд,start from 1.(out)
	* @param buff Ҫд��ֵ���뵽���������.(out)
	* @param buff_len ����ʱָʾbuff�ĳ���.����ʱָʾbuffʹ�õĳ���.
	* @return to ֡��Ŀ�ĵ�ַ.
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int WriteIObj_FrameParser(unsigned char* frame, int frame_len, unsigned char* from,
		unsigned char* to, unsigned char* object_index, unsigned char* prop_id,
		unsigned char* count, unsigned short* start, 
		unsigned char* buff, int* buff_len);

	///////////////////////////// ���豸�� //////////////////////
	/** @brief ����һ���µ����豸. 
	* @param tmpplate_file ����ģ�������ļ���·��.���Ϊ���򷵻ؿյ��豸.  
	* @return ���������������ID��.������� 0xffffffff ��ʧ��.
	*/
#define INVALID_DEVICE_ID -1
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ unsigned int make_dev(const char* template_file);

	/** @brief ɾ��һ�����豸. 
	* @param dev_id Ҫɾ�������豸��ID��. 
	* @return �ɹ�ʱ����0.���ɹ����س�����. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int remove_dev(unsigned int dev_id);

	/** @brief �������е����豸. 
	* @param buff �������豸ID�ŵĻ�����.
	* @param ����ʱ��ʾ�������Ĵ�С.����ʱ��ʾ��������ʹ�õĳ���.����������������ĳ��ȵ��෴��.���ȵĵ�λ���ĸ�octec. 
	* @return �ɹ�ʱ���ع��ж������豸ID�ű����Ƶ�������.����������������򷵻ظ���. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_all_devs(unsigned int* buff, int& buff_len);

	/** @brief �õ����豸�����У��������,����ID����.���뻺��. 
	 * @param dev_id ��־���豸���豸ID��. 
	 * @param buff ������(�������,����ID)�ԵĻ�����.�������ռһ��byte,����IDҲռһ��byte. 
	 * @param buff_len ����ʱ��ʾ�������Ĵ�С.����ʱ��ʾ��������ʹ�õĳ���.����������������ĳ��ȵ��෴��. 
	 * @return �ɹ�ʱ���ع��ж���(�������,����ID)�Ա����Ƶ�������.����������������򷵻ظ���. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len);

	/** @brief �����豸�ı�����ַ. 
	 * @param ��hostaddrָ��ĵ�ַȡ����byte��Ϊ�豸�ı�����ַ. 
	 * @return �ɹ�����0.ʧ�ܷ��ط�0.����hostaddrΪNULL�����ͻ᷵��ʧ��. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_host_addr(unsigned int dev_id, const unsigned char* hostaddr);

	/** @brief �õ�ĳ�豸�ı�����ַ. 
	 * @param dev_id �豸ID��. 
	 * @param addr �豸�ı�����ַ�ᱻд�뵽addrָ��ĵ�ַ��д����bytes. 
	 * @return �ɹ�����0,���򷵻ط�0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_host_addr(unsigned int dev_id, char* addr); 

	/** @brief �õ��豸��ӳ����Ϣ. 
	 * @param dev_id �豸ID��. �����-1 ���� 0xffffffff ���ʾ�������豸.���豸�ܴ���.�Ҳ��ᱻget_all_devs�õ�.�����������
	 * ���豸��������Ϣ����������Ƿ����֡����.
	 * @param map_info ӳ����Ϣ�������ṹ����.��DevMapInfo�Ķ���.��publicĿ¼�µ� prot_structs.h �ļ��ж���. 
	 * @return ���������������豸�ͷ��ط�0.���򷵻�0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_map_info(unsigned int dev_id, DevMapInfo& map_info);

	/** @brief �����豸��ӳ����Ϣ. 
	 * @param dev_id �� GetMapInfo ������ע��.
	 * @param map_info �� GetMapInfo ������ע��.
	 * @return ���������������豸�ͷ��ط�0,���򷵻�0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_map_info(unsigned int dev_id, const DevMapInfo& map_info);

	/** @brief �õ�һ��Ԫ����ռ�ĳ���.��λΪbit. 
	* @param dev_id ��־���豸���豸ID��. 
	* @param object_index �������. 
	* @param prop_id ����ID. 
	* @return �ɹ�����������ΪԪ����ռ����,��λΪbit.ʧ�ܷ��ط�0.����û�����(����,����)��. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief �õ�Ԫ�صĸ���. 
	* @param dev_id ��־���豸���豸ID��. 
	* @return ����Ԫ�صĸ���.������������(����,����)���򷵻ظ���. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief ͨ��������ź�����ID�ŵõ�һ�����������Ľṹ��. 
	* @param dev_id ��־���豸���豸ID��. 
	* @return �ɹ�����0.ʧ�ܷ��ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, PropertyFields& fields);

	/** @brief ͨ��������ź�����ID����һ����������. 
	* @param dev_id ��־���豸���豸ID��. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief ����һ����������. 
	* @return �������Index������ID�Ѿ������򷵻ط�0,��������ھ�������һ����������. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief �õ�һЩԪ�ص�ֵ.���뻺����. 
	 * @brief dev_id ��־���豸��ID��. 
	 * @param object_index �������. 
	 * @param prop_id ����ID��. 
	 * @param start_index Ԫ�صĿ�ʼ���.��1��ʼ����.  
	 * @param count Ҫ�õ���Ԫ�ظ���. 
	 * @param buff �������׵�ַ. 
	 * @param buff_len ����ʱ��ʾ�������ĳ��ȣ�����ʱ��ʾ������ʹ���˶೤. 
	 * @return �ɹ�ʱ����0.ʧ��ʱ���ط�0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int& buff_len);

	/** @brief ����һЩԪ�ص�ֵ.���뻺����. 
	 * �μ�get_elem_values.�����. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int buff_len);

	/** @brief �õ�ĳ�豸���ж��ٸ������. 
	 * @return ���ع��ж��ٸ������. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_gobj_count(unsigned int dev_id);

	//����ɾ�������. 
	///** @brief �ṩ������������ɾ��һ�������.  
	// * �����豸ID�ź������ţ������Ŵ�0��ʼ����. 
	// * @return �ɹ�ʱ����0��ʧ��ʱ���س�����. 
	// */
	//_EXPORT_OR_IMPORT_EIBCOMMNEO_ int rm_gobj_byidx(unsigned int dev_id, unsigned char index);

	/** @brief �õ�ĳ��������ֵ�ĳ���.��λbit. 
	* �����豸ID�ź�������������. 
	* @return �ɹ�ʱ��������.����������ʾʧ��. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_gobj_valuelen_inbit(unsigned int dev_id, unsigned char index);

	/** @brief �õ�ĳ��������ֵ.���뻺����. 
	* @param dev_id �豸ID. 
	* @param index ������INDEX. 
	* @param buff ����. 
	* @param len ����ʱ��ʾ�������ĳ���.��������ʱ��ֵ��ʾ���屻ʹ���˶���byte. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/ 
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len);

	/** @brief ����ĳ��������ֵ. 
	* @param dev_id �豸ID. 
	* @param index ���������. 
	* @param buff �����׵�ַ. 
	* @param only_send_when_changed Ϊtrueʱ,ֻ������ֵ�ı�ʱ�������Ϸ���д��ֵ��֡.�����Ϊfalse��
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_gobj_value_byindex(unsigned int dev_id, unsigned char index, 
		unsigned char* buff, bool only_send_when_changed);

	/** @brief ����ĳ��������ֵ. 
	* @param dev_id �豸ID. 
	* @param gaddr ���ַ.
	* @param buff �����׵�ַ. 
	* @param only_send_when_changed Ϊtrueʱ,ֻ������ֵ�ı�ʱ�������Ϸ���д��ֵ��֡.�����Ϊfalse��
	* ��ʹ��ֵû�б仯Ҳ���������Ϸ���д��ֵ��֡. 
	* @return �ɹ�ʱ����0.���򷵻ط�0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, 
		unsigned char* buff, bool only_send_when_changed);

	///////////////////// testing code //////////////////////////////
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ const char* __test(const char* str);
}

#endif 