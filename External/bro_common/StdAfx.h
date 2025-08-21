// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__95E3D706_1CCC_46C3_8440_B9248E41B50B__INCLUDED_)
#define AFX_STDAFX_H__95E3D706_1CCC_46C3_8440_B9248E41B50B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500

//NOTE [2006-5-26 jovial] : 공박 클라이언트 메모리 얼로케이터
#ifdef _BF_CLIENT
#include <NtixCore/config.h>
//#include <NtixCore/xmemory_hack.h>
#endif

#ifdef _BF_SERVER
#include <NXCore/Base.h>
#endif

#include <tchar.h>
#include <windows.h>
#include <winsock2.h>
#include <Mswsock.h>
#include "SystemLog.h"
#include <assert.h>
#ifndef _BF_SERVER
#include <crtdbg.h>
#endif

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


#include "CustomAllocator.h"

inline void* operator new( size_t Size )
{
	return CbroMemAlloc::Alloc(Size, (char*)0);
}
inline void operator delete( void* Ptr )
{
	CbroMemAlloc::Free(Ptr);
}

inline void* operator new[]( size_t Size )
{
	return CbroMemAlloc::Alloc(Size, (char*)0);
}
inline void operator delete[]( void* Ptr )
{
	CbroMemAlloc::Free(Ptr);
}



// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__95E3D706_1CCC_46C3_8440_B9248E41B50B__INCLUDED_)
