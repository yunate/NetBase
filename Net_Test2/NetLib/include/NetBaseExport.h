#ifndef __EXPORT_H_
#define __EXPORT_H_

#include "Streams.h"
#include <functional>
#include <map>

#define   BUFFSIZE 2048

enum NETBASE_DOWN_STATUS
{
	NDS_NODEFINE = -1,
	NDS_SUCCESS  = 0,
	NDS_RESPONSESTREAM,					// �������Ч 
	NDS_CRACKURLFAILURE,				// url crack ʧ�� 
	NDS_SCHEMEERROR,					// scheme ����http/https
	NDS_INTERNETOPENFAILURE,			// ������ʧ��				��InternetOpen��
	NDS_CONNECTFAILURE,					// ��secssionʧ��			��InternetConnect��
	NDS_REQUESTFAILURE,					// ������ʧ��				��HttpOpenRequest��
	NDS_ADDREQUESTFAILURE,				// ����http headerʧ��		��HttpAddRequestHeaders��
	NDS_SENDREQUESRFAILURE,				// ��������ʧ��				��HttpSendRequestEx��
	NDS_ENDQUESRFAILURE,				// �ر�����ʧ��				��HttpEndRequest��
	NDS_WRITEFILEFAILURE,				// ����bodyʧ��				��InternetWriteFile��
	NDS_TIMEOUT,						// ��ʱ
	NDS_QUERYINFOFAILURE,				//							��HttpQueryInfo��
	NDS_RETURNCODEOVER400,				// ���󷵻�ֵ����400
	NDS_READFILEFAILURE,				// ��ȡ����buff����			��InternetReadFile��
	NDS_FAILURE
}; 

using  HttpHeader =std::map<ZMString, ZMString>;

//���Ȼص�
using ProgressCallback = std::function<void(int id, INT64  contentLength, INT64 readCount, bool& abort)>;

//����ص�
using ResultCallBack = std::function<void(int id, NETBASE_DOWN_STATUS responseCode)>;

typedef struct RequestInfo
{
	unsigned int			m_nId					= 0;		// id���ص�ʱ��ʹ��
	ZMString				m_sUrl					= L"";		// url
	HttpHeader				m_RequestHeadMap;					// httpͷ��β����Ҫ��\0�������ַ�
	ZMStream *				m_ResponseStream		= 0;		// �����
	ProgressCallback		m_ProgressCallback		= 0;		// ���Ȼص�
	ResultCallBack			m_ResultCallback		= 0;		// �����ص�
	ZMStream *				m_RequeseBodyStream		= 0;		// postʹ�ã�http body��������β����Ҫ��\0�������ַ�
	int						m_nTimeOut				= -1;		// ��ʱ(ms)������0ʱ��Ч�����������Чʱ�򽫴���һ��������߳�������ʱ
} RequestInfo;

bool  HttpGet(RequestInfo & info);

bool  HttpPost(RequestInfo & info);

#endif // 
