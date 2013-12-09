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
	// ���dev_idΪNULL,���file_path����� ${dev_id}.dev �ĸ�ʽȡ���豸ID��. 
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

	/// ����ɾ�������. 
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
	// �Ƿ���������Ԫ.��Ԫ��ֵ������buffָʾ�Ļ���. 
	// obj_index, prop_id �������豸ӳ�䵽�Ľӿڶ���Ķ���index������id. 
	// ע��buff�����㹻���.���Ǹ�����ȫ�ĺ���.�����������. 
	bool _DoesThisSoftUnitExist(const char* softunit_name, uchar& obj_index, uchar& prop_id, uchar* buff);

public:
	// ���豸���·����.Uninitʱ��������ļ��浽�µ�·��ָ�����ļ���. 
	void ResetDevDescFilePath(const char* file_path);

	// �������ͬSetElemValues��������.���߻���дcount��Խ��ʱ����.���ú�����enable_appendȡ��ʱ���Զ����. 
	int SetElemValues_Ex(unsigned char object_index, unsigned char prop_id, unsigned short start_index, unsigned char count,
						 const unsigned char* buff, int buff_len, BOOL enable_append);

	// ���һ��Ԫ��. 
	int AppendElemValue(unsigned char obj_index, unsigned char prop_id, const unsigned char* buff, int buff_len);

public:
	// constructor. 
	DeviceImpl_TXT();
	// De-constructor. 
	virtual ~DeviceImpl_TXT(void);

private:
	// ���������豸�ӿڶ�����������ļ��ж�������. 
	int ReadInFile(const char* file_path);
	// ������д���ļ�. 
	int WriteBackFile(const char* file_path);
	// ���������ݵĺϷ���.������Ϸ��ͷ���false.ͨ���������س�����Ϣ. 
	bool CheckValidity(std::string* info);
	// ����Ƿ����ĳ���ؼ���.��������򷵻�һ�����ڻ����0���±�,��������ڷ��ظ���. 
	int ExistThisKeyWord(const char* keyword);

private:
	/// �����������ά��ӳ���. 
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
	typedef std::map<std::string, std::vector<Duplex> >  Mediator;  ///< ӳ���. 
	DataContainer  m_data;
	Mediator       m_mediator;

	//std::vector<GroupObject*> m_vec_gobjs;            ///< ��������. 
	//std::vector<UCHAR*>  m_vec_gaddrs;                ///< �����ַ��. 
	//std::vector<std::pair<UCHAR, UCHAR>> m_vec_links; ///< ������. 
    mem_t   m_mem;   ///< ��User Memory. 
	UINT    m_dev_id;  ///< ���豸��ID��. 

//  Deviceδ��Ӧ�ü���. 
//	HANDLE  m_mutex_objs; // �����ӿڶ���������ǰҪ��������. 

private:
	// ͨ��������ź�����ID�Եõ���Ӧ�������е��Ǹ�������iterator. 
	DataContainer::iterator MapID2FairyIte(unsigned char object_index, unsigned char prop_id);
	Mediator::iterator MapKeyword2DuplexesIte(std::string keyword);
};

#endif