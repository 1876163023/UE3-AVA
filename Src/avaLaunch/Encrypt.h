class FAvaFileManagerWindows : public FFileManagerWindows
{
public :
	FArchive* CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error );
	FArchive* CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error );

	INT FileSize( const TCHAR* Filename );

	virtual void FindFiles( TArray<FString>& FileNames, const TCHAR* Filename, UBOOL Files, UBOOL Directories );

private :
	FArchive* CreateEncryptedFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error );

public :
	void Encrypt( const TCHAR* InFilename, const TCHAR* InSrcFilename, DWORD Flags, FOutputDevice* Error );
};

void AutoGenerateNamesEncrypt();


//! Download가 완료되면 통보해 주는 핸들러.
class DownloadEventHandler
{
public:
	//! Download가 완료되면 해당 파일이름과 함께 함수가 호출되어 진다.
	virtual void OnDownloaded( const FString &Filename, DWORD Size ) = 0;
};


/*! @brief URL파일도 얻을 수 있도록 확장.(2007/06/18 고광록)
	@note
*/
class FAvaFileManagerWindowsInternet : public FAvaFileManagerWindows, public DownloadEventHandler
{
protected:
	FString					DownloadDir;
	TMap<FName, FRunnable*>	DownloaderMap;

public:
	FAvaFileManagerWindowsInternet();

	/*! @brief 파일명이 'http'로 시작하면 URL에서 얻는다.
		@param InFilename.
			인터넷에서 얻기 위해서는 "http://pmang.sayclub.com/ava/icon/clan001.png"같은
			형식으로 넣어주면 된다.
		@param Flags
			FILEREAD_NoFail, FILEREAD_Uncached
		@param Error
			에러난 경우 로그를 얻을 수 있다.
	*/
	FArchive* CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error );

protected:
	//! Download가 완료되면 해당 파일이름과 함께 함수가 호출되어 진다.
	void OnDownloaded( const FString &Filename, DWORD Size );
};

extern FAvaFileManagerWindowsInternet				FileManager;