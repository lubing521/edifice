#ifndef _H_COMMBASE_
#define _H_COMMBASE_

#include <assert.h>
#include <vector>
#include <list>
#include <algorithm>
#include "global.h"
#include "generic_helper.h"
#include "prot_structs.h"
#include "CommBase.h"
#include "Frame.h"

// ��. 
struct TxData
{
	Frame* _frame;       ///< Ҫ����Frame.�����ڶ��Ϸ���.�ӷ��Ͷ������Ƴ�ʱҪע���ͷ������ڴ�.  
	/// �������ʱ��ʱ���.winƽ̨����GetTickCount����.�ɾ�ȷ������.
	/// Linux�¿���gettimeofdayʵ���������(���������,�ڳ�������ʱ������ϵͳʱ��.�ͻ����).  
	uint   _time_stamp;
	uint   _id;     ///< ΨһID��.��Ƿ���List�е����TxData. 
	HANDLE _evt;    ///< ����ͬ��. 
};

// ��. 
struct RxData
{
	Frame* _frame;       ///< �յ���Frame. �����ڶ��Ϸ���.�ӽ��ն������Ƴ�ʱҪע���ͷ������ڴ�.  
	APDUType _type;      ///< frame������.��Ȼ����Frame����õ���Ϊ��һ���Ч�ʵĿ����������ֵ. 
	uint   _time_stamp;  ///< �յ�Frameʱ��ʱ���. 
	uint   _id;          ///< ΨһID��.
};

enum CommType { CT_SERIAL_PORT, CT_UDP };

// ������ʼ������ͨѶ�� SerialComm_MM ��. 
struct SerialCommInitData
{
	unsigned short _port; 
	unsigned int   _baudrate;
};

// ������ʼ��UDPͨ���� UDPComm_MM ��.
struct UDPCommInitData
{
	unsigned int   _bound_ip;  // Ҫ�󶨵ı���IP��ַ.���������ж��IP.����Ҫָ��.�Ѿ��������ֽ���.   
	unsigned short _port;      // Ҫ�󶨵ı���UDP�˿ں�.�Ѿ��������ֽ���. 
};

struct InitData
{
	union
	{
		SerialCommInitData _serial_comm_initdata;
		UDPCommInitData  _udp_comm_initdata;
	};
	CommType _ct;
};

/** @brief �������.��������Ծ����ŵ���ʵ�ʸ���ͨѸ����Ӵ���̳�.
 * ���࿪���������̣߳�һ���������ݲ��������Ϸ���֡������ն���.���ṩ�˺ܶ�������������еĺ�����Ĭ��ʵ��. 
 */
class CommBase
{
public:
    /// ��ʼ������.����init��ʼ���˻��������ݣ�������д�ú�����ʼ�������ض������ݺ���Դ.��Ҫ�ǵõ��ø����init. 
	virtual int Init(const InitData& initdata);
	/// ����ʼ��.������д�ú����ͷ��ض������ݺ���Դ.��Ҫ�ǵõ��ø���ķ���ʼ���� 
	virtual void Uninit();

	////////////////////////////////////////////
	/// ����ĺ������ǲ������շ��Ͷ��е�Ĭ��ʵ��.����������.����һ�㲻��Ҫ��д����.   

	// �����汾.ֱ�����ͳ�ȥ���˲ŷ���. 
	virtual ResponseType tx_sync(const Frame * send_frame);

	/// �������ء�ֻ��֡���ɹ��Ƿ��� RESPONSE_SENDING_OKAY��ʧ��ʱ���� RESPONSE_SENDING_TIMEOUT. 
	// tx_async ʵ��Ϊ����һ�� send_frame �Ŀ�����Ȼ�����ݿ������뷢�Ͷ���.��������������. 
	virtual ResponseType tx_async(const Frame * send_frame);

