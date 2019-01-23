#include "BaseTask.h"

BaseTask::~BaseTask()
{
}

void BaseTask::RegestCallBack(std::function<void(TASK_CALLBACK_EV, LPARAM)> callBack)
{
	m_notifyFun = callBack;
}

void BaseTask::Notify(TASK_CALLBACK_EV event, LPARAM lParam)
{
	if (m_notifyFun)
	{
		m_notifyFun(event, lParam);
	}
}
