#ifndef _H_FRAME_
#define _H_FRAME_

#include "APDU.h"
#include "DeviceManager.h"

/** @brief Frame 描述在将放到物理层传输的帧的基类. 
 */
class Frame
{
public:
	/** @brief 工厂函数.传入APDU，生成 Frame. 
	 */
	static Frame* BuildFrameFromAPDU(const APDU& apdu);

	/** @brief NOTICE.可以废弃这个函数了!!! 
	 * 检查一段内存.鉴定是否是一个正确的Frame.如果正常就构建相应的一个Frame对象. 
	 * @param recved 为真时检查是不是正确的收到的帧.为假时检查是不是正确的发送的帧. 
	 * 判断收,发帧的不同在于主机地址所在的帧的位置. 
	 */
	static Frame* BuildFrameFromMem(const UCHAR* mem, UCHAR mem_len, BOOL recved = TRUE);

	/** @brief 检查一段内存，鉴定是否是一个正确的Frame.如果正常就构建相应的一个Frame对象. 
	 * @BuildFrameFromMem是旧版本.只支持一个本机地址.这个是新版.同时支持大量设备. 
	 * @param recved 为真时检查是不是正确的收到的帧.为假时检查是不是正确的发送的帧. 
	 * 判断收,发帧的不同在于主机地址所在的帧的位置.
	 * @param check_address 如果为真，则会检查Frame里的地址是不是本地设好的地址或加入的组地址.不属于这些地址就丢弃. 
	 * 如果为假.则忽略检查地址信息.
	 */
	static Frame* BuildFrameFromBuff(const UCHAR* buff, UCHAR buff_len, BOOL recved = TRUE, BOOL check_address = TRUE);

	/** @brief 设置指向设备管理的指针. 
	 * @param manager 新的指针. 
	 * @return 返回旧的指针. 
	 */
	static DeviceManager* SetDeviceManager(DeviceManager* manager); 

	/** @brief 每个Frame的子类都要实现这个方法用于返回它包装的apdu包. 
	 * 的首地址和长度. 
	 */
	virtual const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const = 0;

	/** @brief 用于返回Frame的首地址和长度.子类可以重写这个函数. 
	 */
	virtual const UCHAR* GetFrameMem(const UCHAR*& frame_mem, UCHAR& frame_len)const;

	/** @brief 用于返回Frame里存的APDU的类型.也可理解成Frame的类型. 
	 */
	virtual APDUType GetAPDUType();

	/** @brief 是否对应该帧生成一个回复帧. 
	 * @param resp 生成的回复帧.若失败则返回NULL.
	 * @return 0 on success, non-zero on failure.
	 */
	virtual int Response(Frame*& resp)const;  // 旧代码.以后要废弃. 

	/** @brief 是否对应该帧生成一个回复帧. 
	 * @param resps 生成的回复帧们.若失败则vector中没有元素. 
	 * @return 0 on success, non-zero on failure.
	 */
	virtual int Response(std::vector<Frame*>& resps)const;

	/** @brief 得到该帧记录的peer地址. 
	 */
	virtual const UCHAR* GetPeerAddr()const = 0;

	/** @brief 得到该帧记录的host地址. 
	 */
	virtual const UCHAR* GetHostAddr()const = 0;

	/** @brief 这个帧的目的地址是否是组地址. 
	 * 默认实现该函数返回FALSE,使用单个地址. 
	 */
	virtual BOOL DoesPeerUseGroupAddress()const = 0;

	/** @brief 这个帧的目的地址是否用了广播地址. 
	 */
	BOOL DosePeerUseBroadcastAddress()const;

	/** @brief 是短帧.只有一个byte的帧吗?
	 */
	BOOL IsOneByteFrame()const;

	/** @brief 复制一份Frame. 返回的Frame需要用delete释放. */
	virtual Frame* CloneMe()const = 0;

public:
	Frame(const Frame& frame);
	const Frame& operator=(const Frame& frame);

protected:
	Frame();
	void CopyOfFrameData(UCHAR*& mem, UCHAR& len)const;

public:
	~Frame(void);

protected:
	UCHAR  m_frame_len;
	UCHAR* m_frame_mem;

	static DeviceManager* ms_dev_manager;  ///< 指向设备管理类的指针. 
};

/** @brief 只有一个BYTE的. 
 */
class OneByteFrame : public Frame
{
public:
	enum OneByteFrameType{ ACK_FRAME, NACK_FRAME, BUSY_FRAME, NACKBUSY_FRAME };

	OneByteFrame(const APDU& apdu);
	// 拷贝构造函数. 
	OneByteFrame(const OneByteFrame& frame);

	/// 因为总共只有一个BYTE, 实际上没有apdu。所以返回NULL和0.
	const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const;

	OneByteFrameType GetType()const
	{
		return m_type;
	}

	static OneByteFrame* BuildFrameFromMem(const UCHAR* mem, UCHAR len);

	static OneByteFrame* BuildFrameFromBuff(const UCHAR* buff, UCHAR len);

	/** @brief 短帧没有记录peer地址. 
	 */
	const UCHAR* GetPeerAddr()const{ return NULL; }

	/** @brief 短帧没有记录host地址. 
	 */
	const UCHAR* GetHostAddr()const{ return NULL; }

	/** @brief OneByteFrame是没有Response动作的.所以重写基类的Response函数. 
	 */
	int Response(Frame*& resp){ resp = NULL; return 0xf; }

	BOOL DoesPeerUseGroupAddress()const;

	Frame* CloneMe()const;

protected:
	OneByteFrame(OneByteFrameType type);
	OneByteFrameType  m_type;
};

/** @brief 标准帧. 
 */
class StandardFrame : public Frame
{
public:
	StandardFrame(const APDU& apdu);
	StandardFrame(const StandardFrame& frame);

	/// 设置控置字的重复位. 
	void SetReptBit();
	const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const;

	static StandardFrame* BuildFrameFromMem(const UCHAR* mem, UCHAR len, BOOL recved);

	static StandardFrame* BuildFrameFromBuff(const UCHAR* buff, UCHAR buff_len, BOOL recved, BOOL check_address = TRUE);

	/** @brief 创建一个能使设备打开写权限的帧. 
	 *///已经废弃
	// static StandardFrame* BuildEnableWriteFrame();

	const UCHAR* GetPeerAddr()const;

	const UCHAR* GetHostAddr()const;	
	
	BOOL DoesPeerUseGroupAddress()const;

	Frame* CloneMe()const;

protected:
	StandardFrame(){}
};

/** @brief 扩展帧. 
 */
class ExtendedFrame : public Frame
{
public:
	ExtendedFrame(const APDU& apdu);
	ExtendedFrame(const ExtendedFrame& frame);

	/// 设置控置字的重复位. 
	void SetReptBit();
	const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const;

	static ExtendedFrame* BuildFrameFromMem(const UCHAR* mem, UCHAR len, BOOL recved);

	static ExtendedFrame* BuildFrameFromBuff(const UCHAR* buff, UCHAR buff_len, BOOL recved, BOOL check_address = TRUE);

	const UCHAR* GetPeerAddr()const;

	const UCHAR* GetHostAddr()const;

	BOOL DoesPeerUseGroupAddress()const;

	Frame* CloneMe()const;

protected:
	ExtendedFrame(){}
};

#endif