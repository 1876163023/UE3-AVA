/*=============================================================================
 Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "MeshUtils.h"

#if USE_D3D_RHI
#include "D3DMeshUtils.h"
#endif

UBOOL GenerateLOD(UStaticMesh* StaticMesh, INT DesiredLOD, INT DesiredTriangles)
{
#if USE_D3D_RHI
	FD3DMeshUtilities MeshUtils;
	return MeshUtils.GenerateLOD(StaticMesh, DesiredLOD, DesiredTriangles);
#endif
	return FALSE;
}
