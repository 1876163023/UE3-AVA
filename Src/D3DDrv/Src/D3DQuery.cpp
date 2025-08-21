/*=============================================================================
	D3DQuery.cpp: D3D query RHI implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3DDrvPrivate.h"

#if USE_D3D_RHI

FOcclusionQueryRHIRef RHICreateEventQuery()
{
	FOcclusionQueryRHIRef OcclusionQuery;
	VERIFYD3DRESULT(GDirect3DDevice->CreateQuery(D3DQUERYTYPE_EVENT,OcclusionQuery.GetInitReference()));
	return OcclusionQuery;
}

UBOOL RHIGetEventQueryResult(FOcclusionQueryRHIParamRef OcclusionQuery)
{	
	return S_FALSE != OcclusionQuery->GetData( NULL, 0, D3DGETDATA_FLUSH );	
}

FOcclusionQueryRHIRef RHICreateOcclusionQuery()
{
	FOcclusionQueryRHIRef OcclusionQuery;
	VERIFYD3DRESULT(GDirect3DDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION,OcclusionQuery.GetInitReference()));
	return OcclusionQuery;
}

UBOOL RHIGetOcclusionQueryResult(FOcclusionQueryRHIParamRef OcclusionQuery,DWORD& OutNumPixels,UBOOL bWait)
{
	DOUBLE StartTime = appSeconds();

	while(1)
	{
		// Query occlusion query object.
		HRESULT Result = OcclusionQuery->GetData( &OutNumPixels, sizeof(OutNumPixels), D3DGETDATA_FLUSH );

		if( Result == S_OK )
		{
			return TRUE;
		}
		else if(Result == S_FALSE && !bWait)
		{
			// Return failure if the query isn't complete, and waiting wasn't requested.
			return FALSE;
		}
		else if( Result == D3DERR_DEVICELOST )
		{
			GD3DDeviceLost = 1;
			OutNumPixels = 0;
			return FALSE;
		}
		else if( FAILED(Result) )
		{
			VERIFYD3DRESULT(Result);
			return FALSE;
		}

		if((appSeconds() - StartTime) > 0.5)
		{
			debugf(TEXT("Timed out while waiting for GPU to catch up. (500 ms)"));
			return FALSE;
		}
	}
}

#endif