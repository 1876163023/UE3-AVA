/*=============================================================================
	COLLADA common header.
	Based on Feeling Software's Collada import classes [FCollada].
	Copyright ?2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNCOLLADA_H__
#define __UNCOLLADA_H__

#pragma warning(disable : 4548)

#if _MSC_VER >= 1400
// Avoid usage of _CrtDbgReport as we don't link with the debug CRT in debug builds.
#undef _ASSERT
#define _ASSERT(expr)  check((expr))
#undef _ASSERTE
#define _ASSERTE(expr)  checkf((expr), _CRT_WIDE(#expr))
#endif

#pragma pack (push,8)
#pragma pack (pop)

#undef FMatrix // Returns FMatrix to be the Unreal matrix

#pragma warning(default : 4548)

#endif // __UNCOLLADA_H__
