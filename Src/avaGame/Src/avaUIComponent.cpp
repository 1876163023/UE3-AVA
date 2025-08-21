#include "PrecompiledHeaders.h"
#include "avaGame.h"
#include "FConfigCacheIni.h"

IMPLEMENT_CLASS(UavaUIComp_DrawCustomImage);
//	IMPLEMENT_CLASS(UavaUINoticeLoader);
//	IMPLEMENT_CLASS(UavaUINoticePatcher);

//-----------------------------------------------------------------------------
// PNG Loading Class
//-----------------------------------------------------------------------------

#pragma pack(push,1)

struct FPNGHeader
{
	BYTE	Signature[8];	//!< 0x89504E470D0A1A0A.

	DWORD	ChunkLength;	//!< 13 bytes.
	BYTE	IHDR[4];		//!< "IHDR".

	int		Width;			//!< width of image in pixels.
	int		Height;			//!< height of image in pixels.
	BYTE	BitDepth;		//!< 1, 2, 4, 8, or 16 bits/channel.(8 bit만 사용할 예정)
	BYTE	ColorType;		//!< see PNG_COLOR_TYPE_ below.(PNG_COLOR_TYPE_RGB만 사용할 예정)
	BYTE	Compression;	//!< must be PNG_COMPRESSION_TYPE_BASE.
	BYTE	Filter;			//!< must be PNG_FILTER_TYPE_BASE.
	BYTE	Interlace;		//!< One of PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7.

	static int ConvertBigEndian(DWORD value)
	{
		DWORD tmp;
		tmp  = (value&0x000000FF) << 24;
		tmp |= (value&0x0000FF00) << 8;
		tmp |= (value&0x00FF0000) >> 8;
		tmp |= (value&0xFF000000) >> 24;

		return (int)tmp;
	}
};

#pragma pack(pop)

