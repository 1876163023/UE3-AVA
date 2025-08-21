// PacketPool.h: interface for the CPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POOL_H__FE6DB7DB_7AD4_469A_907D_7FDC99D590E0__INCLUDED_)
#define AFX_POOL_H__FE6DB7DB_7AD4_469A_907D_7FDC99D590E0__INCLUDED_

#include <stdio.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_POOL_SIZE	(8192 + 1024)

class PoolEntry
{
public:
	BOOL		m_bFree;
	PoolEntry	*m_lpNext;
	PoolEntry(){};
	virtual ~PoolEntry(){}
};

template <class EntryClass>
class CPool  
{

private:
	int m_nFree;					//for DEBUG 후에 완전히 안정화 돼고 나면 뺀다.
	int	m_nSize;
	CRITICAL_SECTION m_csPool;
	EntryClass	*m_pFree;
	EntryClass	*m_pPool;
//	typedef EntryClass*	EntryClass::m_lpNext;
//	typedef BOOL		EntryClass::m_bFree;
public:
	int	NumberOfFreeSlots() { return m_nFree; }
	BOOL Free(EntryClass *pEntry)
	{
		EnterCriticalSection(&m_csPool);
		if (pEntry == NULL) {
			LeaveCriticalSection(&m_csPool);
			return FALSE;
		}
		if (pEntry->m_bFree) {
			LeaveCriticalSection(&m_csPool);
			return FALSE;
		}
		pEntry->m_lpNext = m_pFree;
		m_pFree = pEntry;
		pEntry->m_bFree = TRUE;
		m_nFree++;
	#ifdef DEBUG_PACKET_POOL
		char tMsg[128];
		sprintf(tMsg, "Free(%d개의 Free Packet이 남아 있습니다!!)\n",m_nFree);
		DisplayMessage(tMsg);
	#endif
		LeaveCriticalSection(&m_csPool);
		return TRUE;
	}

	EntryClass *Alloc()
	{
		EntryClass *pResult;
		EnterCriticalSection(&m_csPool);
		if (m_pFree == NULL) {
			LeaveCriticalSection(&m_csPool);
			return NULL;
		}
		pResult = m_pFree;
		m_pFree = (EntryClass *)m_pFree->m_lpNext;
		pResult->m_lpNext = NULL;
		pResult->m_bFree = FALSE;
		m_nFree--;
	#ifdef DEBUG_PACKET_POOL
		char tMsg[128];
		sprintf(tMsg, "AllocPacket(%d개의 Free Packet이 남아 있습니다!!)\n",m_nFree);
		DisplayMessage(tMsg);
	#endif
		LeaveCriticalSection(&m_csPool);
		return pResult;

	}
	CPool(int nSize=DEFAULT_POOL_SIZE)
	{
		m_nSize = nSize;
		if (m_nSize > 0) {
			m_pPool =  new EntryClass[m_nSize];
			for (int i = 0; i < m_nSize - 1; i++){
				m_pPool[i].m_lpNext = &m_pPool[i+1];
				m_pPool[i].m_bFree = TRUE;
			}
			m_pPool[i].m_lpNext = NULL;
			m_pFree = &m_pPool[0];
			InitializeCriticalSection(&m_csPool);
			m_nFree = m_nSize;
		}
	}	
	CPool(int nSize, EntryClass* pPool)
	{
		m_nSize = nSize;
		if (m_nSize > 0) {
			m_pPool = pPool;
			for (int i = 0; i < m_nSize - 1; i++){
				m_pPool[i].m_lpNext = &m_pPool[i+1];
				m_pPool[i].m_bFree = TRUE;
			}
			m_pPool[i].m_lpNext = NULL;
			m_pFree = &m_pPool[0];
			InitializeCriticalSection(&m_csPool);
			m_nFree = m_nSize;
		}
	}	
	virtual ~CPool()
	{
		delete[] m_pPool;
		DeleteCriticalSection(&m_csPool);
	}
};

#endif // !defined(AFX_POOL_H__FE6DB7DB_7AD4_469A_907D_7FDC99D590E0__INCLUDED_)
