#ifndef __EXPORT_H_
#define __EXPORT_H_
#include "Streams.h"
#include <functional>
#include <map>

#define   HTTP_STATUS_OK   200


enum NETBASE_DOWN_STATUS
{
	NETBASE_DOWN_STATUS_NODEFINE = -1,
	NETBASE_DOWN_STATUS_SUCCESS = 0,
	NETBASE_DOWN_STATUS_FAILURE
};

using  HttpHeader =std::map<ZMString, ZMString>;

//下载进度回调
using ProgressEvent = std::function<void(int id, INT64  contentLength, INT64 readCount, bool& abort)>;

//下载结果
using ResponseResult = std::function<void(int id, NETBASE_DOWN_STATUS responseCode, ZMString)>;

bool  HttpGet(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, const ProgressEvent progress = 0
	, const ResponseResult responseResult = 0);

bool  HttpPost(const ZMString& url
	, int id
	, const HttpHeader& headers
	, ZMStream* responseStream
	, ZMStream* requestBodyStream
	, const ProgressEvent progress = 0
	, const ResponseResult responseResult = 0);
#endif // 
