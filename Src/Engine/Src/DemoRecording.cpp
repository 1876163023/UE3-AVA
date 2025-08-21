/*=============================================================================
DemoRecDrv.cpp: Unreal demo recording network driver.
Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
* Created by Jack Porter.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"
#include "DemoRecording.h"
#define PACKETSIZE 512

/*-----------------------------------------------------------------------------
UDemoRecConnection.
-----------------------------------------------------------------------------*/

UDemoRecConnection::UDemoRecConnection()
{
	MaxPacket   = PACKETSIZE;
	InternalAck = 1;
}

/**
* Intializes a "addressless" connection with the passed in settings
*
* @param InDriver the net driver associated with this connection
* @param InState the connection state to start with for this connection
* @param InURL the URL to init with
* @param InConnectionSpeed Optional connection speed override
*/
void UDemoRecConnection::InitConnection(UNetDriver* InDriver, EConnectionState InState, const FURL& InURL, INT InConnectionSpeed)
{
	// default implementation
	Super::InitConnection(InDriver, InState, InURL, InConnectionSpeed);

	// the driver must be a DemoRecording driver (GetDriver makes assumptions to avoid Cast'ing each time)
	check(InDriver->IsA(UDemoRecDriver::StaticClass()));
}

UDemoRecDriver* UDemoRecConnection::GetDriver()
{
	return (UDemoRecDriver*)Driver;
}

FString UDemoRecConnection::LowLevelGetRemoteAddress()
{
	return TEXT("");
}

void UDemoRecConnection::LowLevelSend( void* Data, INT Count )
{
	if( !GetDriver()->ServerConnection )
	{
		*GetDriver()->FileAr << GetDriver()->LastDeltaTime << GetDriver()->FrameNum << Count;
		GetDriver()->FileAr->Serialize( Data, Count );
		//!!if GetDriver()->GetFileAr()->IsError(), print error, cancel demo recording
	}
}

FString UDemoRecConnection::LowLevelDescribe()
{
	return TEXT("Demo recording driver connection");
}

INT UDemoRecConnection::IsNetReady( UBOOL Saturate )
{
	return 1;
}

void UDemoRecConnection::FlushNet()
{
	// in playback, there is no data to send except
	// channel closing if an error occurs.
	if( !GetDriver()->ServerConnection )
		Super::FlushNet();
}

void UDemoRecConnection::HandleClientPlayer( APlayerController* PC )
{
	Super::HandleClientPlayer(PC);
	PC->bDemoOwner = 1;
}

IMPLEMENT_CLASS(UDemoRecConnection);

/*-----------------------------------------------------------------------------
UDemoRecDriver.
-----------------------------------------------------------------------------*/

UDemoRecDriver::UDemoRecDriver()
{}
UBOOL UDemoRecDriver::InitBase( UBOOL Connect, FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error )
{
	DemoFilename   = ConnectURL.Map;
	Time           = 0;
	FrameNum       = 0;
	DemoEnded      = 0;

	return 1;
}

