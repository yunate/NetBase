#include	"NetBaseHead.h"
#ifdef		USELIB_WININET
#define		_CRT_SECURE_NO_WARNINGS

#include	"NetBaseExport.h"

#include	<WinINet.h>
#include	<thread>
#include	<time.h>

#pragma comment(lib, "WinINet.lib")

ZMString FormateHeader(const HttpHeader & headers)
{
	if (headers.size() == 0)
	{
		return L"";
	}

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

NETBASE_DOWN_STATUS HttpGet(RequestInfo & info)
{
	NETBASE_DOWN_STATUS nStatus				= NDS_SUCCESS;
	HINTERNET			hInternet			= NULL;
	HINTERNET			hSession			= NULL;
	HINTERNET			hRequest			= NULL;
	bool				bEnd				= false;

	auto sendrequest = [&]()
	{
		do
		{
			if (!info.m_ResponseStream)
			{
				nStatus = NDS_RESPONSESTREAMINVALID;
				break;
			}

			// 1、解析URL
			WCHAR						pHostName[INTERNET_MAX_HOST_NAME_LENGTH]    ={ 0 };
			WCHAR						pUserName[INTERNET_MAX_USER_NAME_LENGTH]    ={ 0 };
			WCHAR						pPassword[INTERNET_MAX_PASSWORD_LENGTH]     ={ 0 };
			WCHAR						pURLPath[INTERNET_MAX_URL_LENGTH]           ={ 0 };
			WCHAR						pScheme[INTERNET_MAX_SCHEME_LENGTH]         ={ 0 };

			URL_COMPONENTS urlComponents;
			ZeroMemory(&urlComponents, sizeof(urlComponents));
			urlComponents.dwStructSize      = sizeof(URL_COMPONENTS);
			urlComponents.lpszScheme        = pScheme;
			urlComponents.dwSchemeLength    = INTERNET_MAX_SCHEME_LENGTH;
			urlComponents.lpszHostName      = pHostName;
			urlComponents.dwHostNameLength  = INTERNET_MAX_HOST_NAME_LENGTH;
			urlComponents.lpszUserName      = pUserName;
			urlComponents.dwUserNameLength  = INTERNET_MAX_USER_NAME_LENGTH;
			urlComponents.lpszPassword      = pPassword;
			urlComponents.dwPasswordLength  = INTERNET_MAX_PASSWORD_LENGTH;
			urlComponents.lpszUrlPath       = pURLPath;
			urlComponents.dwUrlPathLength   = INTERNET_MAX_URL_LENGTH;

			if (TRUE != InternetCrackUrl(info.m_sUrl.c_str(), 0, NULL, &urlComponents))
			{
				nStatus = NDS_RESPONSESTREAMINVALID;
				break;
			}

			if (urlComponents.nScheme != INTERNET_SCHEME_HTTP && urlComponents.nScheme != INTERNET_SCHEME_HTTPS)
			{
				nStatus = NDS_SCHEMEERROR;
				break;
			}

			// 2、打开网络 
			hInternet = InternetOpen(_T("ZMNetBase"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

			if (!hInternet)
			{
				nStatus = NDS_INTERNETOPENFAILURE;
				break;
			}

			hSession  = InternetConnect(hInternet
				, urlComponents.lpszHostName
				, urlComponents.nPort
				, urlComponents.lpszUserName
				, urlComponents.lpszPassword
				, INTERNET_SERVICE_HTTP, 0, NULL);

			if (!hSession)
			{
				nStatus = NDS_CONNECTFAILURE;
				break;
			}

			DWORD flags = INTERNET_FLAG_RELOAD
				| INTERNET_FLAG_PRAGMA_NOCACHE			// 不缓存
				| INTERNET_FLAG_NO_CACHE_WRITE;			// 不缓存

			if (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
			{
				flags |= (SECURITY_IGNORE_ERROR_MASK
					| SECURITY_INTERNET_MASK
					| INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP
					| INTERNET_FLAG_SECURE
					| INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP
					| INTERNET_FLAG_RELOAD
					| INTERNET_FLAG_IGNORE_CERT_CN_INVALID
					| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
			}

			wchar_t wcAllSize[33] ={ 0 };

			auto openrequest = [&](bool bResume)
			{
				do
				{
					hRequest = HttpOpenRequest(hSession
						, _T("GET")
						, urlComponents.lpszUrlPath
						, NULL
						, _T(""), NULL, flags, 0);

					if (!hRequest)
					{
						nStatus = NDS_REQUESTFAILURE;
						break;
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
					HttpHeader tmpHead = info.m_RequestHeadMap;

					if (bResume && info.m_ResponseStream->GetSize() > 0)
					{
						wchar_t buffer[255];
						ZMString sTmp = _T("bytes=");
						_i64tow_s(info.m_ResponseStream->GetSize(), buffer, _countof(buffer), 10);
						sTmp += buffer;
						sTmp += _T("-");
						sTmp += wcAllSize;
						tmpHead[_T("Range")] = sTmp;
						info.m_ResponseStream->Seek(0, FILE_END);
					}

					ZMString sHeader = FormateHeader(tmpHead);

					if (wcslen(sHeader.c_str()) > 0)
					{
						if (!HttpAddRequestHeaders(hRequest
							, sHeader.c_str()
							, (DWORD)wcslen(sHeader.c_str())
							, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
						{
							nStatus = NDS_ADDREQUESTFAILURE;
							break;
						}
					}

					// 6、发送请求
					if (!HttpSendRequest(hRequest
						, 0, 0
						, 0, 0))
					{
						nStatus = NDS_SENDREQUESRFAILURE;
						break;
					}

					// 等待...
					DWORD dwStatusCode;
					DWORD dwSizeDW = sizeof(DWORD);

					if (!HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwSizeDW, NULL))
					{
						nStatus = NDS_QUERYINFOFAILURE;
						break;
					}
					else
					{
						if (dwStatusCode >= 400)
						{
							nStatus = NDS_RETURNCODEOVER400;
							break;
						}
					}

					DWORD dwAZLen = sizeof(wcAllSize);

					if (!HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH,
						&wcAllSize, &dwAZLen, NULL))
					{
						wcAllSize[0] = 0;
					}
				} while (0);
			};

			openrequest(false);

			if (nStatus != NDS_SUCCESS)
			{
				break;
			}

			if (info.m_bResume && !bEnd)
			{
				InternetCloseHandle(hRequest);
				openrequest(true);
			}

			if (nStatus != NDS_SUCCESS)
			{
				break;
			}

			WCHAR buf[BUFFSIZE];
			DWORD bufSize = sizeof(buf);
			DWORD bufRead = 0;
			INT64 nCurrentSize = 0;
			INT64 nAllDownSize = _wtoi64(wcAllSize);
			bool bStop = false;

			do
			{
				if (!InternetReadFile(hRequest, &buf, bufSize, &bufRead))
				{
					nStatus = NDS_READFILEFAILURE;
					break;
				}

				nCurrentSize += bufRead;

				if (info.m_ProgressCallback)
				{
					info.m_ProgressCallback(info.m_nId, nAllDownSize, nCurrentSize, bStop);
				}

				if (-1 == info.m_ResponseStream->Write((char*)buf, bufRead))
				{
					nStatus = NDS_RESPONSESTREAMINVALID;
					break;
				}

			} while (bufRead != 0 && !bStop);

		} while (0);
		
		bEnd = true;
	};

	auto releaseWininet = [&]()
	{
		if (hRequest != NULL)
		{
			InternetCloseHandle(hRequest);
		}

		if (hSession)
		{
			InternetCloseHandle(hSession);
		}

		if (hInternet)
		{
			InternetCloseHandle(hInternet);
		}
	};

	if (info.m_nTimeOut > 0)
	{
		DWORD startTime = GetTickCount();
		bool bTimeOut = false;
		std::thread t(sendrequest);

		while (!bEnd)
		{
			DWORD nCurTime = GetTickCount();

			if (int(nCurTime - startTime) > info.m_nTimeOut)
			{
				bEnd = true;
				bTimeOut = true;
				break;
			}

			Sleep(10);
		}

		// 这个地方可能需要同步
		releaseWininet();
		t.join();

		if (bTimeOut)
		{
			nStatus = NDS_TIMEOUT;
		}
	}
	else
	{
		sendrequest();
		releaseWininet();
	}

	if (info.m_ResultCallback)
	{
		info.m_ResultCallback(info.m_nId, nStatus);
	}

	return nStatus;
}

NETBASE_DOWN_STATUS HttpPost(RequestInfo & info)
{
	NETBASE_DOWN_STATUS nStatus				= NDS_SUCCESS;
	HINTERNET			hInternet			= NULL;
	HINTERNET			hSession			= NULL;
	HINTERNET			hRequest			= NULL;
	bool				bEnd				= false;

	auto sendrequest = [&]()
	{
		do
		{
			if (!info.m_ResponseStream)
			{
				nStatus = NDS_RESPONSESTREAMINVALID;
				break;
			}

			// 1、解析URL
			WCHAR							pHostName[INTERNET_MAX_HOST_NAME_LENGTH]    ={ 0 };
			WCHAR							pUserName[INTERNET_MAX_USER_NAME_LENGTH]    ={ 0 };
			WCHAR							pPassword[INTERNET_MAX_PASSWORD_LENGTH]     ={ 0 };
			WCHAR							pURLPath[INTERNET_MAX_URL_LENGTH]           ={ 0 };
			WCHAR							pScheme[INTERNET_MAX_SCHEME_LENGTH]         ={ 0 };

			URL_COMPONENTS					urlComponents;
			ZeroMemory(&urlComponents, sizeof(urlComponents));
			urlComponents.dwStructSize      = sizeof(URL_COMPONENTSA);
			urlComponents.lpszScheme        = pScheme;
			urlComponents.dwSchemeLength    = INTERNET_MAX_SCHEME_LENGTH;
			urlComponents.lpszHostName      = pHostName;
			urlComponents.dwHostNameLength  = INTERNET_MAX_HOST_NAME_LENGTH;
			urlComponents.lpszUserName      = pUserName;
			urlComponents.dwUserNameLength  = INTERNET_MAX_USER_NAME_LENGTH;
			urlComponents.lpszPassword      = pPassword;
			urlComponents.dwPasswordLength  = INTERNET_MAX_PASSWORD_LENGTH;
			urlComponents.lpszUrlPath       = pURLPath;
			urlComponents.dwUrlPathLength   = INTERNET_MAX_URL_LENGTH;

			if (TRUE != InternetCrackUrl(info.m_sUrl.c_str(), 0, NULL, &urlComponents))
			{
				nStatus = NDS_RESPONSESTREAMINVALID;
				break;
			}

			if (urlComponents.nScheme != INTERNET_SCHEME_HTTP && urlComponents.nScheme != INTERNET_SCHEME_HTTPS)
			{
				nStatus = NDS_SCHEMEERROR;
				break;
			}

			// 2、打开网络 
			hInternet = InternetOpen(_T("ZMNetBase"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

			if (!hInternet)
			{
				nStatus = NDS_INTERNETOPENFAILURE;
				break;
			}

			// 3、链接网络（打开session） 
			hSession  = InternetConnect(hInternet
				, urlComponents.lpszHostName
				, urlComponents.nPort
				, urlComponents.lpszUserName
				, urlComponents.lpszPassword
				, INTERNET_SERVICE_HTTP, 0, NULL);

			if (!hSession)
			{
				nStatus = NDS_CONNECTFAILURE;
				break;
			}

			DWORD flags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE;

			if (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
			{
				flags	|= (SECURITY_IGNORE_ERROR_MASK
					| SECURITY_INTERNET_MASK | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP
					| INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP
					| INTERNET_FLAG_RELOAD | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
			}

			wchar_t wcAllSize[33] ={ 0 };

			// 4、打开一个请求
			auto openrequest = [&](bool bResume)
			{
				do 
				{
					hRequest = HttpOpenRequest(hSession
						, _T("POST")
						, urlComponents.lpszUrlPath
						, NULL
						, _T(""), NULL, flags, 0);

					if (hRequest == NULL)
					{
						nStatus = NDS_REQUESTFAILURE;
						break;
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
					HttpHeader tmpHead = info.m_RequestHeadMap;

					if (bResume && info.m_ResponseStream->GetSize() > 0)
					{
						wchar_t buffer[255];
						ZMString sTmp = _T("bytes=");
						_i64tow_s(info.m_ResponseStream->GetSize(), buffer, _countof(buffer), 10);
						sTmp += buffer;
						sTmp += _T("-");
						sTmp += wcAllSize;
						tmpHead[_T("Range")] = sTmp;
						info.m_ResponseStream->Seek(0, FILE_END);
					}

					ZMString sHeader = FormateHeader(tmpHead);

					if (wcslen(sHeader.c_str()) > 0)
					{
						if (!HttpAddRequestHeaders(hRequest, sHeader.c_str(), (DWORD)wcslen(sHeader.c_str()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
						{
							nStatus = NDS_ADDREQUESTFAILURE;
							break;
						}
					}

					// 6、发送请求

					if (info.m_RequeseBodyStream)
					{
						int nPerSize = BUFFSIZE;
						INT64 nAllSize = info.m_RequeseBodyStream->GetSize();
						info.m_RequeseBodyStream->Seek(0, FILE_BEGIN);

						// 发送
						INTERNET_BUFFERS BufferIn ={ 0 };
						BufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
						BufferIn.dwBufferTotal = (DWORD)nAllSize;

						if (!HttpSendRequestEx(hRequest
							, &BufferIn, 0
							, 0, 0))
						{
							nStatus = NDS_SENDREQUESRFAILURE;
							break;
						}

						NETBASE_DOWN_STATUS nTmpStatus = NDS_NODEFINE;

						while (nAllSize > 0)
						{
							int nCurSize = nPerSize;

							if (nCurSize > nAllSize)
							{
								nCurSize = (int)nAllSize;
							}

							nAllSize -= nCurSize;

							std::string buff;
							buff.resize(nCurSize);

							if (-1 == info.m_RequeseBodyStream->Read((WCHAR*)(buff.c_str()), (DWORD)nCurSize))
							{
								nTmpStatus = NDS_REQUESTBODYSTREAMINVALID;
								break;
							}

							DWORD dwBytesWritten = -1;

							if (FALSE == InternetWriteFile(hRequest, buff.c_str(), nCurSize, &dwBytesWritten)
								|| dwBytesWritten != nCurSize)
							{
								nTmpStatus = NDS_WRITEFILEFAILURE;
								break;
							}
						}

						if (nTmpStatus != NDS_NODEFINE)
						{
							nStatus = nTmpStatus;
							break;
						}
						else
						{
							if (!HttpEndRequest(hRequest, NULL, 0, 0))
							{
								nStatus = NDS_ENDQUESRFAILURE;
								break;
							}
						}
					}
					else
					{
						if (!HttpSendRequest(hRequest
							, 0, 0
							, 0, 0))
						{
							nStatus = NDS_SENDREQUESRFAILURE;
							break;
						}
					}

					// 等待...
					DWORD dwStatusCode;
					DWORD dwSizeDW = sizeof(DWORD);

					if (!HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwSizeDW, NULL))
					{
						nStatus = NDS_QUERYINFOFAILURE;
						break;
					}
					else
					{
						if (dwStatusCode >= 400)
						{
							nStatus = NDS_RETURNCODEOVER400;
							break;
						}
					}

					DWORD dwAZLen = sizeof(wcAllSize);

					if (!HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH,
						&wcAllSize, &dwAZLen, NULL))
					{
						wcAllSize[0] = 0;
					}
				} while (0);
			};

			openrequest(false);

			if (nStatus != NDS_SUCCESS)
			{
				break;
			}

			if (info.m_bResume && !bEnd)
			{
				InternetCloseHandle(hRequest);
				openrequest(true);
			}

			if (nStatus != NDS_SUCCESS)
			{
				break;
			}

			WCHAR buf[BUFFSIZE];
			DWORD bufSize = sizeof(buf);
			DWORD bufRead = 0;
			INT64 nCurrentSize = 0;
			INT64 nAllDownSize = _wtoi64(wcAllSize);
			bool bStop = false;

			do
			{
				if (!InternetReadFile(hRequest, &buf, bufSize, &bufRead))
				{
					nStatus = NDS_READFILEFAILURE;
					break;
				}

				nCurrentSize += bufRead;

				if (info.m_ProgressCallback)
				{
					info.m_ProgressCallback(info.m_nId, nAllDownSize, nCurrentSize, bStop);
				}

				if (-1 == info.m_ResponseStream->Write((char*)buf, bufRead))
				{
					nStatus = NDS_RESPONSESTREAMINVALID;
					break;
				}

			} while (bufRead != 0 && !bStop);

		} while (0);

		bEnd = true;
	};

	auto releaseWininet = [&]()
	{
		if (hRequest != NULL)
		{
			InternetCloseHandle(hRequest);
		}

		if (hSession)
		{
			InternetCloseHandle(hSession);
		}

		if (hInternet)
		{
			InternetCloseHandle(hInternet);
		}
	};

	if (info.m_nTimeOut > 0)
	{
		DWORD startTime = GetTickCount();
		bool bTimeOut = false;
		std::thread t(sendrequest);

		while (!bEnd)
		{
			DWORD nCurTime = GetTickCount();

			if (int(nCurTime - startTime) > info.m_nTimeOut)
			{
				bEnd = true;
				bTimeOut = true;
				break;
			}

			Sleep(10);
		}

		releaseWininet();
		t.join();

		if (bTimeOut)
		{
			nStatus = NDS_TIMEOUT;
		}
	}
	else
	{
		sendrequest();
		releaseWininet();
	}

	if (info.m_ResultCallback)
	{
		info.m_ResultCallback(info.m_nId, nStatus);
	}

	return nStatus;
}

NETBASE_DOWN_STATUS DownToFile(const ZMString & sSaveFile, RequestInfo & info, bool bGet /*= true*/, bool bResume/* = false*/)
{
	FILE * file = 0;

	if (0 != _wfopen_s(&file, sSaveFile.c_str(), L"r") || !file)
	{
		if (0 != _wfopen_s(&file, sSaveFile.c_str(), L"a") || !file)
		{
			// 创建文件失败
			return NDS_RESPONSESTREAMINVALID;
		}
	}

	fclose(file);
	ZMFileStream  pOutStream(sSaveFile, fmOpenReadWrite/* | fmCreate*/);

	if (INVALID_HANDLE_VALUE == pOutStream.GetHandler())
	{
		return NDS_RESPONSESTREAMINVALID;
	}

	info.m_ResponseStream = &pOutStream;
	info.m_bResume = bResume;

	if (bResume)
	{
		pOutStream.Seek(0, FILE_END);
	}
	else
	{
		pOutStream.Seek(0, FILE_BEGIN);
	}

	if (bGet)
	{
		HttpGet(info);
	}
	else
	{
		HttpPost(info);
	}
}

#endif