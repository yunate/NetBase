#ifndef __IDOWN_CORE_H_
#define __IDOWN_CORE_H_

#include "DownLoadDef.h"
#include "Streams.h"

#include <functional>

typedef std::function<bool(void * ptr, size_t size)> WriteCallBack;
typedef std::function<bool(double dAllSize, double dNowSize)> ProgressCallBack;

class IDownCore
{
public:
	IDownCore();
	~IDownCore();

	inline void Construct(const RequestInfo * pInfo, ZMStream * pOutStream
		, WriteCallBack writeCall, ProgressCallBack progressCall)
	{
		m_pInfo = pInfo;
		m_pOutStream = pOutStream;
		m_WriteCallBack = writeCall;
		m_ProgressBack = progressCall;
	}

public:
	virtual bool Down() = 0;
	virtual double GetSize() { return 0; };			// 将要下载的大小
	virtual bool Stop() { return m_bStop = true; };
	virtual bool Pause() { return m_bPause = true; };
	virtual bool ReStart() { return m_bPause = false; };


public:
	inline bool Write(void * ptr, size_t size)
	{
		if (m_WriteCallBack)
		{
			return m_WriteCallBack(ptr, size);
		}
		else
		{
			return true;
		}
	}

	inline bool Progress(double dAllSize, double dNowSize)
	{
		if (m_ProgressBack)
		{
			return m_ProgressBack(dAllSize, dNowSize);
		}
		else
		{
			return true;
		}
	}

protected:
	const RequestInfo *			m_pInfo;
	ZMStream *					m_pOutStream;
	WriteCallBack				m_WriteCallBack;
	ProgressCallBack			m_ProgressBack;

	bool						m_bStop;
	bool						m_bPause;
};

#endif
