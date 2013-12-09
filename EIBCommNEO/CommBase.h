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

// 发. 
struct TxData
{
	Frame* _frame;       ///< 要发的Frame.必须在堆上分配.从发送队列中移除时要注意释放它的内存.  
	/// 加入队列时的时间戳.win平台下用GetTickCount函数.可精确到毫秒.
	/// Linux下可用gettimeofday实现这个函数(但如果这样,在程序运行时调整了系统时间.就会出错).  
	uint   _time_stamp;
	uint   _id;     ///< 唯一ID号.标记放入List中的这个TxData. 
	HANDLE _evt;    ///< 用来同步. 
};

// 收. 
struct RxData
{
	Frame* _frame;       ///< 收到的Frame. 必须在堆上分配.从接收队列中移除时要注意释放它的内存.  
	APDUType _type;      ///< frame的类型.虽然可由Frame计算得到，为了一点点效率的考量加入这个值. 
	uint   _time_stamp;  ///< 收到Frame时的时间戳. 
	uint   _id;          ///< 唯一ID号.
};

enum CommType { CT_SERIAL_PORT, CT_UDP };

// 用来初始化串口通讯的 SerialComm_MM 类. 
struct SerialCommInitData
{
	unsigned short _port; 
	unsigned int   _baudrate;
};

// 用来初始化UDP通读的 UDPComm_MM 类.
struct UDPCommInitData
{
	unsigned int   _bound_ip;  // 要绑定的本机IP地址.本机可能有多个IP.所以要指定.已经是网络字节序.   
	unsigned short _port;      // 要绑定的本机UDP端口号.已经是网络字节序. 
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

/** @brief 抽象基类.所以有针对具体信道的实际负责通迅的类从此类继承.
 * 基类开启了两个线程，一个接收数据并分析出合法的帧加入接收队列.并提供了很多操作这两个队列的函数的默认实现. 
 */
class CommBase
{
public:
    /// 初始化工作.基类init初始化了基本的数据，子类重写该函数初始化更多特定的数据和资源.还要记得调用父类的init. 
	virtual int Init(const InitData& initdata);
	/// 反初始化.子类重写该函数释放特定的数据和资源.还要记得调用父类的反初始化。 
	virtual void Uninit();

	////////////////////////////////////////////
	/// 下面的函数都是操作接收发送队列的默认实现.基本够用了.子类一般不需要重写他们.   

	// 阻塞版本.直到发送出去了了才返回. 
	virtual ResponseType tx_sync(const Frame * send_frame);

	/// 立即返回。只发帧。成功是返回 RESPONSE_SENDING_OKAY，失败时返回 RESPONSE_SENDING_TIMEOUT. 
	// tx_async 实现为复制一份 send_frame 的拷贝，然后把这份拷贝放入发送队列.函数就立即返回. 
	virtual ResponseType tx_async(const Frame * send_frame);

	/// 阻塞函数。只读帧。成功时返回 RESPONSE_SHORT 或 RESPONSE_NORMAL 且 recv_pack 指向得到的 Pack.
	/// 失败时返回 RESPONSE_RECVING_TIMEOUT 或 RESPONSE_BAD 且 recv_pack 为 NULL.
	/// waiting_time，这个时间耗完就超时退出函数. 
	virtual ResponseType rx_sync(Frame*& recv_frame, uint waiting_time);

	/// 阻塞函数。只读帧的重载版本.能限制一些接收条件. 
	/// 对读成功时返回 RESPONSE_SHORT 或 RESPONSE_NORMAL 且 recv_pack 指向得到的 Pack.
	/// 失败时返回 RESPONSE_RECVING_TIMEOUT 或 RESPONSE_BAD 且 recv_pack 为 NULL.
	/// 如果sending_time不等于-1,则是用GetTickCount得到的前一个帧的发送时间.回复帧的到达时间必须晚于这个时间并超出配置文件中指定的延迟时间.  
	/// 如果expected_types非NULL并且容器内有元素, 则限制了只能返回这个vector里指定的帧内型.
	/// 如果from不为NULL，则限制了只能是由这个地址发来的帧. 
	/// 如果to 不为NULL，则限制了只能是发往to这个地址的帧.
	/// waiting_time，这个时间耗完就超时退出函数. 
	virtual ResponseType rx_sync(Frame*& recv_frame, uint sending_time, const std::vector<APDUType>* expected_types, 
		                         const uchar* from, const uchar* to, uint waiting_time);

	/// 从信道中读到数据后分析里面的数据，有可能返回多个回复的Frame.比如多播就需要使用这个函数.  
	virtual ResponseType rx_sync_ex(std::vector<Frame*>& recv_frames);

	// 一个重载版本.能指定一些接收条件.参见 rx_sync 的一个重载版本. 
	virtual ResponseType rx_sync_ex(std::vector<Frame*>& recv_frames, uint sending_time, 
									const std::vector<APDUType>* expected_types, const uchar* from, const uchar* to);

	/// 快速退出监听. 
	virtual void QuitRxNow();

	/// 设置回调函数. 
	virtual Reporter SetMonitor(Reporter r);

