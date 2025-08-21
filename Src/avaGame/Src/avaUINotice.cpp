#include "avaGame.h"
#include "FConfigCacheIni.h"

#pragma comment(lib, "Urlmon.lib")

IMPLEMENT_CLASS(AavaUINoticePatcher);

/*! @brief
		인터넷 익스플로러를 이용해 HTTP로 파일을 다운로드받기 위한 COM 인터페이스 클래스.
*/
class CCallBackNotice : public IBindStatusCallback
{
private:
	TCHAR		m_URL[512];		//!< 다운로드받을 파일의 URL.
	double		m_Wait;			//!< 타임아웃.
	double		m_StartTime;	//!< 시작 시간.

public:
	/*! @brief 생성자
		@param pURL
			다운로드 받을 홈페이지 주소.
		@param wait
			응답을 기다리는 시간.(기본 20초)
	*/
	CCallBackNotice(const TCHAR *pURL, double WaitTime=20.0)
	{
		appStrcpy(m_URL, pURL);

		m_Wait      = WaitTime;
		m_StartTime = appSeconds();
	}
	//! 소멸자
	~CCallBackNotice()
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
			return E_ABORT;

		if ( ulProgressMax != 0 )
			debugf(TEXT("Downloading : %f %%"), (float)ulProgress / ulProgressMax * 100.0f);

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
class FUINoticePatcher : public FRunnable
{
	AavaUINoticePatcher*	m_pNoticePatcher;

	FString					m_DownloadDir;
	FString					m_NoticeDir;
	FString					m_URLPrefix;
	FString					m_IniName;
	TArray<FString>			m_Files;

public:
	// 기본 생성자.
	FUINoticePatcher(AavaUINoticePatcher *pPatcher)
	{
		m_pNoticePatcher = pPatcher;

		m_NoticeDir   = pPatcher->GetBasePath() + "\\" + m_pNoticePatcher->SubPath;
		m_DownloadDir = m_NoticeDir + TEXT("\\temp");
		m_URLPrefix   = pPatcher->URL;
		m_IniName     = m_pNoticePatcher->IniName;
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
		CreateDirectory(*m_NoticeDir, NULL);

		// 임시 디렉토리를 생성한다.
		CreateDirectory(*m_DownloadDir, NULL);

		debugf(TEXT("FNoticePatcher::Init"));
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
			RemoveOldFiles();

			// 임시 디렉토리에서 원본 디렉토리로 복사한다.
			CopyFiles();
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

			if ( !DeleteFile(*tempFilename) )
				debugf(TEXT("cannot delete (%s) file."), *tempFilename);
		}
	}

	// 파일을 다운로드 한다.
	UBOOL Download(const FString &filename)
	{
		FString urlFile   = m_URLPrefix + TEXT("/") + filename;
		FString localFile = m_DownloadDir + TEXT("\\") + filename;

		CCallBackNotice callback(*urlFile);
		return (::URLDownloadToFile(NULL, *urlFile, *localFile, 0, &callback) == S_OK);
	}

	/**
	* This is called if a thread is requested to terminate early
	*/
	void Stop(void)
	{
		debugf(TEXT("FNoticePatcher::Stop"));
	}

	/**
	* Called in the context of the aggregating thread to perform any cleanup.
	*/
	void Exit(void)
	{
		// 임시 디렉토리 삭제.
		RemoveDirectory(*m_DownloadDir);

		// 갱신유무를 UI-Scene으로 알려준다.

		// 방법1.
		// 변수 1개를 따로 설정해서 값을 변환하고
		// Timer로 체크해서 UI-Scene에 알려준다??

		m_pNoticePatcher->eventOnDownloaded();

		debugf(TEXT("FNoticePatcher::Exit"));
	}
};

FRunnableThread* pNoticePatchThread = NULL;

void AavaUINoticePatcher::Download()
{
	FUINoticePatcher *pPatcher = new FUINoticePatcher(this);

	// 자동으로 NoticePatchThread가 해제되고, new FUINoticePatcher에 대해서도 자동 해제 되도록 한다.
	pNoticePatchThread = GThreadFactory->CreateThread(pPatcher, TEXT("UINoticePatcher"), 
													  TRUE, TRUE, 0, TPri_Normal);


}

float AavaUINoticePatcher::GetSeconds()
{
	return (float)appSeconds();
}

FString AavaUINoticePatcher::GetBasePath()
{
	FString BinDir  = appBaseDir();
	FString BaseDir = BinDir.Left(BinDir.InStr(TEXT("\\Binaries"), TRUE));

	return BaseDir;
}

FString AavaUINoticePatcher::Trim(const FString& Text, const FString& whitespaces, UBOOL bRight)
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

UTexture2D* AavaUINoticePatcher::LoadImage(const FString& FullFilename)
{
	FString tmp = FullFilename.Right( FullFilename.Len() - (FullFilename.InStr(TEXT("\\"), TRUE) + 1) );
	FString key = PrefixName + tmp.Left( tmp.InStr(TEXT(".")) );

//	debugf(TEXT("### before: AavaUINoticePatcher.key = %s"), *key);

	// 외부 파일을 읽어들여서 적용한다.
	// (같은 key값이 있다면 이전 텍스쳐는 제거되고 새로 만들어 지더라)
	UTexture2D *pTex = ImportObject<UTexture2D>( GEngine, *key, RF_Public|RF_Standalone, *FullFilename, NULL, NULL, 
												 TEXT("CREATEMIPMAPS=0 NoCompression=1 LODGroup=TEXTUREGROUP_UI") );

//	debugf(TEXT("### after: AavaUINoticePatcher.key = %s"), *key);

	return pTex;
}

UBOOL AavaUINoticePatcher::LoadIni(const FString& FullFilename, 
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
