#ifndef __DownloadDef_H_
#define __DownloadDef_H_

#include "NetBaseExport.h"
#include <string>

// 下载的种类
typedef  unsigned int DOWNLOADER_TYPE;

// 请求种类
enum REQUESET_TYPE
{
	REQUESET_TYPE_ERROR = 0,
	REQUESET_TYPE_GET,
	REQUESET_TYPE_POST
	//REQUESET_TYPE_PUT,
	//REQUESET_TYPE_DELETE
};

class ZMStream;

// 准备数据
struct RequestInfo
{
	RequestInfo()
		: m_nRequestType(REQUESET_TYPE_GET)
		, m_nType(0)
		, m_sUrl("")
		, m_pBodyStream(0)
		, m_sSvePath("")
		, m_pOutStream(0)
		, m_userdata(0)
	{

	}

	REQUESET_TYPE				m_nRequestType;		// 请求种类（get，post等）,默认get
	DOWNLOADER_TYPE	            m_nType;			// 下载的是什么种类（默认是0），用于某几个下载的统一处理
	std::string					m_sUrl;				// 下载地址
	ZMStream *					m_pBodyStream;		// 请求内容，
	std::string					m_sHead;			// 请求头
	std::string					m_sSvePath;			// 想要保存的路径
	ZMStream *					m_pOutStream;		// 流，这个参数不为0时，m_sSvePath无效
	unsigned int				m_userdata;			// 用户数据，比如用户将一个界面的指针传过来

	// 下面三个由于需求添加回调使用
	unsigned int				m_nId;
	ProgressEvent				m_ProcessEvent;
	ResponseResult				m_ResponseResEvent;
};

// 过程数据
struct DownloadData
{
	DownloadData()
		: m_bIsStart(false)
		, m_bAllSizeHandle(false)
		, m_fAllSize(0.0)
		, m_fCurSize(0.0)
		, m_bIsEnd(false)
		, m_bResult(false)
		, m_nResCode(0)
	{

	}

	bool	     m_bIsStart;			// 开始下载了吗
	bool	     m_bAllSizeHandle;		// 用来处理m_fAllSize时保证只处理一次
	double	     m_fAllSize;			// 总大小
	double       m_fCurSize;			// 当前下载了的大小
	bool	     m_bIsEnd;				// 下载结束了吗
	bool	     m_bResult;				// 下载结果
	unsigned int m_nResCode;			// 结果返回码，一般 m_bResult == false 的时候有效
};

#endif