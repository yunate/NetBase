#include "NetBaseHead.h"
#ifdef USELIB_WININET
#define  _CRT_SECURE_NO_WARNINGS
#include "DownCore_WinINet.h"
#include "log/LogDoggy.h"


StaticIntHandleObj DownCore_WinINet::s_hInternet;
StaticIntHandleObj DownCore_WinINet::s_hSessison;

DownCore_WinINet::DownCore_WinINet()
{
}

DownCore_WinINet::~DownCore_WinINet()
{
}

bool DownCore_WinINet::Down(NETBASE_DOWN_STATUS & nCode, std::string & sDes)
{
	bool bRes = true;
	HINTERNET hRequest  = NULL;

	try
	{
		if (!m_pOutStream)
		{
			std::string sError = "m_pOutStream null";
			throw std::exception(sError.c_str());
		}

		// 1、解析URL
		if (!CrackUrl())
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
		std::string sSecssionDes = m_urlComponents.lpszHostName;
		sSecssionDes  = sSecssionDes + ":" + _itoa(m_urlComponents.nPort, sTmp, 10);

		if (!s_hSessison || 0 != s_hSessison.GetDes().compare(sSecssionDes))
		{
			s_hSessison  = InternetConnectA(s_hInternet
				, m_urlComponents.lpszHostName
				, m_urlComponents.nPort
				, m_urlComponents.lpszUserName
				, m_urlComponents.lpszPassword
				, INTERNET_SERVICE_HTTP, 0, NULL);

			if (s_hSessison == NULL)
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetConnectA failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

			s_hSessison.SetDes(sSecssionDes);
		}

		DWORD tmpFlag = m_pInfo->m_nRequestType == REQUESET_TYPE_POST ? INTERNET_FLAG_KEEP_CONNECTION :
			(INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE);
		DWORD flags = tmpFlag | INTERNET_FLAG_NO_CACHE_WRITE;

		if (m_urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
		{
			flags |= (SECURITY_IGNORE_ERROR_MASK |
				SECURITY_INTERNET_MASK | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
				INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
				INTERNET_FLAG_RELOAD | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);		}

		// 4、打开一个请求
		hRequest = HttpOpenRequestA(s_hSessison
			, m_pInfo->m_nRequestType == REQUESET_TYPE_GET ? "GET" : "POST"
			, m_urlComponents.lpszUrlPath
			, NULL
			, "", NULL, flags, 0);

		if (hRequest == NULL)
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpOpenRequestA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}

		if (m_urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
		{
			// 忽略证书错误
			DWORD flags = 0; DWORD len = sizeof(flags);
			InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, &len);
			flags |= (SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WRONG_USAGE);
			InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));
		}

		// 5、设置首部字段
		std::string sHeader = m_pInfo->m_sHead;
		
		if (strlen(sHeader.c_str()) > 0)
		{
			if (!HttpAddRequestHeadersA(hRequest, sHeader.c_str(), (DWORD)strlen(sHeader.c_str()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
			{
				throw std::exception("HttpAddRequestHeadersA : failed");			}
		}
		
		__int64 nSize = 0;
		char * pBuff = 0;

		if (m_pInfo->m_pBodyStream)
		{
			nSize = m_pInfo->m_pBodyStream->GetSize();
			pBuff = new char[(unsigned int)nSize];
			m_pInfo->m_pBodyStream->Seek(0, FILE_BEGIN);
			m_pInfo->m_pBodyStream->Read(pBuff, (DWORD)nSize);
		}

		// 6、发送请求
		if (!HttpSendRequestA(hRequest
			, 0, 0
			, (void*)pBuff, DWORD(nSize)))
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

		do
		{
			if (!InternetReadFile(hRequest, &buf, bufSize, &bufRead))
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetReadFile failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

			nCurrentSize += bufRead;
			Progress((double)nAllSize, (double)nCurrentSize);
			Write(buf, bufRead);

			while (m_bPause)
			{
				// 暂停
				Sleep(10);
			}
		} while (bufRead != 0 && !m_bStop);
	}
	catch (const std::exception& e)
	{
		GetDoggy().Bark_Error_Log(e.what());
		bRes = false;
		sDes = e.what();
	}

	if (hRequest != NULL)
	{
		InternetCloseHandle(hRequest);
	}

	return bRes;
}

bool DownCore_WinINet::CrackUrl()
{
	ZeroMemory(&m_urlComponents, sizeof(m_urlComponents));
	m_urlComponents.dwStructSize      = sizeof(URL_COMPONENTSA);
	m_urlComponents.lpszScheme        = m_pScheme;
	m_urlComponents.dwSchemeLength    = INTERNET_MAX_SCHEME_LENGTH;
	m_urlComponents.lpszHostName      = m_pHostName;
	m_urlComponents.dwHostNameLength  = INTERNET_MAX_HOST_NAME_LENGTH;
	m_urlComponents.lpszUserName      = m_pUserName;
	m_urlComponents.dwUserNameLength  = INTERNET_MAX_USER_NAME_LENGTH;
	m_urlComponents.lpszPassword      = m_pPassword;
	m_urlComponents.dwPasswordLength  = INTERNET_MAX_PASSWORD_LENGTH;
	m_urlComponents.lpszUrlPath       = m_pURLPath;
	m_urlComponents.dwUrlPathLength   = INTERNET_MAX_URL_LENGTH;

	BOOL bSuccess = InternetCrackUrlA(m_pInfo->m_sUrl.c_str(), 0, NULL, &m_urlComponents);

	if (bSuccess == FALSE)
	{
		return false;
	}
	else if (m_urlComponents.nScheme != INTERNET_SCHEME_HTTP && m_urlComponents.nScheme != INTERNET_SCHEME_HTTPS)
	{
		return false;
	}

	return true;
}
#endif