UTexture2D* ImportPNG
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	const FString&		CurrentFilename,
	UObject*			Context,
	const TCHAR*		Type,
	const BYTE*&		Buffer,
	const BYTE*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	// if the texture already exists, remember the user settings
	UTexture2D* ExistingTexture = FindObject<UTexture2D>( InParent, *Name.ToString() );

	BYTE			ExistingAddressX	= TA_Wrap;
	BYTE			ExistingAddressY	= TA_Wrap;
	UBOOL			ExistingCompressionFullDynamicRange = FALSE;
	BYTE			ExistingFilter		= TF_Linear;
	BYTE			ExistingLODGroup	= TEXTUREGROUP_UI;
	INT				ExistingLODBias		= 0;
	UBOOL			ExistingNeverStream = FALSE;
	UBOOL			ExistingSRGB		= FALSE;
	FLOAT			ExistingUnpackMin[4];
	FLOAT			ExistingUnpackMax[4];

	int				ExistingSizeX = 0;
	int				ExistingSizeY = 0;
	TArray<BYTE>	ExistingRawData;

	// PNG인 경우 GrayScale일 경우 RGB값을 저장한다.(그래서 위쪽에서 선언)
	const FPNGHeader* PNG = (FPNGHeader *)Buffer;

	// Validate it.
	INT Length = BufferEnd - Buffer;

	// 헤더 크기보다 작거나, PNG파일 포멧이 아닌 경우 리턴 실패.
	if ( Length < sizeof(FPNGHeader) || appMemcmp(PNG->IHDR, "IHDR", 4) != 0 )
	{
		// Unknown format.
		Warn->Logf( NAME_Error, TEXT("Bad png image format for texture import") );
		return NULL;
	}

	// 이미 존재하는 경우.
	if (ExistingTexture)
	{
		// save settings
		ExistingAddressX	= ExistingTexture->AddressX;
		ExistingAddressY	= ExistingTexture->AddressY;
		ExistingCompressionFullDynamicRange = ExistingTexture->CompressionFullDynamicRange;
		ExistingFilter		= ExistingTexture->Filter;
		ExistingLODGroup	= ExistingTexture->LODGroup;
		ExistingLODBias		= ExistingTexture->LODBias;
		ExistingNeverStream = ExistingTexture->NeverStream;
		ExistingSRGB		= ExistingTexture->SRGB;
		appMemcpy(ExistingUnpackMin,ExistingTexture->UnpackMin,sizeof(ExistingUnpackMin));
		appMemcpy(ExistingUnpackMax,ExistingTexture->UnpackMax,sizeof(ExistingUnpackMax));

		//{ 2007/04/04 고광록.
		{
			// 8-bit Grayscale인 경우라면 그전에 로딩된 텍스쳐 RGB 정보를 저장.
			if ( PNG->BitDepth == 8 && PNG->ColorType == PNG_COLOR_TYPE_GRAY )
			{
				ExistingSizeX = ExistingTexture->SizeX;
				ExistingSizeY = ExistingTexture->SizeY;

				// RGB데이터 저장.
				ExistingRawData.Empty(ExistingSizeX * ExistingSizeY * 4);
				ExistingRawData.Add(ExistingSizeX * ExistingSizeY * 4);

				void *pDest = (void*)&ExistingRawData(0);
				void *pSrc;
				if( (pSrc = ExistingTexture->Mips(0).Data.Lock(LOCK_READ_ONLY)) != NULL )
				{
					appMemcpy(pDest, pSrc, ExistingSizeX * ExistingSizeY * 4);
					ExistingTexture->Mips(0).Data.Unlock();
				}
			}
		}
		//}
	}

	UTexture2D* Texture = NULL;

	{
		// png에서 little endian이라 변환해 줘야 하더라.
		int pngWidth  = PNG->ConvertBigEndian((DWORD)PNG->Width);
		int pngHeight = PNG->ConvertBigEndian((DWORD)PNG->Height);

		// Validate the width and height are powers of two
		if (pngWidth & (pngWidth - 1))
		{
			Warn->Logf( NAME_Error, TEXT("Can't import non-power of two texture width (%i)"),pngWidth);
			return NULL;
		}
		if (pngHeight & (pngHeight - 1))
		{
			Warn->Logf( NAME_Error, TEXT("Can't import non-power of two texture height (%i)"),pngHeight);
			return NULL;
		}
		if ( PNG->BitDepth != 8 )
		{
			Warn->Logf( NAME_Error, TEXT("Can't import texture (%d) bit-depth"),PNG->BitDepth);
			return NULL;
		}

		// RGB가 아니면 무조건 리턴.
		if ( PNG->ColorType != PNG_COLOR_TYPE_RGB && 
			 PNG->ColorType != PNG_COLOR_TYPE_GRAY &&
			 PNG->ColorType != PNG_COLOR_TYPE_RGB_ALPHA )
			return NULL;

		// PNG->ColorType - PNG_COLOR_TYPE_RGB인 경우에는 RGB만 채우고,
		//					PNG_COLOR_TYPE_GRAY인 경우에는 기존의 텍스쳐에 Alpha만 채운다.
		if ( PNG->ColorType == PNG_COLOR_TYPE_RGB )
		{
			Texture = CastChecked<UTexture2D>(UObject::StaticConstructObject(Class,InParent,Name,Flags));
			if ( Texture == NULL )
				return NULL;

			Texture->Init(pngWidth,pngHeight,PF_A8R8G8B8);

			FColor *pDest;

			if ( (pDest = (FColor*)Texture->Mips(0).Data.Lock(LOCK_READ_WRITE)) != NULL )
			{
				// PNG에서 Alpha정보만 갱신한다.
				{
					FPNGHelper PNGHelper;
					PNGHelper.InitCompressed( (void*)Buffer, Length, pngWidth, pngHeight );
					TArray<BYTE> RawData = PNGHelper.GetRawData();

					for(INT y = 0; y < pngHeight; y++)
					{
						BYTE *pRGB = &RawData(0) + pngWidth * y * 4;

						for(INT x = 0; x < pngWidth; x++)
						{
							*pDest = FColor(pRGB[0], pRGB[1], pRGB[2]);

							pDest++;
							pRGB+= 3;
						}
					}
				}

				Texture->Mips(0).Data.Unlock();
			}
		}
		else if( PNG->ColorType == PNG_COLOR_TYPE_GRAY )
		{
			// 기존의 텍스쳐가 없다거나 크기가 다르다면 실패처리.
			if ( (ExistingRawData.Num() == 0) || (ExistingSizeX != pngWidth) || (ExistingSizeY != pngHeight) )
				return NULL;

			Texture = CastChecked<UTexture2D>(UObject::StaticConstructObject(Class,InParent,Name,Flags));
			if ( Texture == NULL )
				return NULL;

			Texture->Init(pngWidth,pngHeight,PF_A8R8G8B8);

			FColor *pDest;

			if ( (pDest = (FColor*)Texture->Mips(0).Data.Lock(LOCK_READ_WRITE)) != NULL )
			{
				// PNG에서 Alpha정보만 갱신한다.
				{
					FPNGHelper PNGHelper;
					PNGHelper.InitCompressed( (void*)Buffer, Length, pngWidth, pngHeight );
					TArray<BYTE> RawData = PNGHelper.GetRawData();

					for(INT y = 0; y < pngHeight; y++)
					{
						BYTE*	pA   = &RawData(0) + pngWidth * y * 4;
						FColor*	pRGB = (FColor*)(&ExistingRawData(0) + pngWidth * y * 4);

						for(INT x = 0; x < pngWidth; x++)
						{
							(*pDest) = FColor((*pRGB).R, (*pRGB).G, (*pRGB).B, *pA);

							pDest++;
							pRGB++;
							pA++;
						}
					}
				}

				Texture->Mips(0).Data.Unlock();
			}
		}
		else if( PNG->ColorType == PNG_COLOR_TYPE_RGB_ALPHA )
		{
			Texture = CastChecked<UTexture2D>(UObject::StaticConstructObject(Class,InParent,Name,Flags));
			if ( Texture == NULL )
				return NULL;

			Texture->Init(pngWidth,pngHeight,PF_A8R8G8B8);

			FColor *pDest;

			if ( (pDest = (FColor*)Texture->Mips(0).Data.Lock(LOCK_READ_WRITE)) != NULL )
			{
				// PNG에서 Alpha정보만 갱신한다.
				{
					FPNGHelper PNGHelper;
					PNGHelper.InitCompressed( (void*)Buffer, Length, pngWidth, pngHeight );
					TArray<BYTE> RawData = PNGHelper.GetRawData();

					for(INT y = 0; y < pngHeight; y++)
					{
						BYTE *pRGB = &RawData(0) + pngWidth * y * 4;

						for(INT x = 0; x < pngWidth; x++)
						{
							*pDest = FColor(pRGB[0], pRGB[1], pRGB[2], pRGB[3]);

							pDest++;
							pRGB+= 4;
						}
					}
				}

				Texture->Mips(0).Data.Unlock();
			}
		}
		else
		{
			Warn->Logf( NAME_Error, TEXT("Bad image format for texture import") );
			return NULL;
		}
	}

	// Propagate options.

	Texture->CompressionSettings	= TC_Default;
	Texture->CompressionNone		= FALSE;
	Texture->CompressionNoAlpha		= FALSE;
	Texture->DeferCompression		= FALSE;

	Texture->SourceFilePath         = CurrentFilename;
	Texture->SourceFileTimestamp.Empty();
	FFileManager::timestamp Timestamp;
	if (GFileManager->GetTimestamp( *CurrentFilename, Timestamp ))
	{
		Texture->SourceFileTimestamp = FString::Printf(TEXT("%04d-%02d-%02d %02d:%02d:%02d"), Timestamp.Year, Timestamp.Month+1, Timestamp.Day, Timestamp.Hour, Timestamp.Minute, Timestamp.Second );        
	}

	Texture->LODGroup = TEXTUREGROUP_UI;

	// Restore user set options
	if (ExistingTexture)
	{
		Texture->AddressX		= ExistingAddressX;
		Texture->AddressY		= ExistingAddressY;
		Texture->CompressionFullDynamicRange = ExistingCompressionFullDynamicRange;
		Texture->Filter			= ExistingFilter;
		Texture->LODGroup		= ExistingLODGroup;
		Texture->LODBias		= ExistingLODBias;
		Texture->NeverStream	= ExistingNeverStream;
		Texture->SRGB			= ExistingSRGB;
		appMemcpy(Texture->UnpackMin,ExistingUnpackMin,sizeof(ExistingUnpackMin));
		appMemcpy(Texture->UnpackMax,ExistingUnpackMax,sizeof(ExistingUnpackMax));
	}

