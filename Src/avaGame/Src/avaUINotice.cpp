#include "avaGame.h"
#include "FConfigCacheIni.h"

#pragma comment(lib, "Urlmon.lib")

IMPLEMENT_CLASS(AavaUINoticePatcher);

/*! @brief
		���ͳ� �ͽ��÷η��� �̿��� HTTP�� ������ �ٿ�ε�ޱ� ���� COM �������̽� Ŭ����.
*/
class CCallBackNotice : public IBindStatusCallback
{
private:
	TCHAR		m_URL[512];		//!< �ٿ�ε���� ������ URL.
	double		m_Wait;			//!< Ÿ�Ӿƿ�.
	double		m_StartTime;	//!< ���� �ð�.

public:
	/*! @brief ������
		@param pURL
			�ٿ�ε� ���� Ȩ������ �ּ�.
		@param wait
			������ ��ٸ��� �ð�.(�⺻ 20��)
	*/
	CCallBackNotice(const TCHAR *pURL, double WaitTime=20.0)
	{
		appStrcpy(m_URL, pURL);

		m_Wait      = WaitTime;
		m_StartTime = appSeconds();
	}
	//! �Ҹ���
	~CCallBackNotice()
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


//! ��ġ Ŭ����(������� ó����).
class FUINoticePatcher : public FRunnable
{
	AavaUINoticePatcher*	m_pNoticePatcher;

	FString					m_DownloadDir;
	FString					m_NoticeDir;
	FString					m_URLPrefix;
	FString					m_IniName;
	TArray<FString>			m_Files;

public:
	// �⺻ ������.
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
		// ������ ����ؼ� Notice���丮�� ������ ����.
		CreateDirectory(*m_NoticeDir, NULL);

		// �ӽ� ���丮�� �����Ѵ�.
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
			RemoveOldFiles();

			// �ӽ� ���丮���� ���� ���丮�� �����Ѵ�.
			CopyFiles();
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

			if ( !DeleteFile(*tempFilename) )
				debugf(TEXT("cannot delete (%s) file."), *tempFilename);
		}
	}

	// ������ �ٿ�ε� �Ѵ�.
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
		// �ӽ� ���丮 ����.
		RemoveDirectory(*m_DownloadDir);

		// ���������� UI-Scene���� �˷��ش�.

		// ���1.
		// ���� 1���� ���� �����ؼ� ���� ��ȯ�ϰ�
		// Timer�� üũ�ؼ� UI-Scene�� �˷��ش�??

		m_pNoticePatcher->eventOnDownloaded();

		debugf(TEXT("FNoticePatcher::Exit"));
	}
};

FRunnableThread* pNoticePatchThread = NULL;

void AavaUINoticePatcher::Download()
{
	FUINoticePatcher *pPatcher = new FUINoticePatcher(this);

	// �ڵ����� NoticePatchThread�� �����ǰ�, new FUINoticePatcher�� ���ؼ��� �ڵ� ���� �ǵ��� �Ѵ�.
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

UTexture2D* AavaUINoticePatcher::LoadImage(const FString& FullFilename)
{
	FString tmp = FullFilename.Right( FullFilename.Len() - (FullFilename.InStr(TEXT("\\"), TRUE) + 1) );
	FString key = PrefixName + tmp.Left( tmp.InStr(TEXT(".")) );

//	debugf(TEXT("### before: AavaUINoticePatcher.key = %s"), *key);

	// �ܺ� ������ �о�鿩�� �����Ѵ�.
	// (���� key���� �ִٸ� ���� �ؽ��Ĵ� ���ŵǰ� ���� ����� ������)
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
