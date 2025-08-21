#define _CRT_SECURE_NO_DEPRECATE
#include <atlstr.h>
#include <process.h>
#include <windows.h>
#define MPICH_IGNORE_CXX_SEEK
#include "mpi.h"
#include "mpidist.h"
#include "vraddll.h"
#include "bsplib.h"
#include "vrad.h"
//#include "map_shared.h"
#include "lightmap.h"
#include "threads.h"
#include "mpi.h"

struct Result
{
	UINT iWork;
	UINT nBuffers;		
};


static unsigned __stdcall Internal_MPI_Main( LPVOID pData )
{
	Threads* pThis = reinterpret_cast<Threads*>( pData );

	pThis->MPI_Main();

	_endthreadex(0);

	return 0;
}

Threads::Threads()
: m_nThreads( UINT_MAX ), m_pInterface( NULL )
{}

void Threads::SetNumThreads( UINT nThreads )
{
	if (nThreads == UINT_MAX)	// not set manually
	{
		SYSTEM_INFO info;

		GetSystemInfo (&info);
		m_nThreads = info.dwNumberOfProcessors;
		if (m_nThreads < 1) m_nThreads = 1;
		if (m_nThreads > c_nMaxThreads) m_nThreads = c_nMaxThreads;		
	}	
	else
	{
		m_nThreads = nThreads;
	}

	m_nThreads = 1;

//	Msg( "MPI %i threads\n", m_nThreads );
}

UINT Threads::GetThreadWork()
{
	ScopedMutex sm( m_Mutex );

	if (m_nDispatched == m_nWork) return UINT_MAX;

	float fCurrent = m_Timer.GetElapsed();

	/// 0.5초마다 :)
	if (fCurrent - m_fLast > 0.5f || m_nDispatched+1 == m_nWork)
	{
		Msg( "%d/%d (%3d%%)\n", m_nDispatched+1, m_nWork, (m_nDispatched+1) * 100 / m_nWork );
		m_fLast = fCurrent;
	}

	return m_nDispatched++;
}

static unsigned __stdcall InternalMain( LPVOID pData )
{
	Threads::Data& data = *reinterpret_cast<Threads::Data*>( pData );

	data.pFunction( data.iThread, data.pUserData );	

	_endthreadex(0);

	return 0;
}

void Threads::Start( ThreadFunction fn, LPVOID pUserData )
{	
	m_bThreaded = true;	

	for (UINT iThread=0; iThread<m_nThreads; ++iThread)
	{
		m_Data[iThread].iThread = iThread;
		m_Data[iThread].pFunction = fn;
		m_Data[iThread].pUserData = pUserData;		

		HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, InternalMain, &m_Data[iThread], 0, NULL );

		m_Threads[ iThread ] = hThread;

		if (m_bLowPriority)
		{
			SetThreadPriority( hThread, THREAD_PRIORITY_LOWEST );
		}
	}
}

void Threads::End()
{
	WaitForMultipleObjects( m_nThreads, m_Threads, TRUE, INFINITE );

	for (UINT iThread=0; iThread<m_nThreads; ++iThread)
	{
		CloseHandle( m_Threads[ iThread ] );
	}

	m_bThreaded = false;
}

void Threads::RunOn( LPCSTR szWork, UINT nWork, ThreadFunction fn, LPVOID pUserData )
{
	m_WorkProcessed.resize(0);
	m_nDispatched = 0;	
	m_nWork = nWork;	

	Msg( "<<< 시작 : %s >>>\n", szWork );
	m_fLast = 0;

	m_Timer.Reset();	

	if (m_pInterface)
	{		
		m_hMaster = (HANDLE)_beginthreadex( NULL, 0, Internal_MPI_Main, this, 0, NULL );		
	}	

	Start( fn, pUserData );		

	/// join!
	End();

	int mpi_rank, mpi_size;

	MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );		

	if (mpi_size == 1)
		Msg( "<<< 완료 : %s %.3fs >>>\n", szWork, m_Timer.GetElapsed() );	
}

static void _ThreadWorkerFunction( UINT iThread, LPVOID pUserData )
{
	static_cast<Threads*>(pUserData)->WorkerMain( iThread );
}