//! Editor에서만 필요하다.(원본 데이터 저장용이라고 한다)
/*
	// Compress RGBA textures and also store source art.
	if( Texture->Format == PF_A8R8G8B8 )
	{
		// PNG Compress.
		FPNGHelper PNG;
		PNG.InitRaw( Texture->Mips(0).Data.Lock(LOCK_READ_ONLY), Texture->Mips(0).Data.GetBulkDataSize(), Texture->SizeX, Texture->SizeY );
		TArray<BYTE> CompressedData = PNG.GetCompressedData();
		Texture->Mips(0).Data.Unlock();
		check( CompressedData.Num() );

		// Store source art.
		Texture->SourceArt.Lock(LOCK_READ_WRITE);
		void* SourceArtPointer = Texture->SourceArt.Realloc( CompressedData.Num() );
		appMemcpy( SourceArtPointer, CompressedData.GetData(), CompressedData.Num() );
		Texture->SourceArt.Unlock();

		// PostEditChange below will automatically recompress.
	}
	else
*/	{
		Texture->CompressionNone = 1;
	}

	// Invalidate any materials using the newly imported texture. (occurs if you import over an existing texture)
	Texture->PostEditChange(NULL);

	return Texture;
}


//-----------------------------------------------------------------------------
// Download Class.
//-----------------------------------------------------------------------------

//! Download 관리자.
class FDownloadManager
{
public:
	enum EState
	{
		State_Downloading			= 0,
		State_Downloaded			= 1,
		State_Dummy0				= 2,
		State_Downloaded_Update		= 3,
		State_Complete				= 4,
		State_Max
	};

	// 디버그용.
	static TCHAR *GDownloaderManagerStates[State_Max];

protected:
	TMap<FName, UINT>	NameMap;

public:
	//! 다운로드 할 Ini파일 이름을 등록한다.
	UBOOL	Register(const FString &name)
	{
		UINT *pState = NULL;

		// 이미 등록 되었다면 실패 리턴.
		if ( (pState = NameMap.Find(*name)) != NULL )
			return FALSE;

		// 등록
		NameMap.Set(*name, State_Downloading);

		return TRUE;
	}

	//! 이미 등록된 Ini파일인가?
	UINT*	IsRegistered(const FString &name)
	{
		return NameMap.Find(*name);
	}

