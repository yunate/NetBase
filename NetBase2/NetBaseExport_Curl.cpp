#include "NetBaseHead.h"
#ifdef USELIB_CURL
#include "NetBaseExport.h"
#include "curl/curl.h"
#include "zmbase/string_convert.h"

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

struct Callback
{
	Callback(int nId, ProgressEvent progress, ResponseResult result, ZMStream * outStream)
		: m_nId(nId)
		, m_ProgressCallBack(progress)
		, m_ResponseCallBack(result)
		, m_OutStream(outStream)
	{

	}

	int m_nId;
	ProgressEvent m_ProgressCallBack;
	ResponseResult m_ResponseCallBack;
	ZMStream* m_OutStream;
};

// 两个c语言形式的回掉函数
size_t write_file(void * ptr, size_t size, size_t nmemb, void * obj)
{
	// 返回本次写入的大小
	Callback* pObj = (Callback*)obj;

	if (!pObj)
	{
		return 0;
	}

	return pObj->m_OutStream->Write((char*)ptr, size * nmemb);
}

size_t progress(void *clientp,				// CURLOPT_PROGRESSDATA 传进来的
	double dtotal,							// 下载使用（总量）	
	double dnow,							// 下载使用（已下载）
	double ultotal,							// 上传使用（总量）
	double ulnow)
{
	Callback* pObj = (Callback*)clientp;

	if (!pObj)
	{
		return 0;
	}

	bool bStop = false;

	if (pObj->m_ProgressCallBack)
	{
		pObj->m_ProgressCallBack(pObj->m_nId, (INT64)dtotal, (INT64)dnow, bStop);
	}

	// 返回1表示停止，0表示成功
	if (bStop)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool HttpGet(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, const ProgressEvent progressEvt /*= 0 */
	, const ResponseResult responseResult /*= 0*/)
{
	Callback callback(id, progressEvt, responseResult, responseStream);

	try
	{
		if (!responseStream)
		{
			std::string sError = "responseStream null";
			throw std::exception(sError.c_str());
		}

		// 下载
		CURL *curl = 0;
		curl = curl_easy_init();

		if (!curl)
		{
			throw std::exception("curl init error");
		}

		struct curl_slist* curl_headers = NULL;

		// 设置请求头部
		std::string sHeader = base::UnicodeToMultibytes(FormateHeader(headers), 0);

		if (strlen(sHeader.c_str()) > 0)
		{
			curl_headers = curl_slist_append(curl_headers, sHeader.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
		}

		//设定为验证证书和HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		// 运行重定向
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		/* abort if slower than 30 bytes/sec during TIMEOUT seconds */
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

		std::string sUrl = base::UnicodeToMultibytes(url.c_str(), 0);
		curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

		// 进度条
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &callback);

		// 如果设置的URL不存在的话，服务器返回404错误，但是程序发现不了错误，还是会下载这个404页面
		// CURLOPT_FAILONERROR属性，当HTTP返回值大于等于400的时候，请求失败
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		CURLcode res = curl_easy_perform(curl);

		if (curl_headers)
		{
			curl_slist_free_all(curl_headers);
		}

		curl_easy_cleanup(curl);

		if (CURLE_OK != res)
		{
			std::string sError = "download failure, url:";
			sError += base::UnicodeToMultibytes(url.c_str(), 0);
			throw std::exception(sError.c_str());
		}
	}
	catch (const std::exception& e)
	{
		if (callback.m_ResponseCallBack)
		{
			callback.m_ResponseCallBack(callback.m_nId, NETBASE_DOWN_STATUS_FAILURE, base::MultibytesToUnicode(e.what(), 0));
		}
		return false;
	}

	if (callback.m_ResponseCallBack)
	{
		callback.m_ResponseCallBack(callback.m_nId, NETBASE_DOWN_STATUS_SUCCESS, L"");
	}
	return true;
}

bool HttpPost(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, ZMStream* requestStream
	, const ProgressEvent progressEvt
	, const ResponseResult responseResult)
{
	Callback callback(id, progressEvt, responseResult, responseStream);

	try
	{
		if (!responseStream)
		{
			std::string sError = "responseStream null";
			throw std::exception(sError.c_str());
		}

		// 下载
		CURL *curl = 0;
		curl = curl_easy_init();

		if (!curl)
		{
			throw std::exception("curl init error");
		}

		struct curl_slist* curl_headers = NULL;
		
		//设置为非0表示本次操作为POST
		curl_easy_setopt(curl, CURLOPT_POST, 1);

		// 设置请求头部
		std::string sHeader = base::UnicodeToMultibytes(FormateHeader(headers), 0);
		if (strlen(sHeader.c_str()) > 0)
		{
			curl_headers = curl_slist_append(curl_headers, sHeader.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
		}

		char * pBody = 0;

		if (requestStream)
		{
			INT64 nSize = requestStream->GetSize();
			pBody = new char[(unsigned int)nSize + 1];
			requestStream->Seek(0, FILE_BEGIN);
			requestStream->Read(pBody, (DWORD)nSize);
			pBody[nSize] = 0;

			// 设置要POST的JSON数据
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pBody);

			//设置上传json串长度,这个设置可以忽略
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(pBody));
		}

		//设定为验证证书和HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		// 运行重定向
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		/* abort if slower than 30 bytes/sec during TIMEOUT seconds */
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

		curl_easy_setopt(curl, CURLOPT_URL, base::UnicodeToMultibytes(url.c_str(), 0).c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

		// 进度条
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &callback);

		// 如果设置的URL不存在的话，服务器返回404错误，但是程序发现不了错误，还是会下载这个404页面
		// CURLOPT_FAILONERROR属性，当HTTP返回值大于等于400的时候，请求失败
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		CURLcode res = curl_easy_perform(curl);

		if (curl_headers)
		{
			curl_slist_free_all(curl_headers);
		}

		if (pBody)
		{
			delete[]pBody;
		}

		curl_easy_cleanup(curl);

		if (CURLE_OK != res)
		{
			std::string sError = "download failure, url:";
			sError += base::UnicodeToMultibytes(url.c_str(), 0);
			throw std::exception(sError.c_str());
		}
	}
	catch (const std::exception& e)
	{
		if (callback.m_ResponseCallBack)
		{
			callback.m_ResponseCallBack(callback.m_nId, NETBASE_DOWN_STATUS_FAILURE, base::MultibytesToUnicode(e.what(), 0));
		}
		return false;
	}

	if (callback.m_ResponseCallBack)
	{
		callback.m_ResponseCallBack(callback.m_nId, NETBASE_DOWN_STATUS_SUCCESS, L"");
	}
	return true;
}
#endif