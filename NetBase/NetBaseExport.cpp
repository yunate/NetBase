#include "NetBaseExport.h"
#include "DownLoadDef.h"
#include "DownloadHelper.h"
#include "thread/BaseThread.h"
#include "Streams.h"
#include "string_convert.h"
#include <sstream>

ZMString FormateHeader(const HttpHeader & headers)
{
	ZMString sRes = L"";

	for (auto it : headers)
	{
		sRes += it.first;
		sRes += L":";
		sRes += it.second;
		sRes += L"\r\n";
	}

	sRes = sRes.substr(0, sRes.size() - 2);
	return sRes;
}

bool HttpGet(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, const ProgressEvent progress /*= 0 */
	, const ResponseResult responseResult /*= 0*/)
{
	bool bRes = true;
	RequestInfo info;
	info.m_sUrl = base::UnicodeToMultibytes(url, 0);
	info.m_nId = id;
	info.m_sHead = base::UnicodeToMultibytes(FormateHeader(headers), 0);
	info.m_pOutStream = responseStream;
	info.m_ProcessEvent = progress;
	info.m_ResponseResEvent = responseResult;
	void * handle = DownloadHelper::Create(info);
	DownloadHelper::Execute(handle);

	DownloadHelper::DownDataPeek_Core(handle, [&](DOWNINFOSTATE nType, unsigned int)
	{
		if (nType == DOWNINFOSTATE_FAILURE)
		{
			bRes = false;
		}
	});

	DownloadHelper::Clean(handle);

	return bRes;
}

bool HttpPost(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, ZMStream* requestStream
	, const ProgressEvent progress
	, const ResponseResult responseResult)
{
	bool bRes = true;
	RequestInfo info;

	//  post «Î«Û æ¡–
	info.m_nRequestType = REQUESET_TYPE_POST;
	info.m_sUrl = base::UnicodeToMultibytes(url, 0);
	info.m_nId = id;
	info.m_sHead = base::UnicodeToMultibytes(FormateHeader(headers), 0);
	//info.m_sHead = "Content-Type:application/json;charset=UTF-8";
	info.m_pBodyStream = requestStream;
	info.m_pOutStream = responseStream;
	info.m_ProcessEvent = progress;
	info.m_ResponseResEvent = responseResult;
	void * handle = DownloadHelper::Create(info);
	DownloadHelper::Execute(handle);

	DownloadHelper::DownDataPeek_Core(handle, [&](DOWNINFOSTATE nType, unsigned int)
	{
		if (nType == DOWNINFOSTATE_FAILURE)
		{
			bRes = false;
		}
	});

	DownloadHelper::Clean(handle);
	return bRes;
}
