#ifndef __Downloader_H_
#define __Downloader_H_

#include "DownLoadDef.h"
#include "IDownCore.h"


class DownloaderPool;
class DownloadHelper;

class Downloader
{
private:
	// 不允许构造栈对象
	Downloader(const RequestInfo& info);
	Downloader(const Downloader&);
	~Downloader();

public:
	inline DOWNLOADER_TYPE GetType()
	{
		return m_Info.m_nType;
	}

	double			GetFileSize();
	void			Cancel();

	bool WriteDownFileCallBack(void *ptr, size_t size);
	bool DownProcessCallBack(
		double t,									// 下载使用（总量）	
		double d									// 下载使用（已下载）
	);

	void OnDownLoading(double all, double size);

private:
	void _Execute();

private:
	std::string         m_sTempPath;
	ZMStream *			m_pOutStream;
	DownloadData		m_DwonloadData;
	const RequestInfo	m_Info;
	IDownCore *			m_pDownCore;

	friend DownloaderPool;
	friend DownloadHelper;
};

#endif

