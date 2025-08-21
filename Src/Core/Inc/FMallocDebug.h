/*=============================================================================
	FMallocDebug.h: Debug memory allocator.
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** Whether to track the age and thread of mallocs */
#define MALLOC_AGE_AND_THREAD_TRACKING	0

extern FRunnableThread* GRenderingThread;

// Debug memory allocator.
class FMallocDebug : public FMalloc
{
	// Tags.
	enum {MEM_PreTag =0xf0ed1cee};
	enum {MEM_PostTag=0xdeadf00f};
	enum {MEM_Tag    =0xfe      };
	enum {MEM_WipeTag=0xcd      };

	// Alignment.
	enum {ALLOCATION_ALIGNMENT=16};

	// Number of block sizes to collate (in steps of 4 bytes)
	enum {MEM_SizeMax = 128};
	enum {MEM_Recent = 5000};
	enum {MEM_AgeMax = 80};
	enum {MEM_AgeSlice = 100};

private:
	// Structure for memory debugging.
	struct FMemDebug
	{
		SIZE_T		Size;
		INT			RefCount;
#if MALLOC_AGE_AND_THREAD_TRACKING
		DWORD		Thread;
		DOUBLE		Seconds;
#endif
		INT*		PreTag;
		FMemDebug*	Next;
		FMemDebug**	PrevLink;
	};

	// Variables.
	FMemDebug*	GFirstDebug;
	UBOOL		MemInited;
	/** Total size of allocations */
	SIZE_T		TotalAllocationSize;
	DWORD		ThreadBeingTracked;

public:
	// FMalloc interface.
	FMallocDebug()
	:	GFirstDebug( NULL )
	,	MemInited( 0 )
	,	TotalAllocationSize( 0 )
	,	ThreadBeingTracked( 0 )
	{}

