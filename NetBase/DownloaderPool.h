#ifndef __DownloaderPool_H_
#define __DownloaderPool_H_

#include "thread/ThreadLocker.h"
#include "Downloader.h"

#include <set>

// 每500ms进行一次查询
#define  DOWN_MESSAGE_LOOP 500

typedef void(Downloader::*DownLoaderCallBack)();

class DownloaderPool
{
public:
	~DownloaderPool();

	void Insert(void *);
	void Remove(void *);
	void DoEach(DOWNLOADER_TYPE nType, DownLoaderCallBack Fun);
	void DoEach(DOWNLOADER_TYPE nType, std::function<bool(void *)>);
	void DoEach(std::function<bool (void *)>);

	// 调用者必须保证handle有效
	// 如果handle有效，返回值必定有效，否则崩溃
	DownloadData* GetData(void * handle);

	// 安全的调用，代价是比较慢
	DownloadData* GetData_S(void * handle);

	// 调用者必须保证handle有效
	// 如果handle有效，返回值必定有效，否则崩溃
	const RequestInfo* GetInfo(void * handle);

	// 安全的调用，代价是比较慢
	const RequestInfo* GetInfo_S(void * handle);

private:
	std::set<void*>		m_pool;

private:
	DownloaderPool() {}
	DownloaderPool(const DownloaderPool&);

	static DownloaderPool& GetIns() 
	{
		static DownloaderPool ins;
		return ins;
	}

	friend DownloaderPool& GET_DOWNLOADERPOOL_INS();
};

inline DownloaderPool& GET_DOWNLOADERPOOL_INS()
{
	return DownloaderPool::GetIns();
}

#endif