void Threads::WorkerMain( UINT iThread )
{
	int mpi_rank, mpi_size;

	MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );		

	//Msg( "Start!!!!!!!!!!!!!! %d\n", mpi_rank );

	while (1)
	{
		UINT iWork;

		{
			ScopedMutex sm( m_Mutex );

			iWork = GetThreadWork ();
			if (iWork == UINT_MAX)
				break;

			m_WorkProcessed.push_back( iWork );		
		}		

		m_Worker( iThread, iWork, m_pUserData );
	}

	m_Worker( -1, -1, NULL );
}

void Threads::RunOnIndividual( LPCSTR szWork, UINT nWork, WorkerFunction fn, LPVOID pUserData )
{	
	m_Worker = fn;
	m_pUserData = pUserData;
	
	RunOn( szWork, nWork, _ThreadWorkerFunction, this );
}

char* buf;

void Threads::Update( int count, char* buf )
{
	Result& data = *(Result*)buf;

	char* ptr = buf;
	ptr += sizeof(Result);			

	for (UINT iBuffer = 0; iBuffer < data.nBuffers; ++iBuffer)
	{	
		UINT Size = *(int*)ptr;						

		ptr += 4;

		if (Size)
		{							
			try
			{
				m_pInterface->UpdateResult( data.iWork, iBuffer, ptr, Size );
			}
			catch (...)
			{
				printf( "Error on updating result [%d,%p] %d, %d, %p, %d\n", count, buf, data.iWork, iBuffer, ptr, Size );
				return;
			}			

			ptr += Size;
		}
	}	

	m_nReceived++;
}

bool bUseCache = false;

void Threads::MPI_Run( LPCSTR szWork, UINT nWork, WorkerFunction wf, LPVOID pUserData, MPIInterface* pInterface, bool bMakeCache )
{
	if (!nWork) return;

	m_nReceived = 0;

#define BUFSIZE 1024*1024*64
	buf = (char*)malloc( BUFSIZE );	
	size_t buf_size = BUFSIZE;

	MPI_Barrier( MPI_COMM_WORLD );		

	m_pInterface = pInterface;

	int mpi_rank, mpi_size;

	MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );			
	
	//Msg( "<<< BEGIN : %s %d/%d >>>\n", szWork, mpi_rank, mpi_size );	
	int chk_cache = 0;
	
	if (bMakeCache)
	{
		if (mpi_rank == 0)
		{
			Msg( "Checking cache..." );
			m_nWork = nWork;
			m_szWork = szWork;
			if (bUseCache && ReadCache()) 
			{
				Msg( "OK\n" );
				chk_cache = 1;
				MPI_Bcast( &chk_cache, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD );
				return;
			}

			Msg( "NOT!\n" );

			MPI_Bcast( &chk_cache, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD );
		}	
		else
		{		
			//Msg( "Waiting for cache check work %s\n", m_szWork );

			MPI_Bcast( &chk_cache, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD );

			if (chk_cache)
			{
			//	Msg( "Skip for MPI work %s\n", m_szWork );
				return;
			}

			//Msg( "No cache for work %s\n", m_szWork );
		}
	}	

	if (mpi_rank == 0)
	{		
		MPI_Master( szWork, nWork, wf, pUserData );

		WaitForSingleObject( m_hMaster, INFINITE );
	}
	else
	{
		MPI_Slave( szWork, wf, pUserData );
	}

	//Msg( "<<< END : %s %d/%d >>>\n", szWork, mpi_rank, mpi_size );

	if (mpi_size && mpi_rank == 0)
		Msg( "<<< 대기 : %s >>>\n", szWork );

	//Msg( "<<< MPI_RUN : %s %d/%d >>>\n", szWork, mpi_rank, mpi_size );
	
	m_Timer.Reset();
	m_fLast = 0;

	if (mpi_rank==0 && bMakeCache)
		WriteCache();
	
	free( buf );

	m_pInterface = NULL;

	if (mpi_rank==0)
		Msg( "<<< 완료 : %s >>>\n", szWork );	

	MPI_Barrier( MPI_COMM_WORLD );
}

