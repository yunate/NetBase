#pragma once



#include <objidl.h>
#include <vector>
#include <atlbase.h> 

//512M
#define  BUKETSIZE    0x20000000
#define  MEMORYDELTA  0x2000

using  ZMBytes = std::vector<BYTE>;
using  ZMString = std::wstring;
using  ZMAnsiString = std::string;
#ifdef _WIN64
#define    NativeInt         INT64
#define    NativeUInt        UINT64
#else
#define    NativeInt         INT32
#define    NativeUInt        UINT32
#endif //_WIN64 

using          FileMode = DWORD;
#define     fmCreate            0xff00
#define     fmOpenRead          0x0000
#define     fmOpenWrite         0x0001
#define 	fmOpenReadWrite     0x0002
#define 	fmExclusive         0x0004

#define 	fmShareCompat       0x0000
#define 	fmShareExclusive    0x0010
#define 	fmShareDenyWrite    0x0020
#define 	fmShareDenyRead     0x0030
#define 	fmShareDenyNone     0x0040

HANDLE       FileCreate(const ZMString& AFileName, FileMode  AMode);
HANDLE       FileCreate(const ZMAnsiString& AFileName, FileMode  AMode);
HANDLE       FileOpen(const ZMString& AFileName, FileMode  AMode);
HANDLE       FileOpen(const ZMAnsiString& AFileName, FileMode  AMode);
bool         FileClose(HANDLE hFile);
DWORD        FileRead(HANDLE  hFile, LPVOID lBuf, DWORD  lsize);
DWORD        FileWrite(HANDLE  hFile, LPVOID lBuf, DWORD  lsize);
INT64        FileSeek(HANDLE  hFile, INT64 Offset, DWORD  Origin);

INT64        GetFileSize(const ZMString& AFileName);
bool         GetFileTime(const ZMString& AFileName, FILETIME& ftCreationTime, FILETIME& ftLastAccessTime, FILETIME& ftLastWriteTime);

//流的基类
class  ZMStream 
{
public:
	virtual            ~ZMStream() {};
	virtual    INT64   GetPosition();
	virtual    void    SetPosition(INT64 pos);
	virtual    INT64   GetSize();
	virtual    void    SetSize(INT64 NewSize)                   PURE;
	virtual    int     Read(LPVOID  Buffer, DWORD  Count)       PURE;
	virtual    int     Write(const LPVOID  Buffer, DWORD  Count)      PURE;
	virtual    INT64   Seek(INT64  Offset, DWORD  Origin)       PURE;

	virtual    INT64   Read64(LPVOID  Buffer, INT64 Offset, INT64  Count);
	virtual    INT64   Write64(const LPVOID  Buffer, INT64 Offset, INT64  Count);
	virtual    INT64   ReadBytes(ZMBytes&  Buffer, INT64 Offset = 0, INT64  Count = -1);
	virtual    INT64   WriteBytes(const ZMBytes&  Buffer, INT64 Offset = 0, INT64  Count = -1);
	virtual    ZMBytes ReadAll();
protected:
private:

};

//句柄化的流
class  ZMHandlerStream : public ZMStream
{
public:
	virtual            ~ZMHandlerStream();
	HANDLE             GetHandler();
	virtual    void    SetSize(INT64 NewSize);
	virtual    int     Read(LPVOID  Buffer, DWORD  Count);
	virtual    int     Write(const LPVOID  Buffer, DWORD  Count);
	virtual    INT64   Seek(INT64  Offset, DWORD  Origin);
protected:
	HANDLE       m_Handler;
};

//文件流
class  ZMFileStream : public ZMHandlerStream
{
public:
	ZMFileStream(ZMString  AFileName, FileMode  Mode);
	virtual      ~ZMFileStream();
	ZMString     GetFileName();
	bool         Flush();
protected:
	ZMString     m_FileName;
};

//内存流
class  ZMMemoryStream : public ZMStream
{
public:
	ZMMemoryStream();
	virtual    ~ZMMemoryStream();
	virtual    void    SetSize(INT64 NewSize);
	virtual    int     Read(LPVOID  Buffer, DWORD  Count);
	virtual    int     Write(const LPVOID  Buffer, DWORD  Count);
	virtual    INT64   Seek(INT64  Offset, DWORD  Origin);

