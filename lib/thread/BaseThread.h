#pragma once
#include <functional>
#include <thread>
#include <memory>

/////////////////////////////////BaseThread////////////////////////////////////////////////////
class BaseThread
{
public:
	BaseThread();
	virtual ~BaseThread();

public:
	void Start();

private:
	void End();
	void _Run();

	// �ص�����
	virtual void Run() {}

private:
	std::shared_ptr<std::thread>                    m_thread;
	bool											m_bStart;
};

/////////////////////////////////CallBackThread////////////////////////////////////////////////////
class CallBackThread : public BaseThread
{
public:
	virtual ~CallBackThread();

public:
	// �����ûص���������ֱ����BaseTask������Ϊ�˸��ӵ����
	// Ҳ����˵�������һ��������߳��࣬������(ҵ��)�޹�
	// Ȼ���BaseTask���̳߳ع���
	void RegestRunFunc(std::function<void()> runfun);

private:
	virtual void Run();

private:
	std::function<void()>							m_runfun;
};

#define RUNINTCALLBACKHREAD(callback) \
		CallBackThread * thread = new CallBackThread();\
		thread->RegestRunFunc((callback));\
		thread->Start();\

/////////////////////////////////TaskThread////////////////////////////////////////////////////
class BaseTask;

class TaskThread : public BaseThread
{
public:
	virtual ~TaskThread();

public:
	void SetTask(std::shared_ptr<BaseTask> task);

private:
	virtual void Run();

private:
	// �����߳��������ͷŵģ��������������ָ��
	// ��Ҫ��ָ�룬Ȼ���������������ͷţ�������Ұָ������
	std::shared_ptr<BaseTask>			m_Task;
};

#define RUNINTTASKHREAD(task) \
		TaskThread * thread##task = new TaskThread();\
		thread##task->SetTask(task);\
		thread##task->Start();\

