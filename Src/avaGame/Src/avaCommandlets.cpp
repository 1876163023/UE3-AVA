#include "avaGame.h"
#include "avaCommandlets.h"

IMPLEMENT_CLASS(UDistConsCheckCommandlet);

FArchive& operator<<( FArchive& Ar, FFileManager::timestamp& TimeStamp )
{
	Ar.Serialize( &TimeStamp, sizeof(TimeStamp) );
	return Ar;
}

FArchive& operator<<( FArchive& Ar, UDistConsCheckCommandlet::FDistFileInfo& DistFileInfo )
{
	Ar << DistFileInfo.FileName << DistFileInfo.FileSize << DistFileInfo.FileTimeStamp;
	return Ar;
}

INT UDistConsCheckCommandlet::Main( const FString& Params )
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	if( Tokens.Num() != 1 )
	{
		warnf( NAME_Log, TEXT("not enough parameters. need an input filename") );
		return -1;
	}
	warnf( NAME_Log, TEXT("get a filelist from file %s"), *Tokens(0) );

	FString FileListStr;
	if( ! appLoadFileToString( FileListStr, *Tokens(0)) )
	{
		warnf( NAME_Warning, TEXT("can't read a filelist from %s"), *Tokens(0) );
		return -1;
	}

	TArray<FString> FileList;
	FileListStr.ParseIntoArrayWS( &FileList );

	TArray<FDistFileInfo> DistFiles, InvDistFiles;
	for( INT FileListIndex = 0 ; FileListIndex < FileList.Num() ; FileListIndex++ )
	{
		const TCHAR* itsFileName = *FileList(FileListIndex);
		FDistFileInfo DistFileInfo;

		DistFileInfo.FileName = itsFileName;
		DistFileInfo.FileSize = GFileManager->FileSize( itsFileName );
		GFileManager->GetTimestamp( itsFileName, DistFileInfo.FileTimeStamp );

		warnf( NAME_Log, TEXT("CheckFile = %s, Size = %s, TimeStamp = %02d:%02d:%02d"), *DistFileInfo.FileName, *FString::FormatAsNumber(DistFileInfo.FileSize), DistFileInfo.FileTimeStamp.Hour, DistFileInfo.FileTimeStamp.Minute, DistFileInfo.FileTimeStamp.Second);

		if( DistFileInfo.FileName.Len() > 0 && DistFileInfo.FileSize >= 0 )
			DistFiles.AddItem(DistFileInfo);
		else
			InvDistFiles.AddItem(DistFileInfo);
	}
	warnf( NAME_Log, TEXT(""));

	// ���Ϸ� ����
	{
		FArchive* Writer = GFileManager->CreateFileWriter( GDistConsFileName ,FILEWRITE_EvenIfReadOnly, GLogConsole);
		check(Writer);

		INT DistFileNum = DistFiles.Num();
		*Writer << DistFileNum;
		for( INT DistFileIndex = 0 ; DistFileIndex < DistFiles.Num() ; DistFileIndex++ )
		{
			*Writer << DistFiles(DistFileIndex);
		}

		Writer->Close();
		delete Writer;
		Writer = NULL;
	}

	// �� ������ �ٽ� �о� Ȯ��
	{
		FArchive* Reader = GFileManager->CreateFileReader( GDistConsFileName, 0,GLogConsole);
		check(Reader);

		INT DistFileNum;
		*Reader << DistFileNum;
		for( INT DistFileIndex = 0 ; DistFileIndex < DistFileNum ; DistFileIndex++ )
		{
			FDistFileInfo DistFileInfo;
			*Reader << DistFileInfo;
			warnf( NAME_Log, TEXT("FileStored = %s, Size = %s, TimeStamp = %02d:%02d:%02d"), *DistFileInfo.FileName, *FString::FormatAsNumber(DistFileInfo.FileSize), DistFileInfo.FileTimeStamp.Hour, DistFileInfo.FileTimeStamp.Minute, DistFileInfo.FileTimeStamp.Second);
		}

		Reader->Close();
		delete Reader;
		Reader = NULL;
	}
	warnf( NAME_Log, TEXT("") );


	// ���������� ����ȿ�� ���ϵ��� �����ش�
	SET_WARN_COLOR(COLOR_RED);
	warnf( NAME_Log, TEXT("---- Invalid Files ----") );
	for( INT InvFileIndex = 0 ; InvFileIndex < InvDistFiles.Num() ; InvFileIndex++ )
	{
		const FDistFileInfo& DistFileInfo = InvDistFiles(InvFileIndex);
		warnf( NAME_Log, TEXT("Invalid = %s, Size = %s, TimeStamp = %02d:%02d:%02d"), *DistFileInfo.FileName, *FString::FormatAsNumber(DistFileInfo.FileSize), DistFileInfo.FileTimeStamp.Hour, DistFileInfo.FileTimeStamp.Minute, DistFileInfo.FileTimeStamp.Second);
	}

	CLEAR_WARN_COLOR();

	return 0;
}

/**
* �������� ���Ἲ�� �˻��Ѵ�
*/
UBOOL CheckDistributionConsistency( TArray<FString>* InconsFiles )
{
	UBOOL bResult = TRUE;
	FArchive* Reader = GFileManager->CreateFileReader( GDistConsFileName, 0,GLogConsole);

	// ������ ������ ���Ἲ�� �˻��Ұ� ���°��̴�.
	if( Reader == NULL && GFileManager->FileSize(GDistConsFileName) < 0)
	{
		return TRUE;
	}

	INT DistFileNum;
	*Reader << DistFileNum;
	for( INT DistFileIndex = 0 ; DistFileIndex < DistFileNum ; DistFileIndex++ )
	{
		UDistConsCheckCommandlet::FDistFileInfo DistFileInfo;
		*Reader << DistFileInfo;
		INT FileSize = GFileManager->FileSize( *DistFileInfo.FileName );
		FFileManager::timestamp FileTimeStamp;
		GFileManager->GetTimestamp( *DistFileInfo.FileName, FileTimeStamp );


		if( FileSize < 0 ||									//������ ��ã�Ұų�
			FileSize != DistFileInfo.FileSize ||			//���� ����� ��ġ���� �ʰų�
			!(FileTimeStamp == DistFileInfo.FileTimeStamp) )	//������ Ÿ�ӽ������� ��ġ���� �ʰų�
		{
			bResult = FALSE;
			if( InconsFiles == NULL )
				break;

			InconsFiles->AddItem( DistFileInfo.FileName );
		}
	}

	Reader->Close();
	delete Reader;
	Reader = NULL;

	return bResult;
}