#include "stdafx.h"
#include "DownloaderPool.h"

////////////////////////////////DownloaderPool/////////////////////////////////////////////
DownloaderPool::~DownloaderPool()
{

}

void DownloaderPool::Insert(void * handel)
{
	ThreadLocker locker;

	if (handel)
	{
		m_pool.insert(handel);
	}
}

void DownloaderPool::Remove(void * handel)
{
	ThreadLocker locker;

	if (handel)
	{
		m_pool.erase(handel);
		delete (Downloader*)handel; 
	}
}

void DownloaderPool::DoEach(DOWNLOADER_TYPE nType, DownLoaderCallBack Fun)
{
	ThreadLocker locker;

	for (auto it : m_pool)
	{
		if (nType == GetInfo(it)->m_nType)
		{
			(((Downloader*)it)->*Fun)();
		}
	}
}

void DownloaderPool::DoEach(DOWNLOADER_TYPE nType, std::function<bool(void*)> callback)
{
	ThreadLocker locker;
	std::set<void*>::iterator it = m_pool.begin();

	while (it != m_pool.end())
	{
		std::set<void*>::iterator tmp = it;
		++it;

		if (nType == GetInfo(*tmp)->m_nType)
		{
			if (callback)
			{
				if (!callback(*tmp))
				{
					break;
				}
			}
		}
	}
}

void DownloaderPool::DoEach(std::function<bool(void*)> callback)
{
	ThreadLocker locker;
	std::set<void*>::iterator it = m_pool.begin();

	while (it != m_pool.end())
	{
		std::set<void*>::iterator tmp = it;
		++it;

		if (callback)
		{
			if (!callback(*tmp))
			{
				break;
			}
		}
	}
}

DownloadData * DownloaderPool::GetData(void * handle)
{
	return &((Downloader*)handle)->m_DwonloadData;
}

DownloadData * DownloaderPool::GetData_S(void * handle)
{
	std::set<void*>::iterator it = m_pool.find(handle);

	if (it != m_pool.end())
	{
		return &((Downloader*)handle)->m_DwonloadData;
	}

	return 0;
}

const RequestInfo * DownloaderPool::GetInfo(void * handle)
{
	return &((Downloader*)handle)->m_Info;
}

const RequestInfo * DownloaderPool::GetInfo_S(void * handle)
{
	std::set<void*>::iterator it = m_pool.find(handle);

	if (it != m_pool.end())
	{
		return &((Downloader*)handle)->m_Info;
	}

	return 0;
}

