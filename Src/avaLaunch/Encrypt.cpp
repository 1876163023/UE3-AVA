#include "avaLaunch.h"
#include "Encrypt.h"
#include "Blowfish.h"

#define ENCRYPT_INI 1

#if FINAL_RELEASE
#define USE_ENCRYPTED_INI 1
#endif

static INT Magic = 0x892f;

const TCHAR* GSupportedLanguages[] =
{
	TEXT("INT"),
	TEXT("KOR"),
};

// 일부러 이상한 문자열로 -_-;
char EncryptKey[] = "AsyncFileCache";

FAvaFileManagerWindowsInternet				FileManager;

class FEncryptedFileWriter : public FArchive
{
public :
	FArchive* Relay;
	void* Buffer;
	INT Alloc;
	INT Bytes;
	INT CurPos;

	FEncryptedFileWriter( FArchive* InRelay )
		: Relay( InRelay ), Bytes(0), Alloc(0), Buffer(NULL), CurPos(0)
	{			
	}

	~FEncryptedFileWriter()
	{
		if (Buffer)
		{
			Close();
		}
	}		

	virtual void Seek( INT InPos )
	{
		CurPos = Clamp( InPos, 0, Bytes - 1 );
	}

	virtual INT Tell()
	{
		return CurPos;
	}

	void WriteDown();

	virtual UBOOL Close()
	{
		if (Bytes)
		{
			WriteDown();
			appFree( Buffer );
			Buffer = NULL;
		}

		UBOOL bRet = Relay->Close();
		delete Relay;			

		return bRet;
	}

	virtual void Serialize( void* V, INT Length )
	{
		Buffer = appRealloc( Buffer, Alloc = Max( Alloc, CurPos + Length + 4096 ) );

		appMemcpy( (char*)Buffer + CurPos, V, Length );
		CurPos += Length;

		Bytes = Max( Bytes, CurPos );
	}

	virtual void Flush()
	{

	}
};

void FAvaFileManagerWindows::FindFiles( TArray<FString>& FileNames, const TCHAR* Filename, UBOOL Files, UBOOL Directories )
{
	TArray<FString> Temp;
	TArray<FString> Result;
	FFileManagerWindows::FindFiles( Temp, Filename, Files, Directories );

	for (INT i=0; i<Temp.Num(); ++i)
	{		
		FFilename Filename( Temp(i) );

		INT Chop = Max( 0, Filename.GetExtension().Len() - 3 );

		Result.AddUniqueItem( FFilename( Temp(i).Left( Temp(i).Len() - Chop ) ) );
	}

	FileNames.Append( Result );
}

FArchive* FAvaFileManagerWindows::CreateEncryptedFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error )
{
	FArchive* Raw = FFileManagerWindows::CreateFileReader( InFilename, Flags, Error );

#if ENCRYPT_INI
	if (!Raw)
		return NULL;

	INT Key;
	*Raw << Key;

	if (Key != Magic)
	{
		Raw->Close();
		delete Raw;

		return NULL;
	}

	INT UncompressedSize;
	*Raw << UncompressedSize;

	void* Data = appMalloc( UncompressedSize );
	Raw->SerializeCompressed( Data, UncompressedSize, COMPRESS_LZO );
	delete Raw;	

	Blowfish Blowfish;
	Blowfish.Set_Passwd( EncryptKey );
	Blowfish.Decrypt( (BYTE*)Data, UncompressedSize );	

	return new FBufferReader( Data, UncompressedSize, TRUE );
#else
	return Raw;
#endif
}	

FArchive* CreateEncryptedFileWriter( FArchive* Relay )
{
	return new FEncryptedFileWriter( Relay );
}

void Encrypt( FArchive* Out, FArchive* Raw )
{
	INT UncompressedSize = Raw->TotalSize(), CompressedSize = UncompressedSize + 1024;

	void* Data = appMalloc( UncompressedSize );
	Raw->Serialize( Data, UncompressedSize );		

#if ENCRYPT_INI
	void* Playground = appMalloc( UncompressedSize );
	Blowfish Blowfish;
	Blowfish.Set_Passwd( EncryptKey );
	Blowfish.Encrypt( (BYTE*)Data, UncompressedSize );	
	*Out << Magic;
	*Out << UncompressedSize;
	Out->SerializeCompressed( Data, UncompressedSize, COMPRESS_LZO );			
	appFree( Data );
#else
	Out->Serialize( Data, UncompressedSize );		
#endif		
}

