#ifndef _H_IDEVICE_
#define _H_IDEVICE_

#include <vector>
#include "prot_structs.h"
#include "group_object.h"

#define MAX_USER_MEMORY_SIZE 0x100000 

class mem_t;

/** @brief 这是操作设备数据的接口. 
 */
class IDevice
{
public:
	/** @brief 返回该设备的地址的首地址.地址占两个BYTES. 
	 * @param h 如果 h 不为NULL。则通过h传出一个可读十六进制字符串形式的地址. 
	 */
	virtual const unsigned char* GetHostAddr(std::string* h) = 0;

	/** @brief 返回该设备的SerialNum的首地址.地址占6个bytes. 
	 * @param h 如果 h 不为NULL。则通过h传出一个可读十六进制字符串形式的地址. 
	 */
	virtual const unsigned char* GetSerialNum(std::string* h) = 0;

	/** @brief 设置本机地址. 
	 * @return 成功返回0,否则返回非0. 
	 */
	virtual int SetHostAddr(const unsigned char* addr) = 0;

	/** @brief 得到一个元素所占的长度.单位为bit.
	 * @return 成功返回正数，为元素所占长度,单位为bit.失败返回非0.比如没有这个(对象,属性)对. 
	 */
	virtual int GetPerElemLen_inbit(unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief 得到元素的个数. 
	 * @return 返回元素的个数.如果不存在这个(对象,属性)对则返回负数. 
	 */
	virtual int GetElemCount(unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief 删除一个接口对象. 
	 * @return 成功删除返回0.如果不存在这个接口对象则返回非0. 
	 */
	virtual int RemoveIObject(unsigned char object_index, unsigned char prop_id) = 0;

	/** @brief 通过对象序号和属性ID号得到一个属性描述的结构体.
	 * @return 成功返回0.失败返回非0. 
	 */
	virtual int GetPropFields(unsigned char object_index, unsigned char prop_id, PropertyFields& fields) = 0;

	/** @brief 通过对象序号和属性ID设置一个属性描述. 
	 * @return 成功返回0.失败返回非0.如果不存在该对象index,属性id则会返回非0.
	 */
	virtual int SetPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields) = 0;

	/** @brief 增加一个属性描述. 
	 * @return 成功返回0.失败返回非0.如果存在该对象index,属性id。则会返回非0. 
	 */
	virtual int AddPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields) = 0;

	/** @brief 得到一些元素的值.存入缓冲区. 
	 * 对于长度是几个bit的元素.元素值存入buff后会在首地址对齐。 
	 * @start_index 元素的开始序号.从1开始计数.  
	 * @count 要得到的元素个数. 
	 * @buff 缓冲区首地址.
	 * @buff_len 传入时表示缓冲区的长度，传出时表示缓冲区使用了多长. 
	 * @return 成功时返回0.失败时返回非0.  
	 */
	virtual int GetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
						unsigned char count, unsigned char* buff, int& buff_len) = 0;

	/** @brief 设置一些元素的值.存入缓冲区. 
	 * @return 成功时返回0.失败时返回非0. 
	 */
	virtual int SetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, 
						unsigned char count, const unsigned char* buff, int buff_len) = 0;

	/** @brief 返回该设备所有的对象INDEX和属性ID对. 
	 * @param buff 用来存对象idnex和属性id对，共两个byte.
	 * @buff_len 传入时是缓冲区的长度，传出时是使用的长度.如果缓冲区不够存则不会写入数据，此时buff_len被置为期望长度的相反数.  
	 * @return 返回对象index和属性ID对的个数.此时buff_len为函数返回值的两倍.函数失败时函数返回负数.  
	 */
	virtual int GetAllIDPairs(unsigned char* buff, int& buff_len) = 0;

	/** @brief 返回共有多少个组对象. 
	 */
	virtual int GetGroupObjectCount() = 0;

	/** @brief 按索引号返回GroupObjectHead.
	 * 索引号从0开始计数.如果没有这个索引号就返回-1,否则返回0.并把gobj_head填写好.  
	 */
	virtual int GetGroupObjectHeadByIndex(unsigned char index, GroupObjectHead& gobj_head) = 0;

	/** @brief 设置组对象的值. 
     * @param index 组对象的索引号. 
	 * @param mem 从这个地址得到值. 
	 * @return 成功时返回0,失败返回非0. 
	 */
	virtual int SetGroupObjectValue(unsigned char index, const unsigned char* val) = 0;

	/** @brief 得到组对象的值. 
	 * @param index 组对象的索引号. 
	 * @param mem 把得到的值存入mem. 
	 * @param len 传入时len是mem区的大小单位为byte，传出时len指示写入mem的bit数，单位是bit. 
	 * @return 成功时返回0,失败时返回非0. 
	 */
	virtual int GetGroupObjectValue(unsigned char index, unsigned char* val, int* len) = 0;

	/// 不能删除组对象. 
	/** @brief 提供组对象的索引号删除一个组对象. 
	 * 索引号从0开始计数. 
	 */
