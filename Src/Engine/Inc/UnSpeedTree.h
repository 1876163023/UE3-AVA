//         Name: UnSpeedTree.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2005 IDV, Inc.
//      All Rights Reserved.
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.
//
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
// preprocessor

#ifndef __UNSPEEDTREE_H__
#define __UNSPEEDTREE_H__


//////////////////////////////////////////////////////
// includes

#include "../../../External/SpeedTreeRT/include/SpeedTreeRT.h"
#include "../../../External/SpeedTreeRT/include/SpeedWind.h"


//////////////////////////////////////////////////////
//	prototypes

class UMaterialInstanceConstant;
class USpeedTreeComponent;

#ifdef WITH_SPEEDTREE

//////////////////////////////////////////////////////
//	struct SSpeedTreeVertexPosition 
//	used for position and shadowing

struct SSpeedTreeVertexPosition
{
	FVector			m_cPosition;
	FLOAT			m_fExtrusion;
};


//////////////////////////////////////////////////////
//	struct SSpeedTreeVertexData 
//	used for everything else

struct SSpeedTreeVertexData
{
	FPackedNormal	m_cTangentX,
					m_cTangentY,
					m_cTangentZ;
	FVector2D		m_cTexCoord;
};


//////////////////////////////////////////////////////
//	struct SSpeedTreeVertexDataSelfShadow 
//	adds self-shadowing coordinates

struct SSpeedTreeVertexDataSelfShadow : SSpeedTreeVertexData
{
	FVector2D		m_cShadowTexCoord;
};


//////////////////////////////////////////////////////
//	struct SSpeedTreePositionBuffer

struct SSpeedTreePositionBuffer : FVertexBuffer
{
				SSpeedTreePositionBuffer(void)	{ Stride = sizeof(SSpeedTreeVertexPosition); Size = 0; }
				~SSpeedTreePositionBuffer(void)	{ m_vVertices.Empty( ); }
virtual	void	GetData(void* pData)			{ appMemcpy(pData, &m_vVertices(0), Size); }

	TArray<SSpeedTreeVertexPosition> m_vVertices;
};


//////////////////////////////////////////////////////
//	struct SSpeedTreeDataBuffer

struct SSpeedTreeDataBuffer : FVertexBuffer
{
				SSpeedTreeDataBuffer(void)	{ Stride = sizeof(SSpeedTreeVertexData); Size = 0; }
				~SSpeedTreeDataBuffer(void)	{ m_vVertices.Empty( ); }
virtual	void	GetData(void* pData)		{ appMemcpy(pData, &m_vVertices(0), Size); }

	TArray<SSpeedTreeVertexData> m_vVertices;
};


//////////////////////////////////////////////////////
//	struct SSpeedTreeSelfShadowBuffer

struct SSpeedTreeSelfShadowBuffer : FVertexBuffer
{
				SSpeedTreeSelfShadowBuffer(void)	{ Stride = sizeof(SSpeedTreeVertexDataSelfShadow); Size = 0; }
				~SSpeedTreeSelfShadowBuffer(void)	{ m_vVertices.Empty( ); }
virtual	void	GetData(void* pData)				{ appMemcpy(pData, &m_vVertices(0), Size); }

	TArray<SSpeedTreeVertexDataSelfShadow> m_vVertices;
};


//////////////////////////////////////////////////////
//	struct SSpeedTreeSection

struct SSpeedTreeSection
{
	SSpeedTreeSection(void) :
		m_uiFirstIndex(0),
		m_uiNumTriangles(0),
		m_uiMinVertex(UINT(-1)),
		m_uiMaxVertex(0),
		m_pcVertexFactory(NULL),
		m_pcIndexBuffer(NULL)		
	{
	}

	UINT 						m_uiFirstIndex;
	UINT 						m_uiNumTriangles;
	UINT 						m_uiMinVertex;
	UINT						m_uiMaxVertex;

	FLocalVertexFactory*		m_pcVertexFactory;
	FRawIndexBuffer*			m_pcIndexBuffer;
};

#endif


//////////////////////////////////////////////////////
//	class USpeedTree
class USpeedTree : public UObject
{				
	DECLARE_CLASS(USpeedTree, UObject, CLASS_SafeReplace | CLASS_CollapseCategories | CLASS_Intrinsic, Engine);
public:

#ifdef WITH_SPEEDTREE
			// Construction / Destruction
								USpeedTree(void);
			void				StaticConstructor(void);
	virtual void				Destroy(void);
			void				CleanUp(void);

			// UObject Interface
	virtual void				PostEditChange(UProperty* pPropertyThatChanged);
	virtual void				Serialize(FArchive& Ar);

			// SpeedTreeComponent Interface
			void				Load(const BYTE* pBuffer, UINT uiNumBytes, INT iRandomSeed = 1, UBOOL bUseBranchWind = false);
			UBOOL				IsInitialized(void)			{ return m_bIsInitialized; }
			FBoxSphereBounds	GetBounds(void);
			void				UpdateWind(void);
			void				SetWind(const FVector& cWindDirection, FLOAT fWindStrength);
			void				Precache(void);
			UINT				GetNumCollisionPrimitives(void);
			void				GetCollisionPrimitive(UINT uiIndex, CSpeedTreeRT::ECollisionObjectType& eType, FLOAT* afPosition, FLOAT* afDimensions);

