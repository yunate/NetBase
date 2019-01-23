#pragma once
#include <list>
#include <mutex>
#include "ILog.hpp"
#include "BaseThread.h"

/*************************本地日志*******************************************/
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
/*************************本地日志*******************************************/


/*************************异步日志*******************************************/
class AsyncLogTracerImpl : public ILogTracer, public BaseThread
{
public:
	AsyncLogTracerImpl(ILogTracer* pNormalTracter);
	~AsyncLogTracerImpl();
	virtual void Destory() 
	{
		m_bRun = false;
		// 不要deletethis，因为他交给BaseThread自我释放了
	};

public:
	virtual bool Trace(Unique_ILog upLog);

private:
	virtual void Run();

private:
	ILogTracer*						m_pNormalTracter;		// 即将在线程里处理的tracter
	std::list<Unique_ILog>			m_upLogList;			// 日志池	
	std::mutex*						m_pMutex;
	bool							m_bRun;
};
/*************************异步日志*******************************************/