	void* Malloc( DWORD Size, DWORD Alignment )
	{
		check(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in this allocator");
		checkSlow(MemInited);
		FMemDebug* Ptr = NULL;
		Ptr = (FMemDebug*)malloc( sizeof(FMemDebug) + sizeof(FMemDebug*) + sizeof(INT) + ALLOCATION_ALIGNMENT + Size + sizeof(INT) );
		check(Ptr);
		BYTE* AlignedPtr = Align( (BYTE*) Ptr + sizeof(FMemDebug) + sizeof(FMemDebug*) + sizeof(INT), ALLOCATION_ALIGNMENT );
		Ptr->RefCount = 1;
#if MALLOC_AGE_AND_THREAD_TRACKING
		Ptr->Thread = appGetCurrentThreadId();
		Ptr->Seconds = appSeconds();
#endif
		if( appGetCurrentThreadId() == ThreadBeingTracked )
		{
			INC_DWORD_STAT_BY( STAT_GameToRendererMalloc, Size );
			INC_DWORD_STAT_BY( STAT_GameToRendererNet, Size );
			//GMalloc->LogBacktrace( NULL, Size );
		}

		Ptr->Size = Size;
		Ptr->Next = GFirstDebug;
		Ptr->PrevLink = &GFirstDebug;
		Ptr->PreTag = (INT*) (AlignedPtr - sizeof(INT));
		*Ptr->PreTag = MEM_PreTag;
		*((FMemDebug**)(AlignedPtr - sizeof(INT) - sizeof(FMemDebug*)))	= Ptr;
		*(INT*)(AlignedPtr+Size) = MEM_PostTag;
		appMemset( AlignedPtr, MEM_Tag, Size );
		if( GFirstDebug )
		{
			check(GIsCriticalError||GFirstDebug->PrevLink==&GFirstDebug);
			GFirstDebug->PrevLink = &Ptr->Next;
		}
		GFirstDebug = Ptr;
		TotalAllocationSize += Size;
		check(!(PTRINT(AlignedPtr) & ((PTRINT)0xf)));
		return AlignedPtr;
	}

	void* Realloc( void* InPtr, DWORD NewSize, DWORD Alignment )
	{
		check(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in this allocator");
		checkSlow(MemInited);
		if( InPtr && NewSize )
		{
			FMemDebug* Ptr = *((FMemDebug**)((BYTE*)InPtr - sizeof(INT) - sizeof(FMemDebug*)));
			check(GIsCriticalError||(Ptr->RefCount==1));
			void* Result = Malloc( NewSize, Alignment );
			appMemcpy( Result, InPtr, Min<SIZE_T>(Ptr->Size,NewSize) );
			Free( InPtr );
			return Result;
		}
		else if( InPtr == NULL )
		{
			return Malloc( NewSize, Alignment );
		}
		else
		{
			Free( InPtr );
			return NULL;
		}
	}

	void Free( void* InPtr )
	{
		checkSlow(MemInited);
		if( !InPtr )
		{
			return;
		}
		
		FMemDebug* Ptr = *((FMemDebug**)((BYTE*)InPtr - sizeof(INT) - sizeof(FMemDebug*)));

		check(GIsCriticalError||Ptr->RefCount==1);
		check(GIsCriticalError||*Ptr->PreTag==MEM_PreTag);
		check(GIsCriticalError||*(INT*)((BYTE*)InPtr+Ptr->Size)==MEM_PostTag);
	
		if( appGetCurrentThreadId() == ThreadBeingTracked )
		{
			INC_DWORD_STAT_BY( STAT_GameToRendererFree, Ptr->Size );
			DEC_DWORD_STAT_BY( STAT_GameToRendererNet, Ptr->Size );
			// GMalloc->LogBacktrace( NULL, Size );
		}

		TotalAllocationSize -= Ptr->Size;
		appMemset( InPtr, MEM_WipeTag, Ptr->Size );	
		Ptr->Size = 0;
		Ptr->RefCount = 0;

		check(GIsCriticalError||Ptr->PrevLink);
		check(GIsCriticalError||*Ptr->PrevLink==Ptr);
		*Ptr->PrevLink = Ptr->Next;
		if( Ptr->Next )
		{
			Ptr->Next->PrevLink = Ptr->PrevLink;
		}

		free( Ptr );
	}
	/**
	 * Gathers memory allocations for both virtual and physical allocations.
	 *
	 * @param Virtual	[out] size of virtual allocations
	 * @param Physical	[out] size of physical allocations	
	 */
	void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
	{
		Virtual		= TotalAllocationSize;
		Physical	= 0;
	}

	/** */
	void BeginTrackingThread( void )
	{
		check( !ThreadBeingTracked );
		ThreadBeingTracked = appGetCurrentThreadId();
	}
	/** */
	void EndTrackingThread( void )
	{
		ThreadBeingTracked = 0;
	}

	FString GetThreadName( DWORD ThreadId )
	{
		if( GRenderingThread && GRenderingThread->GetThreadID() == ThreadId )
		{
			return( TEXT( "Rendering" ) );
		}

		return( TEXT( "Other" ) );
	}
	void DumpAllocs( UBOOL bSummaryOnly, FOutputDevice& Ar )
	{
		INT Count = 0;
		INT Chunks = 0;
#if MALLOC_AGE_AND_THREAD_TRACKING
		SIZE_T Index;
		DOUBLE Seconds = appSeconds();

		if( !bSummaryOnly )
		{
			TArray<DWORD>		Threads;

			for( FMemDebug* Mem = GFirstDebug; Mem; Mem = Mem->Next )
			{
				// Make sure QPC is working as expected
				check( fabs( Seconds - Mem->Seconds ) < 365 * 24 * 60 * 60 );

				INT Index = Threads.FindItemIndex( Mem->Thread );
				if( Index == INDEX_NONE )
				{
					Threads.AddItem( Mem->Thread );
				}
			}

			Ar.Logf( TEXT( "Found: %i thread(s)" ), Threads.Num() );

			for( INT i = 0; i < Threads.Num(); i++ )
			{
				TArray<INT>		MemSizes;
				TArray<INT>		MemAges;
				UBOOL			Found;

				MemSizes.Empty( MEM_SizeMax );
				MemSizes.AddZeroed( MEM_SizeMax );
				MemAges.Empty( MEM_AgeMax );
				MemAges.AddZeroed( MEM_AgeMax );
				Found = FALSE;

				for( FMemDebug* Mem = GFirstDebug; Mem; Mem = Mem->Next )
				{
					if( Mem->Thread == Threads( i ) )
					{
						// Collate sizes
						if( ( Seconds - Mem->Seconds ) * 1000.0 < MEM_Recent )
						{
							Index = ( ( Mem->Size + 3 ) & 0xfffffffc ) >> 2;
							if( Index >= MEM_SizeMax )
							{
								Index = MEM_SizeMax - 1;
							}

							MemSizes( Index )++;
							Found = TRUE;
						}

						// Collate ages
						Index = INT( ( Seconds - Mem->Seconds ) * 1000.0 ) / MEM_AgeSlice;
						if( Index < MEM_AgeMax )
						{
							MemAges( Index )++;
							Found = TRUE;
						}
					}
				}

				if( Found )
				{
					// Output data about the number of blocks allocated recently
					Ar.Logf( TEXT( "" ) );

					FString ThreadName = GetThreadName( Threads( i ) );
					Ar.Logf( TEXT( "Thread %i - %s:" ), i, *ThreadName );
					Ar.Logf( TEXT( "Memory allocs in the past %ims (size,count):" ), ( INT )MEM_Recent );

					FString RecentFilename = FString::Printf( TEXT( "%sMallocRecent%i-%s(%ims)-%s-%i-%s.csv" ), *appGameLogDir(), i, *ThreadName, ( INT )MEM_Recent, GGameName, GEngineVersion, *appSystemTimeString() );	
					FArchive* RecentFile = GFileManager->CreateFileWriter( *RecentFilename );

					for( INT j = 0; j < MEM_SizeMax; j++ )
					{
						FString MallocSize = FString::Printf( TEXT( "%i,%i" ), j * 4, MemSizes( j ) );
						// Suppress empty entries for debug spew
						if( MemSizes( j ) )
						{
							Ar.Logf( *MallocSize );
						}

						if( RecentFile )
						{
							MallocSize += LINE_TERMINATOR;
							RecentFile->Serialize( TCHAR_TO_ANSI( *MallocSize ), MallocSize.Len() );
						}
					}

					if( RecentFile )
					{
						RecentFile->Close();
						delete RecentFile;
					}

					Ar.Logf( TEXT( "" ) );

					// Output data about the age of each memory block
					Ar.Logf( TEXT( "Memory block age (ms,count):" ) );

					FString AgeFilename = FString::Printf( TEXT( "%sMallocAge%i-%s(ms)-%s-%i-%s.csv" ), *appGameLogDir(), i, *ThreadName, GGameName, GEngineVersion, *appSystemTimeString() );	
					FArchive* AgeFile = GFileManager->CreateFileWriter( *AgeFilename );				

					for( INT j = 0; j < MEM_AgeMax; j++ )
					{
						FString MallocAge = FString::Printf( TEXT( "%i,%i" ), j * MEM_AgeSlice, MemAges( j ) );
						if( MemAges( j ) )
						{
							Ar.Logf( *MallocAge );
						}

						if( AgeFile )
						{
							MallocAge += LINE_TERMINATOR;
							RecentFile->Serialize( TCHAR_TO_ANSI( *MallocAge ), MallocAge.Len() );
						}
					}

					if( AgeFile )
					{
						AgeFile->Close();
						delete AgeFile;
					}
				}
			}
		}
#endif // MALLOC_AGE_AND_THREAD_TRACKING
		Ar.Logf( TEXT( "" ) );
		Ar.Logf( TEXT( "Unfreed memory:" ) );
		for( FMemDebug* Ptr = GFirstDebug; Ptr; Ptr = Ptr->Next )
		{
			//debugf( TEXT("   % 10i <%s>"), Ptr->Size ); // This is usually not that interesting information.
			Count += Ptr->Size;
			Chunks++;
		}

		Ar.Logf( TEXT( "End of list: %i Bytes still allocated" ), Count );
		Ar.Logf( TEXT( "             %i Chunks allocated" ), Chunks );
	}

	void HeapCheck()
	{
		for( FMemDebug** Link = &GFirstDebug; *Link; Link=&(*Link)->Next )
		{
			check(GIsCriticalError||*(*Link)->PrevLink==*Link);
		}
#if (defined _MSC_VER)
		INT Result = _heapchk();
		check(Result!=_HEAPBADBEGIN);
		check(Result!=_HEAPBADNODE);
		check(Result!=_HEAPBADPTR);
		check(Result!=_HEAPEMPTY);
		check(Result==_HEAPOK);
#endif
	}
	void Init()
	{
		check(!MemInited);
		MemInited=1;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

