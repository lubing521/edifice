#ifndef _H_EIBEmulatorImpl_
#define _H_EIBEmulatorImpl_

#include "CommBase.h"
#include "DeviceManager.h"
#include "Pentagon.h"
#include "IEIBEmulator.h"
#include "SerialComm_MM.h"
#include "global.h"

/** @brief 模拟器的功能的实现在这个类啦. 
 * 后面发现的问题是这个类的实现没必要面向接口.未来基本上只会有这样一个实现.但是为了方便
 * 其它模块申请接口.还是留下 IEIBEmulator. 
 */
// class EIBEmulatorImpl : public IEIBEmulator
class EIBEmulatorImpl : public IBase
{
public:
	virtual int Init(const InitData& initdata);
	virtual int Uninit();

	/// 得到接收队列里共有多少种类型的APDU.
	/// 传入时sz指示types的容量，共能存多少个enum.传出时如果bytes指示的数组够用，则sz指示实际存了多少个.此时函数返回0. 
	/// 如果数组不够用，则返回函数非0,且sz被置为数组期望个数的相反数. 
	int QueryAPDUTypesInRxQueue(APDUType types[], int& sz);

	/// 检索接收列里所有帧.返回一个FramePeek结构体的数组.
	/// filter 可以用 APDUType 的类型指定你需要哪种帧.如果filter是 AT_UNAVAILABLE则返回所有的帧.
	/// 需要配合 CleanFramePeek 释放调用这次函数分配的内存. 
	/// 成功时返回0.失败时返回错误码.
#define FRAME_PEEKER_BUSY -1      // 在一小段时间内没有竞争到互斥锁则认为BUSY.
	int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count);

	/// 把 FramePeeker 函数返回的 frames 的地址传入.释放资源. 
	/// 成功时返回0.失败时返回非0. 
	int ClearFramePeek(void* addr);

	/// 从接收队列中删除某个帧.传入 FramePeeker 中得到的FramePeek结构体标志要删除的接收队列中的帧. 
	/// 成功时返回0.失败时返回非0.
	int RemoveFrameFromRxQueue(const FramePeek& peek);

	/** @brief 读值. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级.0--3.    
	* @param object_index 对象的index号码. 
	* @param property_id 属性的ID号. 
	* @param count 函数签名为传入C++引用.指示期望得到多少个值.函数返回时被置为实际取得的值. 
	* 如果用纯C代码.调用则把unsigned char& 理解成 unsigned char*.传指向无符号char的指针. 
	* @param start 从第几个元素开始取值.最大取值 2^14-1. 
	* @param buff 函数返回时,把取到的值填入这个缓冲. 
	* @param buff_len 传入时指示缓冲区的长度.返回时buff_len被置为缓冲区实际使用的长度. 
	* @return 成功返回0.否则非0. 
	*/
	virtual int PropertyValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned char& count, unsigned short start, unsigned char* buff, int& buff_len);


	/** @brief 写值. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级. 
	* @param object_index 对象的index号码. 
	* @param property_id 属性的ID号. 
	* @param count 写多少个元素的值. 
	* @param start 从第几个元素开始写值. 
	* @param buff 要写入的数据的首地址. 
	* @param buff_len 要写入的数据的长度. 
	* @return 成功返回0.否则非0. 
	*/	 
	virtual int PropertyValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned char count, unsigned short start, const unsigned char* buff, int buff_len);

	/** @brief 读值2. 
	 * 区别于PropertyValue_Read 这个函数每次只读一个值.但是start的值可表示2^16-1.而PropertyValue_Read中的start只能取到2^12-1. 
	 * @param peer 从该地址取两个byte长数据作为目标机地址. 
	 * @param priority 优先级. 
	 * @param object_index 对象的index号码. 
	 * @param property_id 属性的ID号. 
	 * @param start 从第几个元素开始取值. 
	 * @param buff 函数返回时,把取到的值填入这个缓冲. 
	 * @param buff_len 传入时指示缓冲区的长度.返回时buff_len被置为缓冲区实际使用的长度. 
	 * @return 成功返回0.否则非0. 
	 */
	virtual int PropertyValue_Read2(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned short start, unsigned char* buff, int& buff_len);

	/** @brief 写值2. 
	 * 参考 PropertyValue_Read2同PropertyValue_Write.  
	 */
	virtual int PropertyValue_Write2(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, unsigned short start, const unsigned char* buff, int buff_len);

	/** @brief 读属性描述.  
	 * @param host 从该地址取两个byte长数据作为发送机地址. 
	 * @param peer 从该地址取两个byte长数据作为目标机地址. 
	 * @param priority 优先级. 
	 * @param object_index 对象的index号码. 
	 * @param property_id 属性的ID号. 
	 * @param prop_fields 传入一个PropertyFields结构体的引用.返回时被填入属性的描述. 
	 * @return 成功返回0.否则非0. 
	 */
	virtual int Property_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, PropertyFields& prop_fields);

	/** @brief 写属性描述. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级. 
	* @param object_index 对象的index号码. 
	* @param property_id 属性的ID号. 
	* @param prop_fields 传入一个PropertyFields结构体的引用.结构体里的值将被写入到属性描述.  
	* @return 成功返回0.否则非0. 
	*/
	virtual int Property_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char object_index, 
		unsigned char property_id, const PropertyFields& prop_fields);

	/** @brief 把密钥发给通信伙伴,获得相应的权限等级. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级. 
	* @param key 从该地址取四个byte做为密钥. 
	* @param level 如果函数成功则把得到的权限等级写入该地址指向的byte.
	* @return 成功时返回0,否则返回非0. 
	*/
	virtual int Authorize_Query(const unsigned char* host, const unsigned char* peer, unsigned char priority, const unsigned char* key, 
		unsigned char* level);

	/** @brief 通过组地址读值. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 要发往的组地址. 
	* @param priority 优先级. 
	* @param buff 缓冲区.读到的值将填入该buff. 
	* @param buff_len 缓冲区长.若长度不够函数会返回非0并将期望长度的相反数填入buff_len. 
	* @return 成功时返回0,否则返回非0. 
	*/
	virtual int GroupValue_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char* buff, 
		int& buff_len );

	/** @brief 通过组地址写值. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 要发往的组地址. 
	* @param priority 优先级. 
	* @param buff 缓冲区.要写的值填入该buff. 
	* @param buff_len_in_bit 缓冲区使用的长度.千万注意这里的长度单位是Bit. 
	* @return 成功时返回0,否则返回非0. 
	*/
	virtual int GroupValue_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, const unsigned char* buff, 
		int buff_len_in_bit);

	/** @brief 读通讯伙伴的一些链接. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 要发往的地址(单地址). 
	* @param priority 优先级. 
	* @param group_obj_num 指定组对象的本地索引. ??
	* @param start_index 在隶属于组对象的组地址表中第一个被传输的组地址的索引. 
	* @param buff 缓冲区首地址.若成功，把把得到的组对象地址列表存入该缓冲.每个组地址两个byte. 
	* @param buff_len 传入时表示缓冲区的大小.函数成功时,buff_len被置为缓冲区使用的长度.它等于组地址的个数乘以二。
	* 如果缓冲区太小存不下组地址，则函数返回非0.并把期望的缓冲区长度的相反数填入buff_len. 
	* @return 成功时返回0.否则返回非0. 
	*/
	virtual int Link_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char group_obj_num, 
		unsigned char start_index, unsigned char* buff, int& buff_len);
 
	/** @brief 增加或删除通讯伙伴的链接. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 要发往的地址(单地址). 
	* @param priorify 优先级. 
	* @param group_obj_num 指定组对象的本地索引. 
	* @param flag 指定增加还是删除. 
	* @param group_addr 组地址. 
	* @return 成功时返回0.否则返回出错码. 
	*/
	virtual int Link_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned char group_obj_num,
		unsigned char flags, const unsigned char* group_addr);

	/** @brief 读用户内存区域. 
	* @param host 从该地址取两个byte长数据作为发送机地址. 
	* @param peer 要发往的地址(点对点单地址). 
	* @param priority 优先级. 
	* @param address 内存地址.有效范围 0x00000000 -- 0x000FFFFF.即低20位有效.可寻址一兆内存. 
	* @param number 要取多少个八位组.有效范围 1 - 11. 
	* @param buff 取到的八位组会存入buff，要确保buff开始的number个字节是可写安全写入的.  
	* @return 成功时返回0,失败时返回非0.  
	*/
	virtual int UserMemory_Read(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned int address, 
		unsigned char number, unsigned char* buff);

	/** @brief 写用户内存区域. 
	 * @param host 从该地址取两个byte长数据作为发送机地址. 
	 * @param peer 要发往的地址(点对点单地址). 
	 * @param priority 优先级. 
	 * @param address 内存地址.有效范围 0x00000000 -- 0x000FFFFF.即低20位有效.可寻址一兆内存. 
	 * @param number 要取多少个八位组.有效范围 1 - 11. 
	 * @param buff 从buff开始的number个字节被写入.   
	 * @return 成功时返回0,失败时返回非0.  
	 */
	virtual int UserMemory_Write(const unsigned char* host, const unsigned char* peer, unsigned char priority, unsigned int address,
			unsigned char number, unsigned char* buff);

	/** @brief 广播.读通信伙伴的地址. 
	 * 需要手动设置设备是否响应该该帧.默认不响应. 
	 * @param host 发出帧的本机地址. 
	 * @param priority 优先级. 
	 * @param buff 缓冲区 反馈的地址将被填入此此缓冲.
	 * @param buff_len 传入时是缓冲区的大小，返回时指示缓冲区使用的大小.如果缓冲区不够用,则返回期望大小的相反数. 
	 * @return 成功返回0,失败返回非0. 
	 */
	virtual int IndividualAddress_Read(const unsigned char* host, unsigned char priority,  unsigned char* buff, int& buff_len);

	/** @brief 广播.修改通信伙伴的地址. 
	* 需要手动设置设备是否响应该帧.默认不响应该帧. 
	* @param host 本机地址. 
	* @param priority 优先级. 
	* @param new_address 通信伙伴的新地址. 
	* @return 成功时返回0.失败时返回非0. 
	*/
	virtual int IndividualAddress_Write(const unsigned char* host, unsigned char priority, const unsigned char* new_address);


	/** @brief 广播.通过SerialNumber找地址. 
	 * @param host 发出帧的本机地址. 
	 * @param priority 优先级. 
	 * @param serial_num 从这个地址取六个byte作为serial number. 
	 * @param buff 缓冲区 反馈的地址将被填入此缓冲区. 
	 * @param buff_len 传入时是缓冲区的大小，返回时指示缓冲区实际使用的大小.如果缓冲区不够用，则返回期望大小的相反数. 
	 * @return 成功返回0，失败返回非0. 
	 */
	virtual int IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char priority, unsigned char* serial_num,
			unsigned char* buff, int& buff_len);

	/** @brief 广播.通过SerialNumber写地址. 
	 * @param host 发出帧的本机地址. 
	 * @param priority 优先级. 
	 * @param new_addr 从该地址取两个bytes作为要设置的地址. 
	 * @return 成功时返回0,失败时返回非0. 
	 */
	virtual int IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char priority, 
			const unsigned char* serial_num, const unsigned char* new_addr);

	/** @brief 发送原始帧数据(非EIB协议定义应用层服务).  
	 * @param raw_data 指向要发送的数据的首地址. 
	 * @param raw_data_len 要发送数据的长度. 
	 * @param recv_buff 接收到回复时把回复存入此缓冲. 
	 * @param recv_buff_len 缓冲区的长度.如果收到回复时缓冲区不够就不会拷贝数据，这个长度被置为期望长度的相反数. 
	 * @return 成功时返回0.否则非0. 几个返回值 SEND_RAWDATA_ILLEGAL_PARAMS(参数错)  
	 * SEND_RAWDATA_NOT_A_FRAME(传入的数据不是一个合未能的帧) SEND_RAWDAAT_NO_LEGAL_RESPONSE(接收回复时超时)
	 */