void FAvaFileManagerWindows::Encrypt( const TCHAR* InFilename, const TCHAR* InSrcFilename, DWORD Flags, FOutputDevice* Error )
{
	warnf(NAME_Warning, TEXT("Encrypting %s into %s"), InSrcFilename, InFilename );

	FArchive* Raw = FFileManagerWindows::CreateFileReader( InSrcFilename, Flags, Error );
	if (!Raw)
		return;

	FArchive* Out = FFileManagerWindows::CreateFileWriter( InFilename, Flags, Error );
	if (Out)
	{
		::Encrypt( Out, Raw );
		delete Out;
	}

	delete Raw;		
}

static UBOOL CheckExtension( const TCHAR* Filename, const TCHAR* Extension )
{
	return appStristr( Filename, *FString::Printf( TEXT(".%s"), Extension ) ) != NULL;
}

static UBOOL CheckExtensionX( const TCHAR* Filename, const TCHAR* Extension )
{
	return appStristr( Filename, *FString::Printf( TEXT(".%sx"), Extension ) ) != NULL;
}

UBOOL IsPureIni( const TCHAR* InFilename )
{
	if ( appStristr(InFilename, TEXT("notice")) != NULL )
		return TRUE;

	return appStristr( InFilename, TEXT("AVAOptionSettings.ini" )) != NULL;
}

UBOOL IsGeneratedIni( const TCHAR* InFilename )
{
	return appStristr( InFilename, TEXT("\\AVA.ini" )) != NULL;
}

UBOOL IsEncryptedIni( const TCHAR* InFilename )
{		
	if (CheckExtension( InFilename, TEXT("ini") ) && !CheckExtensionX( InFilename, TEXT("ini") ) && !IsPureIni( InFilename )) 
		return TRUE;

	for (INT i=0; i<ARRAY_COUNT(GSupportedLanguages); ++i)
	{
		if (CheckExtension( InFilename, GSupportedLanguages[i]) && !CheckExtensionX( InFilename, GSupportedLanguages[i] ) && !IsPureIni( InFilename ))
			return TRUE;
	}		

	return FALSE;
}

FArchive* FAvaFileManagerWindows::CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error )
{
	if (!IsEncryptedIni( InFilename ))
		return FFileManagerWindows::CreateFileReader( InFilename, Flags, Error );

	FString EncryptedFilename = FString::Printf( TEXT("%sx"), InFilename );

#if USE_ENCRYPTED_INI
	return CreateEncryptedFileReader( *EncryptedFilename, Flags, Error );
#else
	return FFileManagerWindows::CreateFileReader( InFilename, Flags, Error );
#endif
}

FArchive* FAvaFileManagerWindows::CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error )
{
	if (!IsEncryptedIni( Filename ))
		return FFileManagerWindows::CreateFileWriter( Filename, Flags, Error );

#if USE_ENCRYPTED_INI
	FString EncryptedFilename = FString::Printf( TEXT("%sx"), Filename );

	return CreateEncryptedFileWriter( FFileManagerWindows::CreateFileWriter( *EncryptedFilename, Flags, Error ) );
#endif
	return FFileManagerWindows::CreateFileWriter( Filename, Flags, Error );
}

INT FAvaFileManagerWindows::FileSize( const TCHAR* Filename )
{
	if (!IsEncryptedIni( Filename ))
		return FFileManagerWindows::FileSize( Filename );

	// inix reader를 받아다가 해야함
	return FFileManagerGeneric::FileSize( Filename );		
}

void FEncryptedFileWriter::WriteDown()
{
	FBufferReader Reader( Buffer, Bytes, FALSE );

	::Encrypt( Relay, &Reader );			
}

BEGIN_COMMANDLET(EncryptIni,avaGame)
END_COMMANDLET

void EncryptIni( FString InFilename )
{
	FString EncryptedFilename = FString::Printf( TEXT("%sx"), *InFilename );

	FileManager.Encrypt( *EncryptedFilename, *InFilename, 0, NULL);
}

IMPLEMENT_CLASS(UEncryptIniCommandlet);

