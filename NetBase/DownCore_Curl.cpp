#include "NetBaseHead.h"

#include "DownCore_Curl.h"
#include "curl/curl.h"
#include "log/LogDoggy.h"


#pragma comment(lib, "WS2_32")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "libcurl.lib")

DownCore_Curl::DownCore_Curl()
{
}

DownCore_Curl::~DownCore_Curl()
{
}

// 两个c语言形式的回掉函数
size_t write_file(void * ptr, size_t size, size_t nmemb, void * obj)
{
	// 返回本次写入的大小
	DownCore_Curl* pObj = (DownCore_Curl*)obj;

	if (!pObj || pObj->GetStop())
	{
		return 0;
	}

	if (pObj->Write(ptr, size * nmemb))
	{
		return size * nmemb;
	}

	return 0;
}

size_t progress(void *clientp,				// CURLOPT_PROGRESSDATA 传进来的
	double dtotal,							// 下载使用（总量）	
	double dnow,							// 下载使用（已下载）
	double ultotal,							// 上传使用（总量）
	double ulnow)
{
	// 返回1表示停止，0表示成功
	DownCore_Curl* pObj = (DownCore_Curl*)clientp;

	if (!pObj)
	{
		return 1;
	}

	if (pObj->Progress(dtotal, dnow))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool DownCore_Curl::Down()
{
	try
	{
		if (!m_pOutStream)
		{
			std::string sError = "m_pOutStream null";
			throw std::exception(sError.c_str());
		}

		// 下载
		CURL *curl = 0;
		curl = curl_easy_init();

		if (!curl)
		{
			throw std::exception("curl init error");
		}
		
		struct curl_slist* headers = NULL;

		if (m_pInfo->m_nRequestType == REQUESET_TYPE_POST)
		{
			//设置为非0表示本次操作为POST
			curl_easy_setopt(curl, CURLOPT_POST, 1);

			// 设置请求头部
			headers = curl_slist_append(headers, m_pInfo->m_sHead.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			if (m_pInfo->m_pBodyStream)
			{
				INT64 nSize = m_pInfo->m_pBodyStream->GetSize();
				char * pBuff = new char[(unsigned int)nSize];
				m_pInfo->m_pBodyStream->Seek(0, FILE_BEGIN);
				m_pInfo->m_pBodyStream->Read(pBuff, (DWORD)nSize);

				// 设置要POST的JSON数据
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pBuff);

				//设置上传json串长度,这个设置可以忽略
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, nSize);
				delete[] pBuff;
			}
		}

		// 运行重定向
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		/* abort if slower than 30 bytes/sec during TIMEOUT seconds */
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, TIMEOUT);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

		curl_easy_setopt(curl, CURLOPT_URL, m_pInfo->m_sUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

		// 进度条
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);

		// 如果设置的URL不存在的话，服务器返回404错误，但是程序发现不了错误，还是会下载这个404页面
		// CURLOPT_FAILONERROR属性，当HTTP返回值大于等于400的时候，请求失败
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		CURLcode res = curl_easy_perform(curl);

		if (headers)
		{
			curl_slist_free_all(headers);
		}

		curl_easy_cleanup(curl);

		if (!res)
		{
			std::string sError = "download failure, url:";
			sError += m_pInfo->m_sUrl;
			throw std::exception(sError.c_str());
		}
	}
	catch (const std::exception& e)
	{
		GetDoggy().Bark_Error_Log(e.what());
		return false;
	}
	
	return true;
}
