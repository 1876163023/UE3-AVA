/***

	Copyright (c) 2006-2007 Redduck Inc. All rights reserved.

	Project: Engine

	Name: UnNetErrorHandler.cpp

	Description: Interface to handle some UnrealNetwork error.

***/



#include "EnginePrivate.h"
#include "UnNet.h"
#include "UnNetErrorHandler.h"

IUnNetErrorHandler *IUnNetErrorHandler::pHandler = NULL;
