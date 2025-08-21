// TimerManager.h: interface for the CTimerManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMERMANAGER_H__95D3453A_0C0B_412A_A2CF_C076308D489A__INCLUDED_)
#define AFX_TIMERMANAGER_H__95D3453A_0C0B_412A_A2CF_C076308D489A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "Pool.h"
#include "SyncObjects.h"

class	CTimerManager;
extern	CTimerManager	*g_pTimerManager;

#define	MAX_TIMER_INTERVAL	1024	//���� Max Timer �ֱ�� MAX_TIMER_INTERVAL * m_nTimerResolution�̴�.

class CMiliSecResolutionTick
{
public:
	int	GetTick();
	CMiliSecResolutionTick(double fResolution=1000.0);
	~CMiliSecResolutionTick();
protected:
private:
	LARGE_INTEGER	m_nOrgCounter;
	LARGE_INTEGER	m_nFrequency;
	int				m_nOrgTick;
	double			m_fResolution;	// frequency�� �ش� 1/m_fResolution sec�� ó���Ѵ�.
};

class CTimerEntry //: public CCriticalSection
{
public:
	int		KillTimer(HANDLE hTimer);
	HANDLE	SetTimer(int nMiliSec);
	virtual	void	OnTimer(HANDLE nID)=0;
	CTimerEntry()
	{
	}
	virtual ~CTimerEntry(){}

};

typedef CTimerEntry*	LPCTimerEntry;

class CTimerControlBlock : public PoolEntry
{
public:
	CTimerControlBlock() { m_pEntry = NULL; m_nCurrentSlot = -1; m_bLive = FALSE;}
	virtual ~CTimerControlBlock(){}
	void	Init() { m_pEntry = NULL; m_pNext = NULL; m_pPrev = NULL; m_nCurrentSlot = -1; m_bLive = FALSE;}

	CTimerEntry	*m_pEntry;
	CTimerControlBlock	*m_pNext;
	CTimerControlBlock	*m_pPrev;
	int					m_nInterval;
	//int					m_nCurrentTick;
	int					m_nCurrentSlot;
	BOOL				m_bLive;
	HANDLE				m_nID;
};

typedef CTimerControlBlock*	LPCTimerControlBlock;

class CTimerManager : public CThread, public CCriticalSection
{
public:
	void	Stop() {	m_bOperating = FALSE; }
	UINT	Run();
	static	int		KillTimer(HANDLE nID);
	static	HANDLE	SetTimer(CTimerEntry *pEntry, int nInterval);
	CTimerManager(int nResolution);
	virtual ~CTimerManager();

private:
	BOOL						m_bTerminated;
	BOOL						m_bOperating;
	int							m_nCurrentTick;
	int							m_nTimerResolution;	//���� mSec
	LPCTimerControlBlock		m_timeSlot[MAX_TIMER_INTERVAL];
	// �ش� Tick�� OnTimer�� �Ҹ� pTCB���� ����ų �迭�̴�. OnTimer�ȿ��� 
	// SetTimer, KillTimer���� �θ� ���ɼ��� �ֱ⶧����, ������ �迭��
	// OnTimer�� ������ pTCB�� �� �����س������� �ϰ� �����Ų��.
	LPCTimerControlBlock		m_pTimerEntry[DEFAULT_POOL_SIZE];	
	CPool<CTimerControlBlock>	m_tcbPool;
};

#endif // !defined(AFX_TIMERMANAGER_H__95D3453A_0C0B_412A_A2CF_C076308D489A__INCLUDED_)
