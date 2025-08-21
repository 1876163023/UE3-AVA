// P2PHandler.h: interface for the CP2PHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_P2PHANDLER_H__5ACF382A_BF89_4BB2_8EF4_B298BEB49EA5__INCLUDED_)
#define AFX_P2PHANDLER_H__5ACF382A_BF89_4BB2_8EF4_B298BEB49EA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CP2PHandler  
{
public:
	virtual ~CP2PHandler();
	virtual int Execute(DWORD_PTR nParam1, DWORD_PTR nParam2) = 0;
protected:
	CP2PHandler();
};

template <class Receiver>
class CP2PHandlerInvoker : public CP2PHandler  
{
public:
	typedef int (Receiver::*Handler)(DWORD nParam1, DWORD nParam2);
	CP2PHandlerInvoker(Receiver *r, Handler h) :  m_pObj(r), m_mpHandler(h){ }
	virtual int Execute(DWORD_PTR nParam1, DWORD_PTR nParam2){	// 어찌된 영문인지 이 메쏘드를 inline으로 구현하지 않으면 compile할때 unresolved external error가 발생한다.
		return (m_pObj->*m_mpHandler)(nParam1, nParam2);
	}
private:
	Receiver	*m_pObj;
	Handler		m_mpHandler;
};

#endif // !defined(AFX_P2PHANDLER_H__5ACF382A_BF89_4BB2_8EF4_B298BEB49EA5__INCLUDED_)
