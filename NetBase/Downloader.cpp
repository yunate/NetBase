#include "NetBaseHead.h"

#include "Downloader.h"
#include "DownloaderPool.h"
#include "string_convert.h"

#include "DownCore_Curl.h"
#include "DownCore_WinINet.h"

#include <fstream>
#include <functional>
#include <windows.h>

// fwirte缓冲有点奇怪，这里一旦数据大于1048576，强制刷一下
#define BUFFMAXSIZE	1048576	 

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
		m_pOutStream = new ZMFileStream(base::MultibytesToUnicode(m_sTempPath.c_str(), 0), fmOpenReadWrite);
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
	double dtotal,							// 下载使用（总量）	
	double dnow								// 下载使用（已下载）
)
{
	OnDownLoading(dtotal, dnow);
	return true;
}

void Downloader::_Execute()
{
	if (!m_pOutStream)
	{
		return;
	}

	//m_pDownCore = new DownCore_Curl();
	m_pDownCore = new DownCore_WinINet();
	m_pDownCore->Construct(&m_Info
		, m_pOutStream
		, std::bind(&Downloader::WriteDownFileCallBack, this, std::placeholders::_1, std::placeholders::_2)
		, std::bind(&Downloader::DownProcessCallBack, this, std::placeholders::_1, std::placeholders::_2));

	bool bRes = m_pDownCore->Down();
	
	if (!m_Info.m_pOutStream)
	{
		delete m_pOutStream;
		m_pOutStream = 0;
	}

	if (bRes)
	{
		if (!m_Info.m_pOutStream)
		{
			// 将临时文件转换为正常文件
			MoveFileA(m_sTempPath.c_str(), m_Info.m_sSvePath.c_str());
		}
		
		m_DwonloadData.m_bResult = true;
	}
	else
	{
		if (!m_Info.m_pOutStream)
		{
			// 如果意外终止，删除临时文件,不要在其他地方删除
			DeleteFileA(m_sTempPath.c_str());
		}
		
		m_DwonloadData.m_bResult = false;
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