	//! 해당 Ini파일에 대한 다운로드 상태 설정.
	UBOOL	SetState(const FString &name, EState eState)
	{
		UINT *pState = NameMap.Find(*name);
		if ( pState == NULL )
			return FALSE;

		*pState = eState;

		return TRUE;
	}

	//! 상태에 해당하는 문자열을 얻어온다.
	static const TCHAR*	GetStateStr(UINT eState)
	{
		return GDownloaderManagerStates[eState];
	}

} GDownloadManager;

//! 디버그용이다.
TCHAR *FDownloadManager::GDownloaderManagerStates[] = 
{
	TEXT("State_Downloading"),
	TEXT("State_Downloaded"),
	TEXT(""),
	TEXT("State_Downloaded_Update"),
	TEXT("State_Complete"),
};

/*! @brief
		인터넷 익스플로러를 이용해 HTTP로 파일을 다운로드받기 위한 COM 인터페이스 클래스.
*/
class CCallback : public IBindStatusCallback
{
private:
	double		m_Wait;			//!< 타임아웃.
	double		m_StartTime;	//!< 시작 시간.

public:
	HRESULT		hResult;
	ULONG		Progress;
	ULONG		ProgressMax;

public:
	/*! @brief 생성자
		@param pURL
			다운로드 받을 홈페이지 주소.
		@param wait
			응답을 기다리는 시간.(기본 180초)
	*/
	CCallback(double WaitTime=180)
	{
		m_Wait      = WaitTime;
		m_StartTime = appSeconds();
		hResult     = S_OK;
	}
	//! 소멸자
	virtual ~CCallback()
	{

	}

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
		{
			hResult = E_ABORT;
			return E_ABORT;
		}

		Progress    = ulProgress;
		ProgressMax = ulProgressMax;
//		debugf(TEXT("Downloading : %d / %d Bytes"), ulProgress, ulProgressMax);

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


//! 패치 클래스(스레드로 처리됨).
class FDownloader : public FRunnable
{
	UavaUIComp_DrawCustomImage*	m_pOwner;

	FString						m_DownloadDir;
	FString						m_NoticeDir;
	FString						m_URLPrefix;
	FString						m_IniName;
	TArray<FString>				m_Files;
	UBOOL						m_bNewVersion;

public:
	// 기본 생성자.
	FDownloader(UavaUIComp_DrawCustomImage *pOwner)
	{
		m_pOwner      = pOwner;

		m_NoticeDir   = pOwner->GetBasePath() + "\\" + pOwner->SubPath;
		m_DownloadDir = m_NoticeDir + TEXT("\\temp");
		m_URLPrefix   = pOwner->URLPrefix;
		m_IniName     = pOwner->IniName;
		m_bNewVersion = FALSE;
	}

	/**
	 * Allows per runnable object initialization. NOTE: This is called in the
	 * context of the thread object that aggregates this, not the thread that
	 * passes this runnable to a new thread.
	 *
	 * @return True if initialization was successful, false otherwise
	 */
	UBOOL Init(void)
	{
		// 만일을 대비해서 Notice디렉토리도 생성해 본다.
//		CreateDirectory(*m_NoticeDir, NULL);

		// 임시 디렉토리를 생성한다.
		CreateDirectory(*m_DownloadDir, NULL);

		debugf(TEXT("FDownloader::Init"));
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
		// ini파일을 다운 받는다.
		if ( !Download(m_IniName) )
			return 0;

		// ini파일도 추가해 준다.
		m_Files.AddItem(m_IniName);

		// 두 파일의 버전 정보를 얻어온다.
		DWORD LastVersion = GetFileVersion( FString::Printf(TEXT("%s\\%s"), *m_NoticeDir, *m_IniName) );
		DWORD NewVersion  = GetFileVersion( FString::Printf(TEXT("%s\\%s"), *m_DownloadDir, *m_IniName) );

		// 다른 버전인 경우에만
		if ( LastVersion < NewVersion || LastVersion == 0 )
		{
			// 모든 이미지 파일을 다운로드 한다.
			DownloadFiles();

			// 예전 버전의 파일을 제거한다.
//			RemoveOldFiles();

			// 임시 디렉토리에서 원본 디렉토리로 복사한다.
			CopyFiles();

			// 새로운 버전 유무.
			m_bNewVersion = TRUE;
		}

		// 임시 디렉토리의 파일들를 삭제한다.
		RemoveFiles();

		return 0;
	}

	//! ini파일에서 (DWORD)(Major << 16 | Minor)형태로 버전값을 리턴해 준다.
	DWORD GetFileVersion(const FString &filename)
	{
		FConfigFile	cfgFile;
		FString		major, minor;

		LoadAnIniFile(*filename, cfgFile, FALSE);
		if ( cfgFile.Num() == 0 )
			return 0;

		cfgFile.GetString(TEXT("Version"), TEXT("Major"), major);
		cfgFile.GetString(TEXT("Version"), TEXT("Minor"), minor);

		int high = appAtoi(*major);
		int low = appAtoi(*minor);

		return (DWORD)((high<<16) | low);
	}

