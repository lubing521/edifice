#include "StdAfx.h"
#include "SoftUnit.h"
#include "generic_helper.h"
#include "Pentagon.h"
#include "assert.h"

////////////////////  SoftUnit ����  /////////////////

SoftUnit::SoftUnit(UCHAR bit_width)
{
	m_bit_width = bit_width;
	m_byte_len = ceil_divide(bit_width, 8);
	m_bits = new UCHAR[m_byte_len];
	m_bits_save = new UCHAR[m_byte_len];
	memset(m_bits, 0, m_byte_len);
	memset(m_bits_save, 0, m_byte_len);

	m_emulator = NULL;
	m_dev_id = ~0;
}

SoftUnit::SoftUnit(const SoftUnit& su)
{
	m_bit_width = su.m_bit_width;
	m_byte_len = su.m_byte_len;
	m_bits = new UCHAR[m_byte_len];
	m_bits_save = new UCHAR[m_byte_len];
	memcpy(m_bits, su.m_bits, m_byte_len);
	memcpy(m_bits_save, su.m_bits_save, m_byte_len);

	m_emulator = su.m_emulator;
	m_dev_id = su.m_dev_id;

	m_softdev_connected = su.m_softdev_connected;
}

SoftUnit::~SoftUnit(void)
{
	delete m_bits;
	delete m_bits_save;
}

void SoftUnit::SetValue(UCHAR* bits)
{
	if ( bits != NULL )
	{
		memcpy(m_bits_save, m_bits, m_byte_len);
		memcpy(m_bits, bits, m_byte_len);
	}
}

const UCHAR* SoftUnit::GetValue()
{
	return m_bits;
}

const UCHAR* SoftUnit::GetSaveValue()
{
	return m_bits_save;
}

int SoftUnit::SetUnitName(const char* name)
{
	m_softunit_name = name;

	// ͬ���ӿڶ�����ֵ�ͱ����мǵ�ֵ 
	if ( NULL != m_emulator && ~0 != m_dev_id && !m_softunit_name.empty() ) 
	{
		m_softdev_connected = TRUE;
		if ( 0 == OnConnectedEvent() )
			return 0;
		else
			return -1;
	}
	else
	{
		m_softdev_connected = FALSE;
	}
	return 0;
}

int SoftUnit::Connect2SoftDev(IEIBEmulator* emulator, UINT dev_id)
{
	m_emulator = emulator;
	m_dev_id = dev_id;

	// ͬ���ӿڶ�����ֵ�ͱ����мǵ�ֵ 
	if ( NULL != m_emulator && ~0 != m_dev_id && !m_softunit_name.empty() ) 
	{
		m_softdev_connected = TRUE;
		if ( 0 == OnConnectedEvent() )
			return 0;
		else
			return -1;
	}
	else
	{
		m_softdev_connected = FALSE;
	}
	return 0;
}

UCHAR SoftUnit::GetByteLen()
{
	return m_byte_len;
}

UCHAR SoftUnit::GetBitWidth()
{
	return m_bit_width;
}

const SoftUnit& SoftUnit::operator =(const SoftUnit& su)
{
	m_bit_width = su.m_bit_width;
	m_byte_len = su.m_byte_len;
	m_bits = new UCHAR[m_byte_len];
	m_bits_save = new UCHAR[m_byte_len];
	memcpy(m_bits, su.m_bits, m_byte_len);
	memcpy(m_bits_save, su.m_bits_save, m_byte_len);

	m_softdev_connected = su.m_softdev_connected;

	return *this;
}

///////////////////////// ֻ��һ��bitλ��SoftUnit �� X, Y, M, S //////////////////////

SoftUnit_1Bit::SoftUnit_1Bit():SoftUnit(1)
{
}

void SoftUnit_1Bit::SetState(BOOL state)
{
	SetValue((UCHAR*)&state);
	
	// �Ƿ�ͬ�����½ӿڸ������ֵ. 
	if ( m_softdev_connected )
	{
		m_emulator->WriteSoftUnit(m_dev_id, m_softunit_name.c_str(), (uchar*)&state);
	}
}

BOOL SoftUnit_1Bit::GetState()
{
	if ( !m_softdev_connected )  // û�кͽӿڶ���Խ�.��ʹ���Լ�������. 
	{
		const UCHAR* ptr = GetValue();
		BOOL b = *ptr;
		return b;	
	}
	else  // ��ӿڶ���Խ��˾ʹӽӿڶ�����ȡֵ. 
	{
		uchar buff[8];
		m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), buff);
		return !!buff[0];
	}
}

