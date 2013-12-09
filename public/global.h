#ifndef _H_GLOBAL_
#define _H_GLOBAL_

/** @brief ���Ҫ��DLL����һ���ӿ�.��Ҫ�Ӹýӿڼ���. 
 */
class IBase
{
public:
	virtual void Free()
	{
		delete this;
	}

protected:	
	// �ӿ�ֻ���ڶ��Ϸ����ڴ�.����Free�����ͷ�����. 
	virtual ~IBase(){};
};


/** @brief ����ͨѸ���һЩö��. 
 */
enum ResponseType
{
	RESPONSE_UNDEFINED = 100,   // δ�����. 

	RESPONSE_SENDING_TIMEOUT,   // ��������ʱ��ʱ. 
	RESPONSE_SENDING_OKAY,      // ���ͳɹ�. 
	RESPONSE_SENDING_INQUEUE,   // �ɹ����뷢�Ͷ���. 

	RESPONSE_RECVING_TIMEOUT,   // ���ճ�ʱ. 
	RESPONSE_NORMAL,            // �õ�����Ӧ. 
	RESPONSE_SHORT,             // �õ��˶̻ظ�. 

	RESPONSE_ERROR_WAIT4OBJECT, // waitforsingleobject��������. 
	RESPONSE_ERROR_CREATEEVENT, // createEvent��������. 

	RESPONSE_BAD_AUGUMENT,      // ���Ͳ�������ȷ. 
	RESPONSE_BAD_QUERY,         // ��������. 
	RESPONSE_BAD_RESP,          // ���Ļظ�. 
	RESPONSE_QUIT_FORCED        // �ֶ�ǿ���˳�. 
};

/** @brief һ���ص�����������typedef. 
 */
typedef void (*Reporter)(const unsigned char* buff, int len, const void* arg);

#endif