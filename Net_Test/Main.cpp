
#include "NetBaseExport.h"
#include <stdio.h>
#include <time.h>
#include <map>

// ÄÚ´æÐ¹Â¶¼ì²âÆ÷
// #ifndef NDEBUG
// #pragma comment(lib, "..//vld//release//vld.lib")
// #include "..//vld//head//vld.h"
// #endif

void process(int id, INT64  contentLength, INT64 readCount, bool& abort)
{
	int i = 0;
	++i;
}

void end(int id, NETBASE_DOWN_STATUS responseCode)
{
	int i = 0;
	++i;
}

void main()
{
	//_CrtSetBreakAlloc(99);
	__int64 t1 = clock();
	bool bGet = false;

	if (bGet)
	{
		ZMFileStream  pOutStream(L"C:\\Users\\ydh\\Desktop\\d\\2.json", fmOpenReadWrite | fmCreate);
		HttpHeader  headers;
		RequestInfo info;
		info.m_nId = 0;
		//info.m_sUrl = L"http://down.udashi.com/Local%20mode/PE/4.7.38.3/WIN03PE.ISO";
		info.m_sUrl = L"https://docs.microsoft.com/zh-cn/azure/";
	
		info.m_RequestHeadMap = headers;
		info.m_ResponseStream = &pOutStream;
		info.m_ProgressCallback = process;
		info.m_ResultCallback = end;
		//info.m_nTimeOut = 10000;
		HttpGet(info);
	}
	else
	{
		ZMMemoryStream ss;
		HttpHeader  headers;
		headers[L"Content-Type"] = L"application/json;charset=UTF-8";
		ZMMemoryStream sBody;
		char stmp[] = "{\r\n \"data\" :\r\n{\r\n\"mac\" : \"8C-EC-4B-45-AD-38\",\r\n\"mobile\" : \"18362829801\"\r\n},\r\n\"meta\" :\r\n{\r\n\"token\" : \"889cb54dc82603c530b3181f9f07a1cd\"\r\n }\r\n}";
		sBody.Write(stmp, strlen(stmp));
		RequestInfo info;
		info.m_nId = 0;
		info.m_sUrl = L"http://test.api.guangsuss.com/user/query_userinfo";
		info.m_RequestHeadMap = headers;
		info.m_ResponseStream = &ss;
		info.m_ProgressCallback = process;
		info.m_ResultCallback = end;
		info.m_RequeseBodyStream = &sBody;
		info.m_nTimeOut = 1000;
		if (HttpPost(info))
		{
			//// 
			ss.Seek(0, FILE_BEGIN);
			std::vector<BYTE> by  = ss.ReadAll();
			std::string s(by.begin(), by.end());
			printf("%s", s.c_str());
		}
	}

	__int64 t2 = clock();
	printf("\r\n%I64d ms\r\n", t2 - t1);
	system("pause");
}