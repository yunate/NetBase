#include "stdafx.h"

#include "BaseThread.h"
#include "BaseTask.h"

BaseThread::BaseThread()
	: m_bStart(false)
{
}

BaseThread::~BaseThread()
{
	End();
}

void BaseThread::Start()
{
	if (!m_thread)
	{
		m_thread = std::make_shared<std::thread>(&BaseThread::_Run, this);
		m_thread->detach();
	}

	m_bStart = true;
}

void BaseThread::End()
{
	if (m_thread)
	{
// 		m_thread->join();
//  	m_thread.reset();
	}
}

void BaseThread::_Run()
{
	while (!m_bStart)
	{
		Sleep(500);
	}

	Run();
	delete this;
}

/////////////////////////////////CallBackThread////////////////////////////////////////////////////
CallBackThread::~CallBackThread()
{
}

void CallBackThread::RegestRunFunc(std::function<void()> runfun)
{
	m_runfun = runfun;
}

void CallBackThread::Run()
{
	if (m_runfun)
	{
		m_runfun();
	}
}

/////////////////////////////////TaskThread////////////////////////////////////////////////////
TaskThread::~TaskThread()
{
	
}

void TaskThread::SetTask(std::shared_ptr<BaseTask> task)
{
	m_Task = task;
}

void TaskThread::Run()
{
	if (m_Task)
	{
		m_Task->DoTask();
	}
}
