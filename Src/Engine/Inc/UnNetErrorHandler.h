/***

	Copyright (c) 2006-2007 Redduck Inc. All rights reserved.

	Project: Engine

	Name: UnNetErrorHandler.h

	Description: Interface to handle some UnrealNetwork error.

***/

#ifndef _UNNETERRORHANDLER_H_
#define _UNNETERRORHANDLER_H_


class IUnNetErrorHandler
{
public:
	virtual void OnFailure() = 0;
	virtual void OnBrawl() = 0;
	virtual void OnPackageMismatch(const TCHAR *PackageName) = 0;
	virtual void OnPackageNotFound(const TCHAR *PackageName) = 0;
	virtual void OnConnectionError(const TCHAR *Error) = 0;

	static IUnNetErrorHandler *pHandler;
};


#endif
