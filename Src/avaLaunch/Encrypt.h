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


//! Download�� �Ϸ�Ǹ� �뺸�� �ִ� �ڵ鷯.
class DownloadEventHandler
{
public:
	//! Download�� �Ϸ�Ǹ� �ش� �����̸��� �Բ� �Լ��� ȣ��Ǿ� ����.
	virtual void OnDownloaded( const FString &Filename, DWORD Size ) = 0;
};


/*! @brief URL���ϵ� ���� �� �ֵ��� Ȯ��.(2007/06/18 ����)
	@note
*/
class FAvaFileManagerWindowsInternet : public FAvaFileManagerWindows, public DownloadEventHandler
{
protected:
	FString					DownloadDir;
	TMap<FName, FRunnable*>	DownloaderMap;

public:
	FAvaFileManagerWindowsInternet();

	/*! @brief ���ϸ��� 'http'�� �����ϸ� URL���� ��´�.
		@param InFilename.
			���ͳݿ��� ��� ���ؼ��� "http://pmang.sayclub.com/ava/icon/clan001.png"����
			�������� �־��ָ� �ȴ�.
		@param Flags
			FILEREAD_NoFail, FILEREAD_Uncached
		@param Error
			������ ��� �α׸� ���� �� �ִ�.
	*/
	FArchive* CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error );

protected:
	//! Download�� �Ϸ�Ǹ� �ش� �����̸��� �Բ� �Լ��� ȣ��Ǿ� ����.
	void OnDownloaded( const FString &Filename, DWORD Size );
};

extern FAvaFileManagerWindowsInternet				FileManager;