// PacketPool.cpp: implementation of the CPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Pool.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class TTT
{
public:
	TTT()
	{

	}
	TTT(int a)
	{
		aa = a;
	}

	int aa;
};

class SSS
{
	SSS()
	{
		a[0] = TTT(1);
	}

	TTT a[1];
};