INT UEncryptIniCommandlet::Main( const FString& Params )
{	
	EncryptIni( appGameDir() + TEXT("Config\\DefaultEngine.ini"));
	EncryptIni( appGameDir() + TEXT("Config\\DefaultGame.ini"));
	EncryptIni( appGameDir() + TEXT("Config\\DefaultGPU.ini"));
	EncryptIni( appGameDir() + TEXT("Config\\DefaultInput.ini"));
	EncryptIni( appGameDir() + TEXT("Config\\DefaultOptionSettings.ini"));		
	EncryptIni( appGameDir() + TEXT("Config\\DefaultNet.ini"));
	EncryptIni( appGameDir() + TEXT("..\\Engine\\Config\\BaseEngine.ini"));
	EncryptIni( appGameDir() + TEXT("..\\Engine\\Config\\BaseGame.ini"));
	EncryptIni( appGameDir() + TEXT("..\\Engine\\Config\\BaseInput.ini"));				

	const TCHAR* Localization_Engine[] =
	{
		TEXT("Core"),
		TEXT("Descriptions"),
		TEXT("Editor"),
		TEXT("EditorTips"),
		TEXT("Engine"),
		TEXT("Launch"),
		TEXT("Startup"),
		TEXT("UnrealEd"),		
		TEXT("WinDrv"),
		TEXT("XWindow"),		
	};

	const TCHAR* Localization_AVA[] =
	{
		TEXT("avaGame"),
		TEXT("avaNet"),		
		TEXT("avaNetEx"),
		TEXT("Launch"),
		TEXT("avaRules"),
		TEXT("Descriptions"),
		TEXT("UnrealEd")
	};	

	for (INT j=0; j<ARRAY_COUNT(GSupportedLanguages); ++j)
	{
		const TCHAR* Lang = GSupportedLanguages[j];

		for (INT i=0; i<ARRAY_COUNT(Localization_AVA); ++i)
		{
			EncryptIni( appGameDir() + *FString::Printf( TEXT("..\\avaGame\\Localization\\%s\\%s.%s"), Lang, Localization_AVA[i], Lang ) );
		}

		for (INT i=0; i<ARRAY_COUNT(Localization_Engine); ++i)
		{
			EncryptIni( appGameDir() + *FString::Printf( TEXT("..\\Engine\\Localization\\%s\\%s.%s"), Lang, Localization_Engine[i], Lang ) );
		}
	}	


	return 0;
}

void AutoGenerateNamesEncrypt()
{
	UEncryptIniCommandlet::StaticClass();
}


//-----------------------------------------------------------------------------
//	class CCallbackDownload
//-----------------------------------------------------------------------------

/*! @brief
		인터넷 익스플로러를 이용해 HTTP로 파일을 다운로드 할 때
		정보를 얻기 위한 COM 인터페이스 클래스.
*/
class CCallbackDownload : public IBindStatusCallback
{
private:
	double		m_Wait;			//!< 타임아웃.
	double		m_StartTime;	//!< 시작 시간.

public:
	float		fProgress;		//!< 0.0 ~ 100.0f
	ULONG		CurProgress;
	ULONG		MaxProgress;
	UBOOL		bRunning;

public:
	/*! @brief 생성자
		@param pURL
			다운로드 받을 홈페이지 주소.
		@param wait
			응답을 기다리는 시간.(기본 20초)
	*/
	CCallbackDownload(double WaitTime=20.0)
	{
		m_Wait      = WaitTime;
		m_StartTime = appSeconds();

		fProgress   = 0.0f;
		CurProgress = 0;
		MaxProgress = 0;
		bRunning    = true;
	}
	//! 소멸자
	~CCallbackDownload()
	{

	}

	//! 현재 진행된 비율.
	float	GetProgress() const	{ return fProgress; }

public:
	/*! @brief
			진행 상황을 알려주기 위해 인터넷 익스플로러가 호출하게 되는 콜백 함수
		@param ulProgress
			다운로드받은 바이트 수
		@param ulProgressMax
			다운로드받아야할 전체 바이트 수
		@param ulStatusCode
			상태 코드
		@param wszStatusText
			상태 문자열
		@return
			HRESULT 인터넷 익스플로러에게 알려줘야하는 결과값. 다운로드를
			계속해야하는 경우에는 S_OK를 반환해야하고, 중단하려면 E_ABORT를 반환해야 
			한다.
	*/
	STDMETHOD(OnProgress)(
		/* [in] */ ULONG ulProgress,
		/* [in] */ ULONG ulProgressMax,
		/* [in] */ ULONG ulStatusCode,
		/* [in] */ LPCWSTR wszStatusText)
	{
		// 응답시간 초과.
		if ( appSeconds() - m_StartTime > m_Wait )
			return E_ABORT;

		// 취소하는 경우.
		if ( !bRunning )
			return E_ABORT;

//		참조하는 파일 포인터가 없더라도 취소.
//		if ( URLDownloader::Handlers.Num() == 1 )
//			return E_ABORT;

		CurProgress = ulProgress;
		MaxProgress = ulProgressMax;

		if ( ulProgressMax != 0 )
			fProgress = (float)CurProgress / MaxProgress * 100.0f;

		return S_OK;
	}

