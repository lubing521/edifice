#ifndef _H_EIBCOMMNEO_
#define _H_EIBCOMMNEO_

#include "global.h"
#include "prot_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

	// according to the control macro, deciding whether export or import functions. 
#undef _EXPORT_OR_IMPORT_EIBCOMMNEO_

#ifndef __GNUC__   // VC编译器. 
	// according to the control macro, deciding whether export or import functions
	#ifdef _I_AM_EIBCOMMNEO_DLL_
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_  __declspec(dllexport)
	#else
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_  __declspec(dllimport)
	#endif
#endif

#ifdef __GNUC__   // GCC编译器
	#ifdef _I_AM_EIBCOMMNEO_DLL_
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_  __attribute__((visibility("default")))
	#else
	    // linux下如果不是DLL本身,就把这个宏定义为空. 
		#define _EXPORT_OR_IMPORT_EIBCOMMNEO_
	#endif
#endif

	/** @brief 设置配置文件的路径. 
	 * NOTICE 这个函数必须在调用其它导出函数之前才有效. 
	 * 这个函数并不是必须被调用的.如果不调用此函数则dll也会使用默认的EIBComm.ini. 
	 * 但如果要调用这个函数更改配置文件的路径.则并须在其它导出函数之前调用才有效. 
	 * @param ini_file_path 配置文件的路径.如果为NULL，则用默认的 EIBComm.ini. 
	 * @return 没有返回值.如果配置文件无法载入或者非法则会报告错误后,进程会立即退出. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ void SetINIPath(const char* ini_path);

	/** @brief 旧的初始化函数.用串口通讯. 
	* @param port 设置COM端口号.比如开COM5，则设为5. 
	* @param baudrate 设置波特率.比如 115200. 
	* @param hostaddr 设置本机地址. 
	* @return 成功返回0.否则非0. 
	*/  
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Init(int port, int baudrate, const unsigned char* hostaddr);

	/** @brief 初始化函数.用串口通讯.同Init. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Init_sp(int port, int baudrate, const unsigned char* hostaddr);

	/** @brief 初始化函数.用UDP通讯. 
	 * @param 从主机哪个IP地址向外发UDP包.传入句点分隔形式的IP地址字符串.  
     * @param 绑定到本机哪个端口.从这个端口发出UDP包也从这个端口接收UDP包.  
	 * @param 本机的EIB地址. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Init_udp(const char* ip, ushort port, const unsigned char* hostaddr);

	/** @brief 反初始化. 
	* 若不需要调用下面列出的函数时,对应于之前调用的eInit，调用eUninit释放资源. 
	* @return 成功返回0.否则非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Uninit();

	/** @brief 设置/改变本机的地址. 
	* @param hostaddr 新的本机地址.如果为NULL指针，则不改变现在的地址.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ void SetHostAddr(const unsigned char* hostaddr);

	/// 得到接收队列里共有多少种类型的APDU.
	/// 传入时sz指示types的容量，共能存多少个enum.传出时如果bytes指示的数组够用，则sz指示实际存了多少个.此时函数返回0. 
	/// 如果数组不够用，则返回函数非0,且sz被置为数组期望个数的相反数. 
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int QueryAPDUTypesInRxQueue(APDUType types[], int& sz);

	/// 检索接收列里所有帧.返回一个FramePeek结构体的数组.
	/// filter 可以用 APDUType 的类型指定你需要哪种帧.如果filter是 AT_UNAVAILABLE则返回所有的帧.
	/// 需要配合 CleanFramePeek 释放调用这次函数分配的内存. 
	/// 成功时返回0.失败时返回错误码.
#define FRAME_PEEKER_BUSY -1      // 在一小段时间内没有竞争到互斥锁则认为BUSY.
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count);

	/// 把 FramePeeker 函数返回的 FramePeek 的地址传入.释放资源. 
	/// 成功时返回0.失败时返回非0. 
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int ClearFramePeek(void* addr);

	/// 从接收队列中删除某个帧.传入 FramePeeker 中得到的FramePeek结构体标志要删除的接收队列中的帧. 
	/// 成功时返回0.失败时返回非0.
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int RemoveFrameFromRxQueue(const FramePeek& peek);

	/** @brief 分析一个FramePeek结构体. 
	 * @param peek 传入FramePeek结构体的引用.
	 * @param buff 用来存组值的缓冲区.
	 * @param data_len_in_bit 单位bit.传入时表示缓冲区的长度,注意单位bit.传出时data_len_in_bit表示实际使用的长度,单位也是bit.因为有些组值以bit为单位啊。尼玛. 
	 * @return 成功时返回0,失败时返回非0.如果FramePeek不是写组值的帧就失败,如果buff不够也失败. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Parse_GroupValue_Write(const FramePeek& peek, UCHAR* buff, USHORT& data_len_in_bit);

	/** @brief 设置监视收发帧的回调函数. 
	*/
	typedef void (*Reporter)(const unsigned char* buff, int len, const void* arg);
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ Reporter SetMonitor(Reporter r);

	/** @brief 读值. 
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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, 
		unsigned char& count, unsigned short start, unsigned char* buff, int& buff_len);

	/** @brief 写值. 
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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, unsigned char count, 
		unsigned short start, const unsigned char* buff, int buff_len);

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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Read2(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, unsigned short start, 
		unsigned char* buff, int& buff_len);

	/** @brief 写值2. 
	* 参考 PropertyValue_Read2同PropertyValue_Write.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int PropertyValue_Write2(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, unsigned char property_id, 
		unsigned short start, const unsigned char* buff, int buff_len);

	/** @brief 读属性描述.  
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级. 
	* @param object_index 对象的index号码. 
	* @param property_id 属性的ID号. 
	* @param prop_fields 传入一个PropertyFields结构体的引用.返回时被填入属性的描述. 
	* @return 成功返回0.否则非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Property_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, 
		unsigned char property_id, PropertyFields& prop_fields);

	/** @brief 写属性描述. 
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级. 
	* @param object_index 对象的index号码. 
	* @param property_id 属性的ID号. 
	* @param prop_fields 传入一个PropertyFields结构体的引用.结构体里的值将被写入到属性描述.  
	* @return 成功返回0.否则非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Property_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char object_index, 
		unsigned char property_id, const PropertyFields& prop_fields);

	/** @brief 把密钥发给通信伙伴,获得相应的权限等级. 
	* @param peer 从该地址取两个byte长数据作为目标机地址. 
	* @param priority 优先级. 
	* @param key 从该地址取四个byte做为密钥. 
	* @param level 如果函数成功则把得到的权限等级写入该地址指向的byte.
	* @return 成功时返回0,否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Authorize_Query(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, const unsigned char* key, unsigned char* level);

	/** @brief 通过组地址读值. 
	* @param peer 要发往的组地址. 
	* @param priority 优先级. 
	* @param buff 缓冲区.读到的值将填入该buff. 
	* @param buff_len 缓冲区长.若长度不够函数会返回非0并将期望长度的相反数填入buff_len. 
	* @return 成功时返回0,否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int GroupValue_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char* buff, int& buff_len );

	/** @brief 通过组地址写值. 
	* @param peer 要发往的组地址. 
	* @param priority 优先级. 
	* @param buff 缓冲区.要写的值填入该buff. 
	* @param buff_len_in_bit 要写多长.单位是BIT. NOTICE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
	* @return 成功时返回0,否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int GroupValue_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, const unsigned char* buff, int buff_len_in_bit);

	/** @brief 读通讯伙伴的一些链接. 
	* @param peer 要发往的地址(单地址). 
	* @param priority 优先级. 
	* @param group_obj_num 指定组对象的本地索引. ??
	* @param start_index 在隶属于组对象的组地址表中第一个被传输的组地址的索引. 
	* @param buff 缓冲区首地址.若成功，把把得到的组对象地址列表存入该缓冲.每个组地址两个byte. 
	* @param buff_len 传入时表示缓冲区的大小.函数成功时,buff_len被置为缓冲区使用的长度.它等于组地址的个数乘以二。
	* 如果缓冲区太小存不下组地址，则函数返回非0.并把期望的缓冲区长度的相反数填入buff_len. 
	* @return 成功时返回0.否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Link_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char group_obj_num, unsigned char start_index, unsigned char* buff, int& buff_len);

	/** @brief 增加或删除通讯伙伴的链接. 
	* @param peer 要发往的地址(单地址). 
	* @param priorify 优先级. 
	* @param group_obj_num 指定组对象的本地索引. 
	* @param flags 指定增加还是删除..0x00删除,0x01添加.   
	* @param group_addr 组地址. 
	* @return 成功时返回0.否则返回出错码. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Link_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority, 
		unsigned char hop_count_type, unsigned char group_obj_num, unsigned char flags, const unsigned char* group_addr);

	/** @brief 读用户内存区域. 
	* @param peer 要发往的地址(点对点单地址). 
	* @param priority 优先级. 
	* @param address 内存地址.有效范围 0x00000000 -- 0x000FFFFF.即低20位有效.可寻址一兆内存. 
	* @param number 要取多少个八位组.有效范围 1 - 11. 
	* @param buff 取到的八位组会存入buff，要确保buff开始的number个字节是可写安全写入的.  
	* @return 成功时返回0,失败时返回非0.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int UserMemory_Read(unsigned char* peer, unsigned char ack_request, unsigned char priority,
		unsigned char hop_count_type, unsigned int address, unsigned char number, unsigned char* buff);

	/** @brief 写用户内存区域. 
	* @param peer 要发往的地址(点对点单地址). 
	* @param priority 优先级. 
	* @param address 内存地址.有效范围 0x00000000 -- 0x000FFFFF.即低20位有效.可寻址一兆内存. 
	* @param number 要取多少个八位组.有效范围 1 - 11. 
	* @param buff 从buff开始的number个字节被写入.   
	* @return 成功时返回0,失败时返回非0.  
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int UserMemory_Write(unsigned char* peer, unsigned char ack_request, unsigned char priority,
		unsigned char hop_count_type, unsigned int address, unsigned char number, unsigned char* buff);


	/** @brief 广播.读通信伙伴的地址. 
	* 需要手动设置设备是否响应该该帧.默认不响应. 
	* @param host 发出帧的本机地址. 
	* @param priority 优先级. 
	* @param buff 缓冲区 反馈的地址将被填入此此缓冲.
	* @param buff_len 传入时是缓冲区的大小，返回时指示缓冲区使用的大小.如果缓冲区不够用,则返回期望大小的相反数. 
	* @return 成功返回0,失败返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddress_Read(const unsigned char* host, unsigned char ack_request, 
		unsigned char priority, unsigned char hop_count_type, unsigned char* buff, int& buff_len);

	/** @brief 广播.修改通信伙伴的地址. 
	* 需要手动设置设备是否响应该帧.默认不响应该帧. 
	* @param host 本机地址. 
	* @param priority 优先级. 
	* @param new_address 通信伙伴的新地址. 
	* @return 成功时返回0.失败时返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddress_Write(const unsigned char* host, unsigned char ack_request, 
		unsigned char priority, unsigned char hop_count_type, const unsigned char* new_address);


	/** @brief 广播.通过SerialNumber找地址. 
	* @param host 发出帧的本机地址. 
	* @param priority 优先级. 
	* @param serial_num 从这个地址取六个byte作为serial number. 
	* @param buff 缓冲区 反馈的地址将被填入此缓冲区. 
	* @param buff_len 传入时是缓冲区的大小，返回时指示缓冲区实际使用的大小.如果缓冲区不够用，则返回期望大小的相反数. 
	* @return 成功返回0，失败返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddressSerialNumber_Read(const unsigned char* host, unsigned char ack_request, 
		unsigned char priority, unsigned char hop_count_type, unsigned char* serial_num, unsigned char* buff, int& buff_len);

	/** @brief 广播.通过SerialNumber写地址. 
	* @param host 发出帧的本机地址. 
	* @param priority 优先级. 
	* @param new_addr 从该地址取两个bytes作为要设置的地址. 
	* @return 成功时返回0,失败时返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int IndividualAddressSerialNumber_Write(const unsigned char* host, unsigned char ack_request,
		unsigned char priority, unsigned char hop_count_type, const unsigned char* serial_num, const unsigned char* new_addr);

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
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Send_Rawdate(const unsigned char* raw_data, int raw_data_len, unsigned char* recv_buff, 
		int& recv_buff_len);

	/** @brief (要废弃的旧接口.为了兼容.内部实现和旧版不一样)监听信道,收到完整帧后交给每个设备.如果设备需要回复就返回需要回复的帧发出去. 
	* 这是个异步版本.会在后台起线程.函数立即返回.如要监听收到发送了哪些内容.需要设置回调函数SetMonitor.
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Listen_Async();

	/** @brief (要废弃的旧接口.为了兼容.内部实现和旧版不一样)监听信道，收到完整帧后交给每个设备.如果设备需要回复就会返回需要回复的帧.这个是同步版本.函数会阻塞. 
	* @param recv_buff 会把收到的帧填到这个缓冲区.如果缓冲区不够写.就不填.参见recv_buff_len参数. 
	* @param recv_buff_len 传入时指示缓冲区的长度，返回时.如果缓冲区够写就是写了的长度.如果不够写就是期望的长度的相反数. 
	* @param send_buff 会把发送的帧填到这个缓冲区.如果缓冲区不够写.就不填.参见send_buff_len参数.
	* @param send_buff_len 传入时指示缓冲区的长度,返回时.如果缓冲区够写就是写入了多少字节.如果不够写就是期望的长度的相反数.
	* @param waiting_time 最多等多少毫秒函数就返回.如果为-1,就会起到收到数据或发生错误. 
	* @return 如果收到舍法的帧函数就返回0.如果超时返回74(超时谐音). 否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int Listen_Sync(unsigned char* recv_buff, int& recv_buff_len, unsigned char* send_buff, int& send_buff_len, uint waiting_time);
	
	/** @brief (要废弃的旧接口.为了兼容)分析一个写组值的帧.
	* @param frame 指向帧的首地址. 
	* @param frame_len 帧的长度. 
	* @param grp_addr 函数成功则从这个参数返回组地址. 
	* @param buff 函数成功时把组值写入该缓冲. 
	* @param buff_len 缓冲使用的长度. 
	* @return 如果是一个合法的写组值的帧并且缓冲区足够存下组对象的值则返回0,否则返回非0.
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int WriteGrpVal_FrameParser(unsigned char* frame, int frame_len, unsigned char* grp_addr,
		unsigned char* buff, unsigned short& buff_len);

	/** @brief (要废弃的旧接口.为了兼容)分析一个写接口对象的值的帧. 
	* @param frame 帧的首地址.(in) 
	* @param frame_len 帧的长度.(in)
	* @param from 帧的首地址.(out)
	* @param object_index 接口对象的索引号.(out)
	* @param prop_id 属性id.(out)
	* @param count 写多少个元素.(out)
	* @param start 从哪个元素开始写,start from 1.(out)
	* @param buff 要写的值填入到这个缓冲区.(out)
	* @param buff_len 传入时指示buff的长度.传出时指示buff使用的长度.
	* @return to 帧的目的地址.
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int WriteIObj_FrameParser(unsigned char* frame, int frame_len, unsigned char* from,
		unsigned char* to, unsigned char* object_index, unsigned char* prop_id,
		unsigned char* count, unsigned short* start, 
		unsigned char* buff, int* buff_len);

	///////////////////////////// 软设备用 //////////////////////
	/** @brief 生成一个新的软设备. 
	* @param tmpplate_file 传入模板配置文件的路径.如果为空则返回空的设备.  
	* @return 生功返回软件备的ID号.如果返回 0xffffffff 则失败.
	*/