#define SEND_RAWDATA_ILLEGAL_PARAMS 7
#define SEND_RAWDATA_NOT_A_FRAME 2
#define SEND_RAWDATA_NO_LEGAL_RESPONSE  3
	virtual int Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, 
		int& recv_buff_len);

	/** @brief 设置监视收发帧的回调函数. 
	*/
	virtual Reporter SetMonitor(Reporter r);

	/** @brief 监听信道,收到完整帧后交给每个设备.如果设备需要回复就返回需要回复的帧发出去. 
	* 这是个异步版本.会在后台起线程.函数立即返回.如要监听收到发送了哪些内容.需要设置回调函数SetMonitor.
	*/
	virtual int Listen_Async();

	/** @brief 监听信道，收到完整帧后交给每个设备.如果设备需要回复就会返回需要回复的帧.这个是同步版本.函数会阻塞. 
	* @param recv_buff 会把收到的帧填到这个缓冲区.如果缓冲区不够写.就不填.参见recv_buff_len参数. 
	* @param recv_buff_len 传入时指示缓冲区的长度，返回时.如果缓冲区够写就是写了的长度.如果不够写就是期望的长度的相反数. 
	* @param send_buff 会把发送的帧填到这个缓冲区.如果缓冲区不够写.就不填.参见send_buff_len参数.
	* @param send_buff_len 传入时指示缓冲区的长度,返回时.如果缓冲区够写就是写入了多少字节.如果不够写就是期望的长度的相反数。 
	* @param waiting_time 最多等多少毫秒函数就返回.如果为-1,就会起到收到数据或发生错误. 
	* @return 如果收到舍法的帧函数就返回0.如果超时返回74(超时谐音). 否则返回非0.  
	*/
	virtual int Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len, uint waiting_time);

	/** @brief 生成一个新的软设备. 
	* @param template_file 是一个模板文件.指导怎么创建软设备.如果置NULL,则生成一个没有任何接口对象和组对象的软设备. 
	* @return 生功返回软件备的ID号.如果返回 0xffffffff 则失败.
	*/