	STDMETHOD(OnStartBinding)(
		/* [in] */ DWORD dwReserved,
		/* [in] */ IBinding __RPC_FAR *pib)
	{ return E_NOTIMPL; }

	STDMETHOD(GetPriority)(
		/* [out] */ LONG __RPC_FAR *pnPriority)
	{ return E_NOTIMPL; }

	STDMETHOD(OnLowResource)(
		/* [in] */ DWORD reserved)
	{ return E_NOTIMPL; }

	STDMETHOD(OnStopBinding)(
		/* [in] */ HRESULT hresult,
		/* [unique][in] */ LPCWSTR szError)
	{ return E_NOTIMPL; }

	STDMETHOD(GetBindInfo)(
		/* [out] */ DWORD __RPC_FAR *grfBINDF,
		/* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo)
	{ return E_NOTIMPL; }

	STDMETHOD(OnDataAvailable)(
		/* [in] */ DWORD grfBSCF,
		/* [in] */ DWORD dwSize,
		/* [in] */ FORMATETC __RPC_FAR *pformatetc,
		/* [in] */ STGMEDIUM __RPC_FAR *pstgmed)
	{ return E_NOTIMPL; }

	STDMETHOD(OnObjectAvailable)(
		/* [in] */ REFIID riid,
		/* [iid_is][in] */ IUnknown __RPC_FAR *punk)
	{ return E_NOTIMPL; }

	STDMETHOD_(ULONG, AddRef)() { return 0; }
	STDMETHOD_(ULONG, Release)() { return 0; }

	STDMETHOD(QueryInterface)(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{ return E_NOTIMPL; }
};


//-----------------------------------------------------------------------------
//	class URLDownloader
//-----------------------------------------------------------------------------
class URLDownloader : public FRunnable
{
protected:
	FString							Filename;
	FString							URLPrefix;
	FString							LocalPath;
	CCallbackDownload				DownloadStatus;		//!< 다운로드 상태.

	TArray<DownloadEventHandler*>	Handlers;

public:
	//! 기본 생성자.
	URLDownloader(const FString &filename, const FString &urlPrefix, const FString &localPath)
	{
		Filename = filename;
		URLPrefix = urlPrefix;
		LocalPath = localPath;
	}

	//! 다운로드 상태를 얻어온다.
	const CCallbackDownload*	GetDownloadStatus() const	{ return &DownloadStatus; }

	/**
	* Allows per runnable object initialization. NOTE: This is called in the
	* context of the thread object that aggregates this, not the thread that
	* passes this runnable to a new thread.
	*
	* @return True if initialization was successful, false otherwise
	*/
	UBOOL Init(void)
	{
		return TRUE;
	}

	/**
	* This is where all per object thread work is done. This is only called
	* if the initialization was successful.
	*
	* @return The exit code of the runnable object
	*/
	DWORD Run(void)
	{
		FString urlFile   = URLPrefix + TEXT("/") + Filename;
		FString localFile = LocalPath + TEXT("\\") + Filename;

		return (::URLDownloadToFile(NULL, *urlFile, *localFile, 0, &DownloadStatus) == S_OK);
	}

	/**
	* This is called if a thread is requested to terminate early
	*/
	void Stop(void)
	{
		// ::URLDownloadToFile함수에서의 block상태를 풀어준다.
		DownloadStatus.bRunning = false;
	}

	/**
	* Called in the context of the aggregating thread to perform any cleanup.
	*/
	void Exit(void)
	{
		// download가 완료 되었다고 알려준다.
		for(INT i = 0; i < Handlers.Num(); ++i)
			Handlers(i)->OnDownloaded(Filename, DownloadStatus.MaxProgress);
	}