	bool    LoadFromFile(ZMString AFileName);
	bool    LoadFromStream(ZMStream&  AStream);
	bool    SaveToFile(ZMString AFileName);
	bool    SaveToStream(ZMStream&  AStream);

	LPVOID  GetMemory();
	void    Clear();

protected:
	INT64         m_Capacity;
	PBYTE         m_Memory;
	INT64         m_Size;
	INT64         m_Position;
	void          SetMemoryData(LPVOID   ptr, INT64  Size);
	void          SetCapacity(INT64 NewCapacity);
	LPVOID        ReallocMemory(INT64&  NewCapacity);
};

//资源刘
class ZMResourceStream : public ZMMemoryStream
{
public:
	ZMResourceStream(HINSTANCE  instance, ZMString ResName, LPCWSTR ResType);
	ZMResourceStream(HINSTANCE  instance, NativeInt ResId, LPCWSTR ResType);
	virtual            ~ZMResourceStream();
	virtual    int     Write(const LPVOID  Buffer, WORD  Count);
protected:
	HRSRC     m_HResInfo;
	HANDLE    m_HGlobal;
private:
	void Initialize(HINSTANCE  instance, LPCWSTR Name, LPCWSTR ResType);
};

enum  StreamOwnership { soReference, soOwned };

//可以把流转成Windows的IStream
class ZMStreamAdapter : public IStream//, ISequentialStream, IUnknown
{
public:
	ZMStreamAdapter(ZMStream*  lpStream, StreamOwnership AOwnership);
	virtual    ~ZMStreamAdapter();
	//IStream
	virtual  HRESULT STDMETHODCALLTYPE Seek(
		LARGE_INTEGER dlibMove,
		DWORD dwOrigin,
		_Out_opt_  ULARGE_INTEGER *plibNewPosition);

	virtual HRESULT STDMETHODCALLTYPE SetSize(
		ULARGE_INTEGER libNewSize);

	virtual  HRESULT STDMETHODCALLTYPE CopyTo(
		_In_  IStream *pstm,
		ULARGE_INTEGER cb,
		_Out_opt_  ULARGE_INTEGER *pcbRead,
		_Out_opt_  ULARGE_INTEGER *pcbWritten);

	virtual HRESULT STDMETHODCALLTYPE Commit(
		DWORD grfCommitFlags);

	virtual HRESULT STDMETHODCALLTYPE Revert(void);

	virtual HRESULT STDMETHODCALLTYPE LockRegion(
		ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb,
		DWORD dwLockType);

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
		ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb,
		DWORD dwLockType);

	virtual HRESULT STDMETHODCALLTYPE Stat(
		__RPC__out STATSTG *pstatstg,
		DWORD grfStatFlag);

	virtual HRESULT STDMETHODCALLTYPE Clone(
		__RPC__deref_out_opt IStream **ppstm);
	//ISequentialStream
	virtual  HRESULT STDMETHODCALLTYPE Read(
		_Out_writes_bytes_to_(cb, *pcbRead)  void *pv,
		_In_  ULONG cb,
		_Out_opt_  ULONG *pcbRead);

	virtual  HRESULT STDMETHODCALLTYPE Write(
		_In_reads_bytes_(cb)  const void *pv,
		_In_  ULONG cb,
		_Out_opt_  ULONG *pcbWritten);
	//IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		REFIID riid,
		_COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef(void);

	virtual ULONG STDMETHODCALLTYPE Release(void);
protected:
	ZMStream*            m_Stream;
	StreamOwnership     m_Ownership;
	LONG                m_RefCount;
private:
};

//可以把Windows的IStream转成ZMStream
class ZMOleStream : public ZMStream
{
public:
	ZMOleStream(IStream* AStream);
	virtual   ~ZMOleStream();
	virtual    void    SetSize(INT64 NewSize);
	virtual    int     Read(LPVOID  Buffer, DWORD  Count);
	virtual    int     Write(const LPVOID  Buffer, DWORD  Count);
	virtual    INT64   Seek(INT64  Offset, DWORD  Origin);
protected:
	CComPtr<IStream>           m_IStream;
};

