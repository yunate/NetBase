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

// ����c������ʽ�Ļص�����
size_t write_file(void * ptr, size_t size, size_t nmemb, void * obj)
{
	// ���ر���д��Ĵ�С
	Callback* pObj = (Callback*)obj;

	if (!pObj)
	{
		return 0;
	}

	return pObj->m_OutStream->Write((char*)ptr, size * nmemb);
}

size_t progress(void *clientp,				// CURLOPT_PROGRESSDATA ��������
	double dtotal,							// ����ʹ�ã�������	
	double dnow,							// ����ʹ�ã������أ�
	double ultotal,							// �ϴ�ʹ�ã�������
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

	// ����1��ʾֹͣ��0��ʾ�ɹ�
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

		// ����
		CURL *curl = 0;
		curl = curl_easy_init();

		if (!curl)
		{
			throw std::exception("curl init error");
		}

		struct curl_slist* curl_headers = NULL;

		// ��������ͷ��
		std::string sHeader = base::UnicodeToMultibytes(FormateHeader(headers), 0);

		if (strlen(sHeader.c_str()) > 0)
		{
			curl_headers = curl_slist_append(curl_headers, sHeader.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
		}

		//�趨Ϊ��֤֤���HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		// �����ض���
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		/* abort if slower than 30 bytes/sec during TIMEOUT seconds */
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

		std::string sUrl = base::UnicodeToMultibytes(url.c_str(), 0);
		curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

		// ������
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &callback);

		// ������õ�URL�����ڵĻ�������������404���󣬵��ǳ����ֲ��˴��󣬻��ǻ��������404ҳ��
		// CURLOPT_FAILONERROR���ԣ���HTTP����ֵ���ڵ���400��ʱ������ʧ��
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

		// ����
		CURL *curl = 0;
		curl = curl_easy_init();

		if (!curl)
		{
			throw std::exception("curl init error");
		}

		struct curl_slist* curl_headers = NULL;
		
		//����Ϊ��0��ʾ���β���ΪPOST
		curl_easy_setopt(curl, CURLOPT_POST, 1);

		// ��������ͷ��
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

			// ����ҪPOST��JSON����
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pBody);

			//�����ϴ�json������,������ÿ��Ժ���
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(pBody));
		}

		//�趨Ϊ��֤֤���HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		// �����ض���
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		/* abort if slower than 30 bytes/sec during TIMEOUT seconds */
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

		curl_easy_setopt(curl, CURLOPT_URL, base::UnicodeToMultibytes(url.c_str(), 0).c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

		// ������
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &callback);

		// ������õ�URL�����ڵĻ�������������404���󣬵��ǳ����ֲ��˴��󣬻��ǻ��������404ҳ��
		// CURLOPT_FAILONERROR���ԣ���HTTP����ֵ���ڵ���400��ʱ������ʧ��
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