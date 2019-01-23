#include "stdafx.h"
#include "LogDoggy.h"
#include "LogTracerImpl.h"
#include "LogImpl.h"

LogDoggy::LogDoggy()
	:  m_pTracer(0)
{
}

LogDoggy::~LogDoggy()
{
	if (m_pTracer)
	{
		m_pTracer->Destory();
		m_pTracer = 0;
	}
}

void LogDoggy::Init_Local_Async()
{
	ILogTracer* pNormalTracer = new LocalFileTracer("umaster-general");
	AsyncLogTracerImpl* pTracter = new AsyncLogTracerImpl(pNormalTracer);
	GetDoggy().SetTracer(pTracter);
	pTracter->Start();
}

void LogDoggy::Init_Local()
{
	SetTracer(new LocalFileTracer());
}

void LogDoggy::UnInit()
{
	SetTracer(0);
	Sleep(100);
}

void LogDoggy::Bark(Unique_ILog upLog)
{
	if (m_pTracer)
	{
		m_pTracer->Trace(std::move(upLog));
	}
}

void LogDoggy::SetTracer(ILogTracer * pTracer)
{
	if (m_pTracer)
	{
		m_pTracer->Destory();
	}

	m_pTracer = pTracer;
}

void LogDoggy::Bark_Error_Log(std::string sLog)
{
	Unique_ILog upLog(new ErrorLog(sLog));
	upLog->Format();
	Bark(std::move(upLog));
}

void LogDoggy::Bark_Info_Log(std::string sLog)
{
	Unique_ILog upLog(new InfoLog(sLog));
	upLog->Format();
	Bark(std::move(upLog));
}
