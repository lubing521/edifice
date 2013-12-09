#include "stdafx.h"
#include "SerialComm_MM.h"

SerialComm_MM::SerialComm_MM(void)
{
	m_inited = false;
}

SerialComm_MM::~SerialComm_MM(void)
{
	Uninit();
}

int SerialComm_MM::Init(const InitData& initdata)
{
	if ( m_inited )
	{
		return -1;
	}

	int retcode = 0;
	char buff[0xff] = { 0 };

	unsigned short port = initdata._serial_comm_initdata._port;
	unsigned int baudrate = initdata._serial_comm_initdata._baudrate;

	// ������ÿ���ֽ���Ҫ��ʱ��. 
	double time_cost_per_byte_d = 1000.0 / ( baudrate / 10 );
	m_time_cost_per_byte = (time_cost_per_byte_d + 1);

	/////////////////////////// �����ǳ�ʼ���˿ڵĴ��� //////////////////////
#ifndef __GNUC__   // winƽ̨��ʼ���˿�. 
	
	COMMTIMEOUTS TimeOuts;
	
	sprintf(buff, "COM%u", port);
	// ��COM�˿�
	m_hcom = CreateFileA(buff,      // COMx��
		GENERIC_READ|GENERIC_WRITE, //�������д
		0, //��ռ��ʽ
		NULL,
		OPEN_EXISTING, //�򿪶����Ǵ���
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //�ص���ʽ
		// 0, // ͬ����ʽ. 
		NULL);
	if ( m_hcom == INVALID_HANDLE_VALUE )
	{
		retcode = GetLastError();
		goto _out;
	}
	SetupComm(m_hcom, 1024, 1024); //���뻺����������������Ĵ�С����1024.

	// �趨����ʱ
	//TimeOuts.ReadIntervalTimeout=0;       // ��ʹ�ö������ʱ.�����������ֽ�֮��ĳ�ʱ.
	//TimeOuts.ReadTotalTimeoutMultiplier= 2 * m_time_cost_per_byte;
	//TimeOuts.ReadTotalTimeoutConstant= 10 * m_time_cost_per_byte;

	//�ڶ�һ�����뻺���������ݺ���������������أ�
	//�������Ƿ������Ҫ����ַ���
	TimeOuts.ReadIntervalTimeout=MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier=0;
	TimeOuts.ReadTotalTimeoutConstant=0;

	//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier=100;
	TimeOuts.WriteTotalTimeoutConstant=500;
	SetCommTimeouts(m_hcom, &TimeOuts); //���ó�ʱ

	DCB dcb;
	GetCommState(m_hcom, &dcb);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;    //ÿ���ֽ���8λ
	dcb.Parity = NOPARITY; //����żУ��λ
	dcb.StopBits = TWOSTOPBITS; //����ֹͣλ
	SetCommState(m_hcom, &dcb);

	PurgeComm(m_hcom, PURGE_TXCLEAR|PURGE_RXCLEAR);

#else    // linuxƽ̨��ʼ���˿ڵĴ���. 
	
	termios options;
	uint baund_rate_const = GetBaundRateConst(baudrate);

	sprintf(buff, "/dev/ttyS%d", port);  // linux�µ�COM���豸. 
	// ��COM�˿�
	m_fd_comm = open(buff, O_RDWR|O_NOCTTY|O_NONBLOCK);
	if ( m_fd_comm < 0 )
	{
		retcode = errno;
		goto _out;
	}
	
	fcntl(m_fd_comm, F_SETFL, FNDELAY); // ��Ȼǰ���Ѿ����˷�������.

	// get the current options for the port...
	tcgetattr(m_fd_comm, &options);
	
	// set the baud rates.
	cfsetispeed(&options, baund_rate_const);
	cfsetospeed(&options, baund_rate_const);

	// enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;   // ����żУ��. 
	options.c_cflag |= CSTOPB;    // ����ֹͣλ. 
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;      // ÿ���ֽڰ�λ. 

	options.c_cc[VMIN] = 0;

	// set the new options for the port...
	tcsetattr(m_fd_comm, TCSANOW, &options);

#endif
	////////////////////////////////////////////////////////////////////////
	
	retcode = CommBase::Init(initdata);
	if ( retcode != 0 )  // �����ʼ��ʧ����. 
	{
#ifndef __GNUC__
		CloseHandle(m_hcom);
		m_hcom = NULL;
#else
		close(m_fd_comm);
#endif
		goto _out;
	}

	m_inited = TRUE;

_out:
	return retcode;
}

void SerialComm_MM::Uninit()
{
	CommBase::Uninit();

	if ( !m_inited )
	{
		return;
	}

#ifndef __GNUC__
	CloseHandle(m_hcom);
	m_hcom = NULL;
#else
	close(m_fd_comm);
#endif

	m_inited = false;
}