			// Rendering
			void				RenderTree(const FSceneContext& cContext, 
											FPrimitiveRenderInterface* pcPRI,
											FMatrix& cLocalToWorld,
											FMatrix& cWorldToLocal,
											TArray<FMeshElementLightInfo>& aLights,
											FLOAT fNearLOD, 
											FLOAT fFarLOD, 
											FLOAT fLODOverride,
											UBOOL bUseBranches, 
											UBOOL bUseFronds, 
											UBOOL bUseLeaves, 
											UBOOL bUseBillboards,
											UBOOL bSelected);
			void				RenderSection(const FSceneContext& cContext, 
											FPrimitiveRenderInterface* pcPRI,
											FMatrix& cLocalToWorld,
											FMatrix& cWorldToLocal,
											TArray<FMeshElementLightInfo>& aLights,
											SSpeedTreeSection& sSection, 
											UMaterialInstance* puMaterial, 
											FLOAT fAlphaMaskValue,
											UBOOL bSelected);
			void				RenderShadowVolume(USpeedTreeComponent* pComponent,
													const FSceneContext& cContext, 
													FShadowVolumeRenderInterface* pSVRI,
													ULightComponent* pLight);

protected:
			void 				SetupBranchesOrFronds(UBOOL bBranches);
			void 				SetupLeaves(void);
			void				SetupBillboards(void);
			void				SetupShadows(void);
			void				UpdateBillboards(void);
			void				UpdateBranchesAndFronds(void);

protected:
	CSpeedTreeRT*						m_pcSpeedTree;					// the speedtree class

	CSpeedWind							m_cSpeedWind;					// SpeedWind object
	FLOAT								m_fWindStrength;				// wind strength
	FVector								m_cWindDir;						// wind direction

	BITFIELD 							m_bHasBranches		: 1;		// draw branches or not
	BITFIELD 							m_bHasFronds		: 1;		// draw fronds or no  t
	BITFIELD 							m_bHasLeaves		: 1;		// draw leaves or not
	BITFIELD							m_bIsInitialized	: 1;		// is it initialized
	BITFIELD							m_bIsUpdated		: 1;		// is the tree updated for this frame
	BITFIELD							m_bIsWindUpdated	: 1;		// is the wind updated for this frame


	SSpeedTreePositionBuffer			m_sBranchFrondPositionBuffer;	// buffer for vertex position and shadow extrusion
	SSpeedTreeSelfShadowBuffer			m_sBranchFrondDataBuffer;		// buffer for normals, texcoords, etc

	SSpeedTreePositionBuffer			m_sLeafPositionBuffer;			// buffer for leaf position
	SSpeedTreeDataBuffer				m_sLeafDataBuffer;				// buffer for leaf normals, texcoords, etc

	SSpeedTreePositionBuffer			m_sBillboardPositionBuffer;		// buffer for leaves' and billboards' position
	SSpeedTreeDataBuffer				m_sBillboardDataBuffer;			// buffer for leaves' and billboards' data 
	
	FLocalVertexFactory					m_cBranchFrondVertexFactory;	// vertex factory for branches and fronds
	FLocalVertexFactory					m_cLeafVertexFactory;			// vertex factory for the leaves
	FLocalVertexFactory					m_cBillboardVertexFactory;		// vertex factory for billboards
	FLocalShadowVertexFactory			m_cShadowVertexFactory;			// shadow volume vertex factory

	FRawIndexBuffer						m_cIndexBuffer;					// the index buffer (holds everything)

	SSpeedTreeSection*					m_pasBranchSections;			// sections to draw per branch LOD
	SSpeedTreeSection*					m_pasFrondSections;				// sections to draw per frond LOD
	SSpeedTreeSection*					m_pasLeafSections;				// sections to draw per leaf LOD
	SSpeedTreeSection					m_asBillboardSections[2];		// sections to draw per billboard

	TArray<FMeshEdge>					m_vShadowEdges;					// recorded shadow edges for shadow volumes
	UINT								m_uiNumBranchEdges;				// last branch edge index

	TArray<UMaterialInstanceConstant*>	m_acMaterialInstances;			// material instances instantiated during render, to be deleted after the frame is done

public:
	// editor-accesible variables
	INT									RandomSeed;					// random seed for tree creation
	UBOOL								bUseWindyBranches;			// use windy branches or not (more computationally expensive, but branches move)

	UMaterialInstance*					BranchMaterial;				// branch material
	UMaterialInstance*					FrondMaterial;				// frond material
	UMaterialInstance*					LeafMaterial;				// leaf material
	UMaterialInstance*					BillboardMaterial;			// billboard material

#endif // WITH_SPEEDTREE
};

#endif

