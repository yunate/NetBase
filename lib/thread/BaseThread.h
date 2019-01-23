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

	// 回掉函数
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
	// 这里用回掉函数而不直接用BaseTask对象，是为了更加的灵活
	// 也就是说这个类是一个纯粹的线程类，和其他(业务)无关
	// 然后把BaseTask用线程池管理
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
	// 由于线程是自我释放的，所以这个用智能指针
	// 不要用指针，然后在析构函数中释放，这回造成野指针问题
	std::shared_ptr<BaseTask>			m_Task;
};

#define RUNINTTASKHREAD(task) \
		TaskThread * thread##task = new TaskThread();\
		thread##task->SetTask(task);\
		thread##task->Start();\

