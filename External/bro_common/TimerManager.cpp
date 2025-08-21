//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TimerManager.h"

CMiliSecResolutionTick::CMiliSecResolutionTick(double fResolution)
{
	m_fResolution = fResolution;
	QueryPerformanceFrequency(&m_nFrequency);
	m_nOrgTick = GetTickCount();
	QueryPerformanceCounter(&m_nOrgCounter);
}
CMiliSecResolutionTick::~CMiliSecResolutionTick()
{
}
int CMiliSecResolutionTick::GetTick()
{
	LARGE_INTEGER	nCurruntCount;
	__int64			nElpasedCount;
	QueryPerformanceCounter(&nCurruntCount);
	nElpasedCount = nCurruntCount.QuadPart - m_nOrgCounter.QuadPart;
	return m_nOrgTick + (int) ( ( (double)nElpasedCount/(double)m_nFrequency.QuadPart ) * m_fResolution + 0.5 );
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTimerManager	*g_pTimerManager;

CTimerManager::CTimerManager(int nResolution)
{
	g_pTimerManager = this;
	m_bTerminated	= TRUE;
	m_nCurrentTick = 0;
	m_nTimerResolution = nResolution;
	for (int i = 0; i < MAX_TIMER_INTERVAL; i++ ) {
		m_timeSlot[i] = NULL;
	}
}

CTimerManager::~CTimerManager()
{
	Stop();
	while (!m_bTerminated) {
		Sleep(50);
	}
}

//	nInterval mSec������ timer event �߻��ֱ��.
HANDLE CTimerManager::SetTimer(CTimerEntry *pEntry, int nInterval)
{
	int	nSlot;
	CTimerControlBlock	*pOldHead;

	if (nInterval >= MAX_TIMER_INTERVAL * g_pTimerManager->m_nTimerResolution) return NULL;
	g_pTimerManager->Lock();
	CTimerControlBlock	*pTCB = g_pTimerManager->m_tcbPool.Alloc();
	if (pTCB == NULL){
		g_pTimerManager->Unlock();
		return NULL;
	}
	nSlot = ((g_pTimerManager->m_nCurrentTick + nInterval)/g_pTimerManager->m_nTimerResolution) % MAX_TIMER_INTERVAL;

	pOldHead = g_pTimerManager->m_timeSlot[nSlot];
	if (pOldHead) {
		pOldHead->m_pPrev = pTCB;
	}
	g_pTimerManager->m_timeSlot[nSlot] = pTCB;
	pTCB->m_pEntry = pEntry;
	pTCB->m_nID = (HANDLE)pTCB;
	pTCB->m_nInterval = nInterval;
	pTCB->m_nCurrentSlot = nSlot;
	pTCB->m_pPrev = NULL;

	pTCB->m_pNext = pOldHead;
	pTCB->m_bLive = TRUE;
	g_pTimerManager->Unlock();
	//TRACE("SetTimer(%X)\n", pTCB->m_nID);
	return pTCB->m_nID;
}

int CTimerManager::KillTimer(HANDLE nID)
{
	int					nKillSlot, nCurrentSlot, nRemainTime;
	CTimerControlBlock	*pPrev, *pNext;
	CTimerControlBlock	*pTCB = (CTimerControlBlock	*)nID;
	if (pTCB->m_nCurrentSlot < 0 ) {
		//TRACE("==============Already killed timer...===================\n");
		return 0;
	}
	//TRACE("KillTimer(%X)\n", nID);
	g_pTimerManager->Lock();
	//////////////////////////////////////////////
	// ���� Ÿ�̸� �ð��� ����Ѵ�.
	nKillSlot = pTCB->m_nCurrentSlot;
	nCurrentSlot = g_pTimerManager->m_nCurrentTick/g_pTimerManager->m_nTimerResolution;
	nRemainTime = ((nKillSlot + MAX_TIMER_INTERVAL - nCurrentSlot) % MAX_TIMER_INTERVAL) * g_pTimerManager->m_nTimerResolution;
	// �������ٰ� ���� Ÿ�̸� �ð��� �˸� app�� ���Ѱ�찡 ����.
	//////////////////////////////////////////////
	pNext = pTCB->m_pNext;
	pPrev = pTCB->m_pPrev;
	if (pPrev) {
		pPrev->m_pNext = pNext;
		if (g_pTimerManager->m_timeSlot[pTCB->m_nCurrentSlot] == pTCB){
			assert(!"KillTimer(����)");
			//TRACE("������..\n");
			return 0;
		}
	}else {
		g_pTimerManager->m_timeSlot[pTCB->m_nCurrentSlot] = pNext;
		if (pNext == pTCB) {
			assert(!"KillTimer(����)");
			//TRACE("������..\n");
			return 0;
		}
	}
	if (pNext) {
		pNext->m_pPrev = pPrev;
	}
	pTCB->m_pEntry = NULL;
	pTCB->m_pNext = NULL;
	pTCB->m_pPrev = NULL;
	pTCB->m_nID = NULL;
	pTCB->m_bLive = FALSE;
	pTCB->m_nCurrentSlot = -1;

	g_pTimerManager->m_tcbPool.Free(pTCB);
	g_pTimerManager->Unlock();
	return nRemainTime;
}

UINT CTimerManager::Run()
{
	int					nIndex;
	int					nTargetSlot;
	CTimerControlBlock	*pTCB, *pNext, *pPrev, *pOldHead;
	CTimerEntry*		pEntry;

	m_bOperating = TRUE;
	m_bTerminated = FALSE;
	while (m_bOperating) {
		int	nCurrentSlot = m_nCurrentTick/m_nTimerResolution;
		Lock();
		pTCB = m_timeSlot[nCurrentSlot];
		pPrev = NULL;
		nIndex = 0;
		while(pTCB) {	// nCurrentSlot�� ��� pTCB�� ��ȸ�ϸ�
			pNext = pTCB->m_pNext;
			if (pTCB->m_bLive) {
				/////////////////////////////////
				// ���� slot chain���� ����.
				if (pPrev) {
					pPrev->m_pNext = pNext;
				} else {
					m_timeSlot[nCurrentSlot] = pNext;
				}
				if (pNext) {
					pNext->m_pPrev = pPrev;
				}
				// ���� slot chain���� ����.
				/////////////////////////////////

				/////////////////////////////////
				// �� slot chain�� �ִ´�
				nTargetSlot = (nCurrentSlot + pTCB->m_nInterval/m_nTimerResolution) % MAX_TIMER_INTERVAL;
				if (nTargetSlot == nCurrentSlot) {
					assert(!"������ ���ɼ��� ����ϴ�!!!");
				}
				pOldHead = m_timeSlot[nTargetSlot];
				if (pOldHead) {
					pOldHead->m_pPrev = pTCB;		//�ڿ��� ����
				}
				m_timeSlot[nTargetSlot] = pTCB; //��

				pTCB->m_pPrev = NULL;
				pTCB->m_nCurrentSlot = nTargetSlot;
				pTCB->m_pNext = pOldHead;
				// �� slot chain�� �ִ´�
				/////////////////////////////////
				m_pTimerEntry[nIndex] = pTCB;
				nIndex++;
			}else {
				assert(!"���� ū������!!!");
			}
			pTCB = pNext;
		}
		Unlock();
		for (int i = 0; i < nIndex; i++) {
			if (m_pTimerEntry[i]->m_bLive) {
				HANDLE	hId = m_pTimerEntry[i]->m_nID;
				pEntry = m_pTimerEntry[i]->m_pEntry;
				if (pEntry) {
					//pEntry->Lock();
					pEntry->OnTimer(hId);
					//pEntry->Unlock();
				}
			}
		}
		Sleep(m_nTimerResolution);	//
		m_nCurrentTick = (m_nCurrentTick + m_nTimerResolution)%(m_nTimerResolution * MAX_TIMER_INTERVAL);
	}
	m_bTerminated = TRUE;
	return	1;
}

HANDLE CTimerEntry::SetTimer(int nMiliSec)
{
	return CTimerManager::SetTimer(this, nMiliSec);
}

int CTimerEntry::KillTimer(HANDLE hTimer)
{
	return CTimerManager::KillTimer(hTimer);
}
