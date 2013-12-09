#ifndef _H_DEVICEIMPL_TXT_
#define _H_DEVICEIMPL_TXT_

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include "IDevice.h"
#include "generic_helper.h"
#include "group_object.h"

class DeviceImpl_TXT : public IDevice
{
public:
	// 如果dev_id为NULL,则从file_path里根据 ${dev_id}.dev 的格式取得设备ID号. 
	virtual int Init(const char* file_path, uint* dev_id);

	virtual int Uninit();

	virtual int GetPerElemLen_inbit(unsigned char object_index, unsigned char prop_id);

	virtual int GetElemCount(unsigned char object_index, unsigned char prop_id);

	virtual int RemoveIObject(unsigned char object_index, unsigned char prop_id);

	virtual int GetPropFields(unsigned char object_index, unsigned char prop_id, PropertyFields& fields);

	virtual int SetPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	virtual int AddPropFields(unsigned char object_index, unsigned char prop_id, const PropertyFields& fields);

	virtual int GetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, unsigned char count,
			unsigned char* buff, int& buff_len);

	virtual int SetElemValues(unsigned char object_index, unsigned char prop_id, unsigned short start_index, unsigned char count,
			const unsigned char* buff, int buff_len);

	virtual int GetAuthorize(const unsigned char* key, unsigned char& level);

	virtual int GetAllIDPairs(unsigned char* buff, int& buff_len);

	virtual int GetGroupObjectCount();

	virtual int GetGroupObjectHeadByIndex(unsigned char index, GroupObjectHead& gobj_head);

	virtual int SetGroupObjectValue(unsigned char index, const unsigned char* val);

	virtual int GetGroupObjectValue(unsigned char index, unsigned char* val, int* len);

	/// 不能删除组对象. 
	//	virtual int RemoveGroupObjectByIndex(unsigned char index);

	virtual int AddGroupObject(GroupObjectHead* gobjh, unsigned char* val);

	virtual BOOL ExistThisGroupAddr(const unsigned char* g_addr);

	virtual int GetGroupAddrCount();

	virtual const unsigned char* GetGroupAddrByIndex(unsigned char index);

	virtual int RemoveGroupAddrByIndex(unsigned char index);

	virtual int AddGroupAddr(const unsigned char* g_addr);

	virtual int GetLinkCount();

	virtual int GetLinkByIndex(unsigned char index, unsigned char& gobj_idx, unsigned char& gaddr_idx);

	virtual int RemoveLinkByIndex(unsigned char index);

	virtual int AddLink(unsigned char gobj_idx, unsigned char gaddr_idx);

	virtual int GetGaddrsOfGobjs(unsigned char gobj_idx, unsigned char* buff, int& buff_len);

	virtual const unsigned char* GetUserMemory();

	virtual int WriteUserMemory(unsigned int shift, unsigned char* buff, int buff_len);

	virtual bool IsIndividualAddressWriteEnabled();

	virtual bool IsIndividualAddressReadEnabled();

	virtual const unsigned char* GetHostAddr(std::string* h);

	virtual const unsigned char* GetSerialNum(std::string* h);

	virtual int SetHostAddr(const unsigned char* addr);

	virtual void GetMapInfo(DevMapInfo& map_info);

	virtual void SetMapInfo(const DevMapInfo& map_info);

	virtual int CommitSuicide();

	virtual UINT GetDevID();

public:
	virtual bool DoesThisSoftUnitExist(const char* softunit_name);

	virtual const uchar* ReadSoftUnit(const char* softunit_name, uchar* buff);

	virtual int WriteSoftUnit(const char* softunit_name, const uchar* value);

protected:
	// 是否存在这个软单元.软单元的值将填入buff指示的缓冲. 
	// obj_index, prop_id 传出软设备映射到的接口对象的对象index和属性id. 
	// 注意buff分配足够大喔.这是个不安全的函数.缓冲区溢出哇. 
	bool _DoesThisSoftUnitExist(const char* softunit_name, uchar& obj_index, uchar& prop_id, uchar* buff);

public:
	// 重设备这个路径后.Uninit时会把描述文件存到新的路径指定的文件中. 
	void ResetDevDescFilePath(const char* file_path);

	// 这个函数同SetElemValues的区别是.后者会在写count个越界时报错.而该函数在enable_append取真时会自动添加. 
	int SetElemValues_Ex(unsigned char object_index, unsigned char prop_id, unsigned short start_index, unsigned char count,
						 const unsigned char* buff, int buff_len, BOOL enable_append);

	// 添加一个元素. 
	int AppendElemValue(unsigned char obj_index, unsigned char prop_id, const unsigned char* buff, int buff_len);

public:
	// constructor. 
	DeviceImpl_TXT();
	// De-constructor. 
	virtual ~DeviceImpl_TXT(void);

private:
	// 从描述软设备接口对象和组对象的文件中读入数据. 
	int ReadInFile(const char* file_path);
	// 把数据写回文件. 
	int WriteBackFile(const char* file_path);
	// 检查相关数据的合法性.如果不合法就返回false.通过参数返回出错信息. 
	bool CheckValidity(std::string* info);
	// 检查是否存在某个关键字.如果存在则返回一个大于或等于0的下标,如果不存在返回负数. 
	int ExistThisKeyWord(const char* keyword);

private:
	/// 这个东东用来维护映射表. 
//	struct Quadplex
	struct Duplex
	{
		uchar _obj_index;
		uchar _prop_id;
//		uchar _start_pos;
//		uchar _count;
	};

	BOOL    m_inited;
	FILE*   m_file_stream;
	std::string m_dev_desc_path;

	typedef std::map<std::pair<unsigned char, unsigned char>, mem_t> DataContainer;
	typedef std::map<std::string, std::vector<Duplex> >  Mediator;  ///< 映射表. 
	DataContainer  m_data;
	Mediator       m_mediator;

	//std::vector<GroupObject*> m_vec_gobjs;            ///< 存组对象表. 
	//std::vector<UCHAR*>  m_vec_gaddrs;                ///< 存组地址表. 
	//std::vector<std::pair<UCHAR, UCHAR>> m_vec_links; ///< 关联表. 
    mem_t   m_mem;   ///< 存User Memory. 
	UINT    m_dev_id;  ///< 该设备的ID号. 

//  Device未能应该加锁. 
//	HANDLE  m_mutex_objs; // 操作接口对象和组对象前要获得这个锁. 

private:
	// 通过对象序号和属性ID对得到对应的容器中的那个东东的iterator. 
	DataContainer::iterator MapID2FairyIte(unsigned char object_index, unsigned char prop_id);
	Mediator::iterator MapKeyword2DuplexesIte(std::string keyword);
};

#endif