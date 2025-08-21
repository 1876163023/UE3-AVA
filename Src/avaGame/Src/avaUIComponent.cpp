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
	BYTE	BitDepth;		//!< 1, 2, 4, 8, or 16 bits/channel.(8 bit�� ����� ����)
	BYTE	ColorType;		//!< see PNG_COLOR_TYPE_ below.(PNG_COLOR_TYPE_RGB�� ����� ����)
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

	// PNG�� ��� GrayScale�� ��� RGB���� �����Ѵ�.(�׷��� ���ʿ��� ����)
	const FPNGHeader* PNG = (FPNGHeader *)Buffer;

	// Validate it.
	INT Length = BufferEnd - Buffer;

	// ��� ũ�⺸�� �۰ų�, PNG���� ������ �ƴ� ��� ���� ����.
	if ( Length < sizeof(FPNGHeader) || appMemcmp(PNG->IHDR, "IHDR", 4) != 0 )
	{
		// Unknown format.
		Warn->Logf( NAME_Error, TEXT("Bad png image format for texture import") );
		return NULL;
	}

	// �̹� �����ϴ� ���.
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

		//{ 2007/04/04 ����.
		{
			// 8-bit Grayscale�� ����� ������ �ε��� �ؽ��� RGB ������ ����.
			if ( PNG->BitDepth == 8 && PNG->ColorType == PNG_COLOR_TYPE_GRAY )
			{
				ExistingSizeX = ExistingTexture->SizeX;
				ExistingSizeY = ExistingTexture->SizeY;

				// RGB������ ����.
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
		// png���� little endian�̶� ��ȯ�� ��� �ϴ���.
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

		// RGB�� �ƴϸ� ������ ����.
		if ( PNG->ColorType != PNG_COLOR_TYPE_RGB && 
			 PNG->ColorType != PNG_COLOR_TYPE_GRAY &&
			 PNG->ColorType != PNG_COLOR_TYPE_RGB_ALPHA )
			return NULL;

		// PNG->ColorType - PNG_COLOR_TYPE_RGB�� ��쿡�� RGB�� ä���,
		//					PNG_COLOR_TYPE_GRAY�� ��쿡�� ������ �ؽ��Ŀ� Alpha�� ä���.
		if ( PNG->ColorType == PNG_COLOR_TYPE_RGB )
		{
			Texture = CastChecked<UTexture2D>(UObject::StaticConstructObject(Class,InParent,Name,Flags));
			if ( Texture == NULL )
				return NULL;

			Texture->Init(pngWidth,pngHeight,PF_A8R8G8B8);

			FColor *pDest;

			if ( (pDest = (FColor*)Texture->Mips(0).Data.Lock(LOCK_READ_WRITE)) != NULL )
			{
				// PNG���� Alpha������ �����Ѵ�.
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
			// ������ �ؽ��İ� ���ٰų� ũ�Ⱑ �ٸ��ٸ� ����ó��.
			if ( (ExistingRawData.Num() == 0) || (ExistingSizeX != pngWidth) || (ExistingSizeY != pngHeight) )
				return NULL;

			Texture = CastChecked<UTexture2D>(UObject::StaticConstructObject(Class,InParent,Name,Flags));
			if ( Texture == NULL )
				return NULL;

			Texture->Init(pngWidth,pngHeight,PF_A8R8G8B8);

			FColor *pDest;

			if ( (pDest = (FColor*)Texture->Mips(0).Data.Lock(LOCK_READ_WRITE)) != NULL )
			{
				// PNG���� Alpha������ �����Ѵ�.
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
				// PNG���� Alpha������ �����Ѵ�.
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

//! Editor������ �ʿ��ϴ�.(���� ������ ������̶�� �Ѵ�)
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

//! Download ������.
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

	// ����׿�.
	static TCHAR *GDownloaderManagerStates[State_Max];

protected:
	TMap<FName, UINT>	NameMap;

public:
	//! �ٿ�ε� �� Ini���� �̸��� ����Ѵ�.
	UBOOL	Register(const FString &name)
	{
		UINT *pState = NULL;

		// �̹� ��� �Ǿ��ٸ� ���� ����.
		if ( (pState = NameMap.Find(*name)) != NULL )
			return FALSE;

		// ���
		NameMap.Set(*name, State_Downloading);

		return TRUE;
	}

	//! �̹� ��ϵ� Ini�����ΰ�?
	UINT*	IsRegistered(const FString &name)
	{
		return NameMap.Find(*name);
	}

	//! �ش� Ini���Ͽ� ���� �ٿ�ε� ���� ����.
	UBOOL	SetState(const FString &name, EState eState)
	{
		UINT *pState = NameMap.Find(*name);
		if ( pState == NULL )
			return FALSE;

		*pState = eState;

		return TRUE;
	}

	//! ���¿� �ش��ϴ� ���ڿ��� ���´�.
	static const TCHAR*	GetStateStr(UINT eState)
	{
		return GDownloaderManagerStates[eState];
	}

} GDownloadManager;

//! ����׿��̴�.
TCHAR *FDownloadManager::GDownloaderManagerStates[] = 
{
	TEXT("State_Downloading"),
	TEXT("State_Downloaded"),
	TEXT(""),
	TEXT("State_Downloaded_Update"),
	TEXT("State_Complete"),
};

/*! @brief
		���ͳ� �ͽ��÷η��� �̿��� HTTP�� ������ �ٿ�ε�ޱ� ���� COM �������̽� Ŭ����.
*/
class CCallback : public IBindStatusCallback
{
private:
	double		m_Wait;			//!< Ÿ�Ӿƿ�.
	double		m_StartTime;	//!< ���� �ð�.

public:
	HRESULT		hResult;
	ULONG		Progress;
	ULONG		ProgressMax;

public:
	/*! @brief ������
		@param pURL
			�ٿ�ε� ���� Ȩ������ �ּ�.
		@param wait
			������ ��ٸ��� �ð�.(�⺻ 180��)
	*/
	CCallback(double WaitTime=180)
	{
		m_Wait      = WaitTime;
		m_StartTime = appSeconds();
		hResult     = S_OK;
	}
	//! �Ҹ���
	virtual ~CCallback()
	{

	}

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


//! ��ġ Ŭ����(������� ó����).
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
	// �⺻ ������.
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
		// ������ ����ؼ� Notice���丮�� ������ ����.
//		CreateDirectory(*m_NoticeDir, NULL);

		// �ӽ� ���丮�� �����Ѵ�.
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
		// ini������ �ٿ� �޴´�.
		if ( !Download(m_IniName) )
			return 0;

		// ini���ϵ� �߰��� �ش�.
		m_Files.AddItem(m_IniName);

		// �� ������ ���� ������ ���´�.
		DWORD LastVersion = GetFileVersion( FString::Printf(TEXT("%s\\%s"), *m_NoticeDir, *m_IniName) );
		DWORD NewVersion  = GetFileVersion( FString::Printf(TEXT("%s\\%s"), *m_DownloadDir, *m_IniName) );

		// �ٸ� ������ ��쿡��
		if ( LastVersion < NewVersion || LastVersion == 0 )
		{
			// ��� �̹��� ������ �ٿ�ε� �Ѵ�.
			DownloadFiles();

			// ���� ������ ������ �����Ѵ�.
//			RemoveOldFiles();

			// �ӽ� ���丮���� ���� ���丮�� �����Ѵ�.
			CopyFiles();

			// ���ο� ���� ����.
			m_bNewVersion = TRUE;
		}

		// �ӽ� ���丮�� ���ϵ鸦 �����Ѵ�.
		RemoveFiles();

		return 0;
	}

	//! ini���Ͽ��� (DWORD)(Major << 16 | Minor)���·� �������� ������ �ش�.
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

	//! �̹��� ������ �ٿ�޴´�.
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

		// ���� ���.
		const FConfigSection* cfgSection = cfgFile.Find(TEXT("Files"));
		if ( cfgSection == NULL )
		{
			debugf(TEXT("cannot found [Files] Section.(%s)"), *filename);
			return;
		}

		// Ű�� ���� ������ ��´�.
		cfgSection->MultiFind(TEXT("name"), values);

		for(int i = 0; i < values.Num(); i++)
		{
			FString value = values(i);

			// ����ǥ�� ���ش�.
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

	//! ���� ������ ���ϵ��� �����Ѵ�.(������ m_NoticeDir����� ������ �����)
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

	//! �ӽ� ���丮���� ���� ���丮�� �̵�
	void CopyFiles()
	{
		for(int i = 0; i < m_Files.Num(); i++)
		{
			FString Filename     = m_NoticeDir + "\\" + m_Files(i);
			FString tempFilename = m_DownloadDir + "\\" + m_Files(i);

			// ������ ������ �ִٸ� �����ش�.
			if ( !DeleteFile( *Filename ) )
			{
				// ���� ReadOnly��� �Ӽ��� Normal�� �����ϰ�
				SetFileAttributes(*Filename, FILE_ATTRIBUTE_NORMAL);

				// �ٽ� ��������.
				if ( !DeleteFile( *Filename ) )
					debugf(TEXT("cannot delete (%s) file"), *Filename);
			}

			// ���� �ٿ���� ������ �����Ѵ�.
			if ( !CopyFile(*tempFilename, *Filename, FALSE) )
				debugf(TEXT("cannot copy (%s) file."), *m_Files(i));

			debugf(TEXT("copy (%s) file."), *m_Files(i));
		}
	}

	//! �ӽ� ���丮�� ��� ������ �����.
	void RemoveFiles()
	{
		for(int i = 0; i < m_Files.Num(); i++)
		{
			FString tempFilename = m_DownloadDir + "\\" + m_Files(i);

			// ������ ������ �ִٸ� �����ش�.
			if ( !DeleteFile( *tempFilename ) )
			{
				// ���� ReadOnly��� �Ӽ��� Normal�� �����ϰ�
				SetFileAttributes(*tempFilename, FILE_ATTRIBUTE_NORMAL);

				// �ٽ� ��������.
				if ( !DeleteFile( *tempFilename ) )
					debugf(TEXT("cannot delete (%s) file"), *tempFilename);
			}
		}
	}

	// ������ �ٿ�ε� �Ѵ�.
	UBOOL Download(const FString &filename)
	{
		FString urlFile   = m_URLPrefix + TEXT("/") + filename;
		FString localFile = m_DownloadDir + TEXT("\\") + filename;

		debugf(TEXT("Download : %s"), *urlFile);

		CCallback callback;
		if ( ::URLDownloadToFile(NULL, *urlFile, *localFile, 0, &callback) == S_OK )
		{
			// �߰��� ����ð� �ʰ��� �� ������ ��쵵 ���� �� �ִ�.
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
		// �ӽ� ���丮 ����.
//		RemoveDirectory(*m_DownloadDir);

		// ���������� UI-Scene���� �˷��ش�.

		// ���1.
		// ���� 1���� ���� �����ؼ� ���� ��ȯ�ϰ�
		// Timer�� üũ�ؼ� UI-Scene�� �˷��ش�??

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
	// ��� �̷� ���� ����?
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

	// �������� ���.
	//if ( GIsEditor && !GIsGame )
	//{
	//	// �ܺ� ������ �о�鿩�� �����Ѵ�.
	//	return ImportObject<UTexture2D>( GEngine, *key, RF_Public|RF_Standalone, 
	//		*FullFilename, NULL, NULL, TEXT("CREATEMIPMAPS=0 NOCOMPRESSION=1 LODGroup=TEXTUREGROUP_UI") );
	//}

	// UFactory.cpp(Ln130)���� ������.
	TArray<BYTE> Data;
	if( appLoadFileToArray( Data, *FullFilename ) )
	{
		Data.AddItem( 0 );
		const BYTE* Ptr = &Data( 0 );

		// UnEdFact.cpp���� UTextureFactory::FactoryCreateBinary�� ������.
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

	// ini���� �б�.
	LoadAnIniFile(*FullFilename, cfgFile, FALSE);

	// �б� ������ ���.
	if ( cfgFile.Num() == 0 )
		return FALSE;

	// ���� ���.
	cfgSection = cfgFile.Find(Section);

	// Ű�� ���� ������ ��´�.
	cfgSection->MultiFind(KeyName, Values);

	return TRUE;
}

void UavaUIComp_DrawCustomImage::Download()
{
	// �̹� �ٿ�ε尡 �ѹ� ����Ǿ��ٸ�...
	if ( GDownloadManager.IsRegistered(IniName) != NULL )
		return ;

	// �ٿ�ε� �������� �̸��� ����Ѵ�.(���α׷� ���� ������ �Ǵٽ� ȣ����� �ʵ��� �Ѵ�)
	GDownloadManager.Register(IniName);

	FRunnableThread*	pThread     = NULL;
	FDownloader*		pDownloader = new FDownloader(this);
	FString				ThreadName  = TEXT("Download Thread : ");

	ThreadName += IniName;

	// �ڵ����� NoticePatchThread�� �����ǰ�, new FDownloader�� ���ؼ��� �ڵ� ���� �ǵ��� �Ѵ�.
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

		// �����ڸ� ã�´�.
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

		// ã������ 
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

	// ���� ���� ã�´�.
	for (i = 0; i < numTexts; ++i)
	{
		for (j = 0; j < numWhiteSpaces; ++j)
		{
			if ( tmp[i] == whitespaces[j] )
				break;
		}

		// ���鹮�ڰ� �ƴ϶�� �������´�.
		if(j == numWhiteSpaces)
			break;
	}

	// ���鹮�ڰ� �ƴ� �κк��� �����Ѵ�.
	tmp      = tmp.Right(tmp.Len() - i);
	numTexts = tmp.Len();

	// �ʿ��ϴٸ� �����ʵ� ã�´�.
	if ( bRight )
	{
		for (i = numTexts - 1; i >= 0; --i)
		{
			for (j = 0; j < numWhiteSpaces; ++j)
			{
				if ( tmp[i] == whitespaces[j] )
					break;
			}

			// ���鹮�ڰ� �ƴ϶�� �������´�.
			if(j == numWhiteSpaces)
				break;
		}

		tmp = tmp.Left(i + 1);
	}

	return tmp;
}

void UavaUIComp_DrawCustomImage::RenderComponent( FCanvas* Canvas, FRenderParameters Parameters )
{
	// �ٿ�ε� ���� ���.
	if ( bDownloading )
	{
		// �ٿ�δ��� ���¸� ���´�.
		UINT *pState = GDownloadManager.IsRegistered(IniName);
		if ( pState != NULL )
		{
//			debugf(TEXT("bDownloading=%d, *pState=%s"), bDownloading, GDownloadManager.GetStateStr(*pState));

			switch(*pState)
			{
				// Download�� �Ϸ�Ǿ����� ������ �ȵ� ���.
				case FDownloadManager::State_Downloaded_Update:
					GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
					bDownloading = false;
					bNeedUpdate = TRUE;
					// ������ �ʿ��ϴٸ�.
					eventOnChangeIniName();
					bNeedUpdate = FALSE;
					break;

				// Download�� �Ϸ�Ǿ���, ���ŵ� ��ģ ����.
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
			// �̷� ���� ���� �� ����.
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

		// �ִϸ��̼� ó��.
		if ( CustomImageRef->ImageTexture != NULL && ImageInfos.Num() > 0 )
		{
			if ( CurrentInfo < ImageInfos.Num() )
			{
				// �����ð��� �Ѵ´ٸ�
				if ( (float)appSeconds() - LocalTime > ImageInfos(CurrentInfo).delay )
				{
					if ( ++CurrentInfo >= ImageInfos.Num() )
						CurrentInfo = 0;

					// ���ο� �̹����� ��ü�� �ش�.
					SetCustomImage( ImageInfos(CurrentInfo).Tex );
				}
			}
		}
	}

	// ImageTexture�� �ִ� ��쿡�� �׸����� ����.
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

			// �ٿ�ε尡 ������� �ʾҴٸ�...
			if ( pState == NULL )
			{
				debugf(TEXT("avaUIComp_DrawCustomImage.PostLoad, Downloading..."));

				eventOnChangeIniName();

				// URL�� �ִٸ� �ٿ�ε� �Ѵ�.
				if ( URLPrefix.Len() )
					Download();

				// avaUIComp_DrawCustomImage.Render���� ���ŵǵ��� ���ش�
				bDownloading = false;
			}
			else
			{
				// �̹� �ٿ�ε尡 �ѹ� ����Ǿ��ٸ�...
				debugf(TEXT("avaUIComp_DrawCustomImage.PostLoad, *pState=%s"), GDownloadManager.GetStateStr(*pState));

				switch(*pState)
				{
					// Download ���� ����.(avaUIComp_DrawCustomImage.Render���� ���ŵǵ��� ���ش�)
					case FDownloadManager::State_Downloading:
						bDownloading = true;
						break;

					// Download�� �Ϸ�Ǿ����� ������ �ȵ� ���.
					case FDownloadManager::State_Downloaded_Update:
						GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
						bDownloading = false;
						bNeedUpdate = TRUE;
						break;

					// Download�� �Ϸ�Ǿ���, ���ŵ� ��ģ ����.
					case FDownloadManager::State_Downloaded:
						GDownloadManager.SetState(IniName, FDownloadManager::State_Complete);
						bDownloading = false;
						break;

					case FDownloadManager::State_Complete:
						bDownloading = false;
						break;
				}

				// ������ (bNeedUpdate == TRUE)��쿡�� �ؽ��ĸ� ���� �о���̰� �ȴ�.
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

	// ���� ���� ���.
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

	// Registry���� �ش� Ű���� ��´�.
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
