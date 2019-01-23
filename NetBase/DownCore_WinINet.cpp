#include "NetBaseHead.h"
#include "NetBaseHead.h"
#include "DownCore_WinINet.h"
#include "log/LogDoggy.h"

#pragma comment(lib, "WinINet.lib")

StaticIntHandleObj DownCore_WinINet::s_hInternet;
StaticIntHandleObj DownCore_WinINet::s_hSessison;

DownCore_WinINet::DownCore_WinINet()
{
}

DownCore_WinINet::~DownCore_WinINet()
{
}

bool DownCore_WinINet::Down()
{
	bool bRes = true;
	HINTERNET hRequest  = NULL;

	try
	{
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

		// 4、打开一个请求
		hRequest = HttpOpenRequestA(s_hSessison
			, m_pInfo->m_nRequestType == REQUESET_TYPE_GET ? "GET" : "POST"
			, m_urlComponents.lpszUrlPath
			, NULL
			, "" , NULL, 0, 0);

		if (hRequest == NULL)
		{
			char sTmp[MAX_PATH] ={ 0 };
			sprintf_s(sTmp, "HttpOpenRequestA failed, error code : %d", GetLastError());
			throw std::exception(sTmp);
		}
		
		// 5、设置首部字段
// 		std::string sHeader = m_pInfo->m_sBody;
// 		
// 		if (!HttpAddRequestHeadersA(hRequest, sHeader.data(), sHeader.length(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
// 		{
// 			throw std::exception("HttpAddRequestHeadersA : failed");
// 		}

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
			, m_pInfo->m_sHead.c_str(), DWORD(m_pInfo->m_sHead.size())
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

		WCHAR buf[2048];
		DWORD bufSize = sizeof(buf);
		DWORD bufRead = 0;

		do
		{
			if (!InternetReadFile(hRequest, &buf, bufSize, &bufRead))
			{
				char sTmp[MAX_PATH] ={ 0 };
				sprintf_s(sTmp, "InternetReadFile failed, error code : %d", GetLastError());
				throw std::exception(sTmp);
			}

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
	else if (m_urlComponents.nScheme != INTERNET_SCHEME_HTTP)
	{
		return false;
	}

	return true;
}
