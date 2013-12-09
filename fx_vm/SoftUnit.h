#ifndef _H_SOFTUNIT_
#define _H_SOFTUNIT_

#include <vector>
#include "IEIBEmulator.h"

/// ������Ԫ���Ļ���. 
class SoftUnit
{
public:
	// ���������Ԫ��bitλ��. 
	SoftUnit(UCHAR bit_width);
	SoftUnit(const SoftUnit& su);
	virtual ~SoftUnit(void);

	// ����ֵ. 
	virtual void SetValue(UCHAR* bits);

	// �õ�ֵ. 
	virtual const UCHAR* GetValue();

	// �õ�save value. 
	const UCHAR* GetSaveValue();

	// �����豸�ı��. 
	// ����0ʱ�ɹ�,���ط�0ʱʧ��.����SoftUnit���ӵ��ӿڶ�����ӿڶ�����û�з������SoftUnitʱ���ط�0. 
	int SetUnitName(const char* num);

	// ���������豸�ӿڶ���ԽӵĽӿ�ָ��. 
	int Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id);  

	UCHAR  GetByteLen();
	UCHAR  GetBitWidth();

	const SoftUnit& operator =(const SoftUnit& su);

protected:
	// ���ӵ��ӿڶ����ʱ������������ͬ��SoftUnit�Լ����������ͽӿڶ����������. 
	// ����0�ɹ������ط�0��ʧ��.
	virtual int OnConnectedEvent() = 0;

protected:
	UCHAR* m_bits;
	UCHAR* m_bits_save;
	UCHAR  m_bit_width; // m_bits��m_bits_save�ĳ���,��λbit. 
	UCHAR  m_byte_len;  // m_bits��m_bits_save�ĳ���,��λbyte. 
	
	// �����������������úþ��ܶԽӵ��ӿڶ��������. 
	// �������������ͽӿڶ�������ݶԽ�. 
	IEIBEmulator* m_emulator;
	UINT   m_dev_id;
	// �豸�ı��. 
	std::string m_softunit_name;
	
	BOOL m_softdev_connected;  // �Ƿ�����ݶԽӵ��ӿڶ���. 
};

/// ֻ��һ��bitλ����Ԫ��.�� X, Y, M, S
class SoftUnit_1Bit : public SoftUnit
{
public:
	SoftUnit_1Bit();

	/// �õ�Bit��״̬. 
	BOOL GetState();
	/// ����Bit��״̬. 
	void SetState(BOOL state);
	
protected:
	virtual int OnConnectedEvent();
};

// X,Y,M,S,G
typedef SoftUnit_1Bit SoftUnit_X;
typedef SoftUnit_1Bit SoftUnit_Y;
typedef SoftUnit_1Bit SoftUnit_M;
typedef SoftUnit_1Bit SoftUnit_S;
typedef SoftUnit_1Bit SoftUnit_G;

//// Y
//class SoftUnit_Y : public SoftUnit_1Bit
//{
//public:
//	void Set();
//};
//
//// M
//class SoftUnit_M : public SoftUnit_1Bit
//{
//public:
//	void Set();
//};
//
//// S
//class SoftUnit_S : public SoftUnit_1Bit
//{
//public:
//	void Set();
//};

/// ���ݼĴ��� D
class SoftUnit_16Bit : public SoftUnit
{
public:
	SoftUnit_16Bit();

	void SetValue(UCHAR* bits);

	const UCHAR* GetValue();

protected:
	virtual int OnConnectedEvent();
};

typedef SoftUnit_16Bit SoftUnit_D;

/// ������. 
class SoftUnit_C : public SoftUnit
{
public:
	SoftUnit_C();
	
	/// ����ߵ͵�ƽ. 
	void Input(BOOL input);

	/// �õ������ƽ. 
	BOOL Output();

	/// ���ô�����. 
	void SetTriggerPoint(USHORT trigger_point);

	/// ����. 
	void Reset();

	/// �õ�����ֵ. 
	USHORT GetCountValue();

	// ���ü���ֵ. 
	void SetCountValue(USHORT us);

protected:
	virtual int OnConnectedEvent();

private:
	BOOL    m_output;
	USHORT  m_trigger_point;
};

/// ��ʱ��. 
class SoftUnit_T : public SoftUnit
{
public:
	SoftUnit_T();
	~SoftUnit_T();

	/// ����ߵ͵�ƽ. 
	void Input(BOOL input);

	/// �õ������ƽ. 
	BOOL Output();

	/// ���ô�����. 
	void SetTriggerPoint(USHORT trigger_point);

	/// ����. 
	void Reset();

	/// �õ�����ֵ. 
	USHORT GetCountValue();

	/// ���ü���ֵ. 
	void SetCountValue(USHORT us);

protected:
	virtual int OnConnectedEvent();

private:
	friend DWORD WINAPI __thread4SoftUnit_T(void* arg);
	static DWORD _thread();

private:
	BOOL    m_output;
	USHORT  m_trigger_point;
	
	HANDLE  m_mutex;  ///< for SoftUnit_T to protected per-SoftUnit_T's resource. 

	volatile BOOL m_working_now;   ///< ��־�����ʱ�����ڹ���. 
    
	static volatile UINT m_ref2thread;    ///< ÿ����һ��SoftUnit_Tʱ�ӣ��ͻ�����һ�����ü���.  
	static volatile BOOL m_quit_thread;   ///< �Ƿ��˳��߳�. 
	static HANDLE  m_thread;  ///< ����һ���߳�.
	static HANDLE  m_mutex4timers;  ///< �������m_timers. 
	static std::vector<SoftUnit_T*> m_timers;  ///<     �����д����ļ�ʱ��ʵ����ָ��. 
};

#endif