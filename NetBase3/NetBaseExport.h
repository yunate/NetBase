#ifndef __EXPORT_H_
#define __EXPORT_H_

#include "Streams.h"
#include <functional>
#include <map>

#define   BUFFSIZE 2048

/*
	结果
*/
enum NETBASE_DOWN_STATUS
{
	NDS_NODEFINE = -1,
	NDS_SUCCESS  = 0,
	NDS_RESPONSESTREAM,					// 输出流无效 
	NDS_CRACKURLFAILURE,				// url crack 失败 
	NDS_SCHEMEERROR,					// scheme 不是http/https
	NDS_INTERNETOPENFAILURE,			// 打开网络失败				（InternetOpen）
	NDS_CONNECTFAILURE,					// 打开secssion失败			（InternetConnect）
	NDS_REQUESTFAILURE,					// 打开请求失败				（HttpOpenRequest）
	NDS_ADDREQUESTFAILURE,				// 设置http header失败		（HttpAddRequestHeaders）
	NDS_SENDREQUESRFAILURE,				// 发送请求失败				（HttpSendRequestEx）
	NDS_ENDQUESRFAILURE,				// 关闭请求失败				（HttpEndRequest）
	NDS_WRITEFILEFAILURE,				// 发送body失败				（InternetWriteFile）
	NDS_TIMEOUT,						// 超时
	NDS_QUERYINFOFAILURE,				//							（HttpQueryInfo）
	NDS_RETURNCODEOVER400,				// 请求返回值大于400
	NDS_READFILEFAILURE,				// 读取返回buff出错			（InternetReadFile）
	NDS_FAILURE
}; 

using  HttpHeader =std::map<ZMString, ZMString>;

/*
	进度回调
	@param id : 下载表示 对应RequestInfo中的m_nId
	@param contentLength : 总大小
	@param readCount : 当前下载大小
	@param abort : 设置abort = true 下载正常停止
*/
using ProgressCallback = std::function<void(int id, INT64  contentLength, INT64 readCount, bool& abort)>;

/*
	结果回调
	@param id : 下载表示 对应RequestInfo中的m_nId
	@param responseCode : 返回结果，见NETBASE_DOWN_STATUS。（abort = true时候为用户手动停止，此时responseCode = NDS_SUCCESS）
*/
using ResultCallBack = std::function<void(int id, NETBASE_DOWN_STATUS responseCode)>;

typedef struct RequestInfo
{
	unsigned int			m_nId					= 0;		// id，回调时候使用
	ZMString				m_sUrl					= L"";		// url
	HttpHeader				m_RequestHeadMap;					// http头，尾部不要有\0等其它字符
	ZMStream *				m_ResponseStream		= 0;		// 输出流 m_ResponseStream->Write 返回-1认为失败
	ProgressCallback		m_ProgressCallback		= 0;		// 进度回调
	ResultCallBack			m_ResultCallback		= 0;		// 结束回调
	ZMStream *				m_RequeseBodyStream		= 0;		// post使用，http body输入流，尾部不要有\0等其它字符 m_RequeseBodyStream->Read 返回-1认为失败
	int						m_nTimeOut				= -1;		// 超时(ms)，大于0时有效，这个参数有效时候将创建一个额外的线程用来计时
} RequestInfo;

/*
	Get 方式下载
	@param info : 输入,见RequestInfo
*/
bool  HttpGet(RequestInfo & info);

/*
	post 方式下载
	@param info : 输入,见RequestInfo
*/
bool  HttpPost(RequestInfo & info);

#endif // 
