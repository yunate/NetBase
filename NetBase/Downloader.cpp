#include "NetBaseHead.h"

#include "Downloader.h"
#include "DownloaderPool.h"
#include "string_convert.h"

#include "DownCore_Curl.h"
#include "DownCore_WinINet.h"

#include <fstream>
#include <functional>
#include <windows.h>


Downloader::Downloader(const RequestInfo& info) :
	m_Info(info),
	m_pDownCore(0)
{
	m_sTempPath = m_Info.m_sSvePath + ".temp";
	DeleteFileA(m_sTempPath.c_str());

	if (m_Info.m_pOutStream)
	{
		m_pOutStream = m_Info.m_pOutStream;
	}
	else
	{
		m_pOutStream = new ZMFileStream(base::MultibytesToUnicode(m_sTempPath.c_str(), 0), fmOpenReadWrite | fmCreate);
	}
}

Downloader::~Downloader()
{
	if (m_pDownCore)
	{
		delete m_pDownCore;
		m_pDownCore = 0;
	}
}

bool Downloader::WriteDownFileCallBack(void * ptr, size_t size)
{
	m_pOutStream->Write((char*)ptr, size);
	return true;
}

bool Downloader::DownProcessCallBack(
	double dtotal,							// ����ʹ�ã�������	
	double dnow								// ����ʹ�ã������أ�
)
{
	bool bStop = false;
	OnDownLoading(dtotal, dnow);

	if (m_Info.m_ProcessEvent)
	{
		m_Info.m_ProcessEvent(m_Info.m_nId, (INT64)dtotal, (INT64)dnow, bStop);
	}

	if (bStop)
	{
		m_pDownCore->Stop();
	}

	return true;
}

void Downloader::_Execute()
{
	if (!m_pOutStream)
	{
		return;
	}

#if defined USELIB_CURL
	m_pDownCore = new DownCore_Curl();
#elif defined USELIB_WININET
	m_pDownCore = new DownCore_WinINet();
#endif // USELIB_CURL

	m_pDownCore->Construct(&m_Info
		, m_pOutStream
		, std::bind(&Downloader::WriteDownFileCallBack, this, std::placeholders::_1, std::placeholders::_2)
		, std::bind(&Downloader::DownProcessCallBack, this, std::placeholders::_1, std::placeholders::_2));

	NETBASE_DOWN_STATUS nResCode = NETBASE_DOWN_STATUS_NODEFINE;
	std::string sResDes = "";
	bool bRes = m_pDownCore->Down(nResCode, sResDes);
	nResCode = bRes ? NETBASE_DOWN_STATUS_SUCCESS : NETBASE_DOWN_STATUS_FAILURE;

	if (!m_Info.m_pOutStream)
	{
		delete m_pOutStream;
		m_pOutStream = 0;
	}

	if (bRes)
	{
		if (!m_Info.m_pOutStream)
		{
			// ����ʱ�ļ�ת��Ϊ�����ļ�
			MoveFileA(m_sTempPath.c_str(), m_Info.m_sSvePath.c_str());
		}
		
		m_DwonloadData.m_bResult = true;
	}
	else
	{
		if (!m_Info.m_pOutStream)
		{
			// ���������ֹ��ɾ����ʱ�ļ�,��Ҫ�������ط�ɾ��
			DeleteFileA(m_sTempPath.c_str());
		}
		
		m_DwonloadData.m_bResult = false;
	}

	m_DwonloadData.m_bIsEnd = true;

	if (m_Info.m_ResponseResEvent)
	{
		m_Info.m_ResponseResEvent(m_Info.m_nId, nResCode, base::MultibytesToUnicode(sResDes, 0 ));
	}
}

double Downloader::GetFileSize()
{
	if (m_pDownCore)
	{
		return m_pDownCore->GetSize();
	}

	return 0;
}

void Downloader::Cancel()
{
	if (m_pDownCore)
	{
		m_pDownCore->Pause();
	}
}

void Downloader::OnDownLoading(double all, double size)
{
	m_DwonloadData.m_bIsStart = true;
	m_DwonloadData.m_fCurSize = size;
	m_DwonloadData.m_fAllSize = all;
}