UBOOL UDemoRecDriver::InitConnect( FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error )
{
	// handle default initialization
	if (!Super::InitConnect(InNotify, ConnectURL, Error))
	{
		return FALSE;
	}
	if (!InitBase(1, InNotify, ConnectURL, Error))
	{
		return FALSE;
	}

	// Playback, local machine is a client, and the demo stream acts "as if" it's the server.
	ServerConnection = ConstructObject<UDemoRecConnection>(UDemoRecConnection::StaticClass());
	ServerConnection->InitConnection(this, USOCK_Pending, ConnectURL, 1000000);

	// open the pre-recorded demo file
	FileAr = GFileManager->CreateFileReader(*DemoFilename);
	if( !FileAr )
	{
		Error = FString::Printf( TEXT("Couldn't open demo file %s for reading"), *DemoFilename );//!!localize!!
		return 0;
	}

	// Skip the new Demo Info Header
	FileAr->Seek(17);
	INT HeaderCheck;
	*FileAr << HeaderCheck;

	if (HeaderCheck != 1038)
	{
		delete FileAr;
		FileAr = NULL;
		Error = FString::Printf( TEXT("Incompatible Demo file (probably an earlier version)"), *DemoFilename );//!!localize!!
		return 0;
	}

	// Skip the headers 

	FString Str;
	INT    I;

	*FileAr << Str;
	*FileAr << Str;
	*FileAr << I;
	*FileAr << I;
#if CLIENT_DEMO
	*FileAr << I;
#endif
	*FileAr << Str;
	*FileAr << Str;

	FString FileName;
	FGuid	GUID;
	INT		PkgCount, Gen;

	*FileAr << PkgCount;

	for (INT i=0;i<PkgCount;i++)
	{
		*FileAr << FileName;
		*FileAr << GUID;
		*FileAr << Gen;
	}

	LoopURL = ConnectURL;
	NoFrameCap          = ConnectURL.HasOption(TEXT("timedemo"));
	Loop				= ConnectURL.HasOption(TEXT("loop"));

	LastFrameTime = appSeconds();

	return 1;
}
UBOOL UDemoRecDriver::InitListen( FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error )
{
	if( !Super::InitListen( InNotify, ConnectURL, Error ) )
		return 0;
	if( !InitBase( 0, InNotify, ConnectURL, Error ) )
		return 0;

	class AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	if ( !WorldInfo )
	{
		Error = TEXT("No WorldInfo!!");
		return FALSE;
	}

	// Recording, local machine is server, demo stream acts "as if" it's a client.
	UDemoRecConnection* Connection = ConstructObject<UDemoRecConnection>(UDemoRecConnection::StaticClass());
	Connection->InitConnection(this, USOCK_Open, ConnectURL, 1000000);
	Connection->InitOut();

	FileAr = GFileManager->CreateFileWriter( *DemoFilename );
	ClientConnections.AddItem( Connection );

	if( !FileAr )
	{
		Error = FString::Printf( TEXT("Couldn't open demo file %s for writing"), *DemoFilename );//localize!!
		return 0;
	}

	// Setup

	UGameEngine* GameEngine = CastChecked<UGameEngine>(GEngine);
	//	INT ClientDemo = INT(WorldInfo->NetMode == NM_Client);

	// Build package map.

#if CLIENT_DEMO
	if( ClientDemo)
		MasterMap->CopyLinkers( GetLevel()->NetDriver->ServerConnection->PackageMap );
	else
#endif
	{
		MasterMap->AddNetPackages();
		// fixup the RemoteGeneration to be LocalGeneration
		for (INT InfoIndex = 0; InfoIndex < MasterMap->List.Num(); InfoIndex++)
		{
			FPackageInfo& Info = MasterMap->List(InfoIndex);
			Info.RemoteGeneration = Info.LocalGeneration;
		}
		MasterMap->Compute();

#if WILL_NEED
		UPackage::NetObjectNotify = NetDriver;
#endif



#if MAYBE_NEEDED
		UBOOL bNeedsReCompute=false;
		for (int pkg=0;pkg<MasterMap->List.Num();pkg++)
		{
			if ( MasterMap->List(pkg).URL.Right(4) == TEXT(".utx") )
			{
				GWarn->Logf(TEXT("Fixing up %s from %i to 1"),*MasterMap->List(pkg).URL,MasterMap->List(pkg).RemoteGeneration);
				MasterMap->List(pkg).RemoteGeneration = 1;
				bNeedsReCompute=true;
			}

			if (bNeedsReCompute)
				MasterMap->Compute();
		}
#endif
	}

	// Output the Demo Header information

	BYTE VersionText[17];  // UT2004 Demo File<ctrl-z> for nice /type support :)
	VersionText[0]=0x55;   
	VersionText[1]=0x54;
	VersionText[2]=0x32;
	VersionText[3]=0x30;
	VersionText[4]=0x30;
	VersionText[5]=0x34;
	VersionText[6]=0x20;
	VersionText[7]=0x44;
	VersionText[8]=0x45;
	VersionText[9]=0x4D;
	VersionText[10]=0x4F;
	VersionText[11]=0x20;
	VersionText[12]=0x46;
	VersionText[13]=0x49;
	VersionText[14]=0x4C;
	VersionText[15]=0x45;
	VersionText[16]=0x1A;

	FileAr->Serialize(VersionText,17);

	// Output a quick check, basically the sum of the characters above

	INT i,j=0;
	for (i=0;i<17;i++)
		j+=VersionText[i];

	*FileAr << j;

	// Output the actual demo header.  This holds all of the important information regarding the demo

	FString T;
	*FileAr << GWorld->URL.Map;
	if ( WorldInfo->Game )
	{
		T = FString::Printf(TEXT("%s"),*WorldInfo->Game->GetClass()->GetPathName());
	}
	else if ( WorldInfo->GRI )
	{
		T = FString::Printf(TEXT("%s"), *WorldInfo->GRI->GameClass->GetName() );
	}

	*FileAr << T; 

	*FileAr << WorldInfo->GRI->GoalScore;
	*FileAr << WorldInfo->GRI->TimeLimit;
#if CLIENT_DEMO
	*FileAr << ClientDemo;
#endif

	T=FString::Printf( TEXT("%s"), 
		WorldInfo->NetMode == NM_DedicatedServer
		? (WorldInfo->GRI->ShortName != TEXT("") 
		? *WorldInfo->GRI->ShortName 
		: *WorldInfo->GRI->ServerName) 
		: GWorld->URL.GetOption(TEXT("Name="),TEXT("")));
	*FileAr << T;

	T=FString::Printf(TEXT("%s"),appTimestamp());
	*FileAr << T;

	INT PkgCount = MasterMap->List.Num();
	*FileAr << PkgCount;

	FString PackageName;
	for (int pkg=0;pkg<MasterMap->List.Num();pkg++)
	{
		PackageName = FString::Printf(TEXT("%s"),*MasterMap->List(pkg).Parent->GetName());
		*FileAr << PackageName;
		*FileAr << MasterMap->List(pkg).Guid;
		*FileAr << MasterMap->List(pkg).RemoteGeneration;
	}

	// Create the control channel.
	Connection->CreateChannel( CHTYPE_Control, 1, 0 );

	// Send initial message.
	Connection->Logf( TEXT("HELLO P=%i REVISION=0 MINVER=%i VER=%i"),
		(INT)appGetPlatformType(), GEngineMinNetVersion, GEngineVersion );
	Connection->FlushNet();

	// Welcome the player to the level.
	FString WelcomeExtra;

	if( WorldInfo->NetMode == NM_Client )
		WelcomeExtra = TEXT("NETCLIENTDEMO");
	else
		if( WorldInfo->NetMode == NM_Standalone )
			WelcomeExtra = TEXT("CLIENTDEMO");
		else
			WelcomeExtra = TEXT("SERVERDEMO");

	WelcomeExtra = WelcomeExtra + FString::Printf(TEXT(" TICKRATE=%d"), WorldInfo->NetMode == NM_DedicatedServer ? appRound(GameEngine->GetMaxTickRate(0, FALSE)) : appRound(NetServerMaxTickRate) );

	GWorld->WelcomePlayer(Connection, *WelcomeExtra);

	// Spawn the demo recording spectator.
#if CLIENT_DEMO
	if( !ClientDemo )
#endif
	{
		SpawnDemoRecSpectator(Connection);
	}
#if CLIENT_DEMO
	else
	{
		GWorld->NetDriver->ServerConnection->Actor->eventStartClientDemoRec();
	}
#endif

	return 1;
}
void UDemoRecDriver::StaticConstructor()
{
	new(GetClass(),TEXT("DemoSpectatorClass"), RF_Public)UStrProperty(CPP_PROPERTY(DemoSpectatorClass), TEXT("Client"), CPF_Config);
}
void UDemoRecDriver::LowLevelDestroy()
{
	debugf( TEXT("Closing down demo driver.") );

	// Shut down file.
	if( FileAr )
	{	
		delete FileAr;
		FileAr = NULL;
	}
}
UBOOL UDemoRecDriver::UpdateDemoTime( FLOAT* DeltaTime, FLOAT TimeDilation )
{
	UBOOL Result = 0;
	bNoRender = false;

	if( ServerConnection )
	{
		// Ensure LastFrameTime is inside a valid range, so we don't lock up if things get very out of sync.
		LastFrameTime = Clamp<DOUBLE>( LastFrameTime, appSeconds() - 1.0, appSeconds() );

		// Playback
		FrameNum++;
#if MAYBE_NEED
		if( Notify->NotifyGetLevel() && Notify->NotifyGetLevel()->FinishedPrecaching && InitialFrameStart == 0 )
#endif
		{
			PlaybackStartTime = appSeconds();
			InitialFrameStart = FrameNum;
		}

		if( ServerConnection->State==USOCK_Open ) 
		{
			if( !FileAr->AtEnd() && !FileAr->IsError() )
			{
				// peek at next delta time.
				FLOAT NewDeltaTime;
				INT NewFrameNum;

				*FileAr << NewDeltaTime << NewFrameNum;
				FileAr->Seek(FileAr->Tell() - sizeof(NewDeltaTime) - sizeof(NewFrameNum));

				// If the real delta time is too small, sleep for the appropriate amount.
				if( !NoFrameCap )
				{
					if ( !GIsBenchmarking && (appSeconds() > LastFrameTime+(DOUBLE)NewDeltaTime/(1.1*TimeDilation)) )
					{
						bNoRender = true;
					}
					else
					{
						while(appSeconds() < LastFrameTime+(DOUBLE)NewDeltaTime/(1.1*TimeDilation))
						{
							appSleep(0);
						}
					}
				}
				// Lie to the game about the amount of time which has passed.
				*DeltaTime = NewDeltaTime;
			}
		}
		LastDeltaTime = *DeltaTime;
		LastFrameTime = appSeconds();
	}
	else
	{
		// Recording
		BYTE NetMode = GWorld->GetWorldInfo()->NetMode;

		// Accumulate the current DeltaTime for the real frames this demo frame will represent.
		DemoRecMultiFrameDeltaTime += *DeltaTime;

		// Cap client demo recording rate (but not framerate).
		if( NetMode==NM_DedicatedServer || ( (appSeconds()-LastClientRecordTime) >= (DOUBLE)(1.f/NetServerMaxTickRate) ) )
		{
			// record another frame.
			FrameNum++;
			LastClientRecordTime = appSeconds();
			LastDeltaTime = DemoRecMultiFrameDeltaTime;
			DemoRecMultiFrameDeltaTime = 0.f;
			Result = 1;

			// Save the new delta-time and frame number, with no data, in case there is nothing to replicate.
			INT Count = 0;
			*FileAr << LastDeltaTime << FrameNum << Count;
		}
	}

	return Result;
}
void UDemoRecDriver::TickDispatch( FLOAT DeltaTime )
{
	Super::TickDispatch( DeltaTime );

	if( ServerConnection && (ServerConnection->State==USOCK_Pending || ServerConnection->State==USOCK_Open) )
	{	
		BYTE Data[PACKETSIZE + 8];
		// Read data from the demo file
		DWORD PacketBytes;
		INT PlayedThisTick = 0;
		for( ; ; )
		{
			// At end of file?
			if( FileAr->AtEnd() || FileAr->IsError() )
			{
AtEnd:
				ServerConnection->State = USOCK_Closed;
				DemoEnded = 1;

				FLOAT Seconds = appSeconds()-PlaybackStartTime;
				if( NoFrameCap )
				{
					FString Result = FString::Printf(TEXT("Demo %s ended: %d frames in %lf seconds (%.3f fps)"), *DemoFilename, FrameNum-InitialFrameStart, Seconds, (FrameNum-InitialFrameStart)/Seconds );
					debugf(TEXT("%s"),*Result);
					ServerConnection->Actor->eventClientMessage( *Result, NAME_None );//!!localize!!

					if( ParseParam(appCmdLine(),TEXT("EXITAFTERDEMO")) )
					{
						// Output average framerate.
						FString OutputString	= TEXT("");
						INT		LastRand		= appRand();
						appLoadFileToString( OutputString, TEXT("..\\Benchmark\\benchmark.log") );
						OutputString += FString::Printf(TEXT("%f fps         rand[%i]\r\n"), (FrameNum-InitialFrameStart)/Seconds, LastRand );
						appSaveStringToFile( OutputString, TEXT("..\\Benchmark\\benchmark.log") );

						// Get time & date.
						INT Year, Month, DayOfWeek, Day, Hour, Minutes, Sec, MSec;
						appSystemTime( Year, Month, DayOfWeek, Day, Hour, Minutes, Sec, MSec );
						FString DateTime = FString::Printf(TEXT("%i-%02i-%02i-%02i-%02i-%02i"),Year,Month,Day,Hour,Minutes,Sec);

						// Output average framerate to dedicated file.
#if MAYBE_NEED
						OutputString  = FString::Printf(TEXT("%s\r\n%s\r\n%s\r\n%s\r\n\r\n%s\r\n\r\n"),GBuildLabel,GMachineOS,GMachineCPU,GMachineVideo,appCmdLine());
#endif
						OutputString += FString::Printf(TEXT("%f fps         rand[%i]\r\n"), (FrameNum-InitialFrameStart)/Seconds, LastRand );
						appSaveStringToFile( OutputString, *FString::Printf(TEXT("..\\Benchmark\\Results\\avgfps-%s.log"), *DateTime ) );

#if MAYBE_NEED
						// Output average for benchmark launcher.
						if( ParseParam(appCmdLine(),TEXT("UPT") ) )
						{
							OutputString = FString::Printf(TEXT("%f"), (FrameNum-InitialFrameStart)/Seconds );
							appSaveStringToFile( OutputString, TEXT("dummy.ben") );
						}
#endif

						// Copy the log.
						GLog->Flush();
#if MAYBE_NEED
						GFileManager->Copy( *FString::Printf(TEXT("..\\Benchmark\\Logs\\ut2004-%s.log"),*DateTime), TEXT("ut2004.log") );
#endif

						// Exit.
						GIsRequestingExit = 1;
						//appRequestExit(0);
					}
				}
				else
				{
					ServerConnection->Actor->eventClientMessage( *FString::Printf(TEXT("Demo %s ended: %d frames in %f seconds"), *DemoFilename, FrameNum-InitialFrameStart, Seconds ), NAME_None );//!!localize!!
				}

				if( Loop )
				{
					GWorld->Exec( *(FString(TEXT("DEMOPLAY "))+(*LoopURL.String())), *GLog );
				}
				return;
			}

			INT ServerFrameNum;
			FLOAT ServerDeltaTime;

			*FileAr << ServerDeltaTime;
			*FileAr << ServerFrameNum;
			if( ServerFrameNum > FrameNum )
			{
				FileAr->Seek(FileAr->Tell() - sizeof(ServerFrameNum) - sizeof(ServerDeltaTime));
				break;
			}
			*FileAr << PacketBytes;

			if( PacketBytes )
			{
				// Read data from file.
				FileAr->Serialize( Data, PacketBytes );
				if( FileAr->IsError() )
				{
					debugf( NAME_DevNet, TEXT("Failed to read demo file packet") );
					goto AtEnd;
				}

				// Update stats.
				PlayedThisTick++;

				// Process incoming packet.
				ServerConnection->ReceivedRawPacket( Data, PacketBytes );
			}

			// Only play one packet per tick on demo playback, until we're 
			// fully connected.  This is like the handshake for net play.
			if(ServerConnection->State == USOCK_Pending)
			{
				FrameNum = ServerFrameNum;
				break;
			}
		}
	}
}
FString UDemoRecDriver::LowLevelGetNetworkNumber()
{
	return TEXT("");
}