int SoftUnit_1Bit::OnConnectedEvent()
{
	uchar buff[32];
	const uchar* retv = m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), buff);
	if ( retv )
	{
		memcpy(m_bits, buff, m_byte_len);
		return 0;
	}
	else
	{
		return -1;
	}
}

//////////////////////// ��ʮ��������λ����Ԫ��. ////////////////////////

SoftUnit_16Bit::SoftUnit_16Bit():SoftUnit(16)
{
}

void SoftUnit_16Bit::SetValue(UCHAR* bits)
{
	if ( m_softdev_connected )
	{
		m_emulator->WriteSoftUnit(m_dev_id, m_softunit_name.c_str(), bits);
	}

	SoftUnit::SetValue(bits);
}

const UCHAR* SoftUnit_16Bit::GetValue()
{
	const UCHAR* retval = NULL;
	if ( m_softdev_connected )
	{
		uchar buff[8];
		if ( NULL == m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), buff) )
		{
			return NULL;
		}
		// �Ѵӽӿڶ��������ֵ����SoftUnit_16���ѵ�������.
		memcpy(m_bits, buff, 2);
		return m_bits;
	}
	else
	{
		retval = SoftUnit::GetValue();
		return retval;
	}
}

int SoftUnit_16Bit::OnConnectedEvent()
{
	uchar  buff[32];
	const uchar* retv = m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), buff);
	if ( retv )
	{
		memcpy(m_bits, buff, m_byte_len);
		return 0;
	}
	else
	{
		return -1;
	}
}

/////////////////////// ������ /////////////////////////////
SoftUnit_C::SoftUnit_C() : SoftUnit(16)
{
	m_output = FALSE;
}

/// ����ߵ͵�ƽ. 
void SoftUnit_C::Input(BOOL input)
{
	if ( input )  // 1 ��Ч.ÿ����1���ͼ���. 
	{
		USHORT count = *(USHORT*)GetValue();
		USHORT tick = count + 1;
		SetValue((UCHAR*)&tick);
		if ( tick == m_trigger_point )
		{
			if ( m_softdev_connected )
			{
				m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), (uchar*)&m_output);
			}

			m_output ^= 1;  // ��ת. 

			if ( m_softdev_connected )
			{
				m_emulator->WriteSoftUnit(m_dev_id, m_softunit_name.c_str(), (uchar*)&m_output);
			}
		}
	}
}

/// �õ������ƽ. 
BOOL SoftUnit_C::Output()
{
	if ( m_softdev_connected )
	{
		m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), (uchar*)&m_output);
	}

	return m_output;
}

void SoftUnit_C::SetTriggerPoint(USHORT trigger_point)
{
	m_trigger_point = trigger_point;
}

USHORT SoftUnit_C::GetCountValue()
{
	USHORT us = *(USHORT*)GetValue();
	return us;
}

void SoftUnit_C::SetCountValue(USHORT us)
{
	SetValue((UCHAR*)&us);
}

int SoftUnit_C::OnConnectedEvent()
{
	uchar buff[32];
	const uchar* retv = m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), buff);
	if ( retv )
	{
		m_output = !!*buff;
		return 0;
	}
	else
	{
		return -1;
	}
}

/// ����. 
void SoftUnit_C::Reset()
{
	UCHAR mem[2] = { 0 };
	SetValue(mem);
	m_output = 0;
	m_trigger_point = 0;

	if ( m_softdev_connected )
	{
		m_emulator->WriteSoftUnit(m_dev_id, m_softunit_name.c_str(), (uchar*)&m_output);
	}
}

///////////////////// ��ʱ�� //////////////////////////////

static DWORD WINAPI __thread4SoftUnit_T(void* arg)
{
	DWORD retcode = ((SoftUnit_T*)arg)->_thread();
	return retcode;
}

DWORD SoftUnit_T::_thread()
{
	SoftUnit_T * T;
	while ( !m_quit_thread )
	{
		for ( int i = 0; i < m_timers.size(); i++ )
		{
			T = m_timers[i];
			if ( T->m_working_now )
			{
				USHORT count = *(USHORT*)T->GetValue();
				USHORT tick = count + 1;
				T->SetValue((UCHAR*)&tick);
				if ( tick == T->m_trigger_point )
				{
					if ( T->m_softdev_connected )
					{
						T->m_emulator->ReadSoftUnit(T->m_dev_id, T->m_softunit_name.c_str(), (uchar*)&T->m_output);
					}

					T->m_output ^= 1;  // ��ת. 

					if ( T->m_softdev_connected )
					{
						T->m_emulator->WriteSoftUnit(T->m_dev_id, T->m_softunit_name.c_str(), (uchar*)&T->m_output);
					}
				}
			}
		}

		Sleep(1); // ˯һ����. 
	}
	return 0;
}