	//! 등록한다.
	void Register(DownloadEventHandler *pHandler)
	{
		Handlers.AddItem(pHandler);
	}
};


//-----------------------------------------------------------------------------
//	class FArchiveFileReaderWindowsInternet
//-----------------------------------------------------------------------------
class FArchiveFileReaderWindowsInternet : public FArchive, public DownloadEventHandler
{
protected:
	FString			Filename;
	FOutputDevice*	Error;
	FArchive*		pArchive;
	UBOOL			bDownloaded;

public:
	FArchiveFileReaderWindowsInternet(const TCHAR* InFilename, FOutputDevice* InError)
		: Filename(InFilename), Error(InError)
	{
		pArchive    = NULL;
		bDownloaded = false;
	}

	//! 다운로드 완료!!
	void OnDownloaded( const FString &Filename, DWORD Size )
	{
		bDownloaded = true;

		// 파일을 열어준다.
		pArchive = FileManager.CreateFileReader( *Filename, 0, GNull );
	}

	void Seek( INT InPos )
	{
		if ( !bDownloaded )
			return;

		pArchive->Seek( InPos );
	}

	INT Tell()
	{
		if ( !bDownloaded )
			return INDEX_NONE;

		return pArchive->Tell();
	}

	INT TotalSize()
	{
		if ( !bDownloaded )
			return INDEX_NONE;

		return pArchive->TotalSize();
	}

	void Serialize( void* V, INT Length )
	{
		if ( !bDownloaded )
			return ;

		pArchive->Serialize(V, Length);
	}
};


//-----------------------------------------------------------------------------
//	class FAvaFileManagerWindowsInternet
//-----------------------------------------------------------------------------
FAvaFileManagerWindowsInternet::FAvaFileManagerWindowsInternet()
{
	FString BinDir  = appBaseDir();
	FString BaseDir = BinDir.Left(BinDir.InStr(TEXT("\\Binaries"), TRUE));

	// Download경로 설정.
	DownloadDir = BaseDir + TEXT("\\avaGame\\Web");
}

FArchive* FAvaFileManagerWindowsInternet::CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error )
{
	// 파일이름이 URL인가?
	if ( appStrnicmp(InFilename, TEXT("http"), 4) == 0 )
	{
		FString	URL       = InFilename;
		INT		LastSep   = URL.InStr(TEXT("/"), TRUE);
		FString	URLPrefix = URL.Left(LastSep);
		FString	Filename  = URL.Right( URL.Len() - (LastSep + 1) );
		FString	LocalFile = DownloadDir + TEXT("\\") + Filename;

		// 만약 Local에 파일이 있는 경우에는...?
		if ( FileSize( *LocalFile ) > 0 )
		{
			// 기록된 시간을 갱신해 준다.
			TouchFile( *LocalFile );

			return FAvaFileManagerWindows::CreateFileReader(InFilename, Flags, Error);
		}

		FName KeyName = *Filename;

		URLDownloader *pDownloader = (URLDownloader*)DownloaderMap.FindRef(KeyName);
		// 이미 Downloading 중인 파일이 없다면 생성.
		if ( pDownloader == NULL )
		{
			pDownloader = new URLDownloader(Filename, URLPrefix, DownloadDir);

			// 파일 관리자 클래스인 자기자신은 우선 등록.
			// (다운로드가 완료된 후에는 검색 목록에서 제외된다)
			pDownloader->Register( this );

			// 다운로드할 파일을 검색 목록에 등록한다.
			DownloaderMap.Set(KeyName, pDownloader);

			TCHAR ThreadName[256];
			wsprintf(ThreadName, TEXT("Download %s"), ThreadName);

			FRunnableThread* pDownloadThread = GThreadFactory->CreateThread(pDownloader, ThreadName, TRUE, TRUE, 0, TPri_Normal);

			check(pDownloadThread);
		}

		// Local에 없는 경우에만 Internet에서 Download받는다.
		FArchiveFileReaderWindowsInternet* pArc = new FArchiveFileReaderWindowsInternet(*LocalFile, Error);

		// 파일을 등록해 준다.
		pDownloader->Register( pArc );

		// TotalSize()의 값이 INDEX_NONE이 아닐 때까지 사용하지 못한다.
		return pArc;
	}

	return FAvaFileManagerWindows::CreateFileReader(InFilename, Flags, Error);
}

void FAvaFileManagerWindowsInternet::OnDownloaded( const FString &Filename, DWORD Size )
{
	FName KeyName = *Filename;

	// 등록된 해당 파일에 대한 Downloader를 제거한다.
	DownloaderMap.Remove(KeyName);
}
