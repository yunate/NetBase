#pragma once
#include "Ilog.hpp"

/*
	使用前先初始化
*/
class LogDoggy
{
private:
	LogDoggy();
	LogDoggy(const LogDoggy &);

public:
	void Init_Local_Async();
	void Init_Local();

	void UnInit();

public:
	~LogDoggy();

	void Bark(Unique_ILog upLog);
	void SetTracer(ILogTracer* pTracer);

public:
	void Bark_Error_Log(std::string sLog);
	void Bark_Info_Log(std::string sLog);

private:
	ILogTracer* m_pTracer;

	static LogDoggy& GetIns()
	{
		static LogDoggy sIns;
		return sIns;
	}

	friend LogDoggy& GetDoggy();
};

inline LogDoggy& GetDoggy()
{
	return LogDoggy::GetIns();
}