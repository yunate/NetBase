#include "stdafx.h"
#include <fstream>
#include <sstream>

#include "LogTracerImpl.h"
#include "Dog_Lock.h"

#ifndef NDEBUG
#include <assert.h>
#include <iostream>
#endif

#define MAX_LOG_FILE_SIZE 10

/*************************本地日志*******************************************/
LocalFileTracer::LocalFileTracer(std::string sFix/* = "log"*/)
	: m_sFileName(""), m_sFix(sFix)
{
	char exeFullPath[MAX_PATH];
	GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
	std::string strPath = exeFullPath;
	int pos = strPath.find_last_of('\\', strPath.length());
	m_sDirPath = strPath.substr(0, pos) + "\\log\\";
	CreateDirectoryA(m_sDirPath.c_str(), NULL);
	m_sDirPath = m_sDirPath + sFix + "\\";
	CreateDirectoryA(m_sDirPath.c_str(), NULL);
}

bool LocalFileTracer::Trace(Unique_ILog upLog)
{
	std::string sPath = GetAvailableFile();

	FILE* pFile = 0;
	pFile = fopen(sPath.c_str(), "ab");

	if (!pFile)
	{
		return false;
	}

	std::string sLog = upLog->GetLog();
	fwrite(sLog.c_str(), sLog.length(), 1, pFile);
	fclose(pFile);

	
#ifndef NDEBUG
	if (ILog::LOG_ERROR == upLog->GetLogLevel())
	{
		assert(1);
	}

	std::cout << upLog->GetLog();
#endif

	return true;
}

std::string LocalFileTracer::GetAvailableFile()
{
	CreateDirectoryA(m_sDirPath.c_str(), NULL);
	std::string sFullPath;
	std::string sLogIniFullPath = m_sDirPath + m_sFix + "-log.ini";

	while (1)
	{
		// 先找到日志配置文件ini
		DWORD ftyp = GetFileAttributesA(sLogIniFullPath.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
		{
			m_sFileName = "";
		}
		else
		{
			// 打开文件
			std::fstream fileStream(sLogIniFullPath);
			fileStream >> m_sFileName;
			fileStream.close();

			DeleteFileA(sLogIniFullPath.c_str());
		}

		if (0 == strlen(m_sFileName.c_str()))
		{
			m_sFileName = "log-0.txt";
		}

		// 看看文件大小
		sFullPath = m_sDirPath + m_sFileName;
		struct _stat info;

		if (0 != _stat(sFullPath.c_str(), &info))
		{
			// 文件不存在就返回
			break;
		}

		int size = info.st_size >> 20;

		if (size >= MAX_LOG_FILE_SIZE)
		{
			// 文件过大，就在创建一个
			int nIndex = m_sFileName.find('-');
			std::string sPre = m_sFileName.substr(0, nIndex + 1);
			std::string sTail = m_sFileName.substr(nIndex + 1);
			std::stringstream sTmpStream;
			sTmpStream << sTail;
			int nTmp = 0;
			sTmpStream >> nTmp;
			sTmpStream.str("");
			sTmpStream << sPre << (++nTmp) << ".txt";
			m_sFileName = sTmpStream.str();
			sFullPath = m_sDirPath + m_sFileName;
			break;
		}

		break;
	}

	// 将配置写回去
	FILE* fp = 0;
	if (0 == fopen_s(&fp, sLogIniFullPath.c_str(), "w"))
	{
		fwrite(m_sFileName.c_str(), strlen(m_sFileName.c_str()), 1, fp);
		fclose(fp);
		SetFileAttributesA(sLogIniFullPath.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}

	return sFullPath;
}
/*************************本地日志*******************************************/

/*************************异步日志*******************************************/
AsyncLogTracerImpl::AsyncLogTracerImpl(ILogTracer* pNormalTracter)
	: m_pNormalTracter(pNormalTracter), m_bRun(true)
{
	m_pMutex = new std::mutex();
}

AsyncLogTracerImpl::~AsyncLogTracerImpl()
{
	if (m_pNormalTracter)
	{
		m_pNormalTracter->Destory();
	}

	if (m_pMutex)
	{
		delete m_pMutex;
	}
}

bool AsyncLogTracerImpl::Trace(Unique_ILog upLog)
{
	Dog_Lock lock(m_pMutex);
	m_upLogList.push_back(std::move(upLog));
	return true;
}

void AsyncLogTracerImpl::Run()
{
	while (m_bRun)
	{
		Sleep(100);

		if (m_upLogList.size() > 0 && m_pNormalTracter)
		{
			std::list<Unique_ILog>::iterator it = m_upLogList.begin();

			while (m_upLogList.end() != it)
			{
				m_pNormalTracter->Trace(std::move(*it));
				Dog_Lock lock(m_pMutex);
				it = m_upLogList.erase(it);
			}
		}
	}

	Dog_Lock lock(m_pMutex);
	m_upLogList.clear();
}
/*************************异步日志*******************************************/