	/// ����������ֻ��֡���ɹ�ʱ���� RESPONSE_SHORT �� RESPONSE_NORMAL �� recv_pack ָ��õ��� Pack.
	/// ʧ��ʱ���� RESPONSE_RECVING_TIMEOUT �� RESPONSE_BAD �� recv_pack Ϊ NULL.
	/// waiting_time�����ʱ�����ͳ�ʱ�˳�����. 
	virtual ResponseType rx_sync(Frame*& recv_frame, uint waiting_time);

	/// ����������ֻ��֡�����ذ汾.������һЩ��������. 
	/// �Զ��ɹ�ʱ���� RESPONSE_SHORT �� RESPONSE_NORMAL �� recv_pack ָ��õ��� Pack.
	/// ʧ��ʱ���� RESPONSE_RECVING_TIMEOUT �� RESPONSE_BAD �� recv_pack Ϊ NULL.
	/// ���sending_time������-1,������GetTickCount�õ���ǰһ��֡�ķ���ʱ��.�ظ�֡�ĵ���ʱ������������ʱ�䲢���������ļ���ָ�����ӳ�ʱ��.  
	/// ���expected_types��NULL������������Ԫ��, ��������ֻ�ܷ������vector��ָ����֡����.
	/// ���from��ΪNULL����������ֻ�����������ַ������֡. 
	/// ���to ��ΪNULL����������ֻ���Ƿ���to�����ַ��֡.
	/// waiting_time�����ʱ�����ͳ�ʱ�˳�����. 
	virtual ResponseType rx_sync(Frame*& recv_frame, uint sending_time, const std::vector<APDUType>* expected_types, 
		                         const uchar* from, const uchar* to, uint waiting_time);

	/// ���ŵ��ж������ݺ������������ݣ��п��ܷ��ض���ظ���Frame.����ಥ����Ҫʹ���������.  
	virtual ResponseType rx_sync_ex(std::vector<Frame*>& recv_frames);

	// һ�����ذ汾.��ָ��һЩ��������.�μ� rx_sync ��һ�����ذ汾. 
	virtual ResponseType rx_sync_ex(std::vector<Frame*>& recv_frames, uint sending_time, 
									const std::vector<APDUType>* expected_types, const uchar* from, const uchar* to);

	/// �����˳�����. 
	virtual void QuitRxNow();

	/// ���ûص�����. 
	virtual Reporter SetMonitor(Reporter r);

	/// �õ����ն����ﹲ�ж��������͵�APDU.
	/// ����ʱszָʾtypes�����������ܴ���ٸ�enum.����ʱ���bytesָʾ�����鹻�ã���szָʾʵ�ʴ��˶��ٸ�.��ʱ��������0. 
	/// ������鲻���ã��򷵻غ�����0,��sz����Ϊ���������������෴��. 
	virtual int QueryAPDUTypesInRxQueue(APDUType types[], int& sz);

	/// ����������������֡.����һ��FramePeek�ṹ�������.
	/// filter ������ APDUType ������ָ������Ҫ����֡.���filter�� AT_UNAVAILABLE�򷵻����е�֡.
	/// ��Ҫ��� CleanFramePeek �ͷŵ�����κ���������ڴ�. 
	/// �ɹ�ʱ����0.ʧ��ʱ���ش�����.
#define FRAME_PEEKER_BUSY -1      // ��һС��ʱ����û�о���������������ΪBUSY.
	virtual int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count);

	/// �� FramePeeker �������ص� frames �ĵ�ַ����.�ͷ���Դ. 
	virtual void ClearFramePeek(void* addr);

	/// �ӽ��ն�����ɾ��ĳ��֡.���� FramePeeker �еõ���FramePeek�ṹ���־Ҫɾ���Ľ��ն����е�֡. 
	virtual void RemoveFrameFromRxQueue(const FramePeek& peek);

	/// ����һ�Ժ�����EIB�ľɽӿ���.���þɽӿھͻ���FramePeeker����BUSY. 
	virtual void LockFramePeeker();
	virtual void UnlockFramePeeker();

	CommBase(void);
	virtual ~CommBase(void);