// SoftUnit_T ���߳�������. 
volatile BOOL    SoftUnit_T::m_quit_thread;
volatile UINT    SoftUnit_T::m_ref2thread; 
HANDLE  SoftUnit_T::m_mutex4timers;
HANDLE  SoftUnit_T::m_thread;  // ����һ���߳�. 
std::vector<SoftUnit_T*> SoftUnit_T::m_timers;
SoftUnit_T::SoftUnit_T() : SoftUnit(16)
{
	m_working_now = FALSE;
	m_mutex = CreateMutex(NULL, FALSE, NULL);
	if ( NULL == m_mutex )
	{
		report_err_immed("SoftUnit_T::SoftUnit_T failed to call CreateMutex.", 0xdd);
	}

	if ( NULL == m_thread )
	{
		assert(NULL == m_mutex4timers);

		m_quit_thread = false;

		m_mutex4timers = CreateMutex(NULL, FALSE, NULL);
		if ( NULL == m_mutex4timers )
		{
			report_err_immed("SoftUnit_T::SoftUnit_T failed to call CreateMutex.", 0xdd);
		} 

		m_thread = CreateThread(NULL, 0, __thread4SoftUnit_T, this, 0, NULL);
		if ( NULL == m_thread )
		{
			report_err_immed("SoftUnit_T::SoftUnit_T failed to call CreateThread.", 0xde);
		}

		m_ref2thread++;
		assert(1==m_ref2thread);

		HoldTheMutex htm(m_mutex4timers);
		m_timers.clear();
		m_timers.push_back(this);
	}
}

SoftUnit_T::~SoftUnit_T()
{
	CloseHandle(m_mutex);
	m_mutex = NULL;

	m_ref2thread--;
	if ( 0 == m_ref2thread )
	{
		assert(NULL!=m_thread);
		assert(NULL!=m_mutex4timers);
		
		m_quit_thread = TRUE;
		WaitForSingleObject(m_thread, INFINITE);
		
		CloseHandle(m_thread);
		CloseHandle(m_mutex4timers);

		m_thread = NULL;
		m_mutex4timers = NULL;
	}

	HoldTheMutex htm(m_mutex4timers);
	BOOL delete_me = false;
	for ( std::vector<SoftUnit_T*>::iterator ite = m_timers.begin();
		  ite != m_timers.end();
		  ite++ )
	{
		if ( *ite == this )
		{
			m_timers.erase(ite);
			delete_me = true;
			break;
		}
	}
	assert(delete_me);
	if ( 0 == m_ref2thread )
	{
		assert( 0 == m_timers.size() );
	}
}

/// ����ߵ͵�ƽ. 
void SoftUnit_T::Input(BOOL input)
{
	// UNDO
	m_working_now = input;
}

/// �õ������ƽ. 
BOOL SoftUnit_T::Output()
{
	if ( m_softdev_connected )
	{
		m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), (uchar*)&m_output);
	}

	return m_output;
}

/// ���ô�����. 
void SoftUnit_T::SetTriggerPoint(USHORT trigger_point)
{
	m_trigger_point = trigger_point;
}

/// ����. 
void SoftUnit_T::Reset()
{
	UCHAR mem[2] = { 0 };
	SetValue(mem);
	m_output = 0;
	m_trigger_point = 0;
	m_working_now = 0;
}

/// �õ�����ֵ. 
USHORT SoftUnit_T::GetCountValue()
{
	USHORT us;
	us = *(USHORT*)GetValue();
	return us;
}

void SoftUnit_T::SetCountValue(USHORT us)
{
	SetValue((uchar*)&us);
}

int SoftUnit_T::OnConnectedEvent()
{
	uchar buff[32];
	const uchar* retv = m_emulator->ReadSoftUnit(m_dev_id, m_softunit_name.c_str(), buff);
	if ( retv != NULL )
	{
		m_output = !!*buff;
		return 0;
	}
	else
	{
		return -1;
	}
}