//	virtual int RemoveGroupObjectByIndex(unsigned char index) = 0;

	/** @brief 增加一个组对象. 
	 * @param gobjh 参见 GroupObjectHead 类. 
	 * @param val 从val取
	 * @return 成功是返回0,失败返回错误码. 
	 */
	virtual int AddGroupObject(GroupObjectHead* gobjh, unsigned char* val) = 0;

	/** @brief 返回共有多少组地址. 
	 */
	virtual int GetGroupAddrCount() = 0;

	/** @brief 提供组地址的索引号返回一个组地址. 
	 */
	virtual const unsigned char* GetGroupAddrByIndex(unsigned char index) = 0;

	/** @brief 提供组地址的索引号删除该组地址. 
	 * @return 成功返回0,失败返回错误码. 
	 */
	virtual int RemoveGroupAddrByIndex(unsigned char index) = 0;

	/** @brief 增加一个组地址.
	 * @return 成功返回0,失败返回错误码. 
	 */
	virtual int AddGroupAddr(const unsigned char* g_addr) = 0;

	/** @brief 该设备是否存在指定的组地址. 
	 */
	virtual BOOL ExistThisGroupAddr(const unsigned char* g_addr) = 0;

	/** @brief 得到共有多少Link项. 
	 * @return 返回数目. 
	 */
	virtual int GetLinkCount() = 0;

	/** @brief 提供Link的索引号返回一个Link.
	 * @param index 要取的Link的索引号. 
	 * @param gobj_idx 返回这个Link的组对象的索引号. 
	 * @param gaddr_idx 返回这个Link的组地址的索引号. 
	 * @return 如果索引号不存在则函数返回非0. 
	 */
	virtual int GetLinkByIndex(unsigned char index, unsigned char& gobj_idx, unsigned char& gaddr_idx) = 0;

	/** @brief 提供Link的索引号删除该Link.  
	 * @return 成功返回0,失败返回错误码. 
	 */
	virtual int RemoveLinkByIndex(unsigned char index) = 0;

	/** @brief 增加一个Link. 
	 * @return 成功返回0,失败返回错误码. 
	 */
	virtual int AddLink(unsigned char gobj_idx, unsigned char gaddr_idx) = 0;

	/** @brief 得到某个组对象所加入的所有组地址. 
	 * @param gobj_idx 组对象的序号. 
	 * @param buff 得到的组地址填入此缓冲. 
	 * @param buff_len 传入时表示缓冲区的大小，传出时表示使用的大小.如果缓冲区不够，则buff_len为期望长度的相反数. 
	 * @return 成功返回得到的组地址的个数.失败则返回负数. 
	 */
	virtual int GetGaddrsOfGobjs(unsigned char gobj_idx, unsigned char* buff, int& buff_len) = 0;

	/** @brief 验证密钥得到授权. 
	 *@return 成功时返回0,level被设为授权等级.否则返回非0,level未定义. 
	 */
	virtual int GetAuthorize(const unsigned char* key, unsigned char& level) = 0;

	/** @brief 得到模拟控制器用户地址空间的内容. 
	 * @return 返回表示用户地址空间内容的首地址.implying 长度为一兆.  
	 */
	virtual const unsigned char* GetUserMemory() = 0;

	/** @brief 往模拟控制器用户地址空间写数据. 
	 * @shift 偏移量. 
	 * @buff 要写的东东存在这个缓冲区. 
	 * @buff_len 要写多长. 
	 * @return 成功写入返回0.如果要写的数据超过了64K的寻址空间就报错.buff为空buff_len不合理也会报错. 
	 */
	virtual int WriteUserMemory(unsigned int shift, unsigned char* buff, int buff_len) = 0;

	/** @brief 该设备是否响应 IndividualAddress_Write.
	 */
	virtual bool IsIndividualAddressWriteEnabled() = 0;

	/** @brief 该设备是否响应 IndividualAddress_Read. 
	 */
	virtual bool IsIndividualAddressReadEnabled() = 0;

	/** @brief 自杀函数.彻底让自己消失. 
	 * txt版本的实现就是把描述该设备的文件删除.数据库版本实现应该是你把描述该设备的记录或表删除. 
	 */
	virtual int CommitSuicide() = 0;

	/** @brief 得到设备的ID号. 
	 */
	virtual UINT GetDevID() = 0;

    /** @brief 增加一个返回映射信息的函数. 
	 */
	virtual void GetMapInfo(DevMapInfo& map_info) = 0;

	/** @brief 设置映射. 
	 */
	virtual void SetMapInfo(const DevMapInfo& map_info) = 0;

	/** @brief 是否存在某个软单元. 
	 * 用来对接到逻辑单元的代码. 
	 * @param softunit_name 如 "X001", "Y0077". 
	 * @return 如果存在这个软单元则返回true,否则返回false. 
	 */
	virtual bool DoesThisSoftUnitExist(const char* softunit_name) = 0;

	/** @brief 读软元件的值. 
	 * 用来对接到逻辑单元的代码. 
	 * @param softunit_name 软元件的名字. 
	 * @param buff 为佐线程安全.软元件的值将写入该缓冲.缓冲区溢出危险.不安全的函数. 
	 * @return 返回软元件的值的地址.对于不存在的软元件.返回NULL. 
	 */
	virtual const uchar* ReadSoftUnit(const char* softunit_name, uchar* buff) = 0;

	/** @brief 写软元件的值. 
	 * 用来对接到逻辑单元的代码. 
	 * @param softunit_name 软元件的名字. 
	 * @param value 要写入值的首地址. 
	 * @return 写入成功返回0.失败返回非0. 
	 */
	virtual int WriteSoftUnit(const char* softunit_name, const uchar* value) = 0;

	virtual ~IDevice(){}
};

#endif