protected:
	/////////////////////////////////////////////////////////////////////
	/// ������ͨѸ���������д��protected�麯��. 
	//////////////////////////////////////////////////////////////////

	// ����Ҫ����ʵ�ʵķ��ͷ�ʽ��д����������.
	// һ������һ��֡��һ�������ֽ���.
	// buff �Ƿ����ֽ������׵�ַ,buff_len�ǳ���.�ɹ�ʱ����0.ʧ�ܷ��ط�0. 
	virtual int _send_frame(const Frame* frame) = 0;
	// �ɹ�ʱ����0.ʧ�ܷ��ط�0.�ɹ�ʱ�ѵõ����ֽ����浽mem���淵��.   
	virtual int _recv_bytes(mem_t& mem) = 0;

	///////////////////////////////////////////////////////////
	/// ������ͨѸ���������д��protected�麯��. 
	//////////////////////////////////////////////////////////////////

protected:
	/// ͬ RemoveFrameFromRxQueue �������� RemoveFrameFromRxQueue_nolock ���᳢��ȥ��û�����. 
	void RemoveFrameFromRxQueue_nolock(const FramePeek& peek);

	/// �����ڴ��.�ѷ���������ȷ֡����data_list.�������mem������ݣ�ɾ����Ϊ���õ���������.������mem_len. 
	/// check_address�Ƿ���Frame��ĵ�ַ��Ϣƥ�䱾�ص��豸��ַ�򱾵ؼ�������ַ. 
	int FrameParser(unsigned char* mem, int& mem_len, std::list<RxData>& data_list, BOOL check_address);

	// ��������.ֻ����һ��frame.�ð汾��ӡlog.  
	virtual int parse_recv_buff(const unsigned char* buff, int len, Frame*& get_frame, BOOL check_address);

	// ��������.���ܻ᷵�ض��frame.ֻ�е� buff������len�ﶼ��frameû���������ݲŻ᷵��0.
	// ���ص�Frame*����Ҫ��delete�ͷ�. 
	virtual int parse_recv_buff_ex(const unsigned char* buff, int len, std::vector<Frame*>& get_frames, 
		BOOL suppress_log, BOOL check_address);

	// ��������ȡ���Ϸ�֡��ʵ��ʵ�������������. 
	int _parse_recv_buff(const unsigned char* buff, int len, Frame*& get_frame, BOOL suppress_log, BOOL check_address);

	/// ��̫��û��ʹ�õ�֡����.���ض����˶���֡. 
	void ClearOldFrames();

	friend DWORD WINAPI tx_thread(void* arg);
	friend DWORD WINAPI rx_thread(void* arg);
	DWORD  tx_thread();  // �����߳�. 
	DWORD  rx_thread();  // �����߳�. 

protected:
	volatile bool  m_inited;
	volatile bool  m_quit_listen;

	Reporter  m_reporter;     ///< ���ӷ��ͽ������ݵĻص�����ָ��. 

	std::list<TxData> m_tx_list;  ///< ���Ͷ���. 
	uint  m_next_tx_id;  ///< ���Ͷ��е���һ������ID��. 
	HANDLE  m_mutex4tx_list;   ///< ������. 
	HANDLE  m_event4tx_list;   ///< ����֪ͨtx�߳�������Ҫ����. 
	HANDLE  m_thread4tx_list;  ///< �����ݵ��߳�. 
	volatile BOOL  m_quit_tx_thread;  ///< ������. 

	std::list<RxData> m_rx_list;   ///< ���ն���. 
	uint  m_next_rx_id;      ///< ���ն��е���һ������ID��. 
	HANDLE  m_mutex4rx_list;  ///< ������. 
	HANDLE  m_thread4rx_list; ///< �����ݵ��߳�. 
	volatile BOOL m_quit_rx_thread;  ///< �Ͳ�����. 

	HANDLE  m_mutex4lock_peeker;   ///< FramePeeker����Ҫ�ȵõ������.����������ռ��,FramePeeker�ͷ���busy. 
};

#endif