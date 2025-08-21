
#ifndef _INC_AVA_COMMANDLETS
#define _INC_AVA_COMMANDLETS

#include "FFileManagerWindows.h"

#define GDistConsFileName TEXT("DistConsCheck.bin")

UBOOL CheckDistributionConsistency( TArray<FString>* InconsFiles = NULL );

BEGIN_COMMANDLET(DistConsCheck,avaGame)
	
	struct FDistFileInfo
	{
		FString FileName;
		INT FileSize;
		FFileManager::timestamp FileTimeStamp;

		FDistFileInfo( ) : FileSize(0) {}
	};

END_COMMANDLET

#endif