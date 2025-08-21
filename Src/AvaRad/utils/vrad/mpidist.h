#pragma once 

#include <vector>

class Mutex
{
public :
	Mutex()
	{
		InitializeCriticalSection( &cs );
	}

	void lock()
	{
		EnterCriticalSection( &cs );
	}

	void unlock()
	{
		LeaveCriticalSection( &cs );
	}

	~Mutex()
	{
		DeleteCriticalSection( &cs );
	}

	CRITICAL_SECTION cs;
};

class ScopedMutex
{
public :
	ScopedMutex( Mutex& m )
		: m(m)
	{
		m.lock();
	}

	~ScopedMutex()
	{
		m.unlock();
	}

	Mutex& m;
};

class Timer
{
public :
	void Start() {}
	void Stop() {}
	void Reset() {}
	float GetElapsed() {return 0;}
};

class MPIInterface
{
public :
	virtual UINT GetBufferCount( UINT iWork ) = 0;
	virtual void PrepareResultBuffer( UINT iWork, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) = 0;
	virtual void DiscardResultBuffer( UINT iBuffer ) = 0;
	virtual void UpdateResult( UINT iWork, UINT iBuffer, LPVOID pBuffer, UINT Size ) = 0;
};

class Threads
{
public :
	/// 32-way :) 
	/// HT를 사용한다면 double :)
	static const int c_nMaxThreads = 32;

	Threads();

	void SetNumThreads( UINT nThreads = UINT_MAX );

	typedef void(*WorkerFunction)( UINT iThread, UINT iWork, LPVOID pUserData );
	typedef void(*ThreadFunction)( UINT iThread, LPVOID pUserData );

	void RunOn( LPCSTR szWork, UINT nWork, ThreadFunction, LPVOID pUserData );	
	void RunOnIndividual( LPCSTR szWork, UINT nWork, WorkerFunction, LPVOID pUserData );

	void MPI_Run( LPCSTR szWork, UINT nWork, WorkerFunction, LPVOID pUserData, MPIInterface* pInterface, bool bMakeCache );		

	void MPI_Master( LPCSTR szWork, UINT nWork, WorkerFunction, LPVOID pUserData );
	void MPI_Slave( LPCSTR szWork, WorkerFunction, LPVOID pUserData );
	void MPI_Main();

	void Start( ThreadFunction, LPVOID pUserData );
	void End();

	void Update( int count, char* buf );
	void WriteCache();
	bool ReadCache();

	struct Data
	{
		UINT									iThread;
		ThreadFunction							pFunction;
		LPVOID									pUserData;		
	};

	UINT GetThreadWork();

	void WorkerMain( UINT iThread );

	WorkerFunction								m_Worker;
	LPVOID										m_pUserData;
	bool										m_bLowPriority;
	bool										m_bThreaded;
	UINT										m_nThreads;
	Mutex										m_Mutex;
	UINT										m_nDispatched, m_nDispatchedRemotely, m_nWork, m_nReceived;
	Data										m_Data[ c_nMaxThreads ];
	HANDLE										m_Threads[ c_nMaxThreads ], m_hMaster;
	Timer										m_Timer;
	float										m_fLast;

	int											m_MPI_Processing;
	LPCSTR										m_szWork;

	std::vector<UINT>							m_WorkProcessed;
	MPIInterface*								m_pInterface;
};