#define INVALID_DEVICE_ID -1
	virtual unsigned int make_dev(const char* template_file);

	/** @brief 删除一个软设备. 
	* @param dev_id 要删除的软设备的ID号. 
	* @return 成功时返回0.不成功返回出错码. 
	*/
	virtual int remove_dev(unsigned int dev_id);

	/** @brief 返回所有的软设备. 
	* @param buff 接收软设备ID号的缓冲区,每两个字节一个ID号. 
	* @param 传入时表示缓冲区的大小.传出时表示缓冲区被使用的长度.如果出错则是期望的长度的相反数.长度的单位是四个octec. 
	* @return 成功时返回共有多少软设备ID号被复制到缓冲区.如果缓冲区不够用则返回负数. 
	*/
	virtual int get_all_devs(unsigned int* buff, int& buff_len);

	/** @brief 得到该设备的所有（对象序号,属性ID）对.存入缓冲. 
	* @param dev_id 标志该设备的设备ID号. 
	* @param buff 用来存(对象序号,属性ID)对的缓冲区.对象序号占一个byte,属性ID也占一个byte. 
	* @param buff_len 传入时表示缓冲区的大小.传出时表示缓冲区被使用的长度.如果出错则是期望的长度的相反数. 
	* @return 成功时返回共有多少(对象序号,属性ID)对被复制到缓冲区.如果缓冲区不够用则返回负数. 
	*/
	virtual int get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len);

	/** @brief 设置设备的本机地址. 
	 */
	virtual int set_host_addr(unsigned int dev_id, const unsigned char* hostaddr);

	/** @brief 得到本机地址. 
	 */
	virtual int get_host_addr(unsigned int dev_id, char* addr);

	/** @brief 得到设备的映射信息. 
	* @param dev_id 设备ID号. 如果填-1 或者 0xffffffff 则表示特殊软设备.该设备总存在.且不会被get_all_devs得到.根据这个特殊
	* 软设备的配置信息，程序决定是否接受帧数据.
	* @param map_info 映射信息被填到这个结构体里.见DevMapInfo的定句.在public目录下的 prot_structs.h 文件中定义. 
	* @return 如果不存在这个软设备就返回非0.否则返回0. 
	*/
	virtual int get_map_info(unsigned int dev_id, DevMapInfo& map_info);

	/** @brief 设置设备的映射信息. 
	* @param dev_id 见 GetMapInfo 函数的注释.
	* @param map_info 见 GetMapInfo 函数的注释.
	* @return 如果不存在这个软设备就返回非0,否则返回0. 
	*/
	virtual int set_map_info(unsigned int dev_id, const DevMapInfo& map_info);

	/** @brief 得到一个元素所占的长度.单位为bit. 
	* @param dev_id 标志软设备的设备ID号. 
	* @param object_index 对象序号. 
	* @param prop_id 属性ID. 
	* @return 成功返回正数，为元素所占长度,单位为bit.失败返回非0.比如没有这个(对象,属性)对. 
	*/
	virtual int get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief 得到元素的个数. 
	* @param dev_id 标志软设备的设备ID号. 
	* @return 返回元素的个数.如果不存在这个(对象,属性)对则返回负数. 
	*/
	virtual int get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief 通过对象序号和属性ID号得到一个属性描述的结构体. 
	* @param dev_id 标志软设备的设备ID号. 
	* @return 成功返回0.失败返回非0. 
	*/
	virtual int get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, PropertyFields& fields);

	/** @brief 通过对象序号和属性ID设置一个属性描述. 
	* @return 没有没有这个属性描述会报错. 
	*/
	virtual int set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief 新增一个属性描述. 
	* @return 如果已经存在这个属性描述则会报错. 
	*/
	virtual int add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief 得到一些元素的值.存入缓冲区. 
	 * @brief dev_id 标志软设备的ID号. 
	 * @param object_index 对象序号. 
	 * @param prop_id 属性ID号. 
	 * @param start_index 元素的开始序号. 
	 * @param count 要得到的元素个数. 
	 * @param buff 缓冲区首地址. 
	 * @param buff_len 传入时表示缓冲区的长度，传出时表示缓冲区使用了多长. 
	 * @return 成功时返回0.失败时返回非0. 
	 */
	virtual int get_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int& buff_len);

	/** @brief 设置一些元素的值.存入缓冲区. 
	* 参见get_elem_values.差唔多. 
	*/
	virtual int set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int buff_len);

	/** @brief 得到某设备共有多少个组对象. 
	* @return 返回共有多少个组对象. 
	*/
	virtual int get_gobj_count(unsigned int dev_id);

	/** @brief 提供组对象的索引号删除一个组对象.  
	* 传入设备ID号和索引号，索引号从0开始计数. 
	* @return 成功时返回0，失败时返回出错码. 
	*/
	// virtual int rm_gobj_byidx(unsigned int dev_id, unsigned char index);

	/** @brief 得到某个组对象GroupObjectHead. 
	* 传入设备ID号和组对象的索引号. 
	* @gobj_head 函数成功时把gobj_head填入数据.  
	* @return 成功时返回0,失败时返回非0. 
	*/
	virtual int get_gobjhead_byidx(unsigned int dev_id, unsigned char index, GroupObjectHead& gobj_head);

	/** @brief 得到某个组对象的值.存入缓冲区. 
	* @param dev_id 设备ID. 
	* @param index 组对象的INDEX. 
	* @param buff 缓冲. 
	* @param len 传入时表示缓冲区的长度.函数返回时该值表示缓冲被使用了多少byte. 
	* @return 成功时返回0.否则返回非0. 
	*/ 
	virtual int get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len);

	/** @brief 设置某个组对象的值. 
	* @param dev_id 设备ID. 
	* @param index 组对象的序号.    
	* @param buff 传入首地址. 
	* @param only_send_when_changed 为true时,只会在组值改变时往总线上发送写组值的帧.而如果为false，
	* 则即使组值没有变化也会往总线上发送写组值的帧. 
	* @return 成功时返回0.否则返回非0. 
	*/
	virtual int set_gobj_value_byindex(unsigned int dev_id, unsigned char index, unsigned char* buff, bool only_send_when_changed);

	/** @brief 通过组地址设置组对象的值  */
	virtual int set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, unsigned char* buff, bool only_send_when_changed);

	// 下面三个函数对接到ILogic4Dev.h接口.逻辑块有关.逻辑块由fx_vm.dll实现. 
	virtual bool DoesThisSoftUnitExist(unsigned int dev_id, const char* softunit_name);
	virtual const uchar* ReadSoftUnit(unsigned int dev_id, const char* softunit_name, uchar* buff);
	virtual int WriteSoftUnit(unsigned int dev_id, const char* softunit_name, const uchar* value);

