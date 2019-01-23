
#include <stdio.h>
#include <windows.h>

#include "DownLoadDef.h"
#include "DownloadHelper.h"
#include "thread/BaseThread.h"
#include "Streams.h"
#include <sstream>

//// 内存泄露检测器
#ifndef NDEBUG
#pragma comment(lib, "..//vld//release//vld.lib")
#include "..//vld//head//vld.h"
#endif


void main()
{
	//_CrtSetBreakAlloc(121);
	__int64 t1 = clock();
	RequestInfo info;
	ZMMemoryStream ss;
	ZMMemoryStream sBody;

	if (true)
	{
		//  post 请求示列
		info.m_sUrl = "http://test.api.guangsuss.com/user/query_userinfo";
		info.m_pOutStream = &ss;
		info.m_nRequestType = REQUESET_TYPE_POST;
		char stmp[] = "{\r\n \"data\" :\r\n{\r\n\"mac\" : \"8C-EC-4B-45-AD-38\",\r\n\"mobile\" : \"18362829801\"\r\n},\r\n\"meta\" :\r\n{\r\n\"token\" : \"889cb54dc82603c530b3181f9f07a1cd\"\r\n }\r\n}";
		sBody.Write(stmp, sizeof(stmp) - 1);
		info.m_pBodyStream = &sBody;
		info.m_sHead = "Content-Type:application/json;charset=UTF-8";
	}
	else
	{
		// 大文件下载示列
		info.m_sUrl = "http://down.udashi.com/Local%20mode/PE/4.7.38.3/WIN03PE.ISO";
		info.m_sSvePath = "C:\\Users\\ydh\\Desktop\\d\\2.iso";
	}

	void * handle = DownloadHelper::Create(info);

	if (false)
	{
		// 异步示列
		RUNINTCALLBACKHREAD(std::bind(&DownloadHelper::Execute, handle));

		// 等待下载完成
		while (!DownloadHelper::DownDataPeek_Core(handle, 0))
		{
			Sleep(DOWN_MESSAGE_LOOP);
		}
	}
	else
	{
		// 同步示列
		DownloadHelper::Execute(handle);
		
	}
	
	DownloadHelper::Clean(handle);

	if (info.m_pOutStream)
	{
		info.m_pOutStream->Seek(0, FILE_BEGIN);
		std::vector<BYTE> by  = info.m_pOutStream->ReadAll();
		std::string s(by.begin(), by.end());
		printf("%s", s.c_str());
	}

	__int64 t2 = clock();
	printf("\r\n%d ms\r\n", t2 - t1);
	system("pause");
}