INT UDemoRecDriver::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( DemoEnded )
		return 0;
	if( ParseCommand(&Cmd,TEXT("DEMOREC")) || ParseCommand(&Cmd,TEXT("DEMOPLAY")) )
	{
		if( ServerConnection )
			Ar.Logf( TEXT("Demo playback currently active: %s"), *DemoFilename );//!!localize!!
		else
			Ar.Logf( TEXT("Demo recording currently active: %s"), *DemoFilename );//!!localize!!
		return 1;
	}
	else if( ParseCommand(&Cmd,TEXT("DEMOSTOP")) )
	{
		Loop = 0;
		Ar.Logf( TEXT("Demo %s stopped at frame %d"), *DemoFilename, FrameNum );//!!localize!!
		if( !ServerConnection )
		{
			GWorld->DemoRecDriver=NULL;
			// let GC cleanup the object
		}
		else
		{
			ServerConnection->State = USOCK_Closed;
		}

		delete FileAr;
		FileAr = NULL;
		return 1;
	}
	else return 0;
}

void UDemoRecDriver::SpawnDemoRecSpectator( UNetConnection* Connection )
{
	UClass* C = StaticLoadClass( AActor::StaticClass(), NULL, *DemoSpectatorClass, NULL, LOAD_None, NULL );
	APlayerController* Controller = CastChecked<APlayerController>(GWorld->SpawnActor( C ));

	for (FActorIterator It; It; ++It)
	{
		if (It->IsA(APlayerStart::StaticClass()))
		{
			Controller->Location = It->Location;
			Controller->Rotation = It->Rotation;
			break;
		}
	}

	Connection->Actor = Controller;
}
IMPLEMENT_CLASS(UDemoRecDriver);