	//! 이미지 파일을 다운받는다.
	void DownloadFiles()
	{
		FString			filename = FString::Printf(TEXT("%s\\%s"), *m_DownloadDir, *m_IniName);
		FConfigFile		cfgFile;
		TArray<FString>	values;

		LoadAnIniFile(*filename, cfgFile, FALSE);
		if ( cfgFile.Num() == 0 )
		{
			debugf(TEXT("cannot load %s"), *filename);
			return ;
		}

		// 세션 얻기.
		const FConfigSection* cfgSection = cfgFile.Find(TEXT("Files"));
		if ( cfgSection == NULL )
		{
			debugf(TEXT("cannot found [Files] Section.(%s)"), *filename);
			return;
		}

		// 키에 대한 값들을 얻는다.
		cfgSection->MultiFind(TEXT("name"), values);

		for(int i = 0; i < values.Num(); i++)
		{
			FString value = values(i);

			// 따옴표를 없앤다.
			value.Replace(TEXT("\""), TEXT(""));;

			if ( !Download(value) )
			{
				debugf(TEXT("cannot download (%s) file."), *value);
			}
			else
			{
				m_Files.AddItem(*value);
				debugf(TEXT("downloaded (%s) file."), *value);
			}
		}
	}

	//! 이전 버전의 파일들을 제거한다.(기존의 m_NoticeDir경로의 파일을 지운다)
	void RemoveOldFiles()
	{
		FString				SearchDir = m_NoticeDir + TEXT("\\*.*");

		HANDLE				hFind;
		WIN32_FIND_DATA		wfd;

		if( (hFind = FindFirstFile(*SearchDir, &wfd)) == INVALID_HANDLE_VALUE )
			return ;

		for(BOOL bResult = TRUE; bResult; bResult = FindNextFile(hFind, &wfd))
		{
			FString filename = wfd.cFileName;

			if ( filename == TEXT(".") || filename == TEXT("..") )
				continue;

			if ( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
				continue;

			if ( !DeleteFile( *FString::Printf(TEXT("%s\\%s"), *m_NoticeDir, wfd.cFileName) ) )
				debugf(TEXT("cannot delete (%s) file"), wfd.cFileName);
		}

		FindClose(hFind);
	}

	//! 임시 디렉토리에서 공지 디렉토리로 이동
	void CopyFiles()
	{
		for(int i = 0; i < m_Files.Num(); i++)
		{
			FString Filename     = m_NoticeDir + "\\" + m_Files(i);
			FString tempFilename = m_DownloadDir + "\\" + m_Files(i);

			// 기존의 파일이 있다면 지워준다.
			if ( !DeleteFile( *Filename ) )
			{
				// 만약 ReadOnly라면 속성을 Normal로 수정하고
				SetFileAttributes(*Filename, FILE_ATTRIBUTE_NORMAL);

				// 다시 지워본다.
				if ( !DeleteFile( *Filename ) )
					debugf(TEXT("cannot delete (%s) file"), *Filename);
			}

			// 새로 다운받은 파일을 복사한다.
			if ( !CopyFile(*tempFilename, *Filename, FALSE) )
				debugf(TEXT("cannot copy (%s) file."), *m_Files(i));

			debugf(TEXT("copy (%s) file."), *m_Files(i));
		}
	}

	//! 임시 디렉토리의 모든 파일을 지운다.
	void RemoveFiles()
	{
		for(int i = 0; i < m_Files.Num(); i++)
		{
			FString tempFilename = m_DownloadDir + "\\" + m_Files(i);

			// 기존의 파일이 있다면 지워준다.
			if ( !DeleteFile( *tempFilename ) )
			{
				// 만약 ReadOnly라면 속성을 Normal로 수정하고
				SetFileAttributes(*tempFilename, FILE_ATTRIBUTE_NORMAL);

				// 다시 지워본다.
				if ( !DeleteFile( *tempFilename ) )
					debugf(TEXT("cannot delete (%s) file"), *tempFilename);
			}
		}
	}

	// 파일을 다운로드 한다.
	UBOOL Download(const FString &filename)
	{
		FString urlFile   = m_URLPrefix + TEXT("/") + filename;
		FString localFile = m_DownloadDir + TEXT("\\") + filename;

		debugf(TEXT("Download : %s"), *urlFile);

		CCallback callback;
		if ( ::URLDownloadToFile(NULL, *urlFile, *localFile, 0, &callback) == S_OK )
		{
			// 중간에 응답시간 초과로 다 못받은 경우도 있을 수 있다.
			if ( callback.Progress == callback.ProgressMax && callback.hResult == S_OK )
				return TRUE;
		}

		return FALSE;
	}

	/**
	* This is called if a thread is requested to terminate early
	*/
	void Stop(void)
	{
		debugf(TEXT("FDownloader::Stop"));
	}

	/**
	* Called in the context of the aggregating thread to perform any cleanup.
	*/
	void Exit(void)
	{
		// 임시 디렉토리 삭제.
//		RemoveDirectory(*m_DownloadDir);

		// 갱신유무를 UI-Scene으로 알려준다.

		// 방법1.
		// 변수 1개를 따로 설정해서 값을 변환하고
		// Timer로 체크해서 UI-Scene에 알려준다??

		if ( m_bNewVersion )
			GDownloadManager.SetState(m_IniName, FDownloadManager::State_Downloaded_Update);
		else
			GDownloadManager.SetState(m_IniName, FDownloadManager::State_Downloaded);

		debugf(TEXT("FDownloader::Exit - bNeedUpdate(%d)"), m_bNewVersion);
	}
};


//-----------------------------------------------------------------------------
//	avaUIComp_DrawCustomImage
//-----------------------------------------------------------------------------
void UavaUIComp_DrawCustomImage::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if( PropertyName == TEXT("IniName") || PropertyName == TEXT("SubPath") )
			{
				eventOnChangeIniName();
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

FString UavaUIComp_DrawCustomImage::GetBasePath()
{
	FString BinDir  = appBaseDir();
	FString BaseDir = BinDir.Left(BinDir.InStr(TEXT("\\Binaries"), TRUE, TRUE));

	return BaseDir;
}

UTexture2D* UavaUIComp_DrawCustomImage::LoadImage(const FString& FullFilename)
{
	// 어떻게 이럴 수가 있지?
	if ( GEngine == NULL )
	{
		debugf(TEXT("Failed UavaUIComp_DrawCustomImage::LoadImage(%s) - (GEngine == NULL)"), *FullFilename);
		return NULL;
	}

	FString tmp = FullFilename.Right( FullFilename.Len() - (FullFilename.InStr(TEXT("\\"), TRUE) + 1) );
	FString key = PrefixName + tmp.Left( tmp.InStr(TEXT(".")) );

	// if the texture already exists, remember the user settings
	UTexture2D* ExistingTexture = FindObject<UTexture2D>( GEngine, *key );
	if ( ExistingTexture && !bNeedUpdate )
	{
		debugf(TEXT("UavaUIComp_DrawCustomImage - ExistingTexture:%s"), *key);
		return ExistingTexture;
	}

	debugf(TEXT("### key(UavaUIComp_DrawCustomImage) = %s"), *key);

	// 에디터인 경우.
	//if ( GIsEditor && !GIsGame )
	//{
	//	// 외부 파일을 읽어들여서 적용한다.
	//	return ImportObject<UTexture2D>( GEngine, *key, RF_Public|RF_Standalone, 
	//		*FullFilename, NULL, NULL, TEXT("CREATEMIPMAPS=0 NOCOMPRESSION=1 LODGroup=TEXTUREGROUP_UI") );
	//}

	// UFactory.cpp(Ln130)에서 참고함.
	TArray<BYTE> Data;
	if( appLoadFileToArray( Data, *FullFilename ) )
	{
		Data.AddItem( 0 );
		const BYTE* Ptr = &Data( 0 );

		// UnEdFact.cpp에서 UTextureFactory::FactoryCreateBinary를 참고함.
		return ImportPNG(UTexture2D::StaticClass(), GEngine, *key, RF_Public|RF_Standalone, FullFilename, NULL, 
						 *FFilename(FullFilename).GetExtension(), Ptr, Ptr+Data.Num()-1, GWarn);
	}

	return NULL;
}

UBOOL UavaUIComp_DrawCustomImage::LoadIni(const FString& FullFilename, 
										  const FString& Section,
										  const FString& KeyName, 
										  TArray<FString>& Values)
{
	FConfigFile				cfgFile;
	const FConfigSection*	cfgSection;

	// ini파일 읽기.
	LoadAnIniFile(*FullFilename, cfgFile, FALSE);

	// 읽기 실패한 경우.
	if ( cfgFile.Num() == 0 )
		return FALSE;

	// 세션 얻기.
	cfgSection = cfgFile.Find(Section);

	// 키에 대한 값들을 얻는다.
	cfgSection->MultiFind(KeyName, Values);

	return TRUE;
}

void UavaUIComp_DrawCustomImage::Download()
{
	// 이미 다운로드가 한번 실행되었다면...
	if ( GDownloadManager.IsRegistered(IniName) != NULL )
		return ;

	// 다운로드 스레드의 이름을 등록한다.(프로그램 종료 전까지 또다시 호출되지 않도록 한다)
	GDownloadManager.Register(IniName);

	FRunnableThread*	pThread     = NULL;
	FDownloader*		pDownloader = new FDownloader(this);
	FString				ThreadName  = TEXT("Download Thread : ");

	ThreadName += IniName;

	// 자동으로 NoticePatchThread가 해제되고, new FDownloader에 대해서도 자동 해제 되도록 한다.
	pThread = GThreadFactory->CreateThread(pDownloader, (TCHAR*)*ThreadName, TRUE, TRUE, 0, TPri_Normal);
}

INT UavaUIComp_DrawCustomImage::Tokenize(const FString& Text,const FString& delims,TArray<FString>& Values)
{
	if ( Text.Len() == 0 )
		return 0;

	int before = 0;
	int end    = 0;
	int i, j;

	Values.Empty();

	while(before < Text.Len() && end < Text.Len())
	{
		end = -1;

		// 구분자를 찾는다.
		for(i = before; i < Text.Len(); i++)
		{
			for(j = 0; j < delims.Len(); j++)
			{
				if ( Text[i] == delims[j] )
				{
					end = i;
					break;
				}
			}

			if(end != -1)
				break;
		}

		// 찾았으면 
		if(end != -1)
		{
			if(before != end)
				Values.AddItem( Text.Mid(before, end - before) );

			before = end + 1;
			continue;
		}

		Values.AddItem( Text.Mid(before, Text.Len() - before) );
		break;
	}

	return Values.Num();
}

FString UavaUIComp_DrawCustomImage::Trim(const FString& Text, const FString& whitespaces, UBOOL bRight)
{
	int i, j;
	int numTexts;
	int numWhiteSpaces;
	FString tmp = Text;

	numTexts       = tmp.Len();
	numWhiteSpaces = whitespaces.Len();

	// 왼쪽 부터 찾는다.
	for (i = 0; i < numTexts; ++i)
	{
		for (j = 0; j < numWhiteSpaces; ++j)
		{
			if ( tmp[i] == whitespaces[j] )
				break;
		}

		// 공백문자가 아니라면 빠져나온다.
		if(j == numWhiteSpaces)
			break;
	}

	// 공백문자가 아닌 부분부터 복사한다.
	tmp      = tmp.Right(tmp.Len() - i);
	numTexts = tmp.Len();

	// 필요하다면 오른쪽도 찾는다.
	if ( bRight )
	{
		for (i = numTexts - 1; i >= 0; --i)
		{
			for (j = 0; j < numWhiteSpaces; ++j)
			{
				if ( tmp[i] == whitespaces[j] )
					break;
			}

			// 공백문자가 아니라면 빠져나온다.
			if(j == numWhiteSpaces)
				break;
		}

		tmp = tmp.Left(i + 1);
	}

	return tmp;
}

void UavaUIComp_DrawCustomImage::RenderComponent( FCanvas* Canvas, FRenderParameters Parameters )
{
	// 다운로드 중인 경우.
	if ( bDownloading )
	{
		// 다운로더의 상태를 얻어온다.
		UINT *pState = GDownloadManager.IsRegistered(IniName);
		if ( pState != NULL )
		{
//			debugf(TEXT("bDownloading=%d, *pState=%s"), bDownloading, GDownloadManager.GetStateStr(*pState));

			switch(*pState)
			{
				// Download는 완료되었지만 갱신이 안된 경우.
				case FDownloadManager::State_Downloaded_Update:
					GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
					bDownloading = false;
					bNeedUpdate = TRUE;
					// 갱신이 필요하다면.
					eventOnChangeIniName();
					bNeedUpdate = FALSE;
					break;

				// Download가 완료되었고, 갱신도 마친 상태.
				case FDownloadManager::State_Downloaded:
					GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
					bDownloading = false;
					break;

				case FDownloadManager::State_Complete:
					bDownloading = false;
					break;
			}
		}
		else
		{
			// 이런 경우는 있을 수 없다.
			bDownloading = false;
		}
	}

	if ( CustomImageRef != NULL )
	{ 
		if ( !bInitStyleData && HasValidStyleReference() )
		{
			RefreshAppliedStyleData();
			bInitStyleData = true;
		}

		// 애니메이션 처리.
		if ( CustomImageRef->ImageTexture != NULL && ImageInfos.Num() > 0 )
		{
			if ( CurrentInfo < ImageInfos.Num() )
			{
				// 지연시간을 넘는다면
				if ( (float)appSeconds() - LocalTime > ImageInfos(CurrentInfo).delay )
				{
					if ( ++CurrentInfo >= ImageInfos.Num() )
						CurrentInfo = 0;

					// 새로운 이미지로 교체해 준다.
					SetCustomImage( ImageInfos(CurrentInfo).Tex );
				}
			}
		}
	}

	// ImageTexture가 있는 경우에만 그리도록 수정.
	if ( CustomImageRef != NULL && Canvas != NULL )//&& CustomImageRef->ImageTexture != NULL )
	{
		//@todo ronp - there is a discrepancy here in how the coordinates are set.  In widgets which have been updated to use the draw image
		// component, the style's AtlasCoords should always be used for rendering the image, but for widgets which are still using a UITexture
		// directly, they still have seperate coordinate variables
		StyleCustomization.CustomizeCoordinates(Parameters.DrawCoords);
		CustomImageRef->Render_Texture(Canvas, Parameters);
	}
}

void UavaUIComp_DrawCustomImage::PostLoad()
{
	Super::PostLoad();

	if ( CustomImageRef == NULL && !IsTemplate() && GEngine != NULL && !GIsUCC )
	{
		if ( IniName.Len() )
		{
			UINT *pState = GDownloadManager.IsRegistered(IniName);

			// 다운로드가 실행되지 않았다면...
			if ( pState == NULL )
			{
				debugf(TEXT("avaUIComp_DrawCustomImage.PostLoad, Downloading..."));

				eventOnChangeIniName();

				// URL이 있다면 다운로드 한다.
				if ( URLPrefix.Len() )
					Download();

				// avaUIComp_DrawCustomImage.Render에서 갱신되도록 해준다
				bDownloading = false;
			}
			else
			{
				// 이미 다운로드가 한번 실행되었다면...
				debugf(TEXT("avaUIComp_DrawCustomImage.PostLoad, *pState=%s"), GDownloadManager.GetStateStr(*pState));

				switch(*pState)
				{
					// Download 중인 상태.(avaUIComp_DrawCustomImage.Render에서 갱신되도록 해준다)
					case FDownloadManager::State_Downloading:
						bDownloading = true;
						break;

					// Download는 완료되었지만 갱신이 안된 경우.
					case FDownloadManager::State_Downloaded_Update:
						GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
						bDownloading = false;
						bNeedUpdate = TRUE;
						break;

					// Download가 완료되었고, 갱신도 마친 상태.
					case FDownloadManager::State_Downloaded:
						GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
						bDownloading = false;
						break;

					case FDownloadManager::State_Complete:
						bDownloading = false;
						break;
				}

				// 위에서 (bNeedUpdate == TRUE)경우에는 텍스쳐를 새로 읽어들이게 된다.
				eventOnChangeIniName();

				bNeedUpdate = FALSE;
			}
		}

		if ( CustomImageRef != NULL )
			CustomImageRef->SetFlags(RF_Transactional);
	}
}

void UavaUIComp_DrawCustomImage::RefreshAppliedStyleData()
{
	if ( CustomImageRef == NULL )
	{
		// we have no image if we've never been assigned a value...if this is the case, create one
		// so that the style's DefaultImage will be rendererd
		SetCustomImage(NULL);
	}
	else
	{
		// get the style data that should be applied to the image
		UUIStyle_Image* ImageStyleData = GetAppliedImageStyle();

		// ImageStyleData will be NULL if this component has never resolved its style
		if ( ImageStyleData != NULL )
		{
			// apply this component's per-instance image style settings
			FUICombinedStyleData FinalStyleData(ImageStyleData);
			CustomizeAppliedStyle(FinalStyleData);

			// apply the style data to the image
			CustomImageRef->SetImageStyle(FinalStyleData);
		}
	}
}

void UavaUIComp_DrawCustomImage::SetCustomImage( USurface* NewImage )
{
	const UBOOL bInitializeStyleData=(CustomImageRef==NULL);
	if ( CustomImageRef == NULL )
	{
		Modify();
		CustomImageRef = ConstructObject<UUITexture>(UUITexture::StaticClass(), this, NAME_None, 
													 RF_Transactional|GetMaskedFlags(RF_Transient|RF_ArchetypeObject));
	}

	CustomImageRef->Modify();
	CustomImageRef->ImageTexture = NewImage;

	if ( /*bInitializeStyleData &&*/ HasValidStyleReference() )
	{
		RefreshAppliedStyleData();
	}

	LocalTime = (float)appSeconds();
}

/*
FString UavaUIComp_DrawCustomImage::GetRegPath()
{
	FString path;

	// 값이 없는 경우.
	if ( RegSubKey.Len() == 0 || RegValueName.Len() == 0 )
		return path;

	HKEY hMyKey;
	HKEY hKeys[] = 
	{
		HKEY_CLASSES_ROOT,
		HKEY_CURRENT_USER,
		HKEY_LOCAL_MACHINE,
		HKEY_USERS,
	};

	// Registry에서 해당 키값을 얻는다.
	if ( RegOpenKeyEx(hKeys[RegKey], *RegSubKey, 0, KEY_READ|KEY_QUERY_VALUE, &hMyKey) == ERROR_SUCCESS )
	{
		TCHAR	*pBuf;
		DWORD	size;
		DWORD	bufSize;
		DWORD	type = REG_SZ;

		RegQueryValueEx(hMyKey, *RegValueName, 0, &type, NULL, &size);

		pBuf    = new TCHAR[size];
		bufSize = size * sizeof(TCHAR);

		RegQueryValueEx(hMyKey, *RegValueName, 0, &type, (LPBYTE)pBuf, &bufSize);


		//LONG lRet;

		//if ( lRet != ERROR_SUCCESS )
		//{
		//	TCHAR *pMsgBuf = NULL;

		//	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		//						NULL, 
		//						lRet, 
		//						MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		//						(LPTSTR)&pMsgBuf,
		//						0,
		//						NULL))
		//	{
		//		debugf(TEXT("Failed RegQueryValueEx(RegValueName=\"%s\") - %s "), *RegValueName, pMsgBuf);

		//		::LocalFree(pMsgBuf);
		//	}
		//}


		path = pBuf;
		delete pBuf;

		RegCloseKey(hMyKey);
	}

	return path;
}
*/
