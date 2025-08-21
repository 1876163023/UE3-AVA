// ConnectionCheckThread.h: interface for the CConnectionChecker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTIONCHECKER_H__F185CD4D_73EB_4A41_8879_6F2EBDA35D82__INCLUDED_)
#define AFX_CONNECTIONCHECKER_H__F185CD4D_73EB_4A41_8879_6F2EBDA35D82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
enum LRU_STATE {
	NOT_IN_ANY_CHAIN=0,
	IN_LRU_CHAIN,
	IN_ACCEPT_CHAIN,
	IN_DEAD_CHAIN,
};
class CLRUEntry
{
public:
	//virtual int	GetKey()=0;
	virtual int	SecondsConnection()=0;
	CLRUEntry()
	{
		m_pPrev = NULL;
		m_pNext = NULL;
		//2005.2.6 tkhong m_bInLRUChain = FALSE;
		m_nLRUState = NOT_IN_ANY_CHAIN;
	}
	virtual ~CLRUEntry(){}

	time_t		m_timeStamp;
	//2005.2.6 tkhong BOOL		m_bInLRUChain;
	int			m_nLRUState;
	CLRUEntry	*m_pPrev;
	CLRUEntry	*m_pNext;
};
class CConnectionChecker 
{
public:
	CLRUEntry*	GetDeadEntry() { return m_pDeadHead;}
	CLRUEntry*	GetAcceptingEntry() { return m_pLRAccepting;}
	void	RemoveFromAcceptingChain(CLRUEntry *pDeletedEntry);
	void	AddAcceptingChain(CLRUEntry *pAddedEntry);
	void	RemoveFromDeadChain(CLRUEntry *pDeletedEntry);
	void	AddDeadChain(CLRUEntry *pAddedEntry);
	void	CheckConnection();
	void	CheckAccept();
	void	Touch(CLRUEntry *pTouchEntry);
	void	Delete(CLRUEntry *pAddedEntry);
	void	Add(CLRUEntry *pAddedEntry);
	void	Init(int nMaxSilentSeconds, int nMaxConnectSeconds);
	CConnectionChecker();
	virtual ~CConnectionChecker();
protected:
	virtual	void	OnConnectionTimeOut(CLRUEntry *pIdleEntry) = 0;
	virtual	void	OnAcceptTimeOut(CLRUEntry *pIdleEntry) = 0;
private:
	int					m_nInterval;
	int					m_nMaxConnectSeconds;
	int					m_nMaxSilentSeconds;
	CLRUEntry			*m_pMRU;
	CLRUEntry			*m_pLRU;

	CLRUEntry			*m_pDeadHead;
	CLRUEntry			*m_pDeadTail;
	CLRUEntry			*m_pMRAccepting;
	CLRUEntry			*m_pLRAccepting;
	CRITICAL_SECTION	m_csLRU_Chain;
};

#endif // !defined(AFX_CONNECTIONCHECKER_H__F185CD4D_73EB_4A41_8879_6F2EBDA35D82__INCLUDED_)