/*-----------------------------------------------------------------------------
UDemoPlayPendingLevel implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UDemoPlayPendingLevel::UDemoPlayPendingLevel(const FURL& InURL)
:	UPendingLevel( InURL )
{
	NetDriver = NULL;

	// Try to create demo playback driver.
	UClass* DemoDriverClass = StaticLoadClass( UDemoRecDriver::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.DemoRecordingDevice"), NULL, LOAD_None, NULL );
	DemoRecDriver = ConstructObject<UDemoRecDriver>( DemoDriverClass );
	if( DemoRecDriver->InitConnect( this, URL, Error ) )
	{
	}
	else
	{
		DemoRecDriver = NULL;
	}
}
//
// FNetworkNotify interface.
//
void UDemoPlayPendingLevel::NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text )
{
	//debugf( TEXT("DemoPlayPendingLevel received: %s"), Text );
	if( ParseCommand( &Text, TEXT("USES") ) )
	{
		// Dependency information.
		//		FPackageInfo Info(NULL);
		FPackageInfo& Info = *new(Connection->PackageMap->List)FPackageInfo(NULL);
		Connection->ParsePackageInfo(Text, Info);

		// in the seekfree loading case, we load the requested map first and then attempt to load requested packages that haven't been loaded yet
		// as packages referenced by the map might be forced exports and not actually have a file associated with them
		//@see UGameEngine::LoadMap()
		//@todo: figure out some early-out code to detect when missing downloadable content, etc so we don't have to load the level first
		if( !GUseSeekFreeLoading )
		{
			// verify that we have this package, or it is downloadable
			FString Filename;
			if (GPackageFileCache->FindPackageFile(*Info.PackageName.ToString(), &Info.Guid, Filename))
			{
				Info.Parent = CreatePackage(NULL, *Info.PackageName.ToString());
				// check that the GUID matches (meaning it is the same package or it has been conformed)
				BeginLoad();
				ULinkerLoad* Linker = GetPackageLinker(Info.Parent, NULL, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, &Info.Guid);
				EndLoad();
				if (Linker == NULL || Linker->Summary.Guid != Info.Guid)
				{
					// incompatible files
					//@todo FIXME: we need to be able to handle this better - have the client ignore this version of the package and download the correct one
					debugf(NAME_DevNet, TEXT("Package '%s' mismatched - Server GUID: %s Client GUID: %s"), *Info.Parent->GetName(), *Info.Guid.String(), (Linker != NULL) ? *Linker->Summary.Guid.String() : TEXT("None"));
					Error = FString::Printf(TEXT("Package '%s' version mismatch"), *Info.Parent->GetName());
					NetDriver->ServerConnection->Close();
					return;
				}
				else
				{
					Info.LocalGeneration = Linker->Summary.Generations.Num();
					// tell the server what we have
					DemoRecDriver->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Linker->Summary.Guid.String(), Info.LocalGeneration);
				}
			}
			else
			{
				// we need to download this package
				FilesNeeded++;
				Info.PackageFlags |= PKG_Need;
				if (!NetDriver->AllowDownloads || !(Info.PackageFlags & PKG_AllowDownload))
				{
					Error = FString::Printf( TEXT("Downloading '%s' not allowed"), *Info.Parent->GetName() );
					DemoRecDriver->ServerConnection->Close();
					return;
				}
			}
		}

#if PROLLY_NOT_NEEDED
		// Dependency information.
		FPackageInfo& Info = *new(Connection->PackageMap->List)FPackageInfo(NULL);
		TCHAR PackageName[NAME_SIZE]=TEXT("");
		Parse( Text, TEXT("GUID=" ), Info.Guid );
		Parse( Text, TEXT("GEN=" ),  Info.RemoteGeneration );
		Parse( Text, TEXT("SIZE="),  Info.FileSize );
		Info.DownloadSize = Info.FileSize;
		Parse( Text, TEXT("FLAGS="), Info.PackageFlags );
		Parse( Text, TEXT("PKG="), PackageName, ARRAY_COUNT(PackageName) );
		Parse( Text, TEXT("FNAME="), Info.URL );
		Info.Parent = CreatePackage(NULL,PackageName);
#endif
	}
	else if( ParseCommand( &Text, TEXT("WELCOME") ) )
	{
		//		FURL URL;

		// Parse welcome message.
		Parse( Text, TEXT("LEVEL="), URL.Map );

		UBOOL HadMissing = 0;

		// Make sure all packages we need available
		for( INT i=0; i<Connection->PackageMap->List.Num(); i++ )
		{
			FPackageInfo& Info = Connection->PackageMap->List(i);
			FString Filename;
			if (!GPackageFileCache->FindPackageFile(*Info.PackageName.ToString(), &Info.Guid, Filename))
			{
				// We need to download this package.
				FilesNeeded++;
				Info.PackageFlags |= PKG_Need;

#if MAYBE_NEEDED
#if !DEMOVERSION
				if( DemoRecDriver->ClientRedirectURLs.Num()==0 || !DemoRecDriver->AllowDownloads || !(Info.PackageFlags & PKG_AllowDownload) )
#endif
#endif
				{
					if (GEngine->GamePlayers.Num() )
						GEngine->GamePlayers(0)->Actor->eventClientMessage( *FString::Printf(*LocalizeError(TEXT("DemoFileMissing"),TEXT("Engine")), *Info.Parent->GetName()), NAME_None );

					Error = FString::Printf( *LocalizeError(TEXT("DemoFileMissing"),TEXT("Engine")), *Info.Parent->GetName() );
					Connection->State = USOCK_Closed;
					HadMissing = 1;
				}
			}
		}

		if( HadMissing )
			return;

#if MAYBE_NEEDED
#if !DEMOVERSION
		// Send first download request.
		ReceiveNextFile( Connection, 0 );
#endif
#endif

		DemoRecDriver->Time = 0;
		Success = 1;
	}
}
//
// UPendingLevel interface.
//
void UDemoPlayPendingLevel::Tick( FLOAT DeltaTime )
{
	check(DemoRecDriver);
	check(DemoRecDriver->ServerConnection);

	if( DemoRecDriver->ServerConnection && DemoRecDriver->ServerConnection->Download )
		DemoRecDriver->ServerConnection->Download->Tick();

	if( !FilesNeeded )
	{
		// Update demo recording driver.
		DemoRecDriver->UpdateDemoTime( &DeltaTime, 1.f );
		DemoRecDriver->TickDispatch( DeltaTime );
		DemoRecDriver->TickFlush();
	}
}

UNetDriver* UDemoPlayPendingLevel::GetDriver()
{
	return DemoRecDriver;
}

IMPLEMENT_CLASS(UDemoPlayPendingLevel);

