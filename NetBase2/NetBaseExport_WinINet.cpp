#include "NetBaseHead.h"
#ifdef USELIB_WININET
#define _CRT_SECURE_NO_WARNINGS
#include "NetBaseExport.h"
#include "zmbase/string_convert.h"

#include <WinINet.h>


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

class StaticIntHandleObj
{
private:
	StaticIntHandleObj(const StaticIntHandleObj&);

public:
	StaticIntHandleObj()
	{
		m_handle = 0;
		m_sDes = "";
	}

	~StaticIntHandleObj()
	{
		if (m_handle)
		{
			InternetCloseHandle(m_handle);
		}
	}

	operator HINTERNET()
	{
		return m_handle;
	}

	void operator=(const HINTERNET handle)
	{
		if (m_handle)
		{
			InternetCloseHandle(m_handle);
		}

		m_handle = handle;
	}

	void SetDes(std::string sDes)
	{
		m_sDes = sDes;
	}

	std::string & GetDes()
	{
		return m_sDes;
	}

public:
	HINTERNET	m_handle;
	std::string m_sDes;
};

static StaticIntHandleObj	s_hInternet;
static StaticIntHandleObj	s_hSessison;

bool HttpGet(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, const ProgressEvent progress /*= 0 */
	, const ResponseResult responseResult /*= 0*/)
{
	bool bRes = true;
	HINTERNET hRequest  = NULL;

	std::string sRes = "";

	try
	{
		if (!responseStream)
		{
			std::string sError = "responseStream null";
			throw std::exception(sError.c_str());
		}

		// 1、解析URL
		URL_COMPONENTSA urlComponents;
		CHAR						m_pHostName[INTERNET_MAX_HOST_NAME_LENGTH]    ={ 0 };
		CHAR						m_pUserName[INTERNET_MAX_USER_NAME_LENGTH]    ={ 0 };
		CHAR						m_pPassword[INTERNET_MAX_PASSWORD_LENGTH]     ={ 0 };
		CHAR						m_pURLPath[INTERNET_MAX_URL_LENGTH]           ={ 0 };
		CHAR						m_pScheme[INTERNET_MAX_SCHEME_LENGTH]         ={ 0 };

		ZeroMemory(&urlComponents, sizeof(urlComponents));
		urlComponents.dwStructSize      = sizeof(URL_COMPONENTSA);
		urlComponents.lpszScheme        = m_pScheme;
		urlComponents.dwSchemeLength    = INTERNET_MAX_SCHEME_LENGTH;
		urlComponents.lpszHostName      = m_pHostName;
		urlComponents.dwHostNameLength  = INTERNET_MAX_HOST_NAME_LENGTH;
		urlComponents.lpszUserName      = m_pUserName;
		urlComponents.dwUserNameLength  = INTERNET_MAX_USER_NAME_LENGTH;
		urlComponents.lpszPassword      = m_pPassword;
		urlComponents.dwPasswordLength  = INTERNET_MAX_PASSWORD_LENGTH;
		urlComponents.lpszUrlPath       = m_pURLPath;
		urlComponents.dwUrlPathLength   = INTERNET_MAX_URL_LENGTH;

		bool bCrackUrl = TRUE == InternetCrackUrlA(base::UnicodeToMultibytes(url.c_str(), 0).c_str(), 0, NULL, &urlComponents);

		if (bCrackUrl)
		{
			if (urlComponents.nScheme != INTERNET_SCHEME_HTTP && urlComponents.nScheme != INTERNET_SCHEME_HTTPS)
			{
				bCrackUrl = false;;
			}
		}

		if (!bCrackUrl)
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "InternetCrackUrlA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		// 2、打开网络 
		if (!s_hInternet)
		{
			s_hInternet = InternetOpenA("ZMNetBase", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

			if (!s_hInternet)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetOpenA failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}
		}

		// 3、链接网络（打开session） 
		char sTmp[10] ={ 0 };
		std::string sSecssionDes = urlComponents.lpszHostName;
		sSecssionDes  = sSecssionDes + ":" + _itoa(urlComponents.nPort, sTmp, 10);

		if (!s_hSessison || 0 != s_hSessison.GetDes().compare(sSecssionDes))
		{
			s_hSessison  = InternetConnectA(s_hInternet
				, urlComponents.lpszHostName
				, urlComponents.nPort
				, urlComponents.lpszUserName
				, urlComponents.lpszPassword
				, INTERNET_SERVICE_HTTP, 0, NULL);

			if (s_hSessison == NULL)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetConnectA failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

			s_hSessison.SetDes(sSecssionDes);
		}

		DWORD tmpFlag = (INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE);
		DWORD flags = tmpFlag | INTERNET_FLAG_NO_CACHE_WRITE;

		if (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
		{
			flags |= (SECURITY_IGNORE_ERROR_MASK |
				SECURITY_INTERNET_MASK | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
				INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
				INTERNET_FLAG_RELOAD | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);		}

		// 4、打开一个请求
		hRequest = HttpOpenRequestA(s_hSessison
			, "GET"
			, urlComponents.lpszUrlPath
			, NULL
			, "", NULL, flags, 0);

		if (hRequest == NULL)
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpOpenRequestA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		if (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
		{
			// 忽略证书错误
			DWORD flags = 0; DWORD len = sizeof(flags);
			InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, &len);
			flags |= (SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WRONG_USAGE);
			InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));
		}

		// 5、设置首部字段
		std::string sHeader = base::UnicodeToMultibytes(FormateHeader(headers), 0);

		if (strlen(sHeader.c_str()) > 0)
		{
			if (!HttpAddRequestHeadersA(hRequest, sHeader.c_str(), (DWORD)strlen(sHeader.c_str()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
			{
				throw std::exception("HttpAddRequestHeadersA : failed");			}
		}

		// 6、发送请求
		if (!HttpSendRequestA(hRequest
			, 0, 0
			, 0, 0))
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpSendRequestA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		// 等待...
		DWORD dwStatusCode;
		DWORD dwSizeDW = sizeof(DWORD);
		if (!HttpQueryInfoA(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwSizeDW, NULL))
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpQueryInfoA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}
		else
		{
			if (dwStatusCode >= 400)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "HttpQueryInfoA dwStatusCode = %d", dwStatusCode);
				throw std::exception(sTmp);
			}
		}

		DWORD nAllSize;
		if (!HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
			&nAllSize, &dwSizeDW, NULL))
		{
			nAllSize = 0;
		}

		WCHAR buf[2048];
		DWORD bufSize = sizeof(buf);
		DWORD bufRead = 0;
		DWORD nCurrentSize = 0;

		bool bStop = false;

		do
		{
			if (!InternetReadFile(hRequest, &buf, bufSize, &bufRead))
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetReadFile failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

			nCurrentSize += bufRead;

			if (progress)
			{
				progress(id, (INT64)nAllSize, (INT64)nCurrentSize, bStop);
			}

			responseStream->Write((char*)buf, bufRead);
		} while (bufRead != 0 && !bStop);
	}
	catch (const std::exception& e)
	{
		bRes = false;
		sRes = e.what();
	}

	if (hRequest != NULL)
	{
		InternetCloseHandle(hRequest);
	}

	if (responseResult)
	{
		responseResult(id, bRes ? NETBASE_DOWN_STATUS_SUCCESS : NETBASE_DOWN_STATUS_FAILURE, base::MultibytesToUnicode(sRes, 0));
	}

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
	HINTERNET hRequest  = NULL;
	std::string sRes = "";

	try
	{
		if (!responseStream)
		{
			std::string sError = "responseStream null";
			throw std::exception(sError.c_str());
		}

		// 1、解析URL
		URL_COMPONENTSA urlComponents;
		CHAR						m_pHostName[INTERNET_MAX_HOST_NAME_LENGTH]    ={ 0 };
		CHAR						m_pUserName[INTERNET_MAX_USER_NAME_LENGTH]    ={ 0 };
		CHAR						m_pPassword[INTERNET_MAX_PASSWORD_LENGTH]     ={ 0 };
		CHAR						m_pURLPath[INTERNET_MAX_URL_LENGTH]           ={ 0 };
		CHAR						m_pScheme[INTERNET_MAX_SCHEME_LENGTH]         ={ 0 };

		ZeroMemory(&urlComponents, sizeof(urlComponents));
		urlComponents.dwStructSize      = sizeof(URL_COMPONENTSA);
		urlComponents.lpszScheme        = m_pScheme;
		urlComponents.dwSchemeLength    = INTERNET_MAX_SCHEME_LENGTH;
		urlComponents.lpszHostName      = m_pHostName;
		urlComponents.dwHostNameLength  = INTERNET_MAX_HOST_NAME_LENGTH;
		urlComponents.lpszUserName      = m_pUserName;
		urlComponents.dwUserNameLength  = INTERNET_MAX_USER_NAME_LENGTH;
		urlComponents.lpszPassword      = m_pPassword;
		urlComponents.dwPasswordLength  = INTERNET_MAX_PASSWORD_LENGTH;
		urlComponents.lpszUrlPath       = m_pURLPath;
		urlComponents.dwUrlPathLength   = INTERNET_MAX_URL_LENGTH;

		bool bCrackUrl = TRUE == InternetCrackUrlA(base::UnicodeToMultibytes(url.c_str(), 0).c_str(), 0, NULL, &urlComponents);

		if (bCrackUrl)
		{
			if (urlComponents.nScheme != INTERNET_SCHEME_HTTP && urlComponents.nScheme != INTERNET_SCHEME_HTTPS)
			{
				bCrackUrl = false;;
			}
		}

		if (!bCrackUrl)
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "InternetCrackUrlA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		// 2、打开网络 
		if (!s_hInternet)
		{
			s_hInternet = InternetOpenA("ZMNetBase", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

			if (!s_hInternet)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetOpenA failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}
		}

		// 3、链接网络（打开session） 
		char sTmp[10] ={ 0 };
		std::string sSecssionDes = urlComponents.lpszHostName;
		sSecssionDes  = sSecssionDes + ":" + _itoa(urlComponents.nPort, sTmp, 10);

		if (!s_hSessison || 0 != s_hSessison.GetDes().compare(sSecssionDes))
		{
			s_hSessison  = InternetConnectA(s_hInternet
				, urlComponents.lpszHostName
				, urlComponents.nPort
				, urlComponents.lpszUserName
				, urlComponents.lpszPassword
				, INTERNET_SERVICE_HTTP, 0, NULL);

			if (s_hSessison == NULL)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetConnectA failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

			s_hSessison.SetDes(sSecssionDes);
		}

		DWORD tmpFlag = INTERNET_FLAG_KEEP_CONNECTION;
		DWORD flags = tmpFlag | INTERNET_FLAG_NO_CACHE_WRITE;

		if (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
		{
			flags |= (SECURITY_IGNORE_ERROR_MASK |
				SECURITY_INTERNET_MASK | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
				INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
				INTERNET_FLAG_RELOAD | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);		}

		// 4、打开一个请求
		hRequest = HttpOpenRequestA(s_hSessison
			, "POST"
			, urlComponents.lpszUrlPath
			, NULL
			, "", NULL, flags, 0);

		if (hRequest == NULL)
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpOpenRequestA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		if (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
		{
			// 忽略证书错误
			DWORD flags = 0; DWORD len = sizeof(flags);
			InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, &len);
			flags |= (SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WRONG_USAGE);
			InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));
		}

		// 5、设置首部字段
		std::string sHeader = base::UnicodeToMultibytes(FormateHeader(headers), 0);

		if (strlen(sHeader.c_str()) > 0)
		{
			if (!HttpAddRequestHeadersA(hRequest, sHeader.c_str(), (DWORD)strlen(sHeader.c_str()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
			{
				throw std::exception("HttpAddRequestHeadersA : failed");			}
		}

		__int64 nSize = 0;
		char * pBuff = 0;

		if (requestStream)
		{
			nSize =requestStream->GetSize();
			pBuff = new char[(unsigned int)nSize + 1];
			requestStream->Seek(0, FILE_BEGIN);
			requestStream->Read(pBuff, (DWORD)nSize);
			pBuff[nSize] = 0;
		}

		// 6、发送请求
		if (!HttpSendRequestA(hRequest
			, 0, 0
			, (void*)pBuff, strlen(pBuff)))
		{
			if (pBuff)
			{
				delete[] pBuff;
			}

			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpSendRequestA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		if (pBuff)
		{
			delete[] pBuff;
		}

		// 等待...
		DWORD dwStatusCode;
		DWORD dwSizeDW = sizeof(DWORD);
		if (!HttpQueryInfoA(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwSizeDW, NULL))
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpQueryInfoA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}
		else
		{
			if (dwStatusCode >= 400)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "HttpQueryInfoA dwStatusCode = %d", dwStatusCode);
				throw std::exception(sTmp);
			}
		}

		DWORD nAllSize;
		if (!HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
			&nAllSize, &dwSizeDW, NULL))
		{
			nAllSize = 0;
		}

		WCHAR buf[2048];
		DWORD bufSize = sizeof(buf);
		DWORD bufRead = 0;
		DWORD nCurrentSize = 0;

		bool bStop = false;

		do
		{
			if (!InternetReadFile(hRequest, &buf, bufSize, &bufRead))
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetReadFile failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

			nCurrentSize += bufRead;

			if (progress)
			{
				progress(id, (INT64)nAllSize, (INT64)nCurrentSize, bStop);
			}

			responseStream->Write((char*)buf, bufRead);
		} while (bufRead != 0 && !bStop);
	}
	catch (const std::exception& e)
	{
		bRes = false;
		sRes = e.what();
	}

	if (hRequest != NULL)
	{
		InternetCloseHandle(hRequest);
	}

	if (responseResult)
	{
		responseResult(id, bRes ? NETBASE_DOWN_STATUS_SUCCESS : NETBASE_DOWN_STATUS_FAILURE, base::MultibytesToUnicode(sRes, 0));
	}

	return bRes;
}
#endif