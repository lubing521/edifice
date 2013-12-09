#ifndef _H_FRAME_
#define _H_FRAME_

#include "APDU.h"
#include "DeviceManager.h"

/** @brief Frame �����ڽ��ŵ�����㴫���֡�Ļ���. 
 */
class Frame
{
public:
	/** @brief ��������.����APDU������ Frame. 
	 */
	static Frame* BuildFrameFromAPDU(const APDU& apdu);

	/** @brief NOTICE.���Է������������!!! 
	 * ���һ���ڴ�.�����Ƿ���һ����ȷ��Frame.��������͹�����Ӧ��һ��Frame����. 
	 * @param recved Ϊ��ʱ����ǲ�����ȷ���յ���֡.Ϊ��ʱ����ǲ�����ȷ�ķ��͵�֡. 
	 * �ж���,��֡�Ĳ�ͬ����������ַ���ڵ�֡��λ��. 
	 */
	static Frame* BuildFrameFromMem(const UCHAR* mem, UCHAR mem_len, BOOL recved = TRUE);

	/** @brief ���һ���ڴ棬�����Ƿ���һ����ȷ��Frame.��������͹�����Ӧ��һ��Frame����. 
	 * @BuildFrameFromMem�Ǿɰ汾.ֻ֧��һ��������ַ.������°�.ͬʱ֧�ִ����豸. 
	 * @param recved Ϊ��ʱ����ǲ�����ȷ���յ���֡.Ϊ��ʱ����ǲ�����ȷ�ķ��͵�֡. 
	 * �ж���,��֡�Ĳ�ͬ����������ַ���ڵ�֡��λ��.
	 * @param check_address ���Ϊ�棬�����Frame��ĵ�ַ�ǲ��Ǳ�����õĵ�ַ���������ַ.��������Щ��ַ�Ͷ���. 
	 * ���Ϊ��.����Լ���ַ��Ϣ.
	 */
	static Frame* BuildFrameFromBuff(const UCHAR* buff, UCHAR buff_len, BOOL recved = TRUE, BOOL check_address = TRUE);

	/** @brief ����ָ���豸�����ָ��. 
	 * @param manager �µ�ָ��. 
	 * @return ���ؾɵ�ָ��. 
	 */
	static DeviceManager* SetDeviceManager(DeviceManager* manager); 

	/** @brief ÿ��Frame�����඼Ҫʵ������������ڷ�������װ��apdu��. 
	 * ���׵�ַ�ͳ���. 
	 */
	virtual const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const = 0;

	/** @brief ���ڷ���Frame���׵�ַ�ͳ���.���������д�������. 
	 */
	virtual const UCHAR* GetFrameMem(const UCHAR*& frame_mem, UCHAR& frame_len)const;

	/** @brief ���ڷ���Frame����APDU������.Ҳ������Frame������. 
	 */
	virtual APDUType GetAPDUType();

	/** @brief �Ƿ��Ӧ��֡����һ���ظ�֡. 
	 * @param resp ���ɵĻظ�֡.��ʧ���򷵻�NULL.
	 * @return 0 on success, non-zero on failure.
	 */
	virtual int Response(Frame*& resp)const;  // �ɴ���.�Ժ�Ҫ����. 

	/** @brief �Ƿ��Ӧ��֡����һ���ظ�֡. 
	 * @param resps ���ɵĻظ�֡��.��ʧ����vector��û��Ԫ��. 
	 * @return 0 on success, non-zero on failure.
	 */
	virtual int Response(std::vector<Frame*>& resps)const;

	/** @brief �õ���֡��¼��peer��ַ. 
	 */
	virtual const UCHAR* GetPeerAddr()const = 0;

	/** @brief �õ���֡��¼��host��ַ. 
	 */
	virtual const UCHAR* GetHostAddr()const = 0;

	/** @brief ���֡��Ŀ�ĵ�ַ�Ƿ������ַ. 
	 * Ĭ��ʵ�ָú�������FALSE,ʹ�õ�����ַ. 
	 */
	virtual BOOL DoesPeerUseGroupAddress()const = 0;

	/** @brief ���֡��Ŀ�ĵ�ַ�Ƿ����˹㲥��ַ. 
	 */
	BOOL DosePeerUseBroadcastAddress()const;

	/** @brief �Ƕ�֡.ֻ��һ��byte��֡��?
	 */
	BOOL IsOneByteFrame()const;

	/** @brief ����һ��Frame. ���ص�Frame��Ҫ��delete�ͷ�. */
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

	static DeviceManager* ms_dev_manager;  ///< ָ���豸�������ָ��. 
};

/** @brief ֻ��һ��BYTE��. 
 */
class OneByteFrame : public Frame
{
public:
	enum OneByteFrameType{ ACK_FRAME, NACK_FRAME, BUSY_FRAME, NACKBUSY_FRAME };

	OneByteFrame(const APDU& apdu);
	// �������캯��. 
	OneByteFrame(const OneByteFrame& frame);

	/// ��Ϊ�ܹ�ֻ��һ��BYTE, ʵ����û��apdu�����Է���NULL��0.
	const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const;

	OneByteFrameType GetType()const
	{
		return m_type;
	}

	static OneByteFrame* BuildFrameFromMem(const UCHAR* mem, UCHAR len);

	static OneByteFrame* BuildFrameFromBuff(const UCHAR* buff, UCHAR len);

	/** @brief ��֡û�м�¼peer��ַ. 
	 */
	const UCHAR* GetPeerAddr()const{ return NULL; }

	/** @brief ��֡û�м�¼host��ַ. 
	 */
	const UCHAR* GetHostAddr()const{ return NULL; }

	/** @brief OneByteFrame��û��Response������.������д�����Response����. 
	 */
	int Response(Frame*& resp){ resp = NULL; return 0xf; }

	BOOL DoesPeerUseGroupAddress()const;

	Frame* CloneMe()const;

protected:
	OneByteFrame(OneByteFrameType type);
	OneByteFrameType  m_type;
};

/** @brief ��׼֡. 
 */
class StandardFrame : public Frame
{
public:
	StandardFrame(const APDU& apdu);
	StandardFrame(const StandardFrame& frame);

	/// ���ÿ����ֵ��ظ�λ. 
	void SetReptBit();
	const UCHAR* GetApduMem(const UCHAR*& apdu_mem, UCHAR& apdu_len)const;

	static StandardFrame* BuildFrameFromMem(const UCHAR* mem, UCHAR len, BOOL recved);

	static StandardFrame* BuildFrameFromBuff(const UCHAR* buff, UCHAR buff_len, BOOL recved, BOOL check_address = TRUE);

	/** @brief ����һ����ʹ�豸��дȨ�޵�֡. 
	 *///�Ѿ�����
	// static StandardFrame* BuildEnableWriteFrame();

	const UCHAR* GetPeerAddr()const;

	const UCHAR* GetHostAddr()const;	
	
	BOOL DoesPeerUseGroupAddress()const;

	Frame* CloneMe()const;

protected:
	StandardFrame(){}
};

/** @brief ��չ֡. 
 */
class ExtendedFrame : public Frame
{
public:
	ExtendedFrame(const APDU& apdu);
	ExtendedFrame(const ExtendedFrame& frame);

	/// ���ÿ����ֵ��ظ�λ. 
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