// ConnectionCheckThread.cpp: implementation of the CConnectionChecker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ConnectionChecker.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConnectionChecker::CConnectionChecker()
{
	/*
	m_pDead = NULL;
	m_pMRAccepting = NULL;
	m_pLRAccepting = NULL;
	m_pMRU = NULL;
	m_pLRU = NULL;
	m_nMaxConnectSeconds = nMaxConnectSeconds;
	m_nMaxSilentSeconds = nMaxSilentSeconds;
	//m_nInterval = nInterval;	//Constructor���� heartbeat�ֱ⸦ �����ϵ����Ѵ�. ������ mSec�� �Ѵ�.
	*/
	InitializeCriticalSection(&m_csLRU_Chain);
}

CConnectionChecker::~CConnectionChecker()
{
	DeleteCriticalSection(&m_csLRU_Chain);
}

void CConnectionChecker::Init(int nMaxSilentSeconds, int nMaxConnectSeconds)
{
	m_pDeadHead = NULL;
	m_pDeadTail = NULL;
	m_pMRAccepting = NULL;
	m_pLRAccepting = NULL;
	m_pMRU = NULL;
	m_pLRU = NULL;
	m_nMaxConnectSeconds = nMaxConnectSeconds;
	m_nMaxSilentSeconds = nMaxSilentSeconds;
	//m_nInterval = nInterval;	//Constructor���� heartbeat�ֱ⸦ �����ϵ����Ѵ�. ������ mSec�� �Ѵ�.
}


