#ifndef _H_SOFTUNIT_
#define _H_SOFTUNIT_

#include <vector>
#include "IEIBEmulator.h"

/// 所有软元件的基类. 
class SoftUnit
{
public:
	// 传入这个软单元的bit位宽. 
	SoftUnit(UCHAR bit_width);
	SoftUnit(const SoftUnit& su);
	virtual ~SoftUnit(void);

	// 设置值. 
	virtual void SetValue(UCHAR* bits);

	// 得到值. 
	virtual const UCHAR* GetValue();

	// 得到save value. 
	const UCHAR* GetSaveValue();

	// 设置设备的编号. 
	// 返回0时成功,返回非0时失败.例如SoftUnit连接到接口对象而接口对象中没有分配这个SoftUnit时返回非0. 
	int SetUnitName(const char* num);

	// 设置与软设备接口对象对接的接口指针. 
	int Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id);  

	UCHAR  GetByteLen();
	UCHAR  GetBitWidth();

	const SoftUnit& operator =(const SoftUnit& su);

protected:
	// 连接到接口对象的时候调用这个函数同步SoftUnit自己的数据区和接口对象的数据区. 
	// 返回0成功，返回非0，失败.
	virtual int OnConnectedEvent() = 0;

protected:
	UCHAR* m_bits;
	UCHAR* m_bits_save;
	UCHAR  m_bit_width; // m_bits和m_bits_save的长度,单位bit. 
	UCHAR  m_byte_len;  // m_bits和m_bits_save的长度,单位byte. 
	
	// 下面三个变量都设置好就能对接到接口对象的数据. 
	// 两个变量用来和接口对象的数据对接. 
	IEIBEmulator* m_emulator;
	UINT   m_dev_id;
	// 设备的编号. 
	std::string m_softunit_name;
	
	BOOL m_softdev_connected;  // 是否把数据对接到接口对象. 
};

/// 只有一个bit位的软元件.如 X, Y, M, S
class SoftUnit_1Bit : public SoftUnit
{
public:
	SoftUnit_1Bit();

	/// 得到Bit的状态. 
	BOOL GetState();
	/// 设置Bit的状态. 
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

/// 数据寄存器 D
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

/// 计数器. 
class SoftUnit_C : public SoftUnit
{
public:
	SoftUnit_C();
	
	/// 输入高低电平. 
	void Input(BOOL input);

	/// 得到输出电平. 
	BOOL Output();

	/// 设置触发点. 
	void SetTriggerPoint(USHORT trigger_point);

	/// 重置. 
	void Reset();

	/// 得到计数值. 
	USHORT GetCountValue();

	// 设置计数值. 
	void SetCountValue(USHORT us);

protected:
	virtual int OnConnectedEvent();

private:
	BOOL    m_output;
	USHORT  m_trigger_point;
};

/// 定时器. 
class SoftUnit_T : public SoftUnit
{
public:
	SoftUnit_T();
	~SoftUnit_T();

	/// 输入高低电平. 
	void Input(BOOL input);

	/// 得到输出电平. 
	BOOL Output();

	/// 设置触发点. 
	void SetTriggerPoint(USHORT trigger_point);

	/// 重置. 
	void Reset();

	/// 得到计数值. 
	USHORT GetCountValue();

	/// 设置计数值. 
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

	volatile BOOL m_working_now;   ///< 标志这个计时器正在工作. 
    
	static volatile UINT m_ref2thread;    ///< 每创建一个SoftUnit_T时钟，就会申请一个引用计数.  
	static volatile BOOL m_quit_thread;   ///< 是否退出线程. 
	static HANDLE  m_thread;  ///< 共用一个线程.
	static HANDLE  m_mutex4timers;  ///< 互斥访问m_timers. 
	static std::vector<SoftUnit_T*> m_timers;  ///<     记所有创建的计时器实例的指针. 
};

#endif