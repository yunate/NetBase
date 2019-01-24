
#include <stdio.h>
#include <windows.h>

#include "DownLoadDef.h"
#include "DownloadHelper.h"
#include "thread/BaseThread.h"
#include "Streams.h"
#include "NetBaseExport.h"
#include "string_convert.h"
#include <sstream>

//// 内存泄露检测器
#ifndef NDEBUG
#pragma comment(lib, "..//vld//release//vld.lib")
#include "..//vld//head//vld.h"
#endif

void process(int id, INT64  contentLength, INT64 readCount, bool& abort)
{
	//printf("all : %d, current:%d  \r\n", contentLength, readCount);
}

void end(int id, NETBASE_DOWN_STATUS responseCode, ZMString)
{
	int i = 0;
	++i;
}
void main()
{
	//_CrtSetBreakAlloc(121);
	bool bGet = true;
	ZMMemoryStream ss;
	__int64 t1 = clock();
	if (bGet)
	{
	
		ZMFileStream  pOutStream(L"C:\\Users\\ydh\\Desktop\\d\\2.json", fmOpenReadWrite | fmCreate);
		HttpHeader  headers;
		//ZMString sUrl = L"http://down.udashi.com/Local%20mode/PE/4.7.38.3/WIN03PE.ISO";
		ZMString sUrl = L"https://www.baidu.com";
		HttpGet(sUrl, 0, headers, &pOutStream, process, end);
	}
	else
	{
		HttpHeader  headers;
		headers[L"Content-Type"] = L"application/json;charset=UTF-8";
		ZMString sUrl = L"http://test.api.guangsuss.com/user/query_userinfo";
		ZMMemoryStream sBody;
		char stmp[] = "{\r\n \"data\" :\r\n{\r\n\"mac\" : \"8C-EC-4B-45-AD-38\",\r\n\"mobile\" : \"18362829801\"\r\n},\r\n\"meta\" :\r\n{\r\n\"token\" : \"889cb54dc82603c530b3181f9f07a1cd\"\r\n }\r\n}";
		sBody.Write(stmp, sizeof(stmp) - 1);
		HttpPost(sUrl, 0, headers, &ss, &sBody);
	}


// 	RequestInfo info;
// 	ZMMemoryStream ss;
// 	ZMMemoryStream sBody;
// 
// 	if (false)
// 	{
// 		//  post 请求示列
// 		info.m_sUrl = "http://test.api.guangsuss.com/user/query_userinfo";
// 		info.m_pOutStream = &ss;
// 		info.m_nRequestType = REQUESET_TYPE_POST;
// 		char stmp[] = "{\r\n \"data\" :\r\n{\r\n\"mac\" : \"8C-EC-4B-45-AD-38\",\r\n\"mobile\" : \"18362829801\"\r\n},\r\n\"meta\" :\r\n{\r\n\"token\" : \"889cb54dc82603c530b3181f9f07a1cd\"\r\n }\r\n}";
// 		sBody.Write(stmp, sizeof(stmp) - 1);
// 		info.m_pBodyStream = &sBody;
// 		info.m_sHead = "Content-Type:application/json;charset=UTF-8";
// 	}
// 	else
// 	{
// 		// 大文件下载示列
// 		info.m_sUrl = "http://down.udashi.com/Local%20mode/PE/4.7.38.3/WIN03PE.ISO";
// 		info.m_sSvePath = "C:\\Users\\ydh\\Desktop\\d\\2.iso";
// 	}
// 
// 	void * handle = DownloadHelper::Create(info);
// 
// 	if (false)
// 	{
// 		// 异步示列
// 		RUNINTCALLBACKHREAD(std::bind(&DownloadHelper::Execute, handle));
// 
// 		// 等待下载完成
// 		while (!DownloadHelper::DownDataPeek_Core(handle, 0))
// 		{
// 			Sleep(DOWN_MESSAGE_LOOP);
// 		}
// 	}
// 	else
// 	{
// 		// 同步示列
// 		DownloadHelper::Execute(handle);
// 		
// 	}
// 	
// 	DownloadHelper::Clean(handle);
// 
	if (!bGet)
	{
		ss.Seek(0, FILE_BEGIN);
		std::vector<BYTE> by  = ss.ReadAll();
		std::string s(by.begin(), by.end());
		printf("%s", s.c_str());
	}

	__int64 t2 = clock();
	printf("\r\n%d ms\r\n", t2 - t1);
	system("pause");
}