void CConnectionChecker::AddDeadChain(CLRUEntry *pAddedEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);
	if(pAddedEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	if(pAddedEntry->m_nLRUState == IN_DEAD_CHAIN) {	//2005.2.6 tkhong BOOL		m_bInLRUChain;
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	pAddedEntry->m_nLRUState = IN_DEAD_CHAIN;	//2005.2.6 tkhong BOOL		m_bInLRUChain;
	//////////////////////////////////////////////
	// dead chain�� tail�� ���� �ִ´�.
	CLRUEntry*	pPrevEntry;
	pPrevEntry = m_pDeadTail;
	m_pDeadTail = pAddedEntry;
	if (pPrevEntry) {
		pPrevEntry->m_pNext = pAddedEntry;
	} else {
		m_pDeadHead = pAddedEntry;
	}
	pAddedEntry->m_pNext = NULL;
	pAddedEntry->m_pPrev = pPrevEntry;
	/***********************************
	if (m_pDead) {
		m_pDead->m_pPrev = pAddedEntry;
	}
	pAddedEntry->m_pNext = m_pDead;
	pAddedEntry->m_pPrev = NULL;
	m_pDead = pAddedEntry;
	*************************************/
	//////////////////////////////////////////////
	LeaveCriticalSection(&m_csLRU_Chain);
}

void CConnectionChecker::RemoveFromDeadChain(CLRUEntry *pDeletedEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);

	if(pDeletedEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	if(pDeletedEntry->m_nLRUState != IN_DEAD_CHAIN) {	//2005.2.6 tkhong BOOL		m_bInLRUChain;
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	CLRUEntry *pPrev;
	CLRUEntry *pNext;

	pPrev = pDeletedEntry->m_pPrev;
	pNext = pDeletedEntry->m_pNext;

	if(pPrev) {
		pPrev->m_pNext = pNext;
	} else {
		m_pDeadHead = pNext;
	}
	if(pNext) {
		pNext->m_pPrev = pPrev;
	}else{
		m_pDeadTail = pPrev;
	}

	pDeletedEntry->m_pPrev = NULL;
	pDeletedEntry->m_pNext = NULL;
	pDeletedEntry->m_nLRUState = NOT_IN_ANY_CHAIN; 	//2005.2.6 tkhong BOOL		m_bInLRUChain;
	LeaveCriticalSection(&m_csLRU_Chain);
}
void CConnectionChecker::AddAcceptingChain(CLRUEntry *pAddedEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);
	if(pAddedEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	if(pAddedEntry->m_nLRUState == IN_ACCEPT_CHAIN) {	//2005.2.6 tkhong BOOL		m_bInLRUChain;
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	pAddedEntry->m_nLRUState = IN_ACCEPT_CHAIN;	//2005.2.6 tkhong BOOL		m_bInLRUChain;
	//////////////////////////////////////////////
	// Accepting chain�� ���� �ִ´�.
	pAddedEntry->m_pPrev = NULL;
	pAddedEntry->m_pNext = m_pMRAccepting;
	if (m_pMRAccepting) {
		m_pMRAccepting->m_pPrev = pAddedEntry;
	} else {
		m_pLRAccepting = pAddedEntry;
	}
	m_pMRAccepting = pAddedEntry;
	//////////////////////////////////////////////
	LeaveCriticalSection(&m_csLRU_Chain);
}

void CConnectionChecker::RemoveFromAcceptingChain(CLRUEntry *pDeletedEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);

	if(pDeletedEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	if(pDeletedEntry->m_nLRUState != IN_ACCEPT_CHAIN) {	//2005.2.6 tkhong BOOL		m_bInLRUChain;
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
	CLRUEntry *pPrev;
	CLRUEntry *pNext;

	pPrev = pDeletedEntry->m_pPrev;
	pNext = pDeletedEntry->m_pNext;

	if(pPrev) {
		pPrev->m_pNext = pNext;
	} else {
		m_pMRAccepting = pNext;
	}
	if(pNext) {
		pNext->m_pPrev = pPrev;
	} else {
		m_pLRAccepting = pPrev;
	}

	pDeletedEntry->m_pPrev = NULL;
	pDeletedEntry->m_pNext = NULL;
	pDeletedEntry->m_nLRUState = NOT_IN_ANY_CHAIN;
	LeaveCriticalSection(&m_csLRU_Chain);
}

void CConnectionChecker::Add(CLRUEntry *pAddedEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);

	if(pAddedEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}

	if(pAddedEntry->m_nLRUState == IN_LRU_CHAIN) {	//2005.2.6 tkhong BOOL		m_bInLRUChain;
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}

	CLRUEntry *pTempEntry = m_pMRU;
	
	pAddedEntry->m_pPrev = NULL;
	pAddedEntry->m_pNext = pTempEntry;
	
	if(pTempEntry) {
		pTempEntry->m_pPrev = pAddedEntry;
	} else {
		m_pLRU = pAddedEntry;
	}
	m_pMRU = pAddedEntry;
	time(&pAddedEntry->m_timeStamp);
	pAddedEntry->m_nLRUState = IN_LRU_CHAIN;		//2005.2.6 tkhong BOOL		m_bInLRUChain;

	LeaveCriticalSection(&m_csLRU_Chain);
}

void CConnectionChecker::Delete(CLRUEntry *pDeletedEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);

	if(pDeletedEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}

	if(pDeletedEntry->m_nLRUState != IN_LRU_CHAIN) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}

	CLRUEntry *pPrev;
	CLRUEntry *pNext;

	pPrev = pDeletedEntry->m_pPrev;
	pNext = pDeletedEntry->m_pNext;

	if(pPrev)
		pPrev->m_pNext = pNext;
	else
		m_pMRU = pNext;
	
	if(pNext)
		pNext->m_pPrev = pPrev;
	else
		m_pLRU = pPrev;


	pDeletedEntry->m_pPrev = NULL;
	pDeletedEntry->m_pNext = NULL;
	pDeletedEntry->m_timeStamp = 0;
	pDeletedEntry->m_nLRUState = NOT_IN_ANY_CHAIN;

	LeaveCriticalSection(&m_csLRU_Chain);
}


void CConnectionChecker::CheckAccept()
{
	CLRUEntry	*pCheckEntry;
	CLRUEntry	*pPrev, *pNext;
	time_t		currentTime;
	BOOL		bCheckNext;

	time(&currentTime);
	
	EnterCriticalSection(&m_csLRU_Chain);
	bCheckNext = FALSE;

	pCheckEntry = m_pLRAccepting;	
	do {
		if(pCheckEntry == NULL) 
			break;

		// �˻��ϰ� �ִ� User�� ���� �ð� �̳��� ������ ���� ���
		if(pCheckEntry->SecondsConnection() > m_nMaxConnectSeconds) {
			// ���� �˻��ϴ� User�� LRU Chain���� ����.
			pPrev = pCheckEntry->m_pPrev;
			pNext = pCheckEntry->m_pNext;
			pCheckEntry->m_pPrev = NULL;
			pCheckEntry->m_pNext = NULL;
			pCheckEntry->m_nLRUState = NOT_IN_ANY_CHAIN;
			// Prev�� ������ ���� �����Ѵ�.
			if(pPrev) {
				pPrev->m_pNext = pNext;
			} else {
				m_pMRAccepting = pNext;
			}
			// Next�� ������ ���� �����Ѵ�.
			if(pNext) {
				pNext->m_pPrev = pPrev;
			} else {
				m_pLRAccepting = pPrev;
			}
			OnAcceptTimeOut(pCheckEntry);
			// ���� �˻��� User�� �����Ѵ�.
			bCheckNext = TRUE;
			pCheckEntry = pPrev;
		}
		// �˻��ϰ� �ִ� User�� ���� �ð� �̳��� ������ ���� ���
		else {
			// ���� LRU chain�� entry�� �������� �ʴ´�.
			bCheckNext = FALSE;									
		}
	} while(bCheckNext);

	LeaveCriticalSection(&m_csLRU_Chain);
}
// Most Recent   +---------+     +---------+
//     |     <---+prev     |<----+prev     |<-----Least Recent
//     +-------->|     next+---->|     next+-->||
//               +---------+     +---------+
void CConnectionChecker::CheckConnection()
{
	CLRUEntry	*pCheckEntry;
	CLRUEntry	*pPrev, *pNext;
	time_t		currentTime;
	BOOL		bCheckNext;

	time(&currentTime);
	
	EnterCriticalSection(&m_csLRU_Chain);
	__try
	{
		bCheckNext = FALSE;
		
		// ������ ���� ���� ������ User���� �˻��� ������.
		pCheckEntry = m_pLRU;	
		
		do {
			if(pCheckEntry == NULL) 
				break;

			// �˻��ϰ� �ִ� User�� ���� �ð� �̳��� ������ ���� ���
			if(currentTime - pCheckEntry->m_timeStamp > (m_nMaxSilentSeconds + 2)) {
				// ���� �˻��ϴ� User�� LRU Chain���� ����.
				pPrev = pCheckEntry->m_pPrev;
				pNext = pCheckEntry->m_pNext;
				pCheckEntry->m_pPrev = NULL;
				pCheckEntry->m_pNext = NULL;
				pCheckEntry->m_timeStamp = 0;
					pCheckEntry->m_nLRUState = NOT_IN_ANY_CHAIN;
				
				// Prev�� ������ ���� �����Ѵ�.
				if(pPrev) {
					pPrev->m_pNext = pNext;
				} else {
					m_pMRU = pNext;
				}
				// Next�� ������ ���� �����Ѵ�.
				if(pNext) {
					pNext->m_pPrev = pPrev;
				} else {
					m_pLRU = pPrev;
				}
				/***************************************************
				// �˻��� User�� ���ֵ��� Message�� ������.
				// �ٷ� packet���� queue�� �ִ°ɷ� �ٲ۴�.
				//::SendMessage(m_baseWnd, WM_CONNECTION_CHECK_FAIL, 0, pCheckEntry->m_nKey);
				CPacket *pCPacket = NULL;
				while ((pCPacket = CServer::PacketPool()->Alloc()) == NULL);

				S2S_EXIT_CONNECT_FAIL_PACKET *pPacket = (S2S_EXIT_CONNECT_FAIL_PACKET *)pCPacket->m_szBuffer;
				pPacket->Init(pCheckEntry->GetKey());
				if (CServer::Queue()->enQueue(pCPacket) == Q_FULL) {
					return;
				}
				******************************************************/
				OnConnectionTimeOut(pCheckEntry);
				// ���� �˻��� User�� �����Ѵ�.
				bCheckNext = TRUE;
				pCheckEntry = pPrev;
			}
			// �˻��ϰ� �ִ� User�� ���� �ð� �̳��� ������ ���� ���
			else {
				// ���� LRU chain�� entry�� �������� �ʴ´�.
				bCheckNext = FALSE;									
			}
		} while(bCheckNext);
	}
	__except(expLogFilter(GetExceptionCode(), "CConnectionChecker::CheckConnection",true ))
	{
		expFlushLog();
	}
	LeaveCriticalSection(&m_csLRU_Chain);
}

void CConnectionChecker::Touch(CLRUEntry *pTouchEntry)
{
	EnterCriticalSection(&m_csLRU_Chain);

	if(pTouchEntry == NULL) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}

	if(pTouchEntry->m_nLRUState != IN_LRU_CHAIN) {
		LeaveCriticalSection(&m_csLRU_Chain);
		return;
	}
//	SystemPrint(2,"Touch()");

	////////////////////////////////////////////////////////////////////////////////
	// LRU Chain���� ����.

	CLRUEntry *pPrev;
	CLRUEntry *pNext;

	pPrev = pTouchEntry->m_pPrev;
	pNext = pTouchEntry->m_pNext;

	if(pPrev)
		pPrev->m_pNext = pNext;
	else
		m_pMRU = pNext;
	
	if(pNext)
		pNext->m_pPrev = pPrev;
	else
		m_pLRU = pPrev;

	////////////////////////////////////////////////////////////////////////////////
	// LRU Chain�� ���� �ִ´�.

	CLRUEntry *pTempEntry = m_pMRU;
	
	pTouchEntry->m_pPrev = NULL;
	pTouchEntry->m_pNext = pTempEntry;
	
	if(pTempEntry)
		pTempEntry->m_pPrev = pTouchEntry;
	else
		m_pLRU = pTouchEntry;
	m_pMRU = pTouchEntry;
	time(&pTouchEntry->m_timeStamp);


	LeaveCriticalSection(&m_csLRU_Chain);
}


