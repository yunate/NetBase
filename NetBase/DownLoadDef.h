#ifndef __DownloadDef_H_
#define __DownloadDef_H_

#include "NetBaseExport.h"
#include <string>

// ���ص�����
typedef  unsigned int DOWNLOADER_TYPE;

// ��������
enum REQUESET_TYPE
{
	REQUESET_TYPE_ERROR = 0,
	REQUESET_TYPE_GET,
	REQUESET_TYPE_POST
	//REQUESET_TYPE_PUT,
	//REQUESET_TYPE_DELETE
};

class ZMStream;

// ׼������
struct RequestInfo
{
	RequestInfo()
		: m_nRequestType(REQUESET_TYPE_GET)
		, m_nType(0)
		, m_sUrl("")
		, m_pBodyStream(0)
		, m_sSvePath("")
		, m_pOutStream(0)
		, m_userdata(0)
	{

	}

	REQUESET_TYPE				m_nRequestType;		// �������ࣨget��post�ȣ�,Ĭ��get
	DOWNLOADER_TYPE	            m_nType;			// ���ص���ʲô���ࣨĬ����0��������ĳ�������ص�ͳһ����
	std::string					m_sUrl;				// ���ص�ַ
	ZMStream *					m_pBodyStream;		// �������ݣ�
	std::string					m_sHead;			// ����ͷ
	std::string					m_sSvePath;			// ��Ҫ�����·��
	ZMStream *					m_pOutStream;		// �������������Ϊ0ʱ��m_sSvePath��Ч
	unsigned int				m_userdata;			// �û����ݣ������û���һ�������ָ�봫����

	// ������������������ӻص�ʹ��
	unsigned int				m_nId;
	ProgressEvent				m_ProcessEvent;
	ResponseResult				m_ResponseResEvent;
};

// ��������
struct DownloadData
{
	DownloadData()
		: m_bIsStart(false)
		, m_bAllSizeHandle(false)
		, m_fAllSize(0.0)
		, m_fCurSize(0.0)
		, m_bIsEnd(false)
		, m_bResult(false)
		, m_nResCode(0)
	{

	}

	bool	     m_bIsStart;			// ��ʼ��������
	bool	     m_bAllSizeHandle;		// ��������m_fAllSizeʱ��ֻ֤����һ��
	double	     m_fAllSize;			// �ܴ�С
	double       m_fCurSize;			// ��ǰ�����˵Ĵ�С
	bool	     m_bIsEnd;				// ���ؽ�������
	bool	     m_bResult;				// ���ؽ��
	unsigned int m_nResCode;			// ��������룬һ�� m_bResult == false ��ʱ����Ч
};

#endif