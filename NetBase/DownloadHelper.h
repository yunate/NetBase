#ifndef __DownloaderHelper_H_
#define __DownloaderHelper_H_

#include "Downloader.h"
#include "DownloaderPool.h"
#include "Downloader.h"

#include <string>

enum DOWNINFOSTATE
{
	DOWNINFOSTATE_ALLSIZE,
	DOWNINFOSTATE_CURSIZE,
	DOWNINFOSTATE_FAILURE,
	DOWNINFOSTATE_SUCCESS
};

class DownloadHelper
{
public:
	/*
		Create Execute Clean
		这三个是通用下载函数
		，Create创建（初始化）一个下载
		，Execute开始执行一个下载（比如在线程里）
		，Clean下载结束后清理资源
	*/ 
	static void *Create(const RequestInfo&);
	static void Execute(void * handle);
	static void Clean(void * handle);

	/*
		通用下载数据循环核心内容，对每一个nType类型进行处理，如有需要可以重新写一个 
		@param : nType 类型
		@param : callback 一个通用回调
		@return : 返回true说明结束（可能成功也可能失败），false没结束
	*/ 
	static bool DownDataPeek_Core(DOWNLOADER_TYPE nType, std::function<void(DOWNINFOSTATE, unsigned int)> callback);
	static bool DownDataPeek_Core(void * handle, std::function<void(DOWNINFOSTATE, unsigned int)> callback);

	// 带有文件检查的下载
	static void * DownLoadWithMD5Check(const RequestInfo&);

	static void * DownLoadWithFileNameCheck(const RequestInfo&);
};

#endif