#include "NetBaseHead.h"
#include "DownloadHelper.h"
#include "thread/BaseThread.h"

void * DownloadHelper::Create(const RequestInfo& info)
{
	Downloader* pDownloader = new Downloader(info);
	GET_DOWNLOADERPOOL_INS().Insert(pDownloader);
	return pDownloader;
}

void DownloadHelper::Execute(void * handle)
{
	if (handle)
	{
		((Downloader*)handle)->_Execute();
	}
}

void DownloadHelper::Clean(void * handle)
{
	if (handle)
	{
		GET_DOWNLOADERPOOL_INS().Remove(handle);
	}
}

void * DownloadHelper::DownLoadWithMD5Check(const RequestInfo& info)
{
	void * handle = Create(info);
	RUNINTCALLBACKHREAD(std::bind(&DownloadHelper::Execute, handle));
	return handle;
}

void * DownloadHelper::DownLoadWithFileNameCheck(const RequestInfo& info)
{
	void * handle = Create(info);
	RUNINTCALLBACKHREAD(std::bind(&DownloadHelper::Execute, handle));
	return handle;
}

bool DownloadHelper::DownDataPeek_Core(DOWNLOADER_TYPE nType, std::function<void(DOWNINFOSTATE nState, unsigned int)> callback)
{
	bool bEnd = true;
	GET_DOWNLOADERPOOL_INS().DoEach(nType, [&](void * handle) {
		bEnd &= DownDataPeek_Core(handle, callback);
		return true;
	});

	return bEnd;
}

bool DownloadHelper::DownDataPeek_Core(void * handle, std::function<void(DOWNINFOSTATE, unsigned int)> callback)
{
	DownloadData * pData = GET_DOWNLOADERPOOL_INS().GetData(handle);
	const RequestInfo * pInfo = GET_DOWNLOADERPOOL_INS().GetInfo(handle);

	// 下载完成
	if (pData->m_bIsEnd)
	{
		// 下载失败
		if (!pData->m_bResult)
		{
			if (callback)
			{
				callback(DOWNINFOSTATE_FAILURE, pInfo->m_userdata);
			}
		}
		else
		{
			if (callback)
			{
				callback(DOWNINFOSTATE_SUCCESS, pInfo->m_userdata);
			}
		}

		return true;
	}

	// 还没开始
	if (!pData->m_bIsStart)
	{
		return false;
	}

	// 文件大小
	if (!pData->m_bAllSizeHandle)
	{
		pData->m_bAllSizeHandle = true;

		if (callback)
		{
			callback(DOWNINFOSTATE_ALLSIZE, pInfo->m_userdata);
		}
	}

	// 已经下载
	if (callback)
	{
		callback(DOWNINFOSTATE_CURSIZE, pInfo->m_userdata);
	}

	return false;
}