#define INVALID_DEVICE_ID -1
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ unsigned int make_dev(const char* template_file);

	/** @brief 删除一个软设备. 
	* @param dev_id 要删除的软设备的ID号. 
	* @return 成功时返回0.不成功返回出错码. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int remove_dev(unsigned int dev_id);

	/** @brief 返回所有的软设备. 
	* @param buff 接收软设备ID号的缓冲区.
	* @param 传入时表示缓冲区的大小.传出时表示缓冲区被使用的长度.如果出错则是期望的长度的相反数.长度的单位是四个octec. 
	* @return 成功时返回共有多少软设备ID号被复制到缓冲区.如果缓冲区不够用则返回负数. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_all_devs(unsigned int* buff, int& buff_len);

	/** @brief 得到该设备的所有（对象序号,属性ID）对.存入缓冲. 
	 * @param dev_id 标志该设备的设备ID号. 
	 * @param buff 用来存(对象序号,属性ID)对的缓冲区.对象序号占一个byte,属性ID也占一个byte. 
	 * @param buff_len 传入时表示缓冲区的大小.传出时表示缓冲区被使用的长度.如果出错则是期望的长度的相反数. 
	 * @return 成功时返回共有多少(对象序号,属性ID)对被复制到缓冲区.如果缓冲区不够用则返回负数. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_all_idpairs(unsigned int dev_id, unsigned char* buff, int& buff_len);

	/** @brief 更改设备的本机地址. 
	 * @param 从hostaddr指向的地址取两个byte作为设备的本机地址. 
	 * @return 成功返回0.失败返回非0.比如hostaddr为NULL函数就会返回失败. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_host_addr(unsigned int dev_id, const unsigned char* hostaddr);

	/** @brief 得到某设备的本机地址. 
	 * @param dev_id 设备ID号. 
	 * @param addr 设备的本机地址会被写入到addr指向的地址，写两个bytes. 
	 * @return 成功返回0,否则返回非0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_host_addr(unsigned int dev_id, char* addr); 

	/** @brief 得到设备的映射信息. 
	 * @param dev_id 设备ID号. 如果填-1 或者 0xffffffff 则表示特殊软设备.该设备总存在.且不会被get_all_devs得到.根据这个特殊
	 * 软设备的配置信息，程序决定是否接受帧数据.
	 * @param map_info 映射信息被填到这个结构体里.见DevMapInfo的定句.在public目录下的 prot_structs.h 文件中定义. 
	 * @return 如果不存在这个软设备就返回非0.否则返回0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_map_info(unsigned int dev_id, DevMapInfo& map_info);

	/** @brief 设置设备的映射信息. 
	 * @param dev_id 见 GetMapInfo 函数的注释.
	 * @param map_info 见 GetMapInfo 函数的注释.
	 * @return 如果不存在这个软设备就返回非0,否则返回0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_map_info(unsigned int dev_id, const DevMapInfo& map_info);

	/** @brief 得到一个元素所占的长度.单位为bit. 
	* @param dev_id 标志软设备的设备ID号. 
	* @param object_index 对象序号. 
	* @param prop_id 属性ID. 
	* @return 成功返回正数，为元素所占长度,单位为bit.失败返回非0.比如没有这个(对象,属性)对. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_elemlen_inbit(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief 得到元素的个数. 
	* @param dev_id 标志软设备的设备ID号. 
	* @return 返回元素的个数.如果不存在这个(对象,属性)对则返回负数. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_elemcount(unsigned int dev_id, unsigned char object_index, unsigned char prop_id);

	/** @brief 通过对象序号和属性ID号得到一个属性描述的结构体. 
	* @param dev_id 标志软设备的设备ID号. 
	* @return 成功返回0.失败返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, PropertyFields& fields);

	/** @brief 通过对象序号和属性ID设置一个属性描述. 
	* @param dev_id 标志软设备的设备ID号. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief 增加一个属性描述. 
	* @return 如果对象Index和属性ID已经存在则返回非0,如果不存在就增加了一个属性描述. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int add_prop_fields(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	/** @brief 得到一些元素的值.存入缓冲区. 
	 * @brief dev_id 标志软设备的ID号. 
	 * @param object_index 对象序号. 
	 * @param prop_id 属性ID号. 
	 * @param start_index 元素的开始序号.从1开始计数.  
	 * @param count 要得到的元素个数. 
	 * @param buff 缓冲区首地址. 
	 * @param buff_len 传入时表示缓冲区的长度，传出时表示缓冲区使用了多长. 
	 * @return 成功时返回0.失败时返回非0. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int& buff_len);

	/** @brief 设置一些元素的值.存入缓冲区. 
	 * 参见get_elem_values.差唔多. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_elem_values(unsigned int dev_id, unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
		unsigned char count, unsigned char* buff, int buff_len);

	/** @brief 得到某设备共有多少个组对象. 
	 * @return 返回共有多少个组对象. 
	 */
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_gobj_count(unsigned int dev_id);

	//不可删除组对象. 
	///** @brief 提供组对象的索引号删除一个组对象.  
	// * 传入设备ID号和索引号，索引号从0开始计数. 
	// * @return 成功时返回0，失败时返回出错码. 
	// */
	//_EXPORT_OR_IMPORT_EIBCOMMNEO_ int rm_gobj_byidx(unsigned int dev_id, unsigned char index);

	/** @brief 得到某个组对象的值的长度.单位bit. 
	* 传入设备ID号和组对象的索引号. 
	* @return 成功时返回正数.非正数即表示失败. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_gobj_valuelen_inbit(unsigned int dev_id, unsigned char index);

	/** @brief 得到某个组对象的值.存入缓冲区. 
	* @param dev_id 设备ID. 
	* @param index 组对象的INDEX. 
	* @param buff 缓冲. 
	* @param len 传入时表示缓冲区的长度.函数返回时该值表示缓冲被使用了多少byte. 
	* @return 成功时返回0.否则返回非0. 
	*/ 
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int get_gobj_value(unsigned int dev_id, unsigned char index, unsigned char* buff, int& len);

	/** @brief 设置某个组对象的值. 
	* @param dev_id 设备ID. 
	* @param index 组对象的序号. 
	* @param buff 传入首地址. 
	* @param only_send_when_changed 为true时,只会在组值改变时往总线上发送写组值的帧.而如果为false，
	* @return 成功时返回0.否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_gobj_value_byindex(unsigned int dev_id, unsigned char index, 
		unsigned char* buff, bool only_send_when_changed);

	/** @brief 设置某个组对象的值. 
	* @param dev_id 设备ID. 
	* @param gaddr 组地址.
	* @param buff 传入首地址. 
	* @param only_send_when_changed 为true时,只会在组值改变时往总线上发送写组值的帧.而如果为false，
	* 则即使组值没有变化也会往总线上发送写组值的帧. 
	* @return 成功时返回0.否则返回非0. 
	*/
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ int set_gobj_value_bygaddr(unsigned int dev_id, unsigned char* gaddr, 
		unsigned char* buff, bool only_send_when_changed);

	///////////////////// testing code //////////////////////////////
	_EXPORT_OR_IMPORT_EIBCOMMNEO_ const char* __test(const char* str);
}

#endif 