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

	// 估计收每个字节需要的时间. 
	double time_cost_per_byte_d = 1000.0 / ( baudrate / 10 );
	m_time_cost_per_byte = (time_cost_per_byte_d + 1);

	/////////////////////////// 下面是初始化端口的代码 //////////////////////
#ifndef __GNUC__   // win平台初始化端口. 
	
	COMMTIMEOUTS TimeOuts;
	
	sprintf(buff, "COM%u", port);
	// 开COM端口
	m_hcom = CreateFileA(buff,      // COMx口
		GENERIC_READ|GENERIC_WRITE, //允许读和写
		0, //独占方式
		NULL,
		OPEN_EXISTING, //打开而不是创建
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //重叠方式
		// 0, // 同步方式. 
		NULL);
	if ( m_hcom == INVALID_HANDLE_VALUE )
	{
		retcode = GetLastError();
		goto _out;
	}
	SetupComm(m_hcom, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是1024.

	// 设定读超时
	//TimeOuts.ReadIntervalTimeout=0;       // 不使用读间隔超时.即接收两个字节之间的超时.
	//TimeOuts.ReadTotalTimeoutMultiplier= 2 * m_time_cost_per_byte;
	//TimeOuts.ReadTotalTimeoutConstant= 10 * m_time_cost_per_byte;

	//在读一次输入缓冲区的内容后读操作就立即返回，
	//而不管是否读入了要求的字符。
	TimeOuts.ReadIntervalTimeout=MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier=0;
	TimeOuts.ReadTotalTimeoutConstant=0;

	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier=100;
	TimeOuts.WriteTotalTimeoutConstant=500;
	SetCommTimeouts(m_hcom, &TimeOuts); //设置超时

	DCB dcb;
	GetCommState(m_hcom, &dcb);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;    //每个字节有8位
	dcb.Parity = NOPARITY; //无奇偶校验位
	dcb.StopBits = TWOSTOPBITS; //两个停止位
	SetCommState(m_hcom, &dcb);

	PurgeComm(m_hcom, PURGE_TXCLEAR|PURGE_RXCLEAR);

#else    // linux平台初始化端口的代码. 
	
	termios options;
	uint baund_rate_const = GetBaundRateConst(baudrate);

	sprintf(buff, "/dev/ttyS%d", port);  // linux下的COM口设备. 
	// 开COM端口
	m_fd_comm = open(buff, O_RDWR|O_NOCTTY|O_NONBLOCK);
	if ( m_fd_comm < 0 )
	{
		retcode = errno;
		goto _out;
	}
	
	fcntl(m_fd_comm, F_SETFL, FNDELAY); // 虽然前面已经设了非阻塞了.

	// get the current options for the port...
	tcgetattr(m_fd_comm, &options);
	
	// set the baud rates.
	cfsetispeed(&options, baund_rate_const);
	cfsetospeed(&options, baund_rate_const);

	// enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;   // 无奇偶校验. 
	options.c_cflag |= CSTOPB;    // 两个停止位. 
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;      // 每个字节八位. 

	options.c_cc[VMIN] = 0;

	// set the new options for the port...
	tcsetattr(m_fd_comm, TCSANOW, &options);

#endif
	////////////////////////////////////////////////////////////////////////
	
	retcode = CommBase::Init(initdata);
	if ( retcode != 0 )  // 父类初始化失败了. 
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

#ifndef __GNUC__       // win平台下. 
	// 异步写串口. 
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE/*auto*/, FALSE/*non-signaled*/, NULL);

	DWORD   bytes_written = frame_len;
	COMSTAT com_stat;
	DWORD   err_flag;
	BOOL    write_stat;
	ClearCommError(m_hcom, &err_flag, &com_stat);

	// 异步写串口.最终会全部写入咯.但是也不太确定喔.就目前DLL使用情况来看没有问题. 
	write_stat = WriteFile(m_hcom , frame_mem, bytes_written, &bytes_written, &overlapped);
	if(!write_stat)
	{
		if(GetLastError()==ERROR_IO_PENDING)
		{
			DWORD ret = WaitForSingleObject(overlapped.hEvent, INFINITE);//estimated_total_sending_time * 4);
			if ( ret != WAIT_OBJECT_0 )
			{
				log_me("tx: WaitForSingleObject出错.返回值%u", ret);
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
			log_me("tx: 发送数据出错.");
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

#ifndef __GNUC__  // win 下. 

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
	if ( !read_stat )  //如果ReadFile函数返回FALSE
	{
		if(GetLastError()==ERROR_IO_PENDING)
		{
			// GetOverlappedResult函数的最后一个参数设为TRUE,函数会一直等待，直到读操作完成或由于错误而返回。
			GetOverlappedResult(m_hcom, &overlapped, &bytes_read, TRUE);
		}
		else
		{
			bytes_read = 0;
			retcode = -1;
		}
	}

#else   // Linux 下. 
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
