/*=============================================================================
	UnFile.cpp: ANSI C core.
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

#undef clock
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
	Time.
-----------------------------------------------------------------------------*/

//
// String timestamp.
//
const TCHAR* appTimestamp()
{
	static TCHAR Result[1024];
	*Result = 0;
#if _MSC_VER
	//@todo gcc: implement appTimestamp (and move it into platform specific .cpp
#if UNICODE
	if( GUnicodeOS )
	{
		_wstrdate( Result );
		appStrcat( Result, TEXT(" ") );
		_wstrtime( Result + appStrlen(Result) );
	}
	else
#endif
	{
		ANSICHAR Temp[1024]="";
		_strdate( Temp );
		appStrcpy( Result, ANSI_TO_TCHAR(Temp) );
		appStrcat( Result, TEXT(" ") );
		_strtime( Temp );
		appStrcat( Result, ANSI_TO_TCHAR(Temp) );
	}
#endif
	return Result;
}

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

INT appMemcmp( const void* Buf1, const void* Buf2, INT Count )
{
	return memcmp( Buf1, Buf2, Count );
}

UBOOL appMemIsZero( const void* V, int Count )
{
	BYTE* B = (BYTE*)V;
	while( Count-- > 0 )
		if( *B++ != 0 )
			return 0;
	return 1;
}

void* appMemmove( void* Dest, const void* Src, INT Count )
{
	return memmove( Dest, Src, Count );
}


/*-----------------------------------------------------------------------------
	String functions.
-----------------------------------------------------------------------------*/

//
// Copy a string with length checking.
//warning: Behavior differs from strncpy; last character is zeroed.
//
TCHAR* appStrncpy( TCHAR* Dest, const TCHAR* Src, INT MaxLen )
{
#if UNICODE
	wcsncpy( Dest, Src, MaxLen );
#else
	strncpy( Dest, Src, MaxLen );
#endif
	check(MaxLen>0);
	Dest[MaxLen-1]=0;
	return Dest;
}

//
// Concatenate a string with length checking
//
TCHAR* appStrncat( TCHAR* Dest, const TCHAR* Src, INT MaxLen )
{
	INT Len = appStrlen(Dest);
	TCHAR* NewDest = Dest + Len;
	if( (MaxLen-=Len) > 0 )
	{
		appStrncpy( NewDest, Src, MaxLen );
		NewDest[MaxLen-1] = 0;
	}
	return Dest;
}

/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
* (ANSICHAR version) 
*
* @param Dest - destination char buffer to copy to
* @param Src - source char buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
ANSICHAR* appStrncpyANSI( ANSICHAR* Dest, const ANSICHAR* Src, INT MaxLen )
{
#if USE_SECURE_CRT	
	// length of string must be strictly < total buffer length so use (MaxLen-1)
	return (ANSICHAR*)strncpy_s(Dest,MaxLen,Src,MaxLen-1);
#else
	return (ANSICHAR*)strncpy(Dest,Src,MaxLen);
	// length of string includes null terminating character so use (MaxLen)
	Dest[MaxLen-1]=0;
#endif
}

/*
 * Standard string formatted print.
 */
VARARG_BODY( INT, appSprintf, const TCHAR*, VARARG_EXTRA(TCHAR* Dest) )
{
	INT	Result = -1;
	//@warning: make sure code using appSprintf allocates enough memory if the below 1024 is ever changed.
	GET_VARARGS_RESULT(Dest,1024/*!!*/,Fmt,Fmt,Result);
	return Result;
}

VARARG_BODY( INT, appSnprintf, const TCHAR*, VARARG_EXTRA(INT MaxLength) VARARG_EXTRA(TCHAR* Dest) )
{
	INT Result = -1;
	GET_VARARGS_RESULT(Dest, MaxLength, Fmt, Fmt, Result);
	return Result;
}

/*-----------------------------------------------------------------------------
	Sorting.
-----------------------------------------------------------------------------*/

void appQsort( void* Base, INT Num, INT Width, int(CDECL *Compare)(const void* A, const void* B ) )
{
	qsort( Base, Num, Width, Compare );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