public:
	EIBEmulatorImpl(void);
	virtual ~EIBEmulatorImpl(void);

private:
	// 先发后收. 
	ResponseType tx_rx_sync(const Frame* send_frame, Frame*& recv_frame);

	// 先发后收.一直收到TIME_OUT为止.把收到的帧全部记下来. 
	ResponseType tx_rx_till_timeout_sync(const Frame* send_frame, std::vector<Frame*>& recv_frames); 
	
	// 先收后发.如果waiting_time耗完则退回RESPONSE_TIMEOUT,如果waiting_time为-1.则永远阻塞到收到帧为止. 
	ResponseType rx_tx_sync(uint waiting_time);

	// 根据发出帧算出回复帧这个发出帧的限制条件. 
	void impose_response_restrictions(const Frame* send_frame, std::vector<APDUType>& expected_apdutypes, 
				const uchar*& check_from, const uchar*& check_to);

	// 成功时从得到的frame生成要发送的frames放到vector中.
	// 函数返回0.如果失败则send_frame被填为NULL,函数返回非0. 
	int build_send_frames_from_recv_frame(Frame* recv_frame, std::vector<Frame*>& send_frames);

private:
	friend DWORD WINAPI listen_proc(void * arg);  // 异步的Listen_Async函数起线程. 
	uint _listen_proc();

private:
	bool   m_inited;

	CommBase* m_comm; ///< 提供通讯功能. 
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

	DeviceManager* m_dev_manager; ///< 管理所有设备的数据. 

	volatile bool m_quit_listen_thread;  ///< 中断监听线程. 
	HANDLE  m_listen_event;  ///< 用来和监听线程同步. 
	HANDLE  m_listen_thread;  ///< 监听线程的句柄. 
	
	HANDLE  m_mutex_objs; ///< 操作接口对象和组对象前要获得这个锁. 

	Reporter  m_reporter;
};

#endif