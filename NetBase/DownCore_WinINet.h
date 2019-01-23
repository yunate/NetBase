#ifndef __DOWN_CORE_WININET_H_
#define __DOWN_CORE_WININET_H_
#include "IDownCore.h"

#include <string>
#include <windows.h>
#include <WinINet.h>

class StaticIntHandleObj
{
private:
	StaticIntHandleObj(const StaticIntHandleObj&);

public:
	StaticIntHandleObj()
	{
		m_handle = 0;
		m_sDes = "";
	}

	~StaticIntHandleObj()
	{
		if (m_handle)
		{
			InternetCloseHandle(m_handle);
		}
	}

	operator HINTERNET()
	{
		return m_handle;
	}

	void operator=(const HINTERNET handle)
	{
		if (m_handle)
		{
			InternetCloseHandle(m_handle);
		}

		m_handle = handle;
	}

	void SetDes(std::string sDes)
	{
		m_sDes = sDes;
	}

	std::string & GetDes()
	{
		return m_sDes;
	}

public:
	HINTERNET	m_handle;
	std::string m_sDes;
};

class DownCore_WinINet : public IDownCore
{
public:
	DownCore_WinINet();
	~DownCore_WinINet();

public:
	bool Down();

public:
	inline bool GetStop()
	{
		return m_bStop;
	}

	inline bool GetPause()
	{
		return m_bPause;
	}

private:
	bool CrackUrl();

private:
	URL_COMPONENTSA				m_urlComponents;
	CHAR						m_pHostName[INTERNET_MAX_HOST_NAME_LENGTH]    ={ 0 };
	CHAR						m_pUserName[INTERNET_MAX_USER_NAME_LENGTH]    ={ 0 };
	CHAR						m_pPassword[INTERNET_MAX_PASSWORD_LENGTH]     ={ 0 };
	CHAR						m_pURLPath[INTERNET_MAX_URL_LENGTH]           ={ 0 };
	CHAR						m_pScheme[INTERNET_MAX_SCHEME_LENGTH]         ={ 0 };

	static StaticIntHandleObj	s_hInternet;
	static StaticIntHandleObj	s_hSessison;
};

#endif