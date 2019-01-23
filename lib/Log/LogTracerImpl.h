#pragma once
#include <list>
#include <mutex>
#include "ILog.hpp"
#include "BaseThread.h"

/*************************������־*******************************************/
class LocalFileTracer : public ILogTracer
{
public:
	LocalFileTracer(std::string sFix = "log");
	virtual ~LocalFileTracer() {};
	virtual void Destory() 
	{
		delete this;
	};

public:
	virtual bool Trace(Unique_ILog upLog);

private:
	std::string GetAvailableFile();

private:
	std::string m_sFix;
	std::string m_sDirPath;
	std::string m_sFileName;
};
/*************************������־*******************************************/


/*************************�첽��־*******************************************/
class AsyncLogTracerImpl : public ILogTracer, public BaseThread
{
public:
	AsyncLogTracerImpl(ILogTracer* pNormalTracter);
	~AsyncLogTracerImpl();
	virtual void Destory() 
	{
		m_bRun = false;
		// ��Ҫdeletethis����Ϊ������BaseThread�����ͷ���
	};

public:
	virtual bool Trace(Unique_ILog upLog);

private:
	virtual void Run();

private:
	ILogTracer*						m_pNormalTracter;		// �������߳��ﴦ���tracter
	std::list<Unique_ILog>			m_upLogList;			// ��־��	
	std::mutex*						m_pMutex;
	bool							m_bRun;
};
/*************************�첽��־*******************************************/
