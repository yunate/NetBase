#include "stdafx.h"

#include "Downloader.h"
#include "DownloaderPool.h"
#include "functional"
#include "DownCore_Curl.h"
#include <fstream>
#include <windows.h>

// fwirte�����е���֣�����һ�����ݴ���1048576��ǿ��ˢһ��
#define BUFFMAXSIZE	1048576	 

Downloader::Downloader(const RequestInfo& info) :
	m_Info(info),
	m_bCancel(false),
	m_nBuffSize(0),
	m_pDownCore(0)
{
	m_sTempPath = m_Info.m_sSvePath + ".temp";
	DeleteFile(m_sTempPath.c_str());

	if (m_Info.m_pOutStream)
	{
		m_pOutStream = m_Info.m_pOutStream;
	}
	else
	{
		m_pOutStream = new std::ofstream(m_sTempPath.c_str(), std::ios::binary);
	}
}

Downloader::~Downloader()
{
	
}

bool Downloader::WriteDownFileCallBack(void * ptr, size_t size)
{
	m_pOutStream->write((char*)ptr, size);
	m_nBuffSize += size;

	if (BUFFMAXSIZE <= m_nBuffSize)
	{
		m_nBuffSize = 0;
		m_pOutStream->flush();
	}

	return true;
}

bool Downloader::DownProcessCallBack(
	double dtotal,							// ����ʹ�ã�������	
	double dnow								// ����ʹ�ã������أ�
)
{
	if (!IS_DOUBLE_ZERO(dtotal))
	{
		OnDownLoading(dtotal, dnow);
	}

	return true;
}

void Downloader::_Execute()
{
	m_pDownCore = new DownCore_Curl();
	m_pDownCore->Construct(&m_Info
		, m_pOutStream
		, std::bind(&Downloader::WriteDownFileCallBack, this, std::placeholders::_1, std::placeholders::_2)
		, std::bind(&Downloader::DownProcessCallBack, this, std::placeholders::_1, std::placeholders::_2));

	bool bRes = m_pDownCore->Down();
	
	if (bRes)
	{
		if (!m_Info.m_pOutStream)
		{
			// ����ʱ�ļ�ת��Ϊ�����ļ�
			MoveFile(m_sTempPath.c_str(), m_Info.m_sSvePath.c_str());
		}
		
		m_DwonloadData.m_bResult = true;
	}
	else
	{
		if (!m_Info.m_pOutStream)
		{
			// ���������ֹ��ɾ����ʱ�ļ�,��Ҫ�������ط�ɾ��
			DeleteFile(m_sTempPath.c_str());
		}
		
		m_DwonloadData.m_bResult = false;
	}

	m_pOutStream->flush();

	if (!m_Info.m_pOutStream)
	{
		delete m_pOutStream;
		m_pOutStream = 0;
	}

	m_DwonloadData.m_bIsEnd = true;
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

