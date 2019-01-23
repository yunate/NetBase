#pragma once
#include <mutex>
#include <set>
class ThreadLocker
{
public:
	ThreadLocker()
	{
		ThreadLocker::GetStaticMutex().lock();
	}

	~ThreadLocker()
	{
		ThreadLocker::GetStaticMutex().unlock();
	}

private:
	static std::recursive_mutex & GetStaticMutex()
	{ 
		static std::recursive_mutex mtx;
		return mtx;
	}
};

// 这个类主要为了获得一个有效的不重复的id
class UniqueIdCreator
{
public:
	UniqueIdCreator()
	{

	}

	~UniqueIdCreator()
	{

	}

public:
	void RecycleId(unsigned int id)
	{
		m_nValidKeySet.insert(id);
	}

	unsigned int GetValidMapKey()
	{
		ThreadLocker locker;
		if (0 == m_nValidKeySet.size())
		{
			m_nMaxKey++;
			return m_nMaxKey;
		}
		else
		{
			unsigned int nKey = *m_nValidKeySet.begin();
			m_nValidKeySet.erase(m_nValidKeySet.begin());
			return nKey;
		}
	}

private:
	std::set<unsigned int> m_nValidKeySet;
	unsigned int m_nMaxKey;
};