	/// 得到接收队列里共有多少种类型的APDU.
	/// 传入时sz指示types的容量，共能存多少个enum.传出时如果bytes指示的数组够用，则sz指示实际存了多少个.此时函数返回0. 
	/// 如果数组不够用，则返回函数非0,且sz被置为数组期望个数的相反数. 
	virtual int QueryAPDUTypesInRxQueue(APDUType types[], int& sz);

	/// 检索接收列里所有帧.返回一个FramePeek结构体的数组.
	/// filter 可以用 APDUType 的类型指定你需要哪种帧.如果filter是 AT_UNAVAILABLE则返回所有的帧.
	/// 需要配合 CleanFramePeek 释放调用这次函数分配的内存. 
	/// 成功时返回0.失败时返回错误码.
#define FRAME_PEEKER_BUSY -1      // 在一小段时间内没有竞争到互斥锁则认为BUSY.
	virtual int FramePeeker(APDUType filter, FramePeek*& frame_peeks, int& count);

	/// 把 FramePeeker 函数返回的 frames 的地址传入.释放资源. 
	virtual void ClearFramePeek(void* addr);

	/// 从接收队列中删除某个帧.传入 FramePeeker 中得到的FramePeek结构体标志要删除的接收队列中的帧. 
	virtual void RemoveFrameFromRxQueue(const FramePeek& peek);

	/// 下面一对函数给EIB的旧接口用.调用旧接口就会让FramePeeker返回BUSY. 
	virtual void LockFramePeeker();
	virtual void UnlockFramePeeker();

	CommBase(void);
	virtual ~CommBase(void);

protected:
	/////////////////////////////////////////////////////////////////////
	/// 下面是通迅子类必需重写的protected虚函数. 
	//////////////////////////////////////////////////////////////////

	// 子类要根据实际的发送方式重写这两个函数.
	// 一个发走一个帧，一个接收字节流.
	// buff 是发送字节流的首地址,buff_len是长度.成功时返回0.失败返回非0. 
	virtual int _send_frame(const Frame* frame) = 0;
	// 成功时返回0.失败返回非0.成功时把得到的字节流存到mem里面返回.   
	virtual int _recv_bytes(mem_t& mem) = 0;

	///////////////////////////////////////////////////////////
	/// 上面是通迅子类必需重写的protected虚函数. 
	//////////////////////////////////////////////////////////////////

protected:
	/// 同 RemoveFrameFromRxQueue 的区别是 RemoveFrameFromRxQueue_nolock 不会尝试去获得互斥锁. 
	void RemoveFrameFromRxQueue_nolock(const FramePeek& peek);

	/// 传入内存块.把分析到的正确帧放入data_list.并会调整mem里的内容，删掉认为无用的垃圾数据.并调整mem_len. 
	/// check_address是否检查Frame里的地址信息匹配本地的设备地址或本地加入的组地址. 
	int FrameParser(unsigned char* mem, int& mem_len, std::list<RxData>& data_list, BOOL check_address);

	// 分析缓冲.只返回一个frame.该版本打印log.  
	virtual int parse_recv_buff(const unsigned char* buff, int len, Frame*& get_frame, BOOL check_address);

	// 分析缓冲.可能会返回多个frame.只有当 buff，长度len里都是frame没有垃圾数据才会返回0.
	// 返回的Frame*都需要用delete释放. 
	virtual int parse_recv_buff_ex(const unsigned char* buff, int len, std::vector<Frame*>& get_frames, 
		BOOL suppress_log, BOOL check_address);

	// 分析数据取出合法帧的实际实现在这个函数里. 
	int _parse_recv_buff(const unsigned char* buff, int len, Frame*& get_frame, BOOL suppress_log, BOOL check_address);

	/// 把太久没有使用的帧丢弃.返回丢弃了多少帧. 
	void ClearOldFrames();

	friend DWORD WINAPI tx_thread(void* arg);
	friend DWORD WINAPI rx_thread(void* arg);
	DWORD  tx_thread();  // 接收线程. 
	DWORD  rx_thread();  // 发送线程. 

protected:
	volatile bool  m_inited;
	volatile bool  m_quit_listen;

	Reporter  m_reporter;     ///< 监视发送接收数据的回调函数指针. 

	std::list<TxData> m_tx_list;  ///< 发送队列. 
	uint  m_next_tx_id;  ///< 发送队列的下一个可用ID号. 
	HANDLE  m_mutex4tx_list;   ///< 不解释. 
	HANDLE  m_event4tx_list;   ///< 用来通知tx线程有数据要发啦. 
	HANDLE  m_thread4tx_list;  ///< 发数据的线程. 
	volatile BOOL  m_quit_tx_thread;  ///< 不解释. 

	std::list<RxData> m_rx_list;   ///< 接收队列. 
	uint  m_next_rx_id;      ///< 接收队列的下一个可用ID号. 
	HANDLE  m_mutex4rx_list;  ///< 不解释. 
	HANDLE  m_thread4rx_list; ///< 收数据的线程. 
	volatile BOOL m_quit_rx_thread;  ///< 就不解释. 

	HANDLE  m_mutex4lock_peeker;   ///< FramePeeker函数要先得到这个锁.如果这个锁被占用,FramePeeker就返回busy. 
};

#endif