#ifdef __GNUC__
uint SerialComm_MM::GetBaundRateConst(uint baund_rate)
{
	if ( baund_rate < 50 )
	{
		return B0;
	}
	else if ( baund_rate < 75 )
	{
		return B50;
	}
	else if ( baund_rate < 110 )
	{
		return B75;
	}
	else if ( baund_rate < 134 )
	{
		return B110;
	}
	else if ( baund_rate < 150 )
	{
		return B134;
	}
	else if ( baund_rate < 200 )
	{
		return 150;
	}
	else if ( baund_rate < 300 )
	{
		return B200;
	}
	else if ( baund_rate < 600 )
	{
		return B300;
	}
	else if ( baund_rate < 1200 )
	{
		return B600;
	}
	else if ( baund_rate < 1800 )
	{
		return B1200;
	}
	else if ( baund_rate < 2400 )
	{
		return B1800;
	}
	else if ( baund_rate < 4800 )
	{
		return B2400;
	}
	else if ( baund_rate < 9600 )
	{
		return B4800;
	}
	else if ( baund_rate < 19200 )
	{
		return B9600;
	}
	else if ( baund_rate < 38400 )
	{
		return B19200;
	}
	else if ( baund_rate < 57600 )
	{
		return B38400;
	}
	else if ( baund_rate < 76800 )
	{
		return B57600;
	}
	//else if ( baund_rate < 115200 )
	//{
	//	return B76800;
	//}
	else
	{
		return B115200;
	}
}
#endif

int SerialComm_MM::_send_frame(const Frame* frame)
{
	if ( NULL == frame )
	{
		return -1;
	}

	int retcode = 0;
	const uchar* frame_mem = NULL;
	uchar frame_len;
	frame->GetFrameMem(frame_mem, frame_len);

	DWORD  estimated_total_sending_time = m_time_cost_per_byte * frame_len;
	ResponseType  response_type = RESPONSE_SENDING_OKAY;

#ifndef __GNUC__       // winƽ̨��. 
	// �첽д����. 
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE/*auto*/, FALSE/*non-signaled*/, NULL);

	DWORD   bytes_written = frame_len;
	COMSTAT com_stat;
	DWORD   err_flag;
	BOOL    write_stat;
	ClearCommError(m_hcom, &err_flag, &com_stat);

	// �첽д����.���ջ�ȫ��д�뿩.����Ҳ��̫ȷ���.��ĿǰDLLʹ���������û������. 
	write_stat = WriteFile(m_hcom , frame_mem, bytes_written, &bytes_written, &overlapped);
	if(!write_stat)
	{
		if(GetLastError()==ERROR_IO_PENDING)
		{
			DWORD ret = WaitForSingleObject(overlapped.hEvent, INFINITE);//estimated_total_sending_time * 4);
			if ( ret != WAIT_OBJECT_0 )
			{
				log_me("tx: WaitForSingleObject����.����ֵ%u", ret);
				retcode = -2;
				CloseHandle(overlapped.hEvent);
				goto _out;
			}
		}
	}

	CloseHandle(overlapped.hEvent);

#else   // under Linux 

	int retval;
	int bytes_already_sent = 0;
	while ( bytes_already_sent < frame_len )
	{
		retval = write(m_fd_comm, frame_mem, frame_len - bytes_already_sent);
		if ( retval < 0 )
		{
			log_me("tx: �������ݳ���.");
			retcode = -2;
			goto _out;
		}
		bytes_already_sent += retval;
	}

#endif

_out:

	return retcode;
}

int SerialComm_MM::_recv_bytes(mem_t& mem)
{
	int retcode;
	unsigned char  rx_buff[512] = { 0 };
	uint rx_buff_len = sizeof(rx_buff);
	mem.clear();

#ifndef __GNUC__  // win ��. 

	DWORD bytes_read = 0;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(OVERLAPPED));
	overlapped.hEvent = CreateEvent(NULL, FALSE/*auto reset*/, FALSE, NULL);

	COMSTAT  com_stat;
	DWORD    err_flag;
	BOOL     read_stat;
	ClearCommError(m_hcom, &err_flag, &com_stat);

	retcode = 0;
	read_stat = ReadFile(m_hcom, rx_buff, rx_buff_len, &bytes_read, &overlapped);
	if ( !read_stat )  //���ReadFile��������FALSE
	{
		if(GetLastError()==ERROR_IO_PENDING)
		{
			// GetOverlappedResult���������һ��������ΪTRUE,������һֱ�ȴ���ֱ����������ɻ����ڴ�������ء�
			GetOverlappedResult(m_hcom, &overlapped, &bytes_read, TRUE);
		}
		else
		{
			bytes_read = 0;
			retcode = -1;
		}
	}

#else   // Linux ��. 
	int retval;
	uint bytes_read;
	retval = read(m_fd_comm, rx_buff, sizeof(rx_buff));
	if ( retval >= 0 )
	{
		bytes_read = retval;
		retcode = 0;
	}
	else
	{
		bytes_read = 0;
		retcode = -1;
	}

#endif

	if ( bytes_read > 0 )
	{
		mem.resize(bytes_read);
		mem.cp(rx_buff, bytes_read);
	}
	
	return retcode;
}
