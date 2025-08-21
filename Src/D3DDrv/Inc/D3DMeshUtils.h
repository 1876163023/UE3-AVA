/*=============================================================================
 Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#if USE_D3D_RHI

// Temporary vertex for utility class
#pragma pack(push, 1)
struct FUtilVertex
{
	FVector   Position;
	FVector2D UVs[4];
	FColor	  Color;
	FColor    SmoothingMask;
};

#pragma pack(pop)

/**
*
* This utility class builds on D3DX functionality.
* Used for editor helpers.
*
*/
class FD3DMeshUtilities
{

private:

	/**
	* ConstructD3DVertexElement - acts as a constructor for D3DVERTEXELEMENT9
	*/
	D3DVERTEXELEMENT9 ConstructD3DVertexElement(WORD Stream, WORD Offset, BYTE Type, BYTE Method, BYTE Usage, BYTE UsageIndex);

	/**
	* Creates a D3DXMESH from a FStaticMeshRenderData
	* @param SourceMesh Mesh to extract from
	* @param D3DMesh Mesh to create
	* @return Boolean representing success or failure
	*/
	UBOOL ConvertUMeshToD3DXMesh(FStaticMeshRenderData& SourceMesh, LPD3DXMESH& D3DMesh);

	/**
	* Creates a FStaticMeshRenderData from a D3DXMesh
	* @param DestMesh Destination mesh to extract to
	* @param NumUVs Number of UVs
	* @param Elements Elements array
	* @return Boolean representing success or failure
	*/
	UBOOL ConvertD3DXMeshToUMesh(LPD3DXMESH& D3DMesh, FStaticMeshRenderData& DestMesh, INT NumUVs, TArray<FStaticMeshElement>& Elements);

public:

	FD3DMeshUtilities() {}

	/**
	 * Generates a simplified LOD from a static mesh
	 * @param StaticMesh The input/output mesh
	 * @param DesiredLOD The LOD level to generate the simplified mesh for
	 * @param DesiredTriangles The desired triangle count for the LOD. Real triangle count may not be desired triangle count.
	 * @return Boolean representing success or failure
	 */
	UBOOL GenerateLOD(UStaticMesh* StaticMesh, INT DesiredLOD, INT DesiredTriangles);
};

#endif
