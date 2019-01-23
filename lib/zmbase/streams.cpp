#include "Streams.h"
#include <windows.h>


DWORD  AccessMode[3] = { GENERIC_READ ,GENERIC_WRITE,  GENERIC_READ | GENERIC_WRITE };
DWORD  Exclusive[2] = { CREATE_ALWAYS , CREATE_NEW };
DWORD  ShareMode[5] = { 0, 0, FILE_SHARE_READ , FILE_SHARE_WRITE , FILE_SHARE_READ | FILE_SHARE_WRITE };

HANDLE       FileCreate(const ZMString& AFileName, FileMode  AMode)
{
	HANDLE   ret = INVALID_HANDLE_VALUE;
	if ((AMode & 0xF0) <= fmShareDenyNone)
	{
		ret = CreateFile((LPCWSTR)AFileName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			ShareMode[(AMode & 0xf0) >> 4],
			NULL,
			Exclusive[(AMode & 0x0004) >> 2],
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	return ret;
}

HANDLE FileCreate(const ZMAnsiString& AFileName, FileMode AMode)
{
	HANDLE   ret = INVALID_HANDLE_VALUE;
	if ((AMode & 0xF0) <= fmShareDenyNone)
	{
		ret = CreateFileA((LPCSTR)AFileName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			ShareMode[(AMode & 0xf0) >> 4],
			NULL,
			Exclusive[(AMode & 0x0004) >> 2],
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	return ret;
}

HANDLE       FileOpen(const ZMString& AFileName, FileMode  AMode)
{
	HANDLE   ret = INVALID_HANDLE_VALUE;
	if (((AMode & 3) <= fmOpenReadWrite) &&
		((AMode & 0xF0) <= fmShareDenyNone))
	{
		ret = CreateFile((LPCWSTR)AFileName.c_str(),
			AccessMode[(AMode & 3)],
			ShareMode[(AMode & 0xf0) >> 4],
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	return ret;
}

HANDLE    FileOpen(const ZMAnsiString& AFileName, FileMode AMode)
{
	HANDLE   ret = INVALID_HANDLE_VALUE;
	if (((AMode & 3) <= fmOpenReadWrite) &&
		((AMode & 0xF0) <= fmShareDenyNone))
	{
		ret = CreateFileA((LPCSTR)AFileName.c_str(),
			AccessMode[(AMode & 3)],
			ShareMode[(AMode & 0xf0) >> 4],
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	return ret;
}

bool         FileClose(HANDLE hFile)
{
	return FALSE != CloseHandle(hFile);
}
DWORD        FileRead(HANDLE  hFile, LPVOID lBuf, DWORD  lsize)
{
	DWORD    ret;
	if (!ReadFile(hFile, lBuf, lsize, &ret, NULL))
		return -1;
	return ret;
}
DWORD        FileWrite(HANDLE  hFile, LPVOID lBuf, DWORD  lsize)
{
	DWORD    ret;
	if (!WriteFile(hFile, lBuf, lsize, &ret, NULL))
	{
		int  e = GetLastError();
		return -1;
	}
	return ret;
}
INT64        FileSeek(HANDLE  hFile, INT64 Offset, DWORD  Origin)
{
	LARGE_INTEGER  u;
	u.QuadPart = Offset;
	u.LowPart = SetFilePointer(hFile, u.LowPart, &u.HighPart, Origin);
	if ((0xffffffff == u.LowPart) && (GetLastError() != 0))
	{
		u.HighPart = 0xffffffff;
	}
	return u.QuadPart;
}

INT64        GetFileSize(const ZMString& AFileName)
{
	INT64  ret = -1;
	WIN32_FILE_ATTRIBUTE_DATA  attr = { 0 };
	if (GetFileAttributesEx((LPCWSTR)AFileName.c_str(), GetFileExInfoStandard, &attr))
	{
		LARGE_INTEGER  u;
		u.LowPart = attr.nFileSizeLow;
		u.HighPart = attr.nFileSizeHigh;
		return u.QuadPart;
	}
	return ret;
}
bool         GetFileTime(const ZMString& AFileName, FILETIME& ftCreationTime, FILETIME& ftLastAccessTime, FILETIME& ftLastWriteTime)
{
	bool  ret = false;
	WIN32_FILE_ATTRIBUTE_DATA  attr = { 0 };
	if (GetFileAttributesEx((LPCWSTR)AFileName.c_str(), GetFileExInfoStandard, &attr))
	{
		ftCreationTime = attr.ftCreationTime;
		ftLastAccessTime = attr.ftLastAccessTime;
		ftLastWriteTime = attr.ftLastWriteTime;
		return true;
	}
	return ret;
}

INT64 ZMStream::Read64(LPVOID Buffer, INT64 Offset, INT64 Count)
{
	PBYTE     LBuffer = (PBYTE)Buffer;
	INT64     ret = 0;
	while (Count > BUKETSIZE)
	{
		ret += Read(&LBuffer[Offset], BUKETSIZE);
		Offset += BUKETSIZE;
		Count -= BUKETSIZE;
	}
	if (Count > 0)
		ret += Read(&LBuffer[Offset], (DWORD)Count);
	return   ret;
}

INT64 ZMStream::Write64(const LPVOID Buffer, INT64 Offset, INT64 Count)
{
	PBYTE     LBuffer = (PBYTE)Buffer;
	INT64     ret = 0;
	while (Count > BUKETSIZE)
	{
		ret += Write(&LBuffer[Offset], BUKETSIZE);
		Offset += BUKETSIZE;
		Count -= BUKETSIZE;
	}
	if (Count > 0)
		ret += Write(&LBuffer[Offset], (DWORD)Count);
	return   ret;
}

INT64 ZMStream::ReadBytes(ZMBytes& Buffer, INT64 Offset /*= 0*/, INT64 Count /*= -1*/)
{
	return Read64(&Buffer[0], Offset, (Count >= 0) ? Count : (Buffer.size() - Offset));
}

INT64 ZMStream::WriteBytes(const ZMBytes& Buffer, INT64 Offset /*= 0*/, INT64 Count /*= -1*/)
{
	return Write64((const LPVOID)&Buffer[0], Offset, (Count >= 0) ? Count : (Buffer.size() - Offset));
}

ZMBytes ZMStream::ReadAll()
{
	ZMBytes   ret;
	ret.resize((size_t)GetSize());
	if (ret.size() != (size_t)ReadBytes(ret, (INT64)0, (INT64)ret.size()))
	{
		ret.resize(0);
	}
	return ret;
}

INT64 ZMStream::GetPosition()
{
	return   Seek(0, FILE_CURRENT);
}

void ZMStream::SetPosition(INT64 pos)
{
	Seek(0, FILE_BEGIN);
}

INT64 ZMStream::GetSize()
{
	INT64   oldPos = Seek(0, FILE_CURRENT);
	INT64   ret = Seek(0, FILE_END);
	Seek(oldPos, FILE_BEGIN);
	return  ret;
}


ZMHandlerStream::~ZMHandlerStream()
{
	if (INVALID_HANDLE_VALUE != m_Handler)
	{
		FileClose(m_Handler);
		m_Handler = NULL;
	}
}

HANDLE ZMHandlerStream::GetHandler()
{
	return   m_Handler;
}

void ZMHandlerStream::SetSize(INT64 NewSize)
{
	Seek(NewSize, FILE_BEGIN);
	SetEndOfFile(m_Handler);
}

int ZMHandlerStream::Read(LPVOID Buffer, DWORD Count)
{
	return  FileRead(m_Handler, Buffer, Count);
}

int ZMHandlerStream::Write(const LPVOID Buffer, DWORD Count)
{
	return  FileWrite(m_Handler, Buffer, Count);
}

INT64 ZMHandlerStream::Seek(INT64 Offset, DWORD Origin)
{
	return  FileSeek(m_Handler, Offset, Origin);
}


ZMFileStream::ZMFileStream(ZMString AFileName, FileMode Mode)
{
	m_Handler = INVALID_HANDLE_VALUE;
	m_FileName = AFileName;
	if (fmCreate == (Mode& fmCreate))
	{
		FileMode   LMode = Mode & 0xFF;
		if (0xff == LMode)
			LMode = fmShareExclusive;
		m_Handler = FileCreate(m_FileName, LMode);
	}
	else
	{
		m_Handler = FileOpen(m_FileName, Mode);
	}
}

ZMFileStream::~ZMFileStream()
{

}

ZMString ZMFileStream::GetFileName()
{
	return  m_FileName;
}

bool ZMFileStream::Flush()
{
	return FALSE != FlushFileBuffers(m_Handler);
}

ZMMemoryStream::ZMMemoryStream()
{
	m_Capacity = 0;
	m_Position = 0;
	m_Size = 0;
	m_Memory = NULL;
}

ZMMemoryStream::~ZMMemoryStream()
{
	Clear();
}

void ZMMemoryStream::SetSize(INT64 NewSize)
{
	INT64   oldPos = m_Position;
	SetCapacity(NewSize);
	m_Size = NewSize;
	if (oldPos > NewSize)
		Seek(0, FILE_END);
}

int ZMMemoryStream::Read(LPVOID Buffer, DWORD Count)
{
	int   ret = -1;
	if ((m_Position >= 0) && (Count >= 0))
	{
		if ((m_Size - m_Position) > 0)
		{
			if ((m_Size > (Count + m_Position)))
			{
				ret = Count;
			}
			else
			{
				ret = (int)(m_Size - m_Position);
			}
			memcpy(Buffer, &((PBYTE)m_Memory)[m_Position], ret);
			m_Position += ret;
		}
	}
	return ret;
}

int ZMMemoryStream::Write(const LPVOID Buffer, DWORD Count)
{
	int  ret = 0;
	if ((m_Position >= 0) && (Count >= 0))
	{
		INT64 Pos = m_Position + Count;
		if (Pos > 0)
		{
			if (Pos > m_Size)
			{
				if (Pos > m_Capacity)
				{
					SetCapacity(Pos);
				}
				m_Size = Pos;
			}
			memcpy(&m_Memory[m_Position], Buffer, Count);
			m_Position = Pos;
			ret = Count;
		}
	}
	return  ret;
}

INT64 ZMMemoryStream::Seek(INT64 Offset, DWORD Origin)
{
	switch (Origin)
	{
	case FILE_BEGIN:
		m_Position = Offset;
		break;
	case FILE_CURRENT:
		m_Position += Offset;
		break;
	case FILE_END:
		m_Position = m_Size - Offset;
		break;
	default:
		break;
	}
	return m_Position;
}

bool ZMMemoryStream::LoadFromFile(ZMString AFileName)
{
	ZMFileStream  fs(AFileName, fmOpenRead | fmShareDenyWrite);
	if (fs.GetHandler() != INVALID_HANDLE_VALUE)
	{
		return LoadFromStream(fs);
	}
	return false;
}

bool ZMMemoryStream::LoadFromStream(ZMStream& AStream)
{
	AStream.SetPosition(0);
	INT64  Count = AStream.GetSize();
	SetSize(Count);
	if (0 != Count)
	{
		return Count == AStream.Read(m_Memory, (DWORD)Count);
	}
	return false;
}

bool ZMMemoryStream::SaveToFile(ZMString AFileName)
{
	ZMFileStream  fs(AFileName, fmCreate);
	if (fs.GetHandler() != INVALID_HANDLE_VALUE)
	{
		return SaveToStream(fs);
	}
	return false;
}

bool ZMMemoryStream::SaveToStream(ZMStream& AStream)
{
	if (0 != m_Size)
	{
		return AStream.Write(m_Memory, (DWORD)m_Size) == m_Size;
	}
	return false;
}

LPVOID ZMMemoryStream::GetMemory()
{
	return    m_Memory;
}

void ZMMemoryStream::Clear()
{
	SetCapacity(0);
	m_Size = 0;
	m_Position = 0;
}

void ZMMemoryStream::SetMemoryData(LPVOID ptr, INT64 Size)
{
	m_Memory = (PBYTE)ptr;
	m_Size = Size;
}

void ZMMemoryStream::SetCapacity(INT64 NewCapacity)
{
	SetMemoryData(ReallocMemory(NewCapacity), m_Size);
	m_Capacity = NewCapacity;
}

LPVOID ZMMemoryStream::ReallocMemory(INT64& NewCapacity)
{
	LPVOID  ret = m_Memory;
	if ((NewCapacity > 0) && (NewCapacity != m_Size))
	{
		NewCapacity = (NewCapacity + (MEMORYDELTA - 1))& (~(MEMORYDELTA - 1));
	}
	if (NewCapacity != m_Capacity)
	{
		if (0 == NewCapacity)
		{
			free(m_Memory);
			ret = NULL;
		}
		else
		{
			if (0 == m_Capacity)
			{
				ret = malloc((size_t)NewCapacity);
			}
			else
			{
				ret = realloc(ret, (size_t)NewCapacity);
			}
		}
	}
	return ret;
}


ZMResourceStream::ZMResourceStream(HINSTANCE instance, ZMString ResName, LPCWSTR ResType)
{
	m_Capacity = 0;
	m_Position = 0;
	m_Size = 0;
	m_Memory = NULL;
	Initialize(instance, (LPCWSTR)ResName.c_str(), ResType);
}

ZMResourceStream::ZMResourceStream(HINSTANCE instance, NativeInt ResId, LPCWSTR ResType)
{
	m_Capacity = 0;
	m_Position = 0;
	m_Size = 0;
	m_Memory = NULL;
	Initialize(instance, (LPCWSTR)(ResId), ResType);
}

ZMResourceStream::~ZMResourceStream()
{
	if (NULL != m_HGlobal)
	{
		UnlockResource(m_HGlobal);
		FreeResource(m_HGlobal);
		m_HGlobal = NULL;
	}
}

int ZMResourceStream::Write(const LPVOID Buffer, WORD Count)
{
	return  -1;
}

void ZMResourceStream::Initialize(HINSTANCE instance, LPCWSTR Name, LPCWSTR ResType)
{
	m_HResInfo = FindResource(instance, Name, ResType);
	if (NULL != m_HResInfo)
	{
		m_HGlobal = LoadResource(instance, m_HResInfo);
		if (NULL != m_HGlobal)
		{
			SetMemoryData(LockResource(m_HGlobal), SizeofResource(instance, m_HResInfo));
		}
	}
}


ZMStreamAdapter::ZMStreamAdapter(ZMStream*  lpStream, StreamOwnership AOwnership)
{
	m_Ownership = AOwnership;
	m_Stream = lpStream;
	m_RefCount = 0;
}

ZMStreamAdapter::~ZMStreamAdapter()
{
	if (soOwned == m_Ownership)
	{
		delete     m_Stream;
		m_Stream = NULL;
	}
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, _Out_opt_ ULARGE_INTEGER *plibNewPosition)
{
	if ((dwOrigin < STREAM_SEEK_SET) || (dwOrigin > STREAM_SEEK_END))
	{
		return  STG_E_INVALIDFUNCTION;
	}
	INT64  NewPos = m_Stream->Seek(dlibMove.QuadPart, dwOrigin);
	if (plibNewPosition)
	{
		plibNewPosition->QuadPart = NewPos;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::SetSize(ULARGE_INTEGER libNewSize)
{
	INT64  LPosition = m_Stream->GetPosition();
	m_Stream->SetSize(libNewSize.QuadPart);
	if (m_Stream->GetSize() < LPosition)
	{
		LPosition = m_Stream->GetSize();
	}
	m_Stream->SetPosition(LPosition);
	if (m_Stream->GetSize() != libNewSize.QuadPart)
	{
		return   E_FAIL;
	}
	else
	{
		return  S_OK;
	}
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::CopyTo(_In_ IStream *pstm, ULARGE_INTEGER cb, _Out_opt_ ULARGE_INTEGER *pcbRead, _Out_opt_ ULARGE_INTEGER *pcbWritten)
{
	HRESULT  ret = S_OK;
	const  int  BufSize = 1024;
	PBYTE   Buffer[BufSize] = { 0 };
	try
	{
		INT64  BytesRead = 0, BytesWritten = 0;
		while (cb.QuadPart > 0)
		{
			int r = 0;
			ULONG w = 0;
			r = m_Stream->Read(Buffer, BufSize);
			if (0 == r)//读取到流的最后没数据了
				return S_OK;
			BytesRead += r;
			ret = pstm->Write(Buffer, r, &w);
			BytesWritten += w;
			if ((S_OK == ret) && (((int)w) != r))
				ret = E_FAIL;
			if (ret != S_OK)
				return ret;
			cb.QuadPart -= r;
		}
	}
	catch (...)
	{
		ret = E_UNEXPECTED;
	}
	return ret;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Commit(DWORD grfCommitFlags)
{
	return  S_OK;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Revert(void)
{
	return  STG_E_REVERTED;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return  STG_E_INVALIDFUNCTION;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return  STG_E_INVALIDFUNCTION;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Stat(__RPC__out STATSTG *pstatstg, DWORD grfStatFlag)
{
	if (pstatstg)
	{
		memset(pstatstg, 0, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->cbSize.QuadPart = m_Stream->GetSize();
		pstatstg->grfLocksSupported = LOCK_WRITE;
	}
	return  S_OK;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Clone(__RPC__deref_out_opt IStream **ppstm)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Read(_Out_writes_bytes_to_(cb, *pcbRead) void *pv, _In_ ULONG cb, _Out_opt_ ULONG *pcbRead)
{
	try
	{
		if (!pv)
			return STG_E_INVALIDPOINTER;
		int  NumRead = m_Stream->Read(pv, cb);
		if (pcbRead)
		{
			*pcbRead = NumRead;
		}
		return S_OK;
	}
	catch (...)
	{
		return S_FALSE;
	}
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::Write(_In_reads_bytes_(cb) const void *pv, _In_ ULONG cb, _Out_opt_ ULONG *pcbWritten)
{
	try {
		if (!pv)
			return STG_E_INVALIDPOINTER;
		int   NumWriten = m_Stream->Write((const LPVOID)pv, cb);
		if (pcbWritten)
			*pcbWritten = NumWriten;
		return S_OK;
	}
	catch (...)
	{
		return STG_E_CANTSAVE;
	}
}

HRESULT STDMETHODCALLTYPE ZMStreamAdapter::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = (IUnknown*)this;
	}
	else if (riid == IID_ISequentialStream)
	{
		*ppvObject = (ISequentialStream*)this;
	}
	else if (riid == IID_IStream)
	{
		*ppvObject = (IStream*)this;
	}
	else
	{
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE ZMStreamAdapter::AddRef(void)
{
	return InterlockedExchangeAdd(&m_RefCount, 1);
}

ULONG STDMETHODCALLTYPE ZMStreamAdapter::Release(void)
{
	ULONG  ret = InterlockedExchangeAdd(&m_RefCount, -1);
	if (0 == m_RefCount)
	{
		delete  this;
	}
	return  ret;
}

ZMOleStream::ZMOleStream(IStream* AStream)
{
	m_IStream = AStream;
}

ZMOleStream::~ZMOleStream()
{
	m_IStream = NULL;
}

void ZMOleStream::SetSize(INT64 NewSize)
{
	//不是所有的流都支持设置大小,IStream不支持的
}

int ZMOleStream::Read(LPVOID Buffer, DWORD Count)
{
	ULONG  ret = 0;
	if (S_OK != m_IStream->Read(Buffer, Count, &ret))
		return -1;
	return ret;
}

int ZMOleStream::Write(const LPVOID Buffer, DWORD Count)
{
	ULONG  ret = 0;
	if (S_OK != m_IStream->Write((const void*)Buffer, Count, &ret))
		return -1;
	return ret;
}

INT64 ZMOleStream::Seek(INT64 Offset, DWORD Origin)
{
	INT64    ret = 0;
	LARGE_INTEGER   dlibMove;
	ULARGE_INTEGER  plibNewPosition;
	dlibMove.QuadPart = Offset;
	if (S_OK != m_IStream->Seek(dlibMove, Origin, &plibNewPosition))
	{
		//如果失败返回当前的Seek位置
		dlibMove.QuadPart = 0;
		//获取当前的Seek
		if (S_OK != m_IStream->Seek(dlibMove, FILE_CURRENT, &plibNewPosition))
		{//再失败,没天理.返回-1吧
			return -1;
		}
		return plibNewPosition.QuadPart;
	}
	return plibNewPosition.QuadPart;
}
