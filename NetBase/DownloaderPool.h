#ifndef __DownloaderPool_H_
#define __DownloaderPool_H_

#include "thread/ThreadLocker.h"
#include "Downloader.h"

#include <set>

// ÿ500ms����һ�β�ѯ
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

	// �����߱��뱣֤handle��Ч
	// ���handle��Ч������ֵ�ض���Ч���������
	DownloadData* GetData(void * handle);

	// ��ȫ�ĵ��ã������ǱȽ���
	DownloadData* GetData_S(void * handle);

	// �����߱��뱣֤handle��Ч
	// ���handle��Ч������ֵ�ض���Ч���������
	const RequestInfo* GetInfo(void * handle);

	// ��ȫ�ĵ��ã������ǱȽ���
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