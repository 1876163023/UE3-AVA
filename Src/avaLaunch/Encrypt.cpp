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

// �Ϻη� �̻��� ���ڿ��� -_-;
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

	// inix reader�� �޾ƴٰ� �ؾ���
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
		���ͳ� �ͽ��÷η��� �̿��� HTTP�� ������ �ٿ�ε� �� ��
		������ ��� ���� COM �������̽� Ŭ����.
*/
class CCallbackDownload : public IBindStatusCallback
{
private:
	double		m_Wait;			//!< Ÿ�Ӿƿ�.
	double		m_StartTime;	//!< ���� �ð�.

public:
	float		fProgress;		//!< 0.0 ~ 100.0f
	ULONG		CurProgress;
	ULONG		MaxProgress;
	UBOOL		bRunning;

public:
	/*! @brief ������
		@param pURL
			�ٿ�ε� ���� Ȩ������ �ּ�.
		@param wait
			������ ��ٸ��� �ð�.(�⺻ 20��)
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
	//! �Ҹ���
	~CCallbackDownload()
	{

	}

	//! ���� ����� ����.
	float	GetProgress() const	{ return fProgress; }

public:
	/*! @brief
			���� ��Ȳ�� �˷��ֱ� ���� ���ͳ� �ͽ��÷η��� ȣ���ϰ� �Ǵ� �ݹ� �Լ�
		@param ulProgress
			�ٿ�ε���� ����Ʈ ��
		@param ulProgressMax
			�ٿ�ε�޾ƾ��� ��ü ����Ʈ ��
		@param ulStatusCode
			���� �ڵ�
		@param wszStatusText
			���� ���ڿ�
		@return
			HRESULT ���ͳ� �ͽ��÷η����� �˷�����ϴ� �����. �ٿ�ε带
			����ؾ��ϴ� ��쿡�� S_OK�� ��ȯ�ؾ��ϰ�, �ߴ��Ϸ��� E_ABORT�� ��ȯ�ؾ� 
			�Ѵ�.
	*/
	STDMETHOD(OnProgress)(
		/* [in] */ ULONG ulProgress,
		/* [in] */ ULONG ulProgressMax,
		/* [in] */ ULONG ulStatusCode,
		/* [in] */ LPCWSTR wszStatusText)
	{
		// ����ð� �ʰ�.
		if ( appSeconds() - m_StartTime > m_Wait )
			return E_ABORT;

		// ����ϴ� ���.
		if ( !bRunning )
			return E_ABORT;

//		�����ϴ� ���� �����Ͱ� ������ ���.
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
	CCallbackDownload				DownloadStatus;		//!< �ٿ�ε� ����.

	TArray<DownloadEventHandler*>	Handlers;

public:
	//! �⺻ ������.
	URLDownloader(const FString &filename, const FString &urlPrefix, const FString &localPath)
	{
		Filename = filename;
		URLPrefix = urlPrefix;
		LocalPath = localPath;
	}

	//! �ٿ�ε� ���¸� ���´�.
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
		// ::URLDownloadToFile�Լ������� block���¸� Ǯ���ش�.
		DownloadStatus.bRunning = false;
	}

	/**
	* Called in the context of the aggregating thread to perform any cleanup.
	*/
	void Exit(void)
	{
		// download�� �Ϸ� �Ǿ��ٰ� �˷��ش�.
		for(INT i = 0; i < Handlers.Num(); ++i)
			Handlers(i)->OnDownloaded(Filename, DownloadStatus.MaxProgress);
	}

	//! ����Ѵ�.
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

	//! �ٿ�ε� �Ϸ�!!
	void OnDownloaded( const FString &Filename, DWORD Size )
	{
		bDownloaded = true;

		// ������ �����ش�.
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

	// Download��� ����.
	DownloadDir = BaseDir + TEXT("\\avaGame\\Web");
}

FArchive* FAvaFileManagerWindowsInternet::CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error )
{
	// �����̸��� URL�ΰ�?
	if ( appStrnicmp(InFilename, TEXT("http"), 4) == 0 )
	{
		FString	URL       = InFilename;
		INT		LastSep   = URL.InStr(TEXT("/"), TRUE);
		FString	URLPrefix = URL.Left(LastSep);
		FString	Filename  = URL.Right( URL.Len() - (LastSep + 1) );
		FString	LocalFile = DownloadDir + TEXT("\\") + Filename;

		// ���� Local�� ������ �ִ� ��쿡��...?
		if ( FileSize( *LocalFile ) > 0 )
		{
			// ��ϵ� �ð��� ������ �ش�.
			TouchFile( *LocalFile );

			return FAvaFileManagerWindows::CreateFileReader(InFilename, Flags, Error);
		}

		FName KeyName = *Filename;

		URLDownloader *pDownloader = (URLDownloader*)DownloaderMap.FindRef(KeyName);
		// �̹� Downloading ���� ������ ���ٸ� ����.
		if ( pDownloader == NULL )
		{
			pDownloader = new URLDownloader(Filename, URLPrefix, DownloadDir);

			// ���� ������ Ŭ������ �ڱ��ڽ��� �켱 ���.
			// (�ٿ�ε尡 �Ϸ�� �Ŀ��� �˻� ��Ͽ��� ���ܵȴ�)
			pDownloader->Register( this );

			// �ٿ�ε��� ������ �˻� ��Ͽ� ����Ѵ�.
			DownloaderMap.Set(KeyName, pDownloader);

			TCHAR ThreadName[256];
			wsprintf(ThreadName, TEXT("Download %s"), ThreadName);

			FRunnableThread* pDownloadThread = GThreadFactory->CreateThread(pDownloader, ThreadName, TRUE, TRUE, 0, TPri_Normal);

			check(pDownloadThread);
		}

		// Local�� ���� ��쿡�� Internet���� Download�޴´�.
		FArchiveFileReaderWindowsInternet* pArc = new FArchiveFileReaderWindowsInternet(*LocalFile, Error);

		// ������ ����� �ش�.
		pDownloader->Register( pArc );

		// TotalSize()�� ���� INDEX_NONE�� �ƴ� ������ ������� ���Ѵ�.
		return pArc;
	}

	return FAvaFileManagerWindows::CreateFileReader(InFilename, Flags, Error);
}

void FAvaFileManagerWindowsInternet::OnDownloaded( const FString &Filename, DWORD Size )
{
	FName KeyName = *Filename;

	// ��ϵ� �ش� ���Ͽ� ���� Downloader�� �����Ѵ�.
	DownloaderMap.Remove(KeyName);
}
