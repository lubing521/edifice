#ifndef _H_GLOBAL_
#define _H_GLOBAL_

/** @brief 如果要从DLL导出一个接口.就要从该接口继续. 
 */
class IBase
{
public:
	virtual void Free()
	{
		delete this;
	}

protected:	
	// 接口只能在堆上分配内存.并用Free函数释放自已. 
	virtual ~IBase(){};
};


/** @brief 用在通迅类的一些枚举. 
 */
enum ResponseType
{
	RESPONSE_UNDEFINED = 100,   // 未定义的. 

	RESPONSE_SENDING_TIMEOUT,   // 发送请求时超时. 
	RESPONSE_SENDING_OKAY,      // 发送成功. 
	RESPONSE_SENDING_INQUEUE,   // 成功放入发送队列. 

	RESPONSE_RECVING_TIMEOUT,   // 接收超时. 
	RESPONSE_NORMAL,            // 得到了响应. 
	RESPONSE_SHORT,             // 得到了短回复. 

	RESPONSE_ERROR_WAIT4OBJECT, // waitforsingleobject函数出错. 
	RESPONSE_ERROR_CREATEEVENT, // createEvent函数出错. 

	RESPONSE_BAD_AUGUMENT,      // 发送参数不正确. 
	RESPONSE_BAD_QUERY,         // 坏的请求. 
	RESPONSE_BAD_RESP,          // 坏的回复. 
	RESPONSE_QUIT_FORCED        // 手动强行退出. 
};

/** @brief 一个回调函数的类型typedef. 
 */
typedef void (*Reporter)(const unsigned char* buff, int len, const void* arg);

#endif