void Threads::MPI_Main()
{
	m_MPI_Processing = 0;
	m_nDispatchedRemotely = 0;

	int mpi_rank, mpi_size;

	MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );		

	Msg( "Dispatching initial jobs : %d\n", mpi_size );

	Msg( "[[PROGRESS:%s %d %d]]\n", m_szWork, 0, m_nWork );	

	for (int rank=1; rank<mpi_size; ++rank)
	{
		UINT iWork;
		
		iWork = GetThreadWork();
		MPI_Send( &iWork, 1, MPI_UNSIGNED, rank, 100, MPI_COMM_WORLD );				
	}	

	DWORD LastTick, CurTick, LastWork;

	Msg( "[[PROGRESS:%s %d %d]]\n", m_szWork, LastWork = (m_WorkProcessed.size() + m_nReceived), m_nWork );	

	LastTick = ::GetTickCount();

	m_MPI_Processing = mpi_size - 1;

	while (m_MPI_Processing)
	{
		MPI_Status status;		

		MPI_Recv( buf, BUFSIZE, MPI_UNSIGNED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );		

		if (status.MPI_TAG == 100)
		{
			UINT work = *(UINT*)buf;

			CurTick = ::GetTickCount();

			DWORD CurWork = (m_WorkProcessed.size() + m_nReceived);

			if (CurTick - LastTick > 500 || (CurWork*100/m_nWork != LastWork*100/m_nWork) )
			{
				LastTick = CurTick;
				LastWork = CurWork;

				Msg( "[[PROGRESS:%s %d %d]]\n", m_szWork, CurWork, m_nWork );
			}

			if (work != UINT_MAX)
			{
				UINT Works[16];

				UINT N = sizeof(Works)/sizeof(Works[0]);

				if (m_nWork - (m_WorkProcessed.size() + m_nDispatchedRemotely) < mpi_size * 4)
					N = 1;

				UINT NumDispatched = 0;

				for (UINT i=0; i<N; ++i)
				{
					Works[i] = GetThreadWork();

					if (Works[i] == UINT_MAX) break;

					NumDispatched++;
					m_nDispatchedRemotely++;
				}				

				MPI_Send( Works, NumDispatched, MPI_UNSIGNED, status.MPI_SOURCE, 100, MPI_COMM_WORLD );		

				//Msg( "Dispatching job %d to node %d\n", iWork, rank );
			}
			else
			{			
				m_MPI_Processing --;

				Msg( "END from node %d (%d)\n", status.MPI_SOURCE, m_MPI_Processing );
			}
		}
		else if (status.MPI_TAG == 200)
		{
			Update( status.count, buf );
		}
	}		

	while (m_nReceived < m_nDispatchedRemotely)
	{			
		UINT Processed = m_WorkProcessed.size();
		Msg( "[[PROGRESS:%s %d %d]]\n", m_szWork, m_nReceived + m_WorkProcessed.size(), m_nWork);	

		MPI_Status status;				

		MPI_Recv( buf, BUFSIZE, MPI_UNSIGNED, MPI_ANY_SOURCE, 200, MPI_COMM_WORLD, &status );

		Update( status.count, buf );
	}

	Msg( "[[PROGRESS:%s %d %d]]\n", m_szWork, m_nReceived + m_WorkProcessed.size(), m_nWork);	

	//Msg( "***********************ENDS\n" );
}

CString cache_prefix;

bool Threads::ReadCache()
{	
	FILE* fp = fopen( cache_prefix + "." + m_szWork, "rb" );	
	if (!fp)
	{
		Msg("Cannot read cache : %s\n", cache_prefix + "." + m_szWork );
		return false;
	}
	for (UINT iWork = 0; iWork < m_nWork; ++iWork)
	{
		int size;
		fread( &size, sizeof(int), 1, fp );
		fread( buf, size, 1, fp );
		char* ptr = buf;
		int nBuffers = *(int*)ptr;
		ptr += 4;		
		for (UINT iBuffer = 0; iBuffer < nBuffers; ++iBuffer)
		{	
			int buf_size = *(int*)ptr;
			ptr += 4;									
			m_pInterface->UpdateResult( iWork, iBuffer, ptr, buf_size );
			ptr += buf_size;
		}				
	}		
	fclose( fp );
	return true;
}

