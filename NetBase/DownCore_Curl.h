#ifndef __DOWN_CORE_CURL_H_
#define __DOWN_CORE_CURL_H_

#define  CURL_STATICLIB
#define  HTTP_ONLY

#include "IDownCore.h"

class DownCore_Curl : public IDownCore
{
public:
	DownCore_Curl();
	~DownCore_Curl();

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
};

#endif