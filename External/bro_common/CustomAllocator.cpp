#include "stdafx.h"
#include "CustomAllocator.h"



CbroDefaultMemAlloc CbroMemAlloc::defaultAlloc;
IbroMemAlloc* CbroMemAlloc::pAlloc = &CbroMemAlloc::defaultAlloc;