void Threads::WriteCache()
{
	FILE* fp = fopen( cache_prefix + "." + m_szWork, "wb" );
	if (!fp) return;

	Msg( "Writing cache..." );	

	for (UINT iWork = 0; iWork < m_nWork; ++iWork)
	{
		int nBuffers;
		char* ptr = buf;
		*(int*)ptr = nBuffers = m_pInterface->GetBufferCount( iWork );

		ptr += 4;				

		for (UINT iBuffer = 0; iBuffer < nBuffers; ++iBuffer)
		{	
			UINT Size;
			LPVOID pBuffer;

			m_pInterface->PrepareResultBuffer( iWork, iBuffer, &pBuffer, &Size );

			*(int*)ptr = Size;
			ptr += 4;						

			memcpy( ptr, pBuffer, Size );

			ptr += Size;

			m_pInterface->DiscardResultBuffer( iBuffer );
		}		

		int size = ptr - buf;
		fwrite( &size, sizeof(int), 1, fp );
		fwrite( buf, size, 1, fp );
	}		

	fclose( fp );

	Msg( "Done\n" );
}

void Threads::MPI_Master( LPCSTR szWork, UINT nWork, WorkerFunction wf, LPVOID pUserData )
{	
	m_szWork = szWork;

	RunOnIndividual( szWork, nWork, wf, pUserData );	
}

void Threads::MPI_Slave( LPCSTR szWork, WorkerFunction wf, LPVOID pUserData )
{
	//Msg( "<<< 시작 : %s >>>", szWork );
	int mpi_rank, mpi_size;

	MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );		

	m_WorkProcessed.resize(0);

	char* cur_buf = buf;
	MPI_Status send_status[64];
	MPI_Request request[128];
	INT pendingReqs = 0;

	for (;;)
	{
		MPI_Status status;
		UINT works[128];		

		MPI_Recv( works, sizeof(works)/sizeof(works[0]), MPI_UNSIGNED, 0, 100, MPI_COMM_WORLD, &status );		

		UINT work = UINT_MAX;						

		for (int i=0; i<status.count/sizeof(work); ++i)
		{
			work = works[i];

			if (work == UINT_MAX)
				break;

			wf( 0, work, pUserData );			

			m_WorkProcessed.push_back( work );			

			UINT nBuffers = m_pInterface->GetBufferCount( work );						

			UINT BufferTotalSize = sizeof(Result);

			for (UINT iBuffer = 0; iBuffer < nBuffers; ++iBuffer)
			{	
				UINT Size;
				LPVOID pBuffer;

				m_pInterface->PrepareResultBuffer( work, iBuffer, &pBuffer, &Size );				

				BufferTotalSize += Size + 4;
			}

			if (BufferTotalSize + (cur_buf - buf) > BUFSIZE)
			{				
				MPI_Waitall( pendingReqs, request, send_status );
				pendingReqs = 0;
				cur_buf = buf;
			}

			char* ptr = cur_buf;

			Result& data = *(Result*)cur_buf;
			ptr += sizeof(Result);

			data.iWork = work;
			data.nBuffers = nBuffers;						

			for (UINT iBuffer = 0; iBuffer < data.nBuffers; ++iBuffer)
			{	
				UINT Size;
				LPVOID pBuffer;

				m_pInterface->PrepareResultBuffer( data.iWork, iBuffer, &pBuffer, &Size );				

				*(int*)ptr = Size;
				ptr += 4;						

				memcpy( ptr, pBuffer, Size );

				ptr += Size;				

				m_pInterface->DiscardResultBuffer( iBuffer );
			}								

			MPI_Isend( cur_buf, ptr-cur_buf, MPI_CHAR, 0, 200, MPI_COMM_WORLD, &request[pendingReqs++] );
			cur_buf = ptr;
		}		

		if (pendingReqs)
		{
			MPI_Waitall( pendingReqs, request, send_status );
			pendingReqs = 0;
			cur_buf = buf;
		}		
		
		if (work == UINT_MAX)
		{
			break;
		}		

		MPI_Send( &work, 1, MPI_UNSIGNED, 0, 100, MPI_COMM_WORLD );
	}		

	Msg( "Job's done! #%d", mpi_rank );	

	UINT work = UINT_MAX;
	MPI_Send( &work, 1, MPI_UNSIGNED, 0, 100, MPI_COMM_WORLD );

	wf( -1, -1, NULL );
}