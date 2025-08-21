/*=============================================================================
	UnParticleSystemRender.cpp: Particle system rendering functions.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"

#include "UnParticleHelper.h"

//@todo.SAS. Remove this once the Trail packing bug is corrected.
#include "ScenePrivate.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FParticleDynamicData::FParticleDynamicData(UParticleSystemComponent* PartSysComp)
{
	DynamicEmitterDataArray.Empty();
	for (INT EmitterIndex = 0; EmitterIndex < PartSysComp->EmitterInstances.Num(); EmitterIndex++)
	{
		FDynamicEmitterDataBase* NewDynamicEmitterData = NULL;
		FParticleEmitterInstance* EmitterInst = PartSysComp->EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			NewDynamicEmitterData = EmitterInst->GetDynamicData(PartSysComp->IsOwnerSelected());
		}

		if (NewDynamicEmitterData != NULL)
		{
			INT NewIndex = DynamicEmitterDataArray.Add(1);
			check(NewIndex >= 0);
			DynamicEmitterDataArray(NewIndex) = NewDynamicEmitterData;
		}
	}
}

void FDynamicSpriteEmitterDataBase::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
	check(SceneProxy);

	const FMatrix& LocalToWorld = bUseLocalSpace ? SceneProxy->GetLocalToWorld() : FMatrix::Identity;

	FMatrix CameraToWorld = View->ViewMatrix.Inverse();
	FVector CamX = CameraToWorld.TransformNormal(FVector(1,0,0));
	FVector CamY = CameraToWorld.TransformNormal(FVector(0,1,0));

	FLinearColor EmitterEditorColor = FLinearColor(1.0f,1.0f,0);

	for (INT i = 0; i < ParticleCount; i++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

		FVector DrawLocation = LocalToWorld.TransformFVector(Particle.Location);
		if (bCrosses)
		{
			FVector Size = Particle.Size * Scale;
			PDI->DrawLine(DrawLocation - (0.5f * Size.X * CamX), DrawLocation + (0.5f * Size.X * CamX), EmitterEditorColor, DPGIndex);
			PDI->DrawLine(DrawLocation - (0.5f * Size.Y * CamY), DrawLocation + (0.5f * Size.Y * CamY), EmitterEditorColor, DPGIndex);
		}
		else
		{
			PDI->DrawPoint(DrawLocation, EmitterEditorColor, 2, DPGIndex);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	ParticleMeshEmitterInstance
///////////////////////////////////////////////////////////////////////////////
// The resource used to render a UMaterialInstanceConstant.
class FMeshEmitterMaterialInstanceResource : public FMaterialInstance
{
public:
	FMeshEmitterMaterialInstanceResource() : 
	    FMaterialInstance()
	  , Parent(NULL)
	{
	}

	FMeshEmitterMaterialInstanceResource(FMaterialInstance* InParent) : 
	    FMaterialInstance()
	  , Parent(InParent)
	{
	}

	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
	{
		if (ParameterName == NAME_MeshEmitterVertexColor)
		{
			*OutValue = Param_MeshEmitterVertexColor;
			return TRUE;
		}
		else
		if (ParameterName == NAME_TextureOffsetParameter)
		{ 
			*OutValue = Param_TextureOffsetParameter;
			return TRUE;
		}
		else
		if (ParameterName == NAME_TextureScaleParameter)
		{
			*OutValue = Param_TextureScaleParameter;
			return TRUE;
		}

		if (Parent == NULL)
		{
			return FALSE;
		}

		return Parent->GetVectorValue(ParameterName, OutValue);
	}

	UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
	{
		return Parent->GetScalarValue(ParameterName, OutValue);
	}

	UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const
	{
		return Parent->GetTextureValue(ParameterName, OutValue);
	}

	virtual const FMaterial* GetMaterial() const
	{
		return Parent->GetMaterial();
	}

	FMaterialInstance* Parent;
	FLinearColor Param_MeshEmitterVertexColor;
	FLinearColor Param_TextureOffsetParameter;
	FLinearColor Param_TextureScaleParameter;
};

///////////////////////////////////////////////////////////////////////////////
//	FDynamicSpriteEmitterData
///////////////////////////////////////////////////////////////////////////////
UBOOL FDynamicSpriteEmitterData::GetVertexAndIndexData(FParticleSpriteVertex* VertexData, void* FillIndexData, TArray<FParticleOrder>* ParticleOrder, INT BaseVertex, const FMatrix* LocalToWorld)
{
	INT ParticleCount = ActiveParticleCount;
	// 'clamp' the number of particles actually drawn
	//@todo.SAS. If sorted, we really want to render the front 'N' particles...
	// right now it renders the back ones. (Same for SubUV draws)
	INT StartIndex = 0;
	INT EndIndex = ParticleCount;
	if ((MaxDrawCount >= 0) && (ParticleCount > MaxDrawCount))
	{
		ParticleCount = MaxDrawCount;
	}

	// Pack the data
	INT	ParticleIndex;
	INT	ParticlePackingIndex = 0;
	INT	IndexPackingIndex = 0;

	FParticleSpriteVertex* Vertices = VertexData;
	WORD* Indices = (WORD*)FillIndexData;

	UBOOL bSorted = ParticleOrder ? TRUE : FALSE;

	for (INT i = 0; i < ParticleCount; i++)
	{
		if (bSorted)
		{
			ParticleIndex	= (*ParticleOrder)(i).ParticleIndex;
		}
		else
		{
			ParticleIndex	= i;
		}

		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);

		FVector Size = Particle.Size * Scale;
		if (ScreenAlignment == PSA_Square)
		{
			Size.Y = Size.X;
		}

		FVector Location = Particle.Location;
		FVector OldLocation = Particle.OldLocation;

		if (LocalToWorld)
		{
			Location = LocalToWorld->TransformFVector(Location);
			OldLocation = LocalToWorld->TransformFVector(OldLocation);
		}

		// 0
		Vertices[ParticlePackingIndex].Position		= Location;
		Vertices[ParticlePackingIndex].OldPosition	= OldLocation;
		Vertices[ParticlePackingIndex].Size			= Size;
		Vertices[ParticlePackingIndex].Tex_U		= 0.f;
		Vertices[ParticlePackingIndex].Tex_V		= 0.f;
		Vertices[ParticlePackingIndex].Rotation		= Particle.Rotation;
		Vertices[ParticlePackingIndex].Color		= Particle.Color;
		ParticlePackingIndex++;
		// 1
		Vertices[ParticlePackingIndex].Position		= Location;
		Vertices[ParticlePackingIndex].OldPosition	= OldLocation;
		Vertices[ParticlePackingIndex].Size			= Size;
		Vertices[ParticlePackingIndex].Tex_U		= 0.f;
		Vertices[ParticlePackingIndex].Tex_V		= 1.f;
		Vertices[ParticlePackingIndex].Rotation		= Particle.Rotation;
		Vertices[ParticlePackingIndex].Color		= Particle.Color;
		ParticlePackingIndex++;
		// 2
		Vertices[ParticlePackingIndex].Position		= Location;
		Vertices[ParticlePackingIndex].OldPosition	= OldLocation;
		Vertices[ParticlePackingIndex].Size			= Size;
		Vertices[ParticlePackingIndex].Tex_U		= 1.f;
		Vertices[ParticlePackingIndex].Tex_V		= 1.f;
		Vertices[ParticlePackingIndex].Rotation		= Particle.Rotation;
		Vertices[ParticlePackingIndex].Color		= Particle.Color;
		ParticlePackingIndex++;
		// 3
		Vertices[ParticlePackingIndex].Position		= Location;
		Vertices[ParticlePackingIndex].OldPosition	= OldLocation;
		Vertices[ParticlePackingIndex].Size			= Size;
		Vertices[ParticlePackingIndex].Tex_U		= 1.f;
		Vertices[ParticlePackingIndex].Tex_V		= 0.f;
		Vertices[ParticlePackingIndex].Rotation		= Particle.Rotation;
		Vertices[ParticlePackingIndex].Color		= Particle.Color;
		ParticlePackingIndex++;

		if (Indices)
		{
			*Indices++ = (i * 4) + 0 + BaseVertex;
			*Indices++ = (i * 4) + 2 + BaseVertex;
			*Indices++ = (i * 4) + 3 + BaseVertex;
			*Indices++ = (i * 4) + 0 + BaseVertex;
			*Indices++ = (i * 4) + 1 + BaseVertex;
			*Indices++ = (i * 4) + 2 + BaseVertex;
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicSubUVEmitterData
///////////////////////////////////////////////////////////////////////////////
UBOOL FDynamicSubUVEmitterData::GetVertexAndIndexData(FParticleSpriteSubUVVertex* VertexData, void* FillIndexData, TArray<FParticleOrder>* ParticleOrder, INT BaseVertex, const FMatrix* LocalToWorld)
{
	INT ParticleCount = ActiveParticleCount;
	// 'clamp' the number of particles actually drawn
	//@todo.SAS. If sorted, we really want to render the front 'N' particles...
	// right now it renders the back ones. (Same for SubUV draws)
	INT StartIndex = 0;
	INT EndIndex = ParticleCount;
	if ((MaxDrawCount >= 0) && (ParticleCount > MaxDrawCount))
	{
		ParticleCount = MaxDrawCount;
	}

	// Pack the data
	INT	ParticleIndex;
	INT	ParticlePackingIndex = 0;
	INT	IndexPackingIndex = 0;

	INT			SIHorz			= SubImages_Horizontal;
	INT			SIVert			= SubImages_Vertical;
	INT			iTotalSubImages = SIHorz * SIVert;
	FLOAT		baseU			= (1.0f / (FLOAT)SIHorz);
	FLOAT		baseV			= (1.0f / (FLOAT)SIVert);
	FLOAT		U;
	FLOAT		V;

	FParticleSpriteSubUVVertex* Vertices = VertexData;
	WORD* Indices = (WORD*)FillIndexData;

	UBOOL bSorted = ParticleOrder ? TRUE : FALSE;

	for (INT i = 0; i < ParticleCount; i++)
	{
		if (bSorted)
		{
			ParticleIndex	= (*ParticleOrder)(i).ParticleIndex;
		}
		else
		{
			ParticleIndex	= i;
		}

		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);

		FVector Size = Particle.Size * Scale;
		if (ScreenAlignment == PSA_Square)
		{
			Size.Y = Size.X;
		}

		FSubUVSpritePayload* PayloadData = (FSubUVSpritePayload*)(((BYTE*)&Particle) + SubUVDataOffset);

		FVector Location = Particle.Location;
		FVector OldLocation = Particle.OldLocation;

		if (LocalToWorld)
		{
			Location = LocalToWorld->TransformFVector(Location);
			OldLocation = LocalToWorld->TransformFVector(OldLocation);
		}

		// 0
		VertexData[ParticlePackingIndex].Position		= Location;
		VertexData[ParticlePackingIndex].OldPosition	= OldLocation;
		VertexData[ParticlePackingIndex].Size			= Size;
		VertexData[ParticlePackingIndex].Rotation		= Particle.Rotation;
		VertexData[ParticlePackingIndex].Color			= Particle.Color;
		VertexData[ParticlePackingIndex].Interp			= PayloadData->Interpolation;
		if (bDirectUV)
		{
			U	= baseU * (PayloadData->ImageH + 0);
			V	= baseV * (PayloadData->ImageV + 0);
    		VertexData[ParticlePackingIndex].Tex_U	= U;
	    	VertexData[ParticlePackingIndex].Tex_V	= V;
			VertexData[ParticlePackingIndex].Tex_U2	= U;
			VertexData[ParticlePackingIndex].Tex_V2	= V;
		}
		else
		{
    		VertexData[ParticlePackingIndex].Tex_U	= baseU * (appTrunc(PayloadData->ImageH) + 0);
	    	VertexData[ParticlePackingIndex].Tex_V	= baseV * (appTrunc(PayloadData->ImageV) + 0);
			VertexData[ParticlePackingIndex].Tex_U2	= baseU * (appTrunc(PayloadData->Image2H) + 0);
			VertexData[ParticlePackingIndex].Tex_V2	= baseV * (appTrunc(PayloadData->Image2V) + 0);
		}
		VertexData[ParticlePackingIndex].SizeU		= 0.f;
		VertexData[ParticlePackingIndex].SizeV		= 0.f;
		ParticlePackingIndex++;
		// 1
		VertexData[ParticlePackingIndex].Position		= Location;
		VertexData[ParticlePackingIndex].OldPosition	= OldLocation;
		VertexData[ParticlePackingIndex].Size			= Size;
		VertexData[ParticlePackingIndex].Rotation		= Particle.Rotation;
		VertexData[ParticlePackingIndex].Color			= Particle.Color;
		VertexData[ParticlePackingIndex].Interp			= PayloadData->Interpolation;
		if (bDirectUV)
		{
			U	= baseU * (PayloadData->ImageH + 0);
			V	= baseV * (PayloadData->ImageV + PayloadData->Image2V);
			VertexData[ParticlePackingIndex].Tex_U	= U;
			VertexData[ParticlePackingIndex].Tex_V	= V;
			VertexData[ParticlePackingIndex].Tex_U2	= U;
			VertexData[ParticlePackingIndex].Tex_V2	= V;
		}
		else
		{
			VertexData[ParticlePackingIndex].Tex_U	= baseU * (appTrunc(PayloadData->ImageH) + 0);
			VertexData[ParticlePackingIndex].Tex_V	= baseV * (appTrunc(PayloadData->ImageV) + 1);
			VertexData[ParticlePackingIndex].Tex_U2	= baseU * (appTrunc(PayloadData->Image2H) + 0);
			VertexData[ParticlePackingIndex].Tex_V2	= baseV * (appTrunc(PayloadData->Image2V) + 1);
		}
		VertexData[ParticlePackingIndex].SizeU		= 0.f;
		VertexData[ParticlePackingIndex].SizeV		= 1.f;
		ParticlePackingIndex++;
		// 2
		VertexData[ParticlePackingIndex].Position		= Location;
		VertexData[ParticlePackingIndex].OldPosition	= OldLocation;
		VertexData[ParticlePackingIndex].Size			= Size;
		VertexData[ParticlePackingIndex].Rotation		= Particle.Rotation;
		VertexData[ParticlePackingIndex].Color			= Particle.Color;
		VertexData[ParticlePackingIndex].Interp			= PayloadData->Interpolation;
		if (bDirectUV)
		{
			U	= baseU * (PayloadData->ImageH + PayloadData->Image2H);
			V	= baseV * (PayloadData->ImageV + PayloadData->Image2V);
    		VertexData[ParticlePackingIndex].Tex_U	= U;
	    	VertexData[ParticlePackingIndex].Tex_V	= V;
			VertexData[ParticlePackingIndex].Tex_U2	= U;
			VertexData[ParticlePackingIndex].Tex_V2	= V;
		}
		else
		{
    		VertexData[ParticlePackingIndex].Tex_U	= baseU * (appTrunc(PayloadData->ImageH) + 1);
	    	VertexData[ParticlePackingIndex].Tex_V	= baseV * (appTrunc(PayloadData->ImageV) + 1);
			VertexData[ParticlePackingIndex].Tex_U2	= baseU * (appTrunc(PayloadData->Image2H) + 1);
			VertexData[ParticlePackingIndex].Tex_V2	= baseV * (appTrunc(PayloadData->Image2V) + 1);
		}
		VertexData[ParticlePackingIndex].SizeU		= 1.f;
		VertexData[ParticlePackingIndex].SizeV		= 1.f;
		ParticlePackingIndex++;
		// 3
		VertexData[ParticlePackingIndex].Position		= Location;
		VertexData[ParticlePackingIndex].OldPosition	= OldLocation;
		VertexData[ParticlePackingIndex].Size			= Size;
		VertexData[ParticlePackingIndex].Rotation		= Particle.Rotation;
		VertexData[ParticlePackingIndex].Color			= Particle.Color;
		VertexData[ParticlePackingIndex].Interp			= PayloadData->Interpolation;
		if (bDirectUV)
		{
			U	= baseU * (PayloadData->ImageH + PayloadData->Image2H);
			V	= baseV * (PayloadData->ImageV + 0);
    		VertexData[ParticlePackingIndex].Tex_U	= U;
	    	VertexData[ParticlePackingIndex].Tex_V	= V;
			VertexData[ParticlePackingIndex].Tex_U2	= U;
			VertexData[ParticlePackingIndex].Tex_V2	= V;
		}
		else
		{
    		VertexData[ParticlePackingIndex].Tex_U	= baseU * (appTrunc(PayloadData->ImageH) + 1);
	    	VertexData[ParticlePackingIndex].Tex_V	= baseV * (appTrunc(PayloadData->ImageV) + 0);
			VertexData[ParticlePackingIndex].Tex_U2	= baseU * (appTrunc(PayloadData->Image2H) + 1);
			VertexData[ParticlePackingIndex].Tex_V2	= baseV * (appTrunc(PayloadData->Image2V) + 0);
		}
		VertexData[ParticlePackingIndex].SizeU		= 1.f;
		VertexData[ParticlePackingIndex].SizeV		= 0.f;
		ParticlePackingIndex++;

		if (Indices)
		{
			*Indices++ = (i * 4) + 0 + BaseVertex;
			*Indices++ = (i * 4) + 2 + BaseVertex;
			*Indices++ = (i * 4) + 3 + BaseVertex;
			*Indices++ = (i * 4) + 0 + BaseVertex;
			*Indices++ = (i * 4) + 1 + BaseVertex;
			*Indices++ = (i * 4) + 2 + BaseVertex;
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicMeshEmitterData
///////////////////////////////////////////////////////////////////////////////
FDynamicMeshEmitterData::FDynamicMeshEmitterData(INT InParticleCount, INT InParticleStride, const FMaterialInstance* InMaterialResource,
	INT InSubUVInterpMethod, INT InSubUVDataOffset, UStaticMesh* InStaticMesh, const UStaticMeshComponent* InStaticMeshComponent, INT InMaxDrawCount) :
	  FDynamicSpriteEmitterDataBase(InParticleCount, InParticleStride, InMaterialResource, InMaxDrawCount)
	, SubUVInterpMethod(InSubUVInterpMethod)
	, SubUVDataOffset(InSubUVDataOffset)
	, StaticMesh(InStaticMesh)
{
	eEmitterType = DET_Mesh;
	if (InParticleCount > 0)
	{
		check(InParticleCount < 16 * 1024);	// TTP #33375
		check(InParticleStride < 2 * 1024);	// TTP #3375
		ParticleData = (BYTE*)appRealloc(ParticleData, InParticleCount * InParticleStride);
		check(ParticleData);
		ParticleIndices = (WORD*)appRealloc(ParticleIndices, InParticleCount * sizeof(WORD));
		check(ParticleIndices);
	}

	// Build the proxy's LOD data.
	for (INT LODIndex = 0;LODIndex < StaticMesh->LODModels.Num();LODIndex++)
	{
		new(LODs) FLODInfo(InStaticMeshComponent, LODIndex);
	}
}

FDynamicMeshEmitterData::~FDynamicMeshEmitterData()
{
}

/** Information used by the proxy about a single LOD of the mesh. */
FDynamicMeshEmitterData::FLODInfo::FLODInfo(const UStaticMeshComponent* Component,INT LODIndex)
{
	// Gather the materials applied to the LOD.
	Elements.Empty(Component->StaticMesh->LODModels(LODIndex).Elements.Num());
	for (INT MaterialIndex = 0; MaterialIndex < Component->StaticMesh->LODModels(LODIndex).Elements.Num(); MaterialIndex++)
	{
		FElementInfo ElementInfo;

		// Determine the material applied to this element of the LOD.
		ElementInfo.Material = Component->GetMaterial(MaterialIndex,LODIndex);
		if (!ElementInfo.Material)
		{
			ElementInfo.Material = GEngine->DefaultMaterial;
		}

		// Store the element info.
		Elements.AddItem(ElementInfo);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicBeam2EmitterData
///////////////////////////////////////////////////////////////////////////////
void FDynamicBeam2EmitterData::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicTrail2EmitterData
///////////////////////////////////////////////////////////////////////////////
void FDynamicTrail2EmitterData::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
}

///////////////////////////////////////////////////////////////////////////////
//	ParticleSystemSceneProxy
///////////////////////////////////////////////////////////////////////////////
/** Initialization constructor. */
FParticleSystemSceneProxy::FParticleSystemSceneProxy(const UParticleSystemComponent* Component):
	  FPrimitiveSceneProxy(Component)
	, Owner(Component->GetOwner())
	, bSelected(Component->IsOwnerSelected())
	, CullDistanceEx(Component->CullDistanceEx)
	, bCastShadow(Component->CastShadow)
	, bHasTranslucency(Component->HasUnlitTranslucency())
	, bHasDistortion(Component->HasUnlitDistortion())
	, bUsesSceneColor(bHasTranslucency && Component->UsesSceneColor())
	, DynamicData(NULL)
	, LastDynamicData(NULL)
	, SelectedWireframeMaterialInstance(
		GEngine->WireframeMaterial->GetInstanceInterface(FALSE),
		GetSelectionColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),TRUE)
		)
	, DeselectedWireframeMaterialInstance(
		GEngine->WireframeMaterial->GetInstanceInterface(FALSE),
		GetSelectionColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),FALSE)
		)
	, PendingLODDistance(0.0f)
	, ApproximateDepthSortKey((INT)Component->Template)
	, bTranslucencyFence(Component->bTranslucencyFence)
{
	LODMethod = Component->LODMethod;
}

UBOOL FParticleSystemSceneProxy::IsCacheable() const
{
	return TRUE;
}

UBOOL FParticleSystemSceneProxy::IsStillValid( const UPrimitiveComponent* InComponent ) const
{
	const UParticleSystemComponent* Component = static_cast<const UParticleSystemComponent*>(InComponent);

	return GIsGame && FPrimitiveSceneProxy::IsStillValid( InComponent );		
}

UBOOL FDynamicSpriteEmitterDataBase::IsConcatable( FDynamicEmitterDataBase* Base ) const
{
	if (!FDynamicEmitterDataBase::IsConcatable(Base))
		return FALSE;

	FDynamicSpriteEmitterDataBase* Data = (FDynamicSpriteEmitterDataBase*)Base;

	return (MaterialResource == Data->MaterialResource &&
		bSelected == Data->bSelected &&
		ScreenAlignment == Data->ScreenAlignment &&
		//bUseLocalSpace == Data->bUseLocalSpace &&
		//(!bUseLocalSpace || SceneProxy == Data->SceneProxy /*LocalToWorld*/) &&
		bLockAxis == Data->bLockAxis &&
		(!bLockAxis || bLockAxis && LockAxisFlag == Data->LockAxisFlag) &&
		SceneProxy->bCastShadow == Data->SceneProxy->bCastShadow &&
		SceneProxy->bSelected == Data->SceneProxy->bSelected &&
		EmitterRenderMode == Data->EmitterRenderMode);	
}

UBOOL FDynamicSpriteEmitterData::IsConcatable( FDynamicEmitterDataBase* Base ) const
{
	if (!FDynamicSpriteEmitterDataBase::IsConcatable(Base))
		return FALSE;

	FDynamicSpriteEmitterData* Data = (FDynamicSpriteEmitterData*)Base;

	return TRUE;
}

UBOOL FDynamicSubUVEmitterData::IsConcatable( FDynamicEmitterDataBase* Base ) const
{
	if (!FDynamicSpriteEmitterDataBase::IsConcatable(Base))
		return FALSE;

	FDynamicSubUVEmitterData* Data = (FDynamicSubUVEmitterData*)Base;
	
	return 
		(SubUVDataOffset == Data->SubUVDataOffset &&
		SubImages_Horizontal == Data->SubImages_Horizontal &&
		SubImages_Vertical == Data->SubImages_Vertical &&
		bDirectUV == Data->bDirectUV);
}

FParticleSystemSceneProxy::~FParticleSystemSceneProxy()
{
	delete DynamicData;
	DynamicData = NULL;
}

IMPLEMENT_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents,{ return A.Z < B.Z ? 1 : -1; });

struct FSpritePolicy
{
	typedef FDynamicSpriteEmitterData	DataType;
	typedef FParticleSpriteVertex		VertexType;	
};

struct FSubUVPolicy
{
	typedef FDynamicSubUVEmitterData	DataType;
	typedef FParticleSpriteSubUVVertex	VertexType;
};

static void DrawEmitterData( FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, FDynamicEmitterDataBase* Data )
{
	switch (Data->eEmitterType)
	{
	case FDynamicEmitterDataBase::DET_Sprite:
		Data->SceneProxy->DrawDynamicSpriteEmitter(Data, PDI, View, DPGIndex);	
		break;
	case FDynamicEmitterDataBase::DET_SubUV:
		Data->SceneProxy->DrawDynamicSubUVEmitter(Data, PDI, View, DPGIndex);	
		break;
	case FDynamicEmitterDataBase::DET_Mesh:
		Data->SceneProxy->DrawDynamicMeshEmitter(Data, PDI, View, DPGIndex);
		break;
	case FDynamicEmitterDataBase::DET_Beam2:
		Data->SceneProxy->DrawDynamicBeam2Emitter(Data, PDI, View, DPGIndex);	
		break;
	case FDynamicEmitterDataBase::DET_Trail2:
		Data->SceneProxy->DrawDynamicTrail2Emitter(Data, PDI, View, DPGIndex);	
		break;
	}			
}


template <typename SpriteDataType>
struct TParticleBatch
{	
	SpriteDataType*					FirstNode;

	TParticleBatch()
		: FirstNode(NULL)
	{
	}

	void Clear()
	{
		FirstNode = NULL;
	}
	
	void Add( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, FDynamicEmitterDataBase* InData, UBOOL bBurstMode )
	{
		SpriteDataType* Data = (SpriteDataType*)InData;

		Data->Batch_Next = NULL;
		SpriteDataType** Prev = &FirstNode;

		for (SpriteDataType* Itr = FirstNode; Itr; )
		{				
			if (Itr->IsConcatable( Data ))
			{
				// tail뒤에다가 붙임
				Itr->Batch_Tail->Batch_Next = Data;

				// 이게 새로운 tail
				Itr->Batch_Tail = Data;
				return;				
			}
			else if (!bBurstMode)
			{
				Flush( Context, PDI, View, DPGIndex );
				break;
			}
			else if (Itr->ApproximateDepthSortKey > Data->ApproximateDepthSortKey)
			{				
				// tail은 자기 자신
				Data->Batch_Tail = Data;
				Data->Batch_Sibling = Itr;
				*Prev = Data;
				return;
			}

			Prev	= (SpriteDataType**)&Itr->Batch_Sibling;
			Itr		= *Prev;
		}

		// tail은 자기 자신
		Data->Batch_Tail = Data;
		Data->Batch_Sibling = NULL;
		*Prev = Data;
	}

	void Flush( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex )
	{
		Draw(Context,PDI,View,DPGIndex);

		Clear();
	}

	void Draw( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex )
	{
		for (SpriteDataType* Itr = FirstNode; Itr; Itr = (SpriteDataType*)Itr->Batch_Sibling)
		{	
			DrawEmitterData(PDI,View,DPGIndex,Itr);			
		}		
	}
};

struct FParticleSystemBatch
{
	FParticleSystemBatch()
		: bIsRunning( FALSE )
	{
	}

	TParticleBatch<FDynamicSpriteEmitterData> Batch_Sprite;
	TParticleBatch<FDynamicSubUVEmitterData> Batch_SubUV;
	//TParticleBatch<FDynamicMeshEmitterData> Batch_Mesh;	

	UBOOL bIsRunning, bBurstMode, bNeedsFlush, bTranslucentPass;	

	FDynamicEmitterDataBase::EmitterType LastEmitterType;

	void Init( UBOOL bInTranslucentPass )
	{
		LastEmitterType = FDynamicEmitterDataBase::DET_Unknown;		
		bIsRunning = TRUE;
		bBurstMode = TRUE;
		bNeedsFlush = FALSE;
		bTranslucentPass = bInTranslucentPass;
	}

	void SetBurstMode( UBOOL bNewBurstMode )
	{
		if (bNewBurstMode ^ bBurstMode)
		{
			bBurstMode = bNewBurstMode;

			bNeedsFlush = TRUE;
		}	
	}

	/*

	Burst할 때는
	Additive를 가장 마지막에 그리는 게 좋을 것 같은뎅; :)?

	*/

	void Add( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, FDynamicEmitterDataBase* Data )
	{
		check( bIsRunning );		

		// 지원하지 않는 type
		if (Data->eEmitterType != FDynamicEmitterDataBase::DET_Sprite &&
			Data->eEmitterType != FDynamicEmitterDataBase::DET_SubUV ||
			((FDynamicSpriteEmitterDataBase*)Data)->EmitterRenderMode != ERM_Normal ||
			(View->Family->ShowFlags & SHOW_Wireframe))
		{
			if (!bBurstMode)
			{
				Flush( Context, PDI, View, DPGIndex );
			}			

			DrawEmitterData(PDI,View,DPGIndex,Data);			

			return;
		}

		// Translucent pass가 아니거나 fence가 아니면 burst다.
		SetBurstMode( !bTranslucentPass || !Data->bTranslucencyFence );

		// burst mode가 아닌데 batch가 달라지면 flush!
		if (!bBurstMode && LastEmitterType != FDynamicEmitterDataBase::DET_Unknown && LastEmitterType != Data->eEmitterType)
		{
			bNeedsFlush = TRUE;
		}				
			
		if (bNeedsFlush)
		{
			Flush( Context, PDI, View, DPGIndex );
		}

		switch (Data->eEmitterType)
		{
		case FDynamicEmitterDataBase::DET_Sprite:
			Batch_Sprite.Add(Context,PDI,View,DPGIndex,Data,bBurstMode);
			break;
		case FDynamicEmitterDataBase::DET_SubUV:
			Batch_SubUV.Add(Context,PDI,View,DPGIndex,Data,bBurstMode);
			break;
/*		case FDynamicEmitterDataBase::DET_Mesh:
			Batch_Mesh.Add(Context,PDI,View,DPGIndex,Data,bBurstMode);			
			break;		*/
		}			

		LastEmitterType = Data->eEmitterType;
	}	

	const FSceneView* View;

	struct FSpritePolicy
	{
		typedef FDynamicSpriteEmitterData	DataType;
		typedef FParticleSpriteVertex		VertexType;

		enum { PET = PET_Sprite };
	};

	struct FSubUVPolicy
	{
		typedef FDynamicSubUVEmitterData	DataType;
		typedef FParticleSpriteSubUVVertex	VertexType;

		enum { PET = PET_SubUV };
	};
	
	void DrawMesh( FCommandContextRHI* Context )
	{			
		DynamicMeshContext.DrawIndexedPrimitive(Context);
	}

	void Flush( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex )
	{
		bNeedsFlush = FALSE;		

		if (bBurstMode)
		{
			Batch_Sprite.Flush(Context,PDI,View,DPGIndex);				
			Batch_SubUV.Flush(Context,PDI,View,DPGIndex);				
		}
		else
		{			
			switch (LastEmitterType)
			{
			case FDynamicEmitterDataBase::DET_Sprite:
				Batch_Sprite.Flush(Context,PDI,View,DPGIndex);	
				break;
			case FDynamicEmitterDataBase::DET_SubUV:
				Batch_SubUV.Flush(Context,PDI,View,DPGIndex);	
				break;
			/*case FDynamicEmitterDataBase::DET_Mesh:
				Batch_Mesh.Draw(Context,PDI,View,DPGIndex);	*/
				break;					
			}		
		}
	}

	void Uninit()
	{
		bIsRunning = FALSE;
	}

	FDynamicMeshContext DynamicMeshContext;

	template <typename Policy>
	void BatchDraw_Sprites_Inner( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI,const FSceneView* View, UINT DPGIndex, typename Policy::DataType* FirstData )
	{
		typedef Policy::DataType DataType;
		typedef Policy::VertexType VertexType;		
		
		FParticleSystemSceneProxy* Proxy = FirstData->SceneProxy;

		// Setup VertexFactory	
		FirstData->VertexFactory->SetScreenAlignment(FirstData->ScreenAlignment);
		FirstData->VertexFactory->SetLockAxes(FirstData->bLockAxis);
		if (FirstData->bLockAxis)
		{
			FVector Up, Right;
			Proxy->GetAxisLockValues((FDynamicSpriteEmitterDataBase*)FirstData, Up, Right);
			FirstData->VertexFactory->SetLockAxes(Up, Right);
		}

		// If material is using unlit translucency and the blend mode is translucent or 
		// if it is using unlit distortion then we need to sort (back to front)
		const FMaterial* Material = FirstData->MaterialResource->GetMaterial();
		UBOOL bShouldSort = (Material && 
			Material->GetLightingModel() == MLM_Unlit && 
			(Material->GetBlendMode() == BLEND_Translucent || Material->IsDistorted())
			);	

		FMeshElement Mesh;

		UBOOL bUseLocalSpace = FirstData->bUseLocalSpace;
		UBOOL bNeedsToTransform = bUseLocalSpace && FirstData->Batch_Next != NULL;

		Mesh.LCI					= NULL;
		if (bUseLocalSpace && !bNeedsToTransform)
		{
			Mesh.LocalToWorld = Proxy->LocalToWorld;
			Mesh.WorldToLocal = Proxy->LocalToWorld.Inverse();				
		}
		else
		{
			Mesh.LocalToWorld = FMatrix::Identity;
			Mesh.WorldToLocal = FMatrix::Identity;				
		}

		Mesh.UseDynamicData			= TRUE;

		Mesh.ReverseCulling			= Proxy->LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow				= Proxy->bCastShadow;
		Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

		Mesh.BaseVertexIndex		= 0;
		Mesh.MaterialInstance		= FirstData->MaterialResource;
		Mesh.Type					= PT_TriangleList;				
		Mesh.ParticleType			= PET_Batch; 

		appMemzero(&DynamicMeshContext,sizeof(DynamicMeshContext));

		VertexType* Vertex = NULL;
		WORD* Index = NULL;
		INT NumAvailableVertices = 0;
		INT NumAvailableIndices = 0;

		for (DataType* Data = FirstData; Data; Data = (DataType*)(Data->Batch_Next))
		{				
			INT ParticleCount = Data->ActiveParticleCount;			
			Proxy = Data->SceneProxy;

			if (bShouldSort)
			{							
				Data->ParticleOrder.Empty(ParticleCount);

				// Take UseLocalSpace into account!
				UBOOL bLocalSpace = Data->bUseLocalSpace;

				for (INT ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
				{
					DECLARE_PARTICLE(Particle, Data->ParticleData + Data->ParticleStride * Data->ParticleIndices[ParticleIndex]);
					FLOAT InZ;
					if (bLocalSpace)
					{
						InZ = View->ViewProjectionMatrix.TransformFVector(Proxy->GetLocalToWorld().TransformFVector(Particle.Location)).Z;
					}
					else
					{
						InZ = View->ViewProjectionMatrix.TransformFVector(Particle.Location).Z;
					}
					new(Data->ParticleOrder)FParticleOrder(ParticleIndex, InZ);
				}
				Sort<USE_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents)>(&(Data->ParticleOrder(0)),Data->ParticleOrder.Num());					
			}

			INT NumVertices = ParticleCount * 4;
			INT NumIndices = ParticleCount * 6;

			if (NumAvailableIndices < NumIndices || NumAvailableVertices < NumVertices)
			{
				if (DynamicMeshContext.VertexBuffer != NULL)
				{
					CommitDynamicMesh( DynamicMeshContext );

					DynamicMeshContext.GenerateMeshElement( Mesh, *Data->VertexFactory );					

					DrawRichMesh(PDI, Mesh, FLinearColor::White, FLinearColor::White, FLinearColor::White, NULL, FALSE );		
					
					Mesh.bDrawnShared = TRUE;
				}				

				AllocDynamicMesh( DynamicMeshContext, sizeof(VertexType), NumVertices, sizeof(WORD), NumIndices, FALSE );
				NumAvailableVertices = DynamicMeshContext.MaxVertices;
				NumAvailableIndices = DynamicMeshContext.MaxIndices;
				Vertex = (VertexType*)DynamicMeshContext.VertexBuffer;
				Index = (WORD*)DynamicMeshContext.IndexBuffer;
			}			

			/* generate indices */
			NumAvailableVertices -= NumVertices;
			NumAvailableIndices -= NumIndices;				

			INT VertexOffset = DynamicMeshContext.NumVertices;
			Data->GetVertexAndIndexData( Vertex, Index, bShouldSort ? &Data->ParticleOrder : NULL, VertexOffset, bNeedsToTransform ? &Proxy->LocalToWorld : NULL );			

			Vertex += NumVertices;
			Index += NumIndices;

			DynamicMeshContext.NumVertices += NumVertices;
			DynamicMeshContext.NumIndices += NumIndices;								
		}

		if (DynamicMeshContext.VertexBuffer != NULL)
		{
			CommitDynamicMesh( DynamicMeshContext );

			DynamicMeshContext.GenerateMeshElement( Mesh, *FirstData->VertexFactory );					

			DrawRichMesh(PDI, Mesh, FLinearColor::White, FLinearColor::White, FLinearColor::White, NULL, FALSE );		
		}		
	}

	template <typename Policy>
	void BatchDraw_Sprites( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI,const FSceneView* View, UINT DPGIndex, typename Policy::DataType* FirstNode )
	{
		typedef Policy::DataType DataType;
		typedef Policy::VertexType VertexType;


		for (DataType* Itr = FirstNode; Itr; Itr = (DataType*)Itr->Batch_Sibling)
		{	
			if (PDI->IsMaterialIgnored(Itr->MaterialResource))
				continue;

			BatchDraw_Sprites_Inner<Policy>(Context, PDI, View, DPGIndex, Itr);
		}
	}
};

static FParticleSystemBatch GParticleSystemBatch;

template <>
void TParticleBatch<FDynamicSpriteEmitterData>::Draw( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex )
{
	GParticleSystemBatch.BatchDraw_Sprites<FSpritePolicy>( Context, PDI, View, DPGIndex, FirstNode );
}

template <>
void TParticleBatch<FDynamicSubUVEmitterData>::Draw( FCommandContextRHI* Context, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex )
{
	GParticleSystemBatch.BatchDraw_Sprites<FSubUVPolicy>( Context, PDI, View, DPGIndex, FirstNode );
}

void ParticleRendering_StartBatch( UBOOL bTranslucentPass )
{
	GParticleSystemBatch.Init( bTranslucentPass );
}

void ParticleRendering_SetBurstMode( UBOOL bBurstMode )
{
	GParticleSystemBatch.SetBurstMode( bBurstMode );
}

void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex )
{
	GParticleSystemBatch.Flush( NULL, PDI, View, DPGIndex );
	GParticleSystemBatch.Uninit();
}

void ParticleRendering_Add( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, FDynamicEmitterDataBase* Data )
{
	GParticleSystemBatch.Add( NULL, PDI, View, DPGIndex, Data );
}

void ParticleRendering_DrawMesh( FCommandContextRHI* Context )
{
	GParticleSystemBatch.DrawMesh( Context );
}

// FPrimitiveSceneProxy interface.
void FParticleSystemSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	if (View->Family->ShowFlags & SHOW_Particles)
	{
		// Determine the DPG the primitive should be drawn in for this view.
		BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

		if ((DPGIndex == PrimitiveDPG))
		{
			if (DynamicData != NULL)
			{				
				for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
				{
					FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
					if (Data == NULL)
					{
						continue;
					}

					Data->SceneProxy = this;
					Data->ApproximateDepthSortKey = ApproximateDepthSortKey;
					Data->bTranslucencyFence = bTranslucencyFence;

					ParticleRendering_Add( PDI, View, DPGIndex, Data );
				}				
			}
			DetermineLODDistance(View);
		}

#if !FINAL_RELEASE
		if ((DPGIndex == SDPG_Foreground) && (View->Family->ShowFlags & SHOW_Bounds) && (View->Family->ShowFlags & SHOW_Particles) && (GIsGame || !Owner || Owner->IsSelected()))
		{
			// Draw the static mesh's bounding box and sphere.
			DrawWireBox(PDI,PrimitiveSceneInfo->Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		}
#endif
	}
}

void FParticleSystemSceneProxy::DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const class FLightSceneInfo* Light)
{
	checkSlow(UEngine::ShadowVolumesAllowed());
}

/**
 *	Called when the rendering thread adds the proxy to the scene.
 *	This function allows for generating renderer-side resources.
 */
UBOOL FParticleSystemSceneProxy::CreateRenderThreadResources()
{
	// 
	if (DynamicData == NULL)
	{
		return FALSE;
	}

	for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
	{
		FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
		if (Data == NULL)
		{
			continue;
		}

		switch (Data->eEmitterType)
		{
		case FDynamicEmitterDataBase::DET_Sprite:
			{
				FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SpriteData->VertexFactory == NULL)
				{
					SpriteData->VertexFactory = new FParticleVertexFactory();
					check(SpriteData->VertexFactory);
					SpriteData->VertexFactory->Init();
				}
			}
			break;
		case FDynamicEmitterDataBase::DET_SubUV:
			{
				FDynamicSubUVEmitterData* SubUVData = (FDynamicSubUVEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SubUVData->VertexFactory == NULL)
				{
					SubUVData->VertexFactory = new FParticleSubUVVertexFactory();
					check(SubUVData->VertexFactory);
					SubUVData->VertexFactory->Init();
				}
			}
			break;
		case FDynamicEmitterDataBase::DET_Mesh:
			{
                FDynamicMeshEmitterData* MeshData = (FDynamicMeshEmitterData*)Data;
			}
			break;
		case FDynamicEmitterDataBase::DET_Beam2:
			{
				FDynamicBeam2EmitterData* BeamData = (FDynamicBeam2EmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (BeamData->VertexFactory == NULL)
				{
					BeamData->VertexFactory = new FParticleBeamTrailVertexFactory();
					check(BeamData->VertexFactory);
					BeamData->VertexFactory->Init();
				}
			}
			break;
		case FDynamicEmitterDataBase::DET_Trail2:
			{
				FDynamicTrail2EmitterData* TrailData = (FDynamicTrail2EmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (TrailData->VertexFactory == NULL)
				{
					TrailData->VertexFactory = new FParticleBeamTrailVertexFactory();
					check(TrailData->VertexFactory);
					TrailData->VertexFactory->Init();
				}
			}
			break;
		default:
			break;
		}
	}

	return TRUE;
}

/**
 *	Called when the rendering thread removes the dynamic data from the scene.
 */
UBOOL FParticleSystemSceneProxy::ReleaseRenderThreadResources()
{
	// 
	if (DynamicData == NULL)
	{
		return FALSE;
	}

	for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
	{
		FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
		if (Data == NULL)
		{
			continue;
		}

		switch (Data->eEmitterType)
		{
		case FDynamicEmitterDataBase::DET_Sprite:
			{
				FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SpriteData->VertexFactory)
				{
					SpriteData->VertexFactory->Release();
				}
			}
			break;
		case FDynamicEmitterDataBase::DET_SubUV:
			{
				FDynamicSubUVEmitterData* SubUVData = (FDynamicSubUVEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SubUVData->VertexFactory)
				{
					SubUVData->VertexFactory->Release();
				}
			}
			break;
		case FDynamicEmitterDataBase::DET_Mesh:
			{
				FDynamicMeshEmitterData* MeshData = (FDynamicMeshEmitterData*)Data;
			}
			break;
		case FDynamicEmitterDataBase::DET_Beam2:
			{
				FDynamicBeam2EmitterData* BeamData = (FDynamicBeam2EmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (BeamData->VertexFactory)
				{
					BeamData->VertexFactory->Release();
				}
			}
			break;
		case FDynamicEmitterDataBase::DET_Trail2:
			{
				FDynamicTrail2EmitterData* TrailData = (FDynamicTrail2EmitterData*)Data;
				if (TrailData->VertexFactory)
				{
					TrailData->VertexFactory->Release();
				}
			}
			break;
		default:
			break;
		}
	}

	return TRUE;
}

void FParticleSystemSceneProxy::UpdateData(FParticleDynamicData* NewDynamicData)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ParticleUpdateDataCommand,
		FParticleSystemSceneProxy*, Proxy, this,
		FParticleDynamicData*, NewDynamicData, NewDynamicData,
		{
			Proxy->UpdateData_RenderThread(NewDynamicData);
		}
		);
}

void FParticleSystemSceneProxy::UpdateData_RenderThread(FParticleDynamicData* NewDynamicData)
{
	ReleaseRenderThreadResources();
	if (DynamicData != NewDynamicData)
	{
		delete DynamicData;
	}
	DynamicData = NewDynamicData;
	CreateRenderThreadResources();
}

void FParticleSystemSceneProxy::DrawDynamicSpriteEmitter(FDynamicEmitterDataBase* Data, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)Data;

	if (SpriteData->EmitterRenderMode == ERM_Normal)
	{
		// Don't render if the material will be ignored
		if (PDI->IsMaterialIgnored(SpriteData->MaterialResource) && !(View->Family->ShowFlags & SHOW_Wireframe))
		{
			return;
		}

		SpriteData->VertexFactory->SetScreenAlignment(SpriteData->ScreenAlignment);
		SpriteData->VertexFactory->SetLockAxes(SpriteData->bLockAxis);
		if (SpriteData->bLockAxis)
		{
			FVector Up, Right;
			GetAxisLockValues((FDynamicSpriteEmitterDataBase*)SpriteData, Up, Right);
			SpriteData->VertexFactory->SetLockAxes(Up, Right);
		}

		INT ParticleCount = SpriteData->ActiveParticleCount;

		UBOOL bSorted = FALSE;
		// If material is using unlit translucency and the blend mode is translucent or 
		// if it is using unlit distortion then we need to sort (back to front)
		const FMaterial* Material = SpriteData->MaterialResource->GetMaterial();
		if (Material && 
			Material->GetLightingModel() == MLM_Unlit && 
			(Material->GetBlendMode() == BLEND_Translucent || Material->IsDistorted())
			)
		{
			SpriteData->ParticleOrder.Empty(ParticleCount);

			// Take UseLocalSpace into account!
			UBOOL bLocalSpace = SpriteData->bUseLocalSpace;

			for (INT ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
			{
				DECLARE_PARTICLE(Particle, SpriteData->ParticleData + SpriteData->ParticleStride * SpriteData->ParticleIndices[ParticleIndex]);
				FLOAT InZ;
				if (bLocalSpace)
				{
					InZ = View->ViewProjectionMatrix.TransformFVector(LocalToWorld.TransformFVector(Particle.Location)).Z;
				}
				else
				{
					InZ = View->ViewProjectionMatrix.TransformFVector(Particle.Location).Z;
				}
				new(SpriteData->ParticleOrder)FParticleOrder(ParticleIndex, InZ);
			}
			Sort<USE_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents)>(&(SpriteData->ParticleOrder(0)),SpriteData->ParticleOrder.Num());
			bSorted	= TRUE;
		}

		FMeshElement Mesh;
		
		Mesh.UseDynamicData			= TRUE;
		Mesh.IndexBuffer			= NULL;
		Mesh.VertexFactory			= SpriteData->VertexFactory;
		Mesh.DynamicVertexData		= SpriteData;
		Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertex);
		Mesh.DynamicIndexData		= NULL;
		Mesh.DynamicIndexStride		= 0;
		if (bSorted == TRUE)
		{
			Mesh.DynamicIndexData	= (void*)(&(SpriteData->ParticleOrder));
		}
		Mesh.LCI					= NULL;
		if (SpriteData->bUseLocalSpace == TRUE)
		{
			Mesh.LocalToWorld = LocalToWorld;
			Mesh.WorldToLocal = LocalToWorld.Inverse();			
		}
		else
		{
			Mesh.LocalToWorld = FMatrix::Identity;
			Mesh.WorldToLocal = FMatrix::Identity;			
		}
		Mesh.FirstIndex				= 0;
		Mesh.MinVertexIndex			= 0;
		Mesh.MaxVertexIndex			= SpriteData->VertexCount - 1;
		Mesh.ParticleType			= PET_Sprite;
		Mesh.ReverseCulling			= LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow				= bCastShadow;
		Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

		Mesh.MaterialInstance		= SpriteData->MaterialResource;
		Mesh.NumPrimitives			= ParticleCount;
		Mesh.Type					= PT_TriangleList;

		DrawRichMesh(
			PDI, 
			Mesh, 
			FLinearColor(1.0f, 0.0f, 0.0f),	//WireframeColor,
			FLinearColor(1.0f, 1.0f, 0.0f),	//LevelColor,
			FLinearColor(1.0f, 1.0f, 1.0f),	//PropertyColor,		
			PrimitiveSceneInfo,
			bSelected
		);
	}
	else
	if (SpriteData->EmitterRenderMode == ERM_Point)
	{
		SpriteData->RenderDebug(PDI, View, DPGIndex, FALSE);
	}
	else
    if (SpriteData->EmitterRenderMode == ERM_Cross)
	{
		SpriteData->RenderDebug(PDI, View, DPGIndex, TRUE);
	}
}

void FParticleSystemSceneProxy::DrawDynamicSubUVEmitter(FDynamicEmitterDataBase* Data, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	FDynamicSubUVEmitterData* SubUVData = (FDynamicSubUVEmitterData*)Data;

	if (SubUVData->EmitterRenderMode == ERM_Normal)
	{
		// Don't render if the material will be ignored
		if (PDI->IsMaterialIgnored(SubUVData->MaterialResource) && !(View->Family->ShowFlags & SHOW_Wireframe))
		{
			return;
		}

		SubUVData->VertexFactory->SetScreenAlignment(SubUVData->ScreenAlignment);
		SubUVData->VertexFactory->SetLockAxes(SubUVData->bLockAxis);
		if (SubUVData->bLockAxis)
		{
			FVector Up, Right;
			GetAxisLockValues((FDynamicSpriteEmitterDataBase*)SubUVData, Up, Right);
			SubUVData->VertexFactory->SetLockAxes(Up, Right);
		}

		INT ParticleCount = SubUVData->ActiveParticleCount;

		UBOOL bSorted = FALSE;
		// If material is using unlit translucency and the blend mode is translucent or 
		// if it is using unlit distortion then we need to sort (back to front)
		const FMaterial* Material = SubUVData->MaterialResource->GetMaterial();
		if (Material && 
			Material->GetLightingModel() == MLM_Unlit && 
			(Material->GetBlendMode() == BLEND_Translucent || Material->IsDistorted())
			)
		{
			SubUVData->ParticleOrder.Empty(ParticleCount);

			// Take UseLocalSpace into account!
			UBOOL bLocalSpace = SubUVData->bUseLocalSpace;

			for (INT ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
			{
				DECLARE_PARTICLE(Particle, SubUVData->ParticleData + SubUVData->ParticleStride * SubUVData->ParticleIndices[ParticleIndex]);
				FLOAT InZ;
				if (bLocalSpace)
				{
					InZ = View->ViewProjectionMatrix.TransformFVector(LocalToWorld.TransformFVector(Particle.Location)).Z;
				}
				else
				{
					InZ = View->ViewProjectionMatrix.TransformFVector(Particle.Location).Z;
				}
				new(SubUVData->ParticleOrder)FParticleOrder(ParticleIndex, InZ);
			}
			Sort<USE_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents)>(&(SubUVData->ParticleOrder(0)),SubUVData->ParticleOrder.Num());
			bSorted	= TRUE;
		}		

		FMeshElement Mesh;

		Mesh.UseDynamicData			= TRUE;
		Mesh.IndexBuffer			= NULL;
		Mesh.VertexFactory			= SubUVData->VertexFactory;
		Mesh.DynamicVertexData		= SubUVData;
		Mesh.DynamicVertexStride	= sizeof(FParticleSpriteSubUVVertex);
		Mesh.DynamicIndexData		= NULL;
		Mesh.DynamicIndexStride		= 0;
		if (bSorted == TRUE)
		{
			Mesh.DynamicIndexData	= (void*)(&(SubUVData->ParticleOrder));
		}
		Mesh.LCI					= NULL;
		if (SubUVData->bUseLocalSpace == TRUE)
		{
			Mesh.LocalToWorld = LocalToWorld;
			Mesh.WorldToLocal = LocalToWorld.Inverse();			
		}
		else
		{
			Mesh.LocalToWorld = FMatrix::Identity;
			Mesh.WorldToLocal = FMatrix::Identity;			
		}
		Mesh.FirstIndex				= 0;
		Mesh.MinVertexIndex			= 0;
		Mesh.MaxVertexIndex			= SubUVData->VertexCount - 1;
		Mesh.ParticleType			= PET_SubUV;
		Mesh.ReverseCulling			= LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow				= bCastShadow;
		Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

		Mesh.MaterialInstance		= SubUVData->MaterialResource;
		Mesh.NumPrimitives			= ParticleCount;
		Mesh.Type					= PT_TriangleList;
		
		DrawRichMesh(
			PDI, 
			Mesh, 
			FLinearColor(1.0f, 0.0f, 0.0f),	//WireframeColor,
			FLinearColor(1.0f, 1.0f, 0.0f),	//LevelColor,
			FLinearColor(1.0f, 1.0f, 1.0f),	//PropertyColor,		
			PrimitiveSceneInfo,
			bSelected
		);
	}
	else
	if (SubUVData->EmitterRenderMode == ERM_Point)
	{
		SubUVData->RenderDebug(PDI, View, DPGIndex, FALSE);
	}
	else
    if (SubUVData->EmitterRenderMode == ERM_Cross)
	{
		SubUVData->RenderDebug(PDI, View, DPGIndex, TRUE);
	}
}

void FParticleSystemSceneProxy::DrawDynamicMeshEmitter(FDynamicEmitterDataBase* Data, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	FDynamicMeshEmitterData* MeshData = (FDynamicMeshEmitterData*)Data;

	if (MeshData->EmitterRenderMode == ERM_Normal)
	{
		EParticleSubUVInterpMethod eSubUVMethod = (EParticleSubUVInterpMethod)(MeshData->SubUVInterpMethod);

		UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));

		FMatrix kMat(FMatrix::Identity);
		// Reset velocity and size.

		INT ParticleCount = MeshData->ActiveParticleCount;
		if ((MeshData->MaxDrawCount >= 0) && (ParticleCount > MeshData->MaxDrawCount))
		{
			ParticleCount = MeshData->MaxDrawCount;
		}

		for (INT i = ParticleCount - 1; i >= 0; i--)
		{
			const INT	CurrentIndex	= MeshData->ParticleIndices[i];
			const BYTE* ParticleBase	= MeshData->ParticleData + CurrentIndex * MeshData->ParticleStride;
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);

			if (Particle.RelativeTime < 1.0f)
			{
				FTranslationMatrix kTransMat(Particle.Location);
				FScaleMatrix kScaleMat(Particle.Size * Data->Scale);
				FRotator kRotator;

				if (MeshData->ScreenAlignment == PSA_TypeSpecific)
				{
					FVector	DirToCamera	= View->ViewOrigin - Particle.Location;
					DirToCamera.Normalize();
					if (DirToCamera.SizeSquared() <	0.5f)
					{
						// Assert possible if DirToCamera is not normalized
						DirToCamera	= FVector(1,0,0);
					}

					FVector	LocalSpaceFacingAxis = FVector(1,0,0); // facing axis is taken to be the local x axis.	
					FVector	LocalSpaceUpAxis = FVector(0,0,1); // up axis is taken to be the local z axis

					if (MeshData->MeshAlignment == PSMA_MeshFaceCameraWithLockedAxis)
					{
						// TODO: Allow an arbitrary	vector to serve	as the locked axis

						// For the locked axis behavior, only rotate to	face the camera	about the
						// locked direction, and maintain the up vector	pointing towards the locked	direction
						FVector	LockedAxis = MeshData->LockedAxis;

						// Find	the	rotation that points the localupaxis towards the targetupaxis
						FQuat PointToUp	= FQuatFindBetween(LocalSpaceUpAxis, LockedAxis);

						// Add in rotation about the TargetUpAxis to point the facing vector towards the camera
						FVector	DirToCameraInRotationPlane = DirToCamera - ((DirToCamera | LockedAxis)*LockedAxis);
						DirToCameraInRotationPlane.Normalize();
						FQuat PointToCamera	= FQuatFindBetween(PointToUp.RotateVector(LocalSpaceFacingAxis), DirToCameraInRotationPlane);

						// Set kRotator	to the composed	rotation
						FQuat MeshRotation = PointToCamera*PointToUp;
						kRotator = FRotator(MeshRotation);
					}
					else
					if (MeshData->MeshAlignment == PSMA_MeshFaceCameraWithSpin)
					{
						// Implement a tangent-rotation	version	of point-to-camera.	 The facing	direction points to	the	camera,
						// with	no roll, and has addtional sprite-particle rotation	about the tangential axis
						// (c.f. the roll rotation is about	the	radial axis)

						// Find	the	rotation that points the facing	axis towards the camera
						FRotator PointToRotation = FRotator(FQuatFindBetween(LocalSpaceFacingAxis, DirToCamera));

						// When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
						// otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
						PointToRotation.Roll = 0;

						// Add in the tangential rotation we do	want.
						FVector	vPositivePitch = FVector(0,0,1); //	this is	set	by the rotator's yaw/pitch/roll	reference frame
						FVector	vTangentAxis = vPositivePitch^DirToCamera;
						vTangentAxis.Normalize();
						if (vTangentAxis.SizeSquared() < 0.5f)
						{
							vTangentAxis = FVector(1,0,0); // assert is	possible if	FQuat axis/angle constructor is	passed zero-vector
						}

						FQuat AddedTangentialRotation =	FQuat(vTangentAxis,	Particle.Rotation);

						// Set kRotator	to the composed	rotation
						FQuat MeshRotation = AddedTangentialRotation*PointToRotation.Quaternion();
						kRotator = FRotator(MeshRotation);
					}
					else
	//				if (MeshTypeData->MeshAlignment == PSMA_MeshFaceCameraWithRoll)
					{
						// Implement a roll-rotation version of	point-to-camera.  The facing direction points to the camera,
						// with	no roll, and then rotates about	the	direction_to_camera	by the spriteparticle rotation.

						// Find	the	rotation that points the facing	axis towards the camera
						FRotator PointToRotation = FRotator(FQuatFindBetween(LocalSpaceFacingAxis, DirToCamera));

						// When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
						// otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
						PointToRotation.Roll = 0;

						// Add in the roll we do want.
						FQuat AddedRollRotation	= FQuat(DirToCamera, Particle.Rotation);

						// Set kRotator	to the composed	rotation
						FQuat MeshRotation = AddedRollRotation*PointToRotation.Quaternion();
						kRotator = FRotator(MeshRotation);
					}
				}
				else
				if (MeshData->bMeshRotationActive)
				{
					FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshData->MeshRotationOffset);
					kRotator = FRotator::MakeFromEuler(PayloadData->Rotation);
				}
				else
				{
					FLOAT fRot = Particle.Rotation * 180.0f / PI;
					FVector kRotVec = FVector(fRot, fRot, fRot);
					kRotator = FRotator::MakeFromEuler(kRotVec);
				}
				
				FRotationMatrix kRotMat(kRotator);
				kMat = kScaleMat * kRotMat * kTransMat;
				if (MeshData->bUseLocalSpace)
				{
					kMat *= LocalToWorld;
				}

				UBOOL bBadParent = FALSE;

				TArray<FMeshEmitterMaterialInstanceResource*> MEMatInstRes;
				//@todo. Handle LODs...
	//			for (INT LODIndex = 0; LODIndex < MeshData->LODs.Num(); LODIndex++)
				for (INT LODIndex = 0; LODIndex < 1; LODIndex++)
				{
					FDynamicMeshEmitterData::FLODInfo* LODInfo = &(MeshData->LODs(LODIndex));

					MEMatInstRes.Empty();
					for (INT ElementIndex = 0; ElementIndex < LODInfo->Elements.Num(); ElementIndex++)
					{
						INT MIIndex = MEMatInstRes.Add();
						check(MIIndex >= 0);
						FMeshEmitterMaterialInstanceResource* MIRes = ::new FMeshEmitterMaterialInstanceResource();
						check(MIRes);
						MEMatInstRes(MIIndex) = MIRes;
						UMaterialInstanceConstant* MatInstConst = Cast<UMaterialInstanceConstant>(LODInfo->Elements(ElementIndex).Material);
						if (MatInstConst)
						{
							MIRes->Parent = MatInstConst->GetInstanceInterface(FALSE);
							MIRes->Param_MeshEmitterVertexColor = Particle.Color;
							if (MeshData->SubUVInterpMethod != PSUVIM_None)
							{
								FSubUVMeshPayload* SubUVPayload = NULL;

								if ((MeshData->SubUVInterpMethod == PSUVIM_Random) || (MeshData->SubUVInterpMethod == PSUVIM_Random_Blend))
								{
									FSubUVMeshRandomPayload* TempPayload = (FSubUVMeshRandomPayload*)(((BYTE*)&Particle) + MeshData->SubUVDataOffset);
									SubUVPayload	= (FSubUVMeshPayload*)(TempPayload);
								}
								else
								{
									SubUVPayload	= (FSubUVMeshPayload*)(((BYTE*)&Particle) + MeshData->SubUVDataOffset);
								}

								MIRes->Param_TextureOffsetParameter = 
									FLinearColor(SubUVPayload->UVOffset.X, SubUVPayload->UVOffset.Y, 0.0f, 0.0f);

								if (MeshData->bScaleUV)
								{
									MIRes->Param_TextureScaleParameter = 
										FLinearColor((1.0f / (FLOAT)MeshData->SubImages_Horizontal),
													(1.0f / (FLOAT)MeshData->SubImages_Vertical),
													0.0f, 0.0f);
								}
								else
								{
									MIRes->Param_TextureScaleParameter = 
										FLinearColor(1.0f, 1.0f, 0.0f, 0.0f);
								}
							}
						}
						else
						{
							UMaterial* Material = Cast<UMaterial>(LODInfo->Elements(ElementIndex).Material);
							if (Material)
							{
								MIRes->Parent = Material->GetInstanceInterface(FALSE);
							}
						}

						if (MIRes->Parent == NULL)
						{
							bBadParent = TRUE;
						}
					}
				}

				if (bBadParent == TRUE)
				{
					continue;
				}
				
				// Fill in the MeshElement and draw...
				const FStaticMeshRenderData& LODModel = MeshData->StaticMesh->LODModels(0);
				FDynamicMeshEmitterData::FLODInfo* LODInfo = &(MeshData->LODs(0));

				// Draw the static mesh elements.
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					FMeshEmitterMaterialInstanceResource* MIRes = MEMatInstRes(ElementIndex);
					const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);
					FMeshElement Mesh;
					Mesh.VertexFactory = &LODModel.VertexFactory;
					Mesh.DynamicVertexData = NULL;
					Mesh.LCI = NULL;

					Mesh.LocalToWorld = kMat;
					Mesh.WorldToLocal = kMat.Inverse();					

					Mesh.FirstIndex = Element.FirstIndex;
					Mesh.MinVertexIndex = Element.MinVertexIndex;
					Mesh.MaxVertexIndex = Element.MaxVertexIndex;
					Mesh.UseDynamicData = FALSE;
					Mesh.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
					Mesh.CastShadow = bCastShadow;
					Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)DPGIndex;
					if( bWireframe && LODModel.WireframeIndexBuffer.IsInitialized() )
					{
						Mesh.IndexBuffer = &LODModel.WireframeIndexBuffer;
						Mesh.MaterialInstance = &DeselectedWireframeMaterialInstance;
						Mesh.Type = PT_LineList;
						Mesh.NumPrimitives = LODModel.WireframeIndexBuffer.Indices.Num() / 2;
					}
					else
					{
						Mesh.bWireframe = bWireframe;
						Mesh.IndexBuffer = &LODModel.IndexBuffer;
						Mesh.MaterialInstance = MIRes;
						Mesh.Type = PT_TriangleList;
						Mesh.NumPrimitives = Element.NumTriangles;
					}
					PDI->DrawMesh(Mesh);
				}

				for (INT MEMIRIndex = 0; MEMIRIndex < MEMatInstRes.Num(); MEMIRIndex++)
				{
					FMeshEmitterMaterialInstanceResource* Res = MEMatInstRes(MEMIRIndex);
					if (Res)
					{
						delete Res;
						MEMatInstRes(MEMIRIndex) = NULL;
					}
				}
				MEMatInstRes.Empty();
			}
			else
			{
				// Remove it from the scene???
			}
		}
	}
	else
	if (MeshData->EmitterRenderMode == ERM_Point)
	{
		MeshData->RenderDebug(PDI, View, DPGIndex, FALSE);
	}
	else
    if (MeshData->EmitterRenderMode == ERM_Cross)
	{
		MeshData->RenderDebug(PDI, View, DPGIndex, TRUE);
	}
}

void FParticleSystemSceneProxy::DrawDynamicBeam2Emitter(FDynamicEmitterDataBase* Data, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	FDynamicBeam2EmitterData* BeamData = (FDynamicBeam2EmitterData*)Data;

	BeamData->VertexFactory->SetScreenAlignment(BeamData->ScreenAlignment);
	BeamData->VertexFactory->SetLockAxes(BeamData->bLockAxis);
	if (BeamData->bLockAxis)
	{
		FVector Up, Right;
		GetAxisLockValues((FDynamicSpriteEmitterDataBase*)BeamData, Up, Right);
		BeamData->VertexFactory->SetLockAxes(Up, Right);
	}

	// Allocate and generate the data...
	// Determine the required particle count
	if (BeamData->VertexData == NULL)
	{
		BeamData->VertexData = (FParticleSpriteVertex*)appRealloc(
			BeamData->VertexData, BeamData->VertexCount * sizeof(FParticleSpriteVertex));
		check(BeamData->VertexData);
	}

	INT	TrianglesToRender;

	INT	TrianglesToRender_Index = FillBeam2EmitterIndexData(BeamData, PDI, View, DPGIndex);
//	if (BeamData->bSmoothNoise_Enabled || BeamData->bLowFreqNoise_Enabled || BeamData->bHighFreqNoise_Enabled)
	if (BeamData->bLowFreqNoise_Enabled)
	{
		TrianglesToRender = FillBeam2EmitterVertexData_Noise(BeamData, PDI, View, DPGIndex);
	}
	else
	{
		TrianglesToRender = FillBeam2EmitterVertexData_NoNoise(BeamData, PDI, View, DPGIndex);
	}
	TrianglesToRender = TrianglesToRender_Index;

	if (TrianglesToRender > 0)
	{
		FMeshElement Mesh;

		Mesh.IndexBuffer			= NULL;
		Mesh.VertexFactory			= BeamData->VertexFactory;
		Mesh.DynamicVertexData		= BeamData->VertexData;
		Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertex);
		Mesh.DynamicIndexData		= BeamData->IndexData;
		Mesh.DynamicIndexStride		= BeamData->IndexStride;
		Mesh.LCI					= NULL;
		if (BeamData->bUseLocalSpace == TRUE)
		{
			Mesh.LocalToWorld = LocalToWorld;
			Mesh.WorldToLocal = LocalToWorld.Inverse();			
		}
		else
		{
			Mesh.LocalToWorld = FMatrix::Identity;
			Mesh.WorldToLocal = FMatrix::Identity;			
		}
		Mesh.FirstIndex				= 0;
		if ((TrianglesToRender % 2) != 0)
		{
			TrianglesToRender--;
		}
		Mesh.NumPrimitives			= TrianglesToRender;
		Mesh.MinVertexIndex			= 0;
		Mesh.MaxVertexIndex			= BeamData->VertexCount - 1;
		Mesh.UseDynamicData			= TRUE;
		Mesh.ReverseCulling			= LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow				= bCastShadow;
		Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

//		if ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials))
		if (0)
		{
			Mesh.MaterialInstance		= &DeselectedWireframeMaterialInstance;
			Mesh.Type					= PT_LineList;
		}
		else
		{
			Mesh.MaterialInstance		= BeamData->MaterialResource;
			Mesh.Type					= PT_TriangleStrip;
		}

//		PDI->DrawMesh(Mesh);
		DrawRichMesh(
			PDI,
			Mesh,
			FLinearColor(1.0f, 0.0f, 0.0f),
			FLinearColor(1.0f, 1.0f, 0.0f),
			FLinearColor(1.0f, 1.0f, 1.0f),
			PrimitiveSceneInfo,
			bSelected
			);
 	}
}

void FParticleSystemSceneProxy::DrawDynamicTrail2Emitter(FDynamicEmitterDataBase* Data, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	if (Data == NULL)
	{
		return;
	}

	check(PDI);
	check(Data->eEmitterType == FDynamicEmitterDataBase::DET_Trail2);
	FDynamicTrail2EmitterData* TrailData = (FDynamicTrail2EmitterData*)Data;

	if ((TrailData->VertexCount <= 0) || 
		(TrailData->ActiveParticleCount <= 0) ||
		(TrailData->IndexCount < 3))
	{
		return;
	}

	TrailData->VertexFactory->SetScreenAlignment(TrailData->ScreenAlignment);
	TrailData->VertexFactory->SetLockAxes(TrailData->bLockAxis);
	if (TrailData->bLockAxis)
	{
		FVector Up, Right;
		GetAxisLockValues((FDynamicSpriteEmitterDataBase*)TrailData, Up, Right);
		TrailData->VertexFactory->SetLockAxes(Up, Right);
	}

	INT	TessFactor	= TrailData->TessFactor;
	INT	Sheets		= TrailData->Sheets;

	// Allocate and generate the data...
	// Determine the required particle count
	INT	TrianglesToRender	= 0;

	if (TrailData->VertexData == NULL)
	{
		TrailData->VertexData = (FParticleSpriteVertex*)appRealloc(
			TrailData->VertexData, TrailData->VertexCount * sizeof(FParticleSpriteVertex));
	}
	check(TrailData->VertexData);

	INT TriCountIndex = FillTrail2EmitterIndexData(TrailData, PDI, View, DPGIndex);
	INT TriCountVertex = FillTrail2EmitterVertexData(TrailData, PDI, View, DPGIndex);

	UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));

	FMeshElement Mesh;
	
	Mesh.IndexBuffer			= NULL;
	Mesh.VertexFactory			= TrailData->VertexFactory;
	Mesh.DynamicVertexData		= TrailData->VertexData;
	Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertex);
	Mesh.DynamicIndexData		= TrailData->IndexData;
	Mesh.DynamicIndexStride		= TrailData->IndexStride;
	Mesh.LCI					= NULL;
	if (TrailData->bUseLocalSpace == TRUE)
	{
		Mesh.LocalToWorld = LocalToWorld;
		Mesh.WorldToLocal = LocalToWorld.Inverse();		
	}
	else
	{
		Mesh.LocalToWorld = FMatrix::Identity;
		Mesh.WorldToLocal = FMatrix::Identity;		
	}
	Mesh.FirstIndex				= 0;
	Mesh.NumPrimitives			= TriCountIndex;
	Mesh.MinVertexIndex			= 0;
	Mesh.MaxVertexIndex			= TrailData->VertexCount - 1;
	Mesh.UseDynamicData			= TRUE;
	Mesh.ReverseCulling			= LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
	Mesh.CastShadow				= bCastShadow;
	Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

	if (bWireframe)
	{
		Mesh.MaterialInstance	= &DeselectedWireframeMaterialInstance;
		Mesh.Type				= PT_LineList;
	}
	else
	{
		check(TriCountIndex == TrailData->PrimitiveCount);

		Mesh.MaterialInstance	= TrailData->MaterialResource;
		Mesh.Type				= PT_TriangleStrip;
	}

	PDI->DrawMesh(Mesh);
}

void FParticleSystemSceneProxy::DetermineLODDistance(const FSceneView* View)
{
	INT	LODIndex = -1;

	if (LODMethod == PARTICLESYSTEMLODMETHOD_Automatic)
	{
		// Default to the highest LOD level
		FVector	CameraPosition	= View->ViewOrigin;
		FVector	CompPosition	= LocalToWorld.GetOrigin();
		FVector	DistDiff		= CompPosition - CameraPosition;
		FLOAT	Distance		= DistDiff.Size() * View->LODDistanceFactor;
		PendingLODDistance = Distance;
	}
}

INT FParticleSystemSceneProxy::FillBeam2EmitterIndexData(FDynamicBeam2EmitterData* BeamData, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT	TrianglesToRender = 0;

	SCOPE_CYCLE_COUNTER(STAT_BeamIBPackingTime);

	// Beam2 polygons are packed and joined as follows:
	//
	// 1--3--5--7--9-...
	// |\ |\ |\ |\ |\...
	// | \| \| \| \| ...
	// 0--2--4--6--8-...
	//
	// (ie, the 'leading' edge of polygon (n) is the trailing edge of polygon (n+1)
	//
	// NOTE: This is primed for moving to tri-strips...
	//
	INT Sheets		= BeamData->Sheets ? BeamData->Sheets : 1;
	INT TessFactor	= BeamData->InterpolationPoints ? BeamData->InterpolationPoints : 1;

//	UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));
	UBOOL bWireframe = FALSE;

	if (BeamData->IndexData == NULL)
	{
		BeamData->IndexCount = 0;
		for (INT ii = 0; ii < BeamData->TrianglesPerSheet.Num(); ii++)
		{
			INT Triangles = BeamData->TrianglesPerSheet(ii);
			if (bWireframe)
			{
				BeamData->IndexCount += (8 * Triangles + 2) * Sheets;
			}
			else
			{
				if (BeamData->IndexCount == 0)
				{
					BeamData->IndexCount = 2;
				}
				BeamData->IndexCount += Triangles * Sheets;
				BeamData->IndexCount += 4 * (Sheets - 1);	// Degenerate indices between sheets
				if ((ii + 1) < BeamData->TrianglesPerSheet.Num())
				{
					BeamData->IndexCount += 4;	// Degenerate indices between beams
				}
			}
		}
		BeamData->IndexData = appRealloc(BeamData->IndexData, BeamData->IndexCount * BeamData->IndexStride);
		check(BeamData->IndexData);
	}

	if (BeamData->IndexStride == sizeof(WORD))
	{
		WORD*	Index				= (WORD*)BeamData->IndexData;
		WORD	VertexIndex			= 0;
		WORD	StartVertexIndex	= 0;

		for (INT Beam = 0; Beam < BeamData->ActiveParticleCount; Beam++)
		{
			DECLARE_PARTICLE_PTR(Particle, BeamData->ParticleData + BeamData->ParticleStride * Beam);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + BeamData->BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}

			if (BeamData->InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + BeamData->InterpolatedPointsOffset);
			}
			if (BeamData->NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + BeamData->NoiseRateOffset);
			}
			if (BeamData->NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + BeamData->NoiseDeltaTimeOffset);
			}
			if (BeamData->TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + BeamData->TargetNoisePointsOffset);
			}
			if (BeamData->NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + BeamData->NextNoisePointsOffset);
			}
			if (BeamData->TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + BeamData->TaperValuesOffset);
			}

			if (bWireframe)
			{
				for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
				{
					VertexIndex = 0;

					// The 'starting' line
					TrianglesToRender += 1;
					*(Index++) = StartVertexIndex + 0;
					*(Index++) = StartVertexIndex + 1;

					// 4 lines per quad
					INT TriCount = BeamData->TrianglesPerSheet(Beam);
					INT QuadCount = TriCount / 2;
					TrianglesToRender += TriCount * 2;

					for (INT i = 0; i < QuadCount; i++)
					{
						*(Index++) = StartVertexIndex + VertexIndex + 0;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 1;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 1;
						*(Index++) = StartVertexIndex + VertexIndex + 3;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 3;

						VertexIndex += 2;
					}

					StartVertexIndex += TriCount + 2;
				}
			}
			else
			{
				// 
				if (Beam == 0)
				{
					*(Index++) = VertexIndex++;	// SheetIndex + 0
					*(Index++) = VertexIndex++;	// SheetIndex + 1
				}

				for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
				{
					// 2 triangles per tessellation factor
					TrianglesToRender += BeamPayloadData->TriangleCount;

					// Sequentially step through each triangle - 1 vertex per triangle
					for (INT i = 0; i < BeamPayloadData->TriangleCount; i++)
					{
						*(Index++) = VertexIndex++;
					}

					// Degenerate tris
					if ((SheetIndex + 1) < Sheets)
					{
						*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
						*(Index++) = VertexIndex;		// First vertex of the next sheet
						*(Index++) = VertexIndex++;		// First vertex of the next sheet
						*(Index++) = VertexIndex++;		// Second vertex of the next sheet

						TrianglesToRender += 4;
					}
				}
				if ((Beam + 1) < BeamData->ActiveParticleCount)
				{
					*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
					*(Index++) = VertexIndex;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// Second vertex of the next sheet

					TrianglesToRender += 4;
				}
			}
		}
	}
	else
	{
		check(!TEXT("Rendering beam with > 5000 vertices!"));
		DWORD*	Index		= (DWORD*)BeamData->IndexData;
		DWORD	VertexIndex	= 0;
		for (INT Beam = 0; Beam < BeamData->ActiveParticleCount; Beam++)
		{
			DECLARE_PARTICLE_PTR(Particle, BeamData->ParticleData + BeamData->ParticleStride * Beam);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + BeamData->BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}
			if (BeamData->InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + BeamData->InterpolatedPointsOffset);
			}
			if (BeamData->NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + BeamData->NoiseRateOffset);
			}
			if (BeamData->NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + BeamData->NoiseDeltaTimeOffset);
			}
			if (BeamData->TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + BeamData->TargetNoisePointsOffset);
			}
			if (BeamData->NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + BeamData->NextNoisePointsOffset);
			}
			if (BeamData->TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + BeamData->TaperValuesOffset);
			}

			// 
			if (Beam == 0)
			{
				*(Index++) = VertexIndex++;	// SheetIndex + 0
				*(Index++) = VertexIndex++;	// SheetIndex + 1
			}

			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
			{
				// 2 triangles per tessellation factor
				TrianglesToRender += BeamPayloadData->TriangleCount;

				// Sequentially step through each triangle - 1 vertex per triangle
				for (INT i = 0; i < BeamPayloadData->TriangleCount; i++)
				{
					*(Index++) = VertexIndex++;
				}

				// Degenerate tris
				if ((SheetIndex + 1) < Sheets)
				{
					*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
					*(Index++) = VertexIndex;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// Second vertex of the next sheet
					TrianglesToRender += 4;
				}
			}
			if ((Beam + 1) < BeamData->ActiveParticleCount)
			{
				*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
				*(Index++) = VertexIndex;		// First vertex of the next sheet
				*(Index++) = VertexIndex++;		// First vertex of the next sheet
				*(Index++) = VertexIndex++;		// Second vertex of the next sheet
				TrianglesToRender += 4;
			}
		}
	}

	return TrianglesToRender;
}

INT FParticleSystemSceneProxy::FillBeam2EmitterVertexData_NoNoise(FDynamicBeam2EmitterData* BeamData, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT	TrianglesToRender = 0;

	FParticleSpriteVertex*			Vertex			= (FParticleSpriteVertex*)BeamData->VertexData;
	const INT						ScreenAlignment	= BeamData->ScreenAlignment;
	FMatrix							CameraToWorld	= View->ViewMatrix.Inverse();
	INT								Sheets			= BeamData->Sheets ? BeamData->Sheets : 1;
	INT								TessFactor		= BeamData->InterpolationPoints ? BeamData->InterpolationPoints : 1;

	FVector	ViewOrigin	= CameraToWorld.GetOrigin();

	if (TessFactor <= 1)
	{
		FVector	Offset, LastOffset;
		FVector	Size;

		for (INT i = 0; i < BeamData->ActiveParticleCount; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, BeamData->ParticleData + BeamData->ParticleStride * i);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + BeamData->BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}
			if (BeamData->InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + BeamData->InterpolatedPointsOffset);
			}
			if (BeamData->NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + BeamData->NoiseRateOffset);
			}
			if (BeamData->NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + BeamData->NoiseDeltaTimeOffset);
			}
			if (BeamData->TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + BeamData->TargetNoisePointsOffset);
			}
			if (BeamData->NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + BeamData->NextNoisePointsOffset);
			}
			if (BeamData->TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + BeamData->TaperValuesOffset);
			}

			// Pin the size to the X component
			Size	= FVector(Particle->Size.X * BeamData->Scale.X);

			FVector EndPoint	= Particle->Location;
			FVector Location	= BeamPayloadData->SourcePoint;
			FVector Right		= Location - EndPoint;
			Right.Normalize();
			FVector Up			= Right ^  (Location - ViewOrigin);
			Up.Normalize();
			FVector WorkingUp	= Up;

			FLOAT	fUEnd;
			FLOAT	Tiles		= 1.0f;
			if (BeamData->TextureTileDistance > KINDA_SMALL_NUMBER)
			{
				FVector	Direction	= BeamPayloadData->TargetPoint - BeamPayloadData->SourcePoint;
				FLOAT	Distance	= Direction.Size();
				Tiles				= Distance / BeamData->TextureTileDistance;
			}
			fUEnd		= Tiles;

			if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
			{
				fUEnd	= Tiles * BeamPayloadData->TravelRatio;
			}

			// For the direct case, this isn't a big deal, as it will not require much work per sheet.
			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
			{
				if (SheetIndex)
				{
					FLOAT	Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
					FQuat	QuatRotator	= FQuat(Right, Angle);
					WorkingUp			= QuatRotator.RotateVector(Up);
				}

				FLOAT	Taper	= 1.0f;

				if (BeamData->TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				Offset.X		= WorkingUp.X * Size.X * Taper;
				Offset.Y		= WorkingUp.Y * Size.Y * Taper;
				Offset.Z		= WorkingUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= 0.0f;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;

				Vertex->Position	= Location - Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= 0.0f;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;

				if (BeamData->TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[1];
				}

				Offset.X		= WorkingUp.X * Size.X * Taper;
				Offset.Y		= WorkingUp.Y * Size.Y * Taper;
				Offset.Z		= WorkingUp.Z * Size.Z * Taper;

				//
				Vertex->Position	= EndPoint + Offset;
				Vertex->OldPosition	= Particle->OldLocation;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fUEnd;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;

				Vertex->Position	= EndPoint - Offset;
				Vertex->OldPosition	= Particle->OldLocation;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fUEnd;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
			}
		}
	}
	else
	{
		FVector	Offset;
		FVector	Size;

		FLOAT	fTextureIncrement	= 1.0f / BeamData->InterpolationPoints;;

		for (INT i = 0; i < BeamData->ActiveParticleCount; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, BeamData->ParticleData + BeamData->ParticleStride * i);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + BeamData->BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}
			if (BeamData->InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + BeamData->InterpolatedPointsOffset);
			}
			if (BeamData->NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + BeamData->NoiseRateOffset);
			}
			if (BeamData->NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + BeamData->NoiseDeltaTimeOffset);
			}
			if (BeamData->TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + BeamData->TargetNoisePointsOffset);
			}
			if (BeamData->NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + BeamData->NextNoisePointsOffset);
			}
			if (BeamData->TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + BeamData->TaperValuesOffset);
			}

			if (BeamData->TextureTileDistance > KINDA_SMALL_NUMBER)
			{
				FVector	Direction	= BeamPayloadData->TargetPoint - BeamPayloadData->SourcePoint;
				FLOAT	Distance	= Direction.Size();
				FLOAT	Tiles		= Distance / BeamData->TextureTileDistance;
				fTextureIncrement	= Tiles / BeamData->InterpolationPoints;
			}

			// Pin the size to the X component
			Size	= FVector(Particle->Size.X * BeamData->Scale.X);

			FLOAT	Angle;
			FQuat	QuatRotator(0, 0, 0, 0);

			FVector Location;
			FVector EndPoint;
			FVector Right;
			FVector Up;
			FVector WorkingUp;
			FLOAT	fU;

			check(InterpolatedPoints);	// TTP #33139
			// For the direct case, this isn't a big deal, as it will not require much work per sheet.
			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
			{
				fU			= 0.0f;
				Location	= BeamPayloadData->SourcePoint;
				EndPoint	= InterpolatedPoints[0];
				Right		= Location - EndPoint;
				Right.Normalize();
				Up			= Right ^  (Location - ViewOrigin);
				Up.Normalize();

				if (SheetIndex)
				{
					Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
					QuatRotator	= FQuat(Right, Angle);

					WorkingUp	= QuatRotator.RotateVector(Up);
				}
				else
				{
					WorkingUp	= Up;
				}

				FLOAT	Taper	= 1.0f;

				if (BeamData->TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				Offset.X	= WorkingUp.X * Size.X * Taper;
				Offset.Y	= WorkingUp.Y * Size.Y * Taper;
				Offset.Z	= WorkingUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;

				Vertex->Position	= Location - Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;

				for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
				{
					EndPoint	= InterpolatedPoints[StepIndex];
					Up			= Right ^  (Location - ViewOrigin);
					Up.Normalize();
					if (SheetIndex)
					{
						WorkingUp		= QuatRotator.RotateVector(Up);
					}
					else
					{
						WorkingUp	= Up;
					}

					if (BeamData->TaperMethod != PEBTM_None)
					{
						check(TaperValues);
						Taper	= TaperValues[StepIndex + 1];
					}

					Offset.X		= WorkingUp.X * Size.X * Taper;
					Offset.Y		= WorkingUp.Y * Size.Y * Taper;
					Offset.Z		= WorkingUp.Z * Size.Z * Taper;

					//
					Vertex->Position	= EndPoint + Offset;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU + fTextureIncrement;
					Vertex->Tex_V		= 0.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;

					Vertex->Position	= EndPoint - Offset;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU + fTextureIncrement;
					Vertex->Tex_V		= 1.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;

					Location			 = EndPoint;
					fU					+= fTextureIncrement;
				}

				if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
				{
/***
					check(BEAM2_TYPEDATA_LOCKED(BeamPayloadData->Lock_Max_NumNoisePoints) == 0);

					if (BeamPayloadData->Steps < TessFactor)
					{
						if (BeamData->TaperMethod != PEBTM_None)
						{
							FLOAT	TargetTaper	= TaperValues[BeamPayloadData->Steps];
							Taper	= (Taper + TargetTaper) / 2.0f;
						}

						// This is jsut a partial line along the beam
						EndPoint	= Location + (InterpolatedPoints[BeamPayloadData->Steps] - Location) * BeamPayloadData->TravelRatio;
						Up			= Right ^  (EndPoint - ViewOrigin);
						Up.Normalize();
						if (SheetIndex)
						{
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						//
						Vertex->Position	= EndPoint + Offset;
						Vertex->OldPosition	= EndPoint;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU + fTextureIncrement * BeamPayloadData->TravelRatio;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;

						Vertex->Position	= EndPoint - Offset;
						Vertex->OldPosition	= EndPoint;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU + fTextureIncrement * BeamPayloadData->TravelRatio;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
					}
					else
					{
						// Most likely, have gone past the target
						//@todo. How to handle this case...
					}
***/
				}
			}
		}
	}

	return TrianglesToRender;
}

INT FParticleSystemSceneProxy::FillBeam2EmitterVertexData_Noise(FDynamicBeam2EmitterData* BeamData, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT	TrianglesToRender = 0;

	if (BeamData->InterpolationPoints > 0)
	{
		return FillBeam2EmitterVertexData_NoNoise(BeamData, PDI, View, DPGIndex);
	}

	FParticleSpriteVertex*			Vertex			= (FParticleSpriteVertex*)BeamData->VertexData;
	const INT						ScreenAlignment	= BeamData->ScreenAlignment;
	FMatrix							CameraToWorld	= View->ViewMatrix.Inverse();
	INT								Sheets			= BeamData->Sheets ? BeamData->Sheets : 1;

	FVector	ViewOrigin	= CameraToWorld.GetOrigin();

	// Frequency is the number of noise points to generate, evenly distributed along the line.
	INT	Frequency	= BeamData->Frequency ? BeamData->Frequency : 1;
	// NoiseTessellation is the amount of tessellation that should occur between noise points.
	INT	TessFactor	= BeamData->NoiseTessellation ? BeamData->NoiseTessellation : 1;
	
	FLOAT	InvTessFactor	= 1.0f / TessFactor;
	FColor	Color(255,0,0);
	INT		i;

	// The last position processed
	FVector	LastPosition, LastDrawPosition, LastTangent;
	// The current position
	FVector	CurrPosition, CurrDrawPosition;
	// The target
	FVector	TargetPosition, TargetDrawPosition;
	// The next target
	FVector	NextTargetPosition, NextTargetDrawPosition, TargetTangent;
	// The interperted draw position
	FVector InterpDrawPos;
	FVector	InterimDrawPosition;

	FVector	Size;

	FLOAT	Angle;
	FQuat	QuatRotator;

	FVector Location;
	FVector EndPoint;
	FVector Right;
	FVector Up;
	FVector WorkingUp;
	FVector LastUp;
	FVector WorkingLastUp;
	FVector Offset;
	FVector LastOffset;
	FLOAT	fStrength;
	FLOAT	fTargetStrength;

	FLOAT	fU;
	FLOAT	TextureIncrement	= 1.0f / (((BeamData->Frequency > 0) ? BeamData->Frequency : 1) * TessFactor);	// TTP #33140/33159

	INT	 VertexCount	= 0;

	// Tessellate the beam along the noise points
	for (i = 0; i < BeamData->ActiveParticleCount; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, BeamData->ParticleData + BeamData->ParticleStride * i);

		// Retrieve the beam data from the particle.
		FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
		FVector*				InterpolatedPoints	= NULL;
		FLOAT*					NoiseRate			= NULL;
		FLOAT*					NoiseDelta			= NULL;
		FVector*				TargetNoisePoints	= NULL;
		FVector*				NextNoisePoints		= NULL;
		FLOAT*					TaperValues			= NULL;

		BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + BeamData->BeamDataOffset);
		if (BeamPayloadData->TriangleCount == 0)
		{
			continue;
		}
		if (BeamData->InterpolatedPointsOffset != -1)
		{
			InterpolatedPoints = (FVector*)((BYTE*)Particle + BeamData->InterpolatedPointsOffset);
		}
		if (BeamData->NoiseRateOffset != -1)
		{
			NoiseRate = (FLOAT*)((BYTE*)Particle + BeamData->NoiseRateOffset);
		}
		if (BeamData->NoiseDeltaTimeOffset != -1)
		{
			NoiseDelta = (FLOAT*)((BYTE*)Particle + BeamData->NoiseDeltaTimeOffset);
		}
		if (BeamData->TargetNoisePointsOffset != -1)
		{
			TargetNoisePoints = (FVector*)((BYTE*)Particle + BeamData->TargetNoisePointsOffset);
		}
		if (BeamData->NextNoisePointsOffset != -1)
		{
			NextNoisePoints = (FVector*)((BYTE*)Particle + BeamData->NextNoisePointsOffset);
		}
		if (BeamData->TaperValuesOffset != -1)
		{
			TaperValues = (FLOAT*)((BYTE*)Particle + BeamData->TaperValuesOffset);
		}

		FVector* NoisePoints	= TargetNoisePoints;
		FVector* NextNoise		= NextNoisePoints;

		FLOAT NoiseRangeScaleFactor = BeamData->NoiseRangeScale;
		//@todo. How to handle no noise points?
		// If there are no noise points, why are we in here?
		if (NoisePoints == NULL)
		{
			continue;
		}

		// Pin the size to the X component
		Size	= FVector(Particle->Size.X * BeamData->Scale.X);

		if (TessFactor <= 1)
		{
		}
		else
		{
			// Setup the current position as the source point
			CurrPosition		= BeamPayloadData->SourcePoint;
			CurrDrawPosition	= CurrPosition;

			// Setup the source tangent & strength
			if (BeamData->bUseSource)
			{
				// The source module will have determined the proper source tangent.
				LastTangent	= BeamPayloadData->SourceTangent;
				fStrength	= BeamPayloadData->SourceStrength;
			}
			else
			{
				// We don't have a source module, so use the orientation of the emitter.
				LastTangent	= LocalToWorld.Inverse().GetAxis(0);
				fStrength	= BeamData->NoiseTangentStrength;
			}
			LastTangent.Normalize();
			LastTangent *= fStrength;

			// Setup the target tangent strength
			if (BeamData->bUseTarget)
			{
				// The target module will have determined the strength of the target tangent.
				//@todo. Do we want to 'distribute' it over all noise points?
				fTargetStrength	= BeamPayloadData->TargetStrength;
			}
			else
			{
				fTargetStrength	= BeamData->NoiseTangentStrength;
			}

			// Set the last draw position to the source so we don't get 'under-hang'
			LastPosition		= CurrPosition;
			LastDrawPosition	= CurrDrawPosition;

			UBOOL	bLocked	= BEAM2_TYPEDATA_LOCKED(BeamPayloadData->Lock_Max_NumNoisePoints);

			FVector	UseNoisePoint, CheckNoisePoint;
			FVector	NoiseDir;
			FVector	NoiseSpeed	= BeamData->NoiseSpeed;

			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
			{
				// Reset the texture coordinate
				fU					= 0.0f;
				LastPosition		= BeamPayloadData->SourcePoint;
				LastDrawPosition	= LastPosition;

				// Determine the current position by stepping the direct line and offsetting with the noise point. 
				CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

				if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
				{
					NoiseDir		= NextNoise[0] - NoisePoints[0];
					NoiseDir.Normalize();
					CheckNoisePoint	= NoisePoints[0] + NoiseDir * NoiseSpeed * *NoiseDelta;
					if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[0].X) < BeamData->NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[0].Y) < BeamData->NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[0].Z) < BeamData->NoiseLockRadius))
					{
						NoisePoints[0]	= NextNoise[0];
					}
					else
					{
						NoisePoints[0]	= CheckNoisePoint;
					}
				}

				CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[0]);

				// Determine the offset for the leading edge
				Location	= LastDrawPosition;
				EndPoint	= CurrDrawPosition;
				Right		= Location - EndPoint;
				Right.Normalize();
				LastUp		= Right ^  (Location - CameraToWorld.GetOrigin());
				LastUp.Normalize();

				if (SheetIndex)
				{
					Angle			= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
					QuatRotator		= FQuat(Right, Angle);
					WorkingLastUp	= QuatRotator.RotateVector(LastUp);
				}
				else
				{
					WorkingLastUp	= LastUp;
				}

				FLOAT	Taper	= 1.0f;

				if (BeamData->TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				LastOffset.X	= WorkingLastUp.X * Size.X * Taper;
				LastOffset.Y	= WorkingLastUp.Y * Size.Y * Taper;
				LastOffset.Z	= WorkingLastUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + LastOffset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				VertexCount++;

				Vertex->Position	= Location - LastOffset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				VertexCount++;

				fU	+= TextureIncrement;

				for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
				{
					// Determine the current position by stepping the direct line and offsetting with the noise point. 
					CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

					if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[StepIndex] - NoisePoints[StepIndex];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[StepIndex] + NoiseDir * NoiseSpeed * *NoiseDelta;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex].X) < BeamData->NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex].Y) < BeamData->NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex].Z) < BeamData->NoiseLockRadius))
						{
							NoisePoints[StepIndex]	= NextNoise[StepIndex];
						}
						else
						{
							NoisePoints[StepIndex]	= CheckNoisePoint;
						}
					}

					CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex]);

					// Prep the next draw position to determine tangents
					UBOOL bTarget = FALSE;
					NextTargetPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
					if (bLocked && ((StepIndex + 1) == BeamPayloadData->Steps))
					{
						// If we are locked, and the next step is the target point, set the draw position as such.
						// (ie, we are on the last noise point...)
						NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
						if (BeamData->bTargetNoise)
						{
							if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
							{
								NoiseDir		= NextNoise[Frequency] - NoisePoints[Frequency];
								NoiseDir.Normalize();
								CheckNoisePoint	= NoisePoints[Frequency] + NoiseDir * NoiseSpeed * *NoiseDelta;
								if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Frequency].X) < BeamData->NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Frequency].Y) < BeamData->NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Frequency].Z) < BeamData->NoiseLockRadius))
								{
									NoisePoints[Frequency]	= NextNoise[Frequency];
								}
								else
								{
									NoisePoints[Frequency]	= CheckNoisePoint;
								}
							}

							NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Frequency]);
						}
						TargetTangent = BeamPayloadData->TargetTangent;
					}
					else
					{
						// Just another noise point... offset the target to get the draw position.
						if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[StepIndex + 1] - NoisePoints[StepIndex + 1];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[StepIndex + 1] + NoiseDir * NoiseSpeed * *NoiseDelta;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex + 1].X) < BeamData->NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex + 1].Y) < BeamData->NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex + 1].Z) < BeamData->NoiseLockRadius))
							{
								NoisePoints[StepIndex + 1]	= NextNoise[StepIndex + 1];
							}
							else
							{
								NoisePoints[StepIndex + 1]	= CheckNoisePoint;
							}
						}

						NextTargetDrawPosition	= NextTargetPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex + 1]);

						//@todo. Add support for using the noise curve tangents.
		//				if (BeamModule_Noise->bUseNoiseTangents)
		//				{
		//				}
		//				else
						{
							TargetTangent = ((1.0f - BeamData->NoiseTension) / 2.0f) * 
								(NextTargetDrawPosition - LastDrawPosition);
						}
					}
					TargetTangent.Normalize();
					TargetTangent *= fTargetStrength;

					InterimDrawPosition = LastDrawPosition;
					// Tessellate between the current position and the last position
					for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							LastDrawPosition, LastTangent,
							CurrDrawPosition, TargetTangent,
							InvTessFactor * (TessIndex + 1));

						Location	= InterimDrawPosition;
						EndPoint	= InterpDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();
						Up			= Right ^  (Location - CameraToWorld.GetOrigin());
						Up.Normalize();

						if (SheetIndex)
						{
							Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
							QuatRotator	= FQuat(Right, Angle);
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (BeamData->TaperMethod != PEBTM_None)
						{
							check(TaperValues);
							Taper	= TaperValues[StepIndex * TessFactor + TessIndex];
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						// Generate the vertex
						Vertex->Position	= InterpDrawPos + Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						VertexCount++;

						Vertex->Position	= InterpDrawPos - Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						VertexCount++;

						fU	+= TextureIncrement;
						InterimDrawPosition	= InterpDrawPos;
					}
					LastPosition		= CurrPosition;
					LastDrawPosition	= CurrDrawPosition;
					LastTangent			= TargetTangent;
				}

				if (bLocked)
				{
					// Draw the line from the last point to the target
					CurrDrawPosition	= BeamPayloadData->TargetPoint;
					if (BeamData->bTargetNoise)
					{
						if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[Frequency] - NoisePoints[Frequency];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[Frequency] + NoiseDir * NoiseSpeed * *NoiseDelta;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Frequency].X) < BeamData->NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Frequency].Y) < BeamData->NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Frequency].Z) < BeamData->NoiseLockRadius))
							{
								NoisePoints[Frequency]	= NextNoise[Frequency];
							}
							else
							{
								NoisePoints[Frequency]	= CheckNoisePoint;
							}
						}

						CurrDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Frequency]);
					}

					if (BeamData->bUseTarget)
					{
						TargetTangent = BeamPayloadData->TargetTangent;
					}
					else
					{
						NextTargetDrawPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
						TargetTangent = ((1.0f - BeamData->NoiseTension) / 2.0f) * 
							(NextTargetDrawPosition - LastDrawPosition);
					}
					TargetTangent.Normalize();
					TargetTangent *= fTargetStrength;

					// Tessellate this segment
					InterimDrawPosition = LastDrawPosition;
					for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							LastDrawPosition, LastTangent,
							CurrDrawPosition, TargetTangent,
							InvTessFactor * (TessIndex + 1));

						Location	= InterimDrawPosition;
						EndPoint	= InterpDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();
						Up			= Right ^  (Location - CameraToWorld.GetOrigin());
						Up.Normalize();

						if (SheetIndex)
						{
							Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
							QuatRotator	= FQuat(Right, Angle);
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (BeamData->TaperMethod != PEBTM_None)
						{
							check(TaperValues);
							Taper	= TaperValues[BeamPayloadData->Steps * TessFactor + TessIndex];
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						// Generate the vertex
						Vertex->Position	= InterpDrawPos + Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						VertexCount++;

						Vertex->Position	= InterpDrawPos - Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						VertexCount++;

						fU	+= TextureIncrement;
						InterimDrawPosition	= InterpDrawPos;
					}
				}
				else
				if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
				{
//@todo.SAS. Re-implement partial-segment beams
/***
					if (BeamPayloadData->TravelRatio <= 1.0f)
					{
						NextTargetPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
						if (BeamPayloadData->Steps == BeamData->Frequency)
						{
							NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
							if (BeamData->bTargetNoise)
							{
								if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
								{
									NoiseDir		= NextNoise[Frequency] - NoisePoints[Frequency];
									NoiseDir.Normalize();
									CheckNoisePoint	= NoisePoints[Frequency] + NoiseDir * NoiseSpeed * *NoiseDelta;
									if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Frequency].X) < BeamData->NoiseLockRadius) &&
										(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Frequency].Y) < BeamData->NoiseLockRadius) &&
										(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Frequency].Z) < BeamData->NoiseLockRadius))
									{
										NoisePoints[Frequency]	= NextNoise[Frequency];
									}
									else
									{
										NoisePoints[Frequency]	= CheckNoisePoint;
									}
								}

								NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[BeamData->Frequency]);
							}
							TargetTangent = BeamPayloadData->TargetTangent;
						}
						else
						{
							NextTargetDrawPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

							INT	Steps = BeamPayloadData->Steps;

							if ((BeamData->NoiseLockTime >= 0.0f) && BeamData->bSmoothNoise_Enabled)
							{
								NoiseDir		= NextNoise[Steps] - NoisePoints[Steps];
								NoiseDir.Normalize();
								CheckNoisePoint	= NoisePoints[Steps] + NoiseDir * NoiseSpeed * *NoiseDelta;
								if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Steps].X) < BeamData->NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Steps].Y) < BeamData->NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Steps].Z) < BeamData->NoiseLockRadius))
								{
									NoisePoints[Steps]	= NextNoise[Steps];
								}
								else
								{
									NoisePoints[Steps]	= CheckNoisePoint;
								}
							}

							NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Steps]);
						}

						INT	Count	= appFloor(BeamPayloadData->TravelRatio * TessFactor);
						// Tessellate this segment
						InterimDrawPosition = LastDrawPosition;
						for (INT TessIndex = 0; TessIndex < Count; TessIndex++)
						{
							InterpDrawPos = CubicInterp(
								CurrDrawPosition, LastTangent,
								NextTargetDrawPosition, TargetTangent,
								InvTessFactor * (TessIndex + 1));

							Location	= InterimDrawPosition;
							EndPoint	= InterpDrawPos;
							Right		= Location - EndPoint;
							Right.Normalize();
							Up			= Right ^  (Location - CameraToWorld.GetOrigin());
							Up.Normalize();

							if (SheetIndex)
							{
								Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
								QuatRotator	= FQuat(Right, Angle);
								WorkingUp	= QuatRotator.RotateVector(Up);
							}
							else
							{
								WorkingUp	= Up;
							}

							if (BeamData->TaperMethod != PEBTM_None)
							{
								Taper	= TaperValues[BeamPayloadData->Steps * TessFactor + TessIndex];
							}

							Offset.X	= WorkingUp.X * Size.X * Taper;
							Offset.Y	= WorkingUp.Y * Size.Y * Taper;
							Offset.Z	= WorkingUp.Z * Size.Z * Taper;

							// Generate the vertex
							Vertex->Position	= InterpDrawPos + Offset;
							Vertex->OldPosition	= InterpDrawPos;
							Vertex->Size		= Size;
							Vertex->Tex_U		= fU;
							Vertex->Tex_V		= 0.0f;
							Vertex->Rotation	= Particle->Rotation;
							Vertex->Color		= Particle->Color;
							Vertex++;
							VertexCount++;

							Vertex->Position	= InterpDrawPos - Offset;
							Vertex->OldPosition	= InterpDrawPos;
							Vertex->Size		= Size;
							Vertex->Tex_U		= fU;
							Vertex->Tex_V		= 1.0f;
							Vertex->Rotation	= Particle->Rotation;
							Vertex->Color		= Particle->Color;
							Vertex++;
							VertexCount++;

							fU	+= TextureIncrement;
							InterimDrawPosition	= InterpDrawPos;
						}
					}
***/
				}
			}
		}
	}

	check(VertexCount <= BeamData->VertexCount);

	return TrianglesToRender;
}

INT FParticleSystemSceneProxy::FillTrail2EmitterIndexData(FDynamicTrail2EmitterData* TrailData, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT	TrianglesToRender = 0;

	// Trail2 polygons are packed and joined as follows:
	//
	// 1--3--5--7--9-...
	// |\ |\ |\ |\ |\...
	// | \| \| \| \| ...
	// 0--2--4--6--8-...
	//
	// (ie, the 'leading' edge of polygon (n) is the trailing edge of polygon (n+1)
	//
	// NOTE: This is primed for moving to tri-strips...
	//
	check(TrailData);

	INT	Sheets		= 1;
	INT	TessFactor	= TrailData->TessFactor ? TrailData->TessFactor : 1;
	UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));
	if (TrailData->IndexData == NULL)
	{
		if (bWireframe)
		{
			TrailData->IndexCount = 0;
			for (INT Trail = 0; Trail < TrailData->ActiveParticleCount; Trail++)
			{
				DECLARE_PARTICLE_PTR(Particle, TrailData->ParticleData + TrailData->ParticleStride * TrailData->ParticleIndices[Trail]);

				INT	CurrentOffset = TrailData->TrailDataOffset;

				FTrail2TypeDataPayload* TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)Particle + CurrentOffset);
				CurrentOffset += sizeof(FTrail2TypeDataPayload);

				FLOAT* TaperValues = (FLOAT*)((BYTE*)Particle + CurrentOffset);
				CurrentOffset += sizeof(FLOAT);

				if (TRAIL_EMITTER_IS_START(TrailPayload->Flags) == FALSE)
				{
					continue;
				}

				INT Triangles = TrailPayload->TriangleCount;
				if (Triangles > 0)
				{
					TrailData->IndexCount += (4 * Triangles + 2) * Sheets;
				}
			}
		}

		if ((UINT)TrailData->IndexCount > 65535)
		{
			FString TemplateName = TEXT("*** UNKNOWN PSYS ***");
			UParticleSystemComponent* PSysComp = Cast<UParticleSystemComponent>(PrimitiveSceneInfo->Component);
			if (PSysComp)
			{
				if (PSysComp->Template)
				{
					TemplateName = PSysComp->Template->GetName();
				}
			}

			FString ErrorOut = FString::Printf(
				TEXT("*** PLEASE SUBMIT IMMEDIATELY ***%s")
				TEXT("Trail Index Error			- %s%s")
				TEXT("\tPosition				- %s%s")				
				TEXT("\tPrimitiveCount			- %d%s")
				TEXT("\tVertexCount				- %d%s")
				TEXT("\tVertexData				- 0x%08x%s"),
				LINE_TERMINATOR,
				*TemplateName, LINE_TERMINATOR,
				*LocalToWorld.GetOrigin().ToString(), LINE_TERMINATOR,				
				TrailData->PrimitiveCount, LINE_TERMINATOR,
				TrailData->VertexCount, LINE_TERMINATOR,
				TrailData->VertexData, LINE_TERMINATOR
				);
			ErrorOut += FString::Printf(
				TEXT("\tIndexCount				- %d%s")
				TEXT("\tIndexStride				- %d%s")
				TEXT("\tIndexData				- 0x%08x%s")
				TEXT("\tVertexFactory			- 0x%08x%s"),
				TrailData->IndexCount, LINE_TERMINATOR,
				TrailData->IndexStride, LINE_TERMINATOR,
				TrailData->IndexData, LINE_TERMINATOR,
				TrailData->VertexFactory, LINE_TERMINATOR
				);
			ErrorOut += FString::Printf(
				TEXT("\tTrailDataOffset			- %d%s")
				TEXT("\tTaperValuesOffset		- %d%s")
				TEXT("\tParticleSourceOffset	- %d%s")
				TEXT("\tTrailCount				- %d%s"),
				TrailData->TrailDataOffset, LINE_TERMINATOR,
				TrailData->TaperValuesOffset, LINE_TERMINATOR,
				TrailData->ParticleSourceOffset, LINE_TERMINATOR,
				TrailData->TrailCount, LINE_TERMINATOR
				);
			ErrorOut += FString::Printf(
				TEXT("\tSheets					- %d%s")
				TEXT("\tTessFactor				- %d%s")
				TEXT("\tTessStrength			- %d%s")
				TEXT("\tTessFactorDistance		- %f%s")
				TEXT("\tActiveParticleCount		- %d%s"),
				TrailData->Sheets, LINE_TERMINATOR,
				TrailData->TessFactor, LINE_TERMINATOR,
				TrailData->TessStrength, LINE_TERMINATOR,
				TrailData->TessFactorDistance, LINE_TERMINATOR,
				TrailData->ActiveParticleCount, LINE_TERMINATOR
				);

			appErrorf(*ErrorOut);
		}
		TrailData->IndexData = appRealloc(TrailData->IndexData, TrailData->IndexCount * TrailData->IndexStride);
		check(TrailData->IndexData);
	}

	INT	CheckCount	= 0;

	WORD*	Index		= (WORD*)TrailData->IndexData;
	WORD	VertexIndex	= 0;

	for (INT Trail = 0; Trail < TrailData->ActiveParticleCount; Trail++)
	{
		DECLARE_PARTICLE_PTR(Particle, TrailData->ParticleData + TrailData->ParticleStride * TrailData->ParticleIndices[Trail]);

		INT	CurrentOffset = TrailData->TrailDataOffset;

		FTrail2TypeDataPayload* TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FTrail2TypeDataPayload);

		FLOAT* TaperValues = (FLOAT*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FLOAT);

		if (TRAIL_EMITTER_IS_START(TrailPayload->Flags) == FALSE)
		{
			continue;
		}

		if (TrailPayload->TriangleCount == 0)
		{
			continue;
		}

		if (bWireframe)
		{
			INT TriCount = TrailPayload->TriangleCount;
			if (TriCount > 0)
			{
				WORD StartVertexIndex	= 0;
				for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
				{
					VertexIndex = 0;

					// The 'starting' line
					TrianglesToRender += 1;
					*(Index++) = StartVertexIndex + 0;
					*(Index++) = StartVertexIndex + 1;

					// 4 lines per quad
					INT QuadCount = TriCount / 2;
					TrianglesToRender += TriCount * 2;

					for (INT i = 0; i < QuadCount; i++)
					{
						*(Index++) = StartVertexIndex + VertexIndex + 0;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 1;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 1;
						*(Index++) = StartVertexIndex + VertexIndex + 3;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 3;

						VertexIndex += 2;
					}

					StartVertexIndex += TriCount + 2;
				}
			}
		}
		else
		{
			INT LocalTrianglesToRender = TrailPayload->TriangleCount;

			if (LocalTrianglesToRender > 0)
			{
				for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
				{
					// 2 triangles per tessellation factor
					if (SheetIndex == 0)
					{
						// Only need the starting two for the first sheet
						*(Index++) = VertexIndex++;	// SheetIndex + 0
						*(Index++) = VertexIndex++;	// SheetIndex + 1

						CheckCount += 2;
					}

					// Sequentially step through each triangle - 1 vertex per triangle
					for (INT i = 0; i < LocalTrianglesToRender; i++)
					{
						*(Index++) = VertexIndex++;
						CheckCount++;
						TrianglesToRender++;
					}

					// Degenerate tris
					if ((SheetIndex + 1) < Sheets)
					{
						*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
						*(Index++) = VertexIndex;		// First vertex of the next sheet
						*(Index++) = VertexIndex++;		// First vertex of the next sheet
						*(Index++) = VertexIndex++;		// Second vertex of the next sheet
						TrianglesToRender += 4;
						CheckCount += 4;
					}
				}
			}
		}

		if ((Trail + 1) < TrailData->TrailCount)
		{
			*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
			*(Index++) = VertexIndex;		// First vertex of the next sheet
			*(Index++) = VertexIndex++;		// First vertex of the next sheet
			*(Index++) = VertexIndex++;		// Second vertex of the next sheet
			TrianglesToRender += 4;
			CheckCount += 4;
		}
	}

//#if defined(_DEBUG_TRAIL_DATA_)
#if 0
	if (CheckCount > TrailData->PrimitiveCount + 2)
	{
		FString DebugOut = FString::Printf(TEXT("Trail2 index buffer - CheckCount = %4d --> TriCnt + 2 = %4d"), CheckCount, TrailData->PrimitiveCount + 2);
		OutputDebugString(*DebugOut);
		OutputDebugString(TEXT("\n"));
		DebugOut = FString::Printf(TEXT("                      ActiveParticles	= %4d, Sheets = %2d, TessFactor = %2d"), TrailData->ParticleCount, Sheets, TessFactor);
		OutputDebugString(*DebugOut);
		OutputDebugString(TEXT("\n"));
//		check(0);
		INT dummy = 0;
	}
#endif

	return TrianglesToRender;
}

INT FParticleSystemSceneProxy::FillTrail2EmitterVertexData(FDynamicTrail2EmitterData* TrailData, 
	FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT	TrianglesToRender = 0;

	FParticleSpriteVertex*			Vertex			= (FParticleSpriteVertex*)(TrailData->VertexData);
	const INT						ScreenAlignment	= TrailData->ScreenAlignment;
	FMatrix							CameraToWorld	= View->ViewMatrix.Inverse();
	INT								TessFactor		= TrailData->TessFactor ? TrailData->TessFactor : 1;
	INT								Sheets			= TrailData->Sheets ? TrailData->Sheets : 1;

	FLOAT	InvTessFactor	= 1.0f / (FLOAT)TessFactor;
	FLOAT	TessStrength	= TrailData->TessStrength;
	FVector	InterpDrawPos;

	FVector	ViewOrigin	= CameraToWorld.GetOrigin();

	FVector	Offset, LastOffset;
	FLOAT	TextureIncrement;
	FLOAT	fU;
	FLOAT	Angle;
	FQuat	QuatRotator(0, 0, 0, 0);
	FVector	CurrSize, CurrPosition, CurrTangent;
	FVector EndPoint, Location, Right;
	FVector Up, WorkingUp, NextUp, WorkingNextUp;
	FVector	NextSize, NextPosition, NextTangent;
	FVector	TempDrawPos;
	FColor	CurrColor, NextColor, InterpColor;
	FColor	CurrLinearColor, NextLinearColor, InterpLinearColor, LastInterpLinearColor;
	
	FVector	TessDistCheck;
	INT		SegmentTessFactor;
	FLOAT	TessRatio;

	INT		PackedVertexCount	= 0;
	for (INT i = 0; i < TrailData->ActiveParticleCount; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, TrailData->ParticleData + TrailData->ParticleStride * TrailData->ParticleIndices[i]);

		INT	CurrentOffset = TrailData->TrailDataOffset;

		FTrail2TypeDataPayload* TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FTrail2TypeDataPayload);

		FLOAT* TaperValues = (FLOAT*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FLOAT);

		// Pin the size to the X component
		CurrSize	= FVector(Particle->Size.X * TrailData->Scale.X);
		CurrColor	= Particle->Color;

		if (TRAIL_EMITTER_IS_START(TrailPayload->Flags))
		{
			//@todo. This will only work for a single trail!
			TextureIncrement	= 1.0f / (TessFactor * TrailData->ActiveParticleCount + 1);
			UBOOL	bFirstInSheet	= TRUE;
			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
			{
				if (SheetIndex)
				{
					Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
					QuatRotator	= FQuat(Right, Angle);
				}

				fU	= 0.0f;

				// Set the current position to the source...
/***
				if (TrailSource)
				{
//					TrailSource->ResolveSourcePoint(Owner, *Particle, *TrailData, CurrPosition, CurrTangent);
				}
				else
***/
				{
					FVector	Dir			= TrailData->SceneProxy->LocalToWorld.GetAxis(0);
					Dir.Normalize();

					CurrTangent			=  Dir * TessStrength;
				}

				CurrPosition	= TrailData->SourcePosition(TrailPayload->TrailIndex);

				NextPosition	= Particle->Location;
				NextSize		= FVector(Particle->Size.X * TrailData->Scale.X);
				NextTangent		= TrailPayload->Tangent * TessStrength;
				NextColor		= Particle->Color;
				TempDrawPos		= CurrPosition;

				CurrLinearColor	= FLinearColor(CurrColor);
				NextLinearColor	= FLinearColor(NextColor);

				SegmentTessFactor	= TessFactor;
#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
				if (TrailTypeData->TessellationFactorDistance > KINDA_SMALL_NUMBER)
#else	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
				if (0)
#endif	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
				{
					TessDistCheck		= (CurrPosition - NextPosition);
					TessRatio			= TessDistCheck.Size() / TrailData->TessFactorDistance;
					if (TessRatio <= 0.0f)
					{
						SegmentTessFactor	= 1;
					}
					else
					if (TessRatio < 1.0f)
					{
						SegmentTessFactor	= appTrunc((TessFactor + 1) * TessRatio);
					}
				}
				// Tessellate the current to next...
#if !defined(_TRAIL2_TESSELLATE_TO_SOURCE_)
				SegmentTessFactor = 1;
#endif	//#if !defined(_TRAIL2_TESSELLATE_TO_SOURCE_)
				InvTessFactor	= 1.0f / SegmentTessFactor;

				for (INT TessIndex = 0; TessIndex < SegmentTessFactor; TessIndex++)
				{
					InterpDrawPos = CubicInterp(
						CurrPosition, CurrTangent,
						NextPosition, NextTangent,
						InvTessFactor * (TessIndex + 1));

					InterpLinearColor	= Lerp<FLinearColor>(
						CurrLinearColor, NextLinearColor, InvTessFactor * (TessIndex + 1));
					InterpColor			= FColor(InterpLinearColor);

					EndPoint	= InterpDrawPos;
					Location	= TempDrawPos;
					Right		= Location - EndPoint;
					Right.Normalize();

					if (bFirstInSheet)
					{
						Up	= Right ^  (Location - ViewOrigin);
						Up.Normalize();
						if (SheetIndex)
						{
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (WorkingUp.IsNearlyZero())
						{
							WorkingUp	= CameraToWorld.GetAxis(2);
							WorkingUp.Normalize();
						}

						// Setup the lead verts
						Vertex->Position	= Location + WorkingUp * CurrSize;
						Vertex->OldPosition	= Location;
						Vertex->Size		= CurrSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= CurrColor;
						Vertex++;
						PackedVertexCount++;

						Vertex->Position	= Location - WorkingUp * CurrSize;
						Vertex->OldPosition	= Location;
						Vertex->Size		= CurrSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= CurrColor;
						Vertex++;
						PackedVertexCount++;

						fU	+= TextureIncrement;
						bFirstInSheet	= FALSE;
					}

					// Setup the next verts
					NextUp	= Right ^  (EndPoint - ViewOrigin);
					NextUp.Normalize();
					if (SheetIndex)
					{
						WorkingNextUp	= QuatRotator.RotateVector(NextUp);
					}
					else
					{
						WorkingNextUp	= NextUp;
					}

					if (WorkingNextUp.IsNearlyZero())
					{
						WorkingNextUp	= CameraToWorld.GetAxis(2);
						WorkingNextUp.Normalize();
					}
					Vertex->Position	= EndPoint + WorkingNextUp * NextSize;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size		= NextSize;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 0.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= InterpColor;
					Vertex++;
					PackedVertexCount++;

					Vertex->Position	= EndPoint - WorkingNextUp * NextSize;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size		= NextSize;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 1.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= InterpColor;
					Vertex++;
					PackedVertexCount++;

					fU	+= TextureIncrement;

					TempDrawPos				= InterpDrawPos;
					LastInterpLinearColor	= InterpLinearColor;
				}

				CurrPosition	= NextPosition;
				CurrTangent		= NextTangent;
				CurrSize		= NextSize;
				CurrColor		= NextColor;

				UBOOL	bDone	= TRAIL_EMITTER_IS_ONLY(TrailPayload->Flags);

				while (!bDone)
				{
					// Grab the next particle
					INT	NextIndex	= TRAIL_EMITTER_GET_NEXT(TrailPayload->Flags);

					DECLARE_PARTICLE_PTR(NextParticle, TrailData->ParticleData + TrailData->ParticleStride * NextIndex);

					CurrentOffset = TrailData->TrailDataOffset;

					TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)NextParticle + CurrentOffset);
					CurrentOffset += sizeof(FTrail2TypeDataPayload);

					TaperValues = (FLOAT*)((BYTE*)NextParticle + CurrentOffset);
					CurrentOffset += sizeof(FLOAT);

					NextPosition	= NextParticle->Location;
					NextTangent		= TrailPayload->Tangent * TrailData->TessStrength;
					NextSize		= FVector(NextParticle->Size.X * TrailData->Scale.X);
					NextColor		= NextParticle->Color;

					TempDrawPos	= CurrPosition;

					CurrLinearColor	= FLinearColor(CurrColor);
					NextLinearColor	= FLinearColor(NextColor);

					SegmentTessFactor	= TessFactor;
#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
					if (TrailTypeData->TessellationFactorDistance > KINDA_SMALL_NUMBER)
#else	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
					if (0)
#endif	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
					{
						TessDistCheck		= (CurrPosition - NextPosition);
						TessRatio			= TessDistCheck.Size() / TrailData->TessFactorDistance;
						if (TessRatio <= 0.0f)
						{
							SegmentTessFactor	= 1;
						}
						else
						if (TessRatio < 1.0f)
						{
							SegmentTessFactor	= appTrunc((TessFactor + 1) * TessRatio);
						}
					}
					InvTessFactor	= 1.0f / SegmentTessFactor;

					for (INT TessIndex = 0; TessIndex < SegmentTessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							CurrPosition, CurrTangent,
							NextPosition, NextTangent,
							InvTessFactor * (TessIndex + 1));

						InterpLinearColor	= Lerp<FLinearColor>(
							CurrLinearColor, NextLinearColor, InvTessFactor * (TessIndex + 1));
						InterpColor			= FColor(InterpLinearColor);

						EndPoint	= InterpDrawPos;
						Location	= TempDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();

						// Setup the next verts
						NextUp	= Right ^  (EndPoint - ViewOrigin);
						NextUp.Normalize();
						if (SheetIndex)
						{
							WorkingNextUp	= QuatRotator.RotateVector(NextUp);
						}
						else
						{
							WorkingNextUp	= NextUp;
						}

						if (WorkingNextUp.IsNearlyZero())
						{
							WorkingNextUp	= CameraToWorld.GetAxis(2);
							WorkingNextUp.Normalize();
						}
						Vertex->Position	= EndPoint + WorkingNextUp * NextSize;
						Vertex->OldPosition	= EndPoint;
						Vertex->Size		= NextSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= NextParticle->Rotation;
						Vertex->Color		= InterpColor;
						Vertex++;
						PackedVertexCount++;

						Vertex->Position	= EndPoint - WorkingNextUp * NextSize;
						Vertex->OldPosition	= EndPoint;
						Vertex->Size		= NextSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= NextParticle->Rotation;
						Vertex->Color		= InterpColor;
						Vertex++;
						PackedVertexCount++;

						fU	+= TextureIncrement;

						TempDrawPos	= InterpDrawPos;
					}

					CurrPosition	= NextPosition;
					CurrTangent		= NextTangent;
					CurrSize		= NextSize;
					CurrColor		= NextColor;

					if (TRAIL_EMITTER_IS_END(TrailPayload->Flags) ||
						TRAIL_EMITTER_IS_ONLY(TrailPayload->Flags))
					{
						bDone = TRUE;
					}
				}
			}
		}
	}

	return TrianglesToRender;
}

/**
 *	Retrieve the appropriate camera Up and Right vectors for LockAxis situations
 *
 *	@param	DynamicData		The emitter dynamic data the values are being retrieved for
 *	@param	CameraUp		OUTPUT - the resulting camera Up vector
 *	@param	CameraRight		OUTPUT - the resulting camera Right vector
 */
void FParticleSystemSceneProxy::GetAxisLockValues(FDynamicSpriteEmitterDataBase* DynamicData, FVector& CameraUp, FVector& CameraRight)
{
	const FMatrix& AxisLocalToWorld = DynamicData->bUseLocalSpace ? LocalToWorld: FMatrix::Identity;

	switch (DynamicData->LockAxisFlag)
	{
	case EPAL_X:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	=  AxisLocalToWorld.GetAxis(1);
		break;
	case EPAL_Y:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	= -AxisLocalToWorld.GetAxis(0);
		break;
	case EPAL_Z:
		CameraUp		=  AxisLocalToWorld.GetAxis(0);
		CameraRight	= -AxisLocalToWorld.GetAxis(1);
		break;
	case EPAL_NEGATIVE_X:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	= -AxisLocalToWorld.GetAxis(1);
		break;
	case EPAL_NEGATIVE_Y:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	=  AxisLocalToWorld.GetAxis(0);
		break;
	case EPAL_NEGATIVE_Z:
		CameraUp		=  AxisLocalToWorld.GetAxis(0);
		CameraRight	=  AxisLocalToWorld.GetAxis(1);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ParticleSystemComponent
///////////////////////////////////////////////////////////////////////////////

/** Returns true if the prim is using a material with unlit distortion */
UBOOL UParticleSystemComponent::HasUnlitTranslucency() const
{
	UBOOL bResult = FALSE;

	// iterate over all emiiters for this primitive
	if (Template)
	{
		for (INT EmitterIndex = 0; EmitterIndex < Template->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter = Template->Emitters(EmitterIndex);
			if (Emitter)
			{
				UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(Emitter);
				if (SpriteEmitter)
				{
					UMaterialInstance* MaterialInst = SpriteEmitter->Material;
					check(SpriteEmitter->LODLevels.Num() >= 1);

					UParticleLODLevel* LODLevel = SpriteEmitter->LODLevels(0);
					if (LODLevel && LODLevel->TypeDataModule)
					{
						UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
						if (MeshTD && MeshTD->Mesh)
						{
							if (MeshTD->Mesh->LODModels(0).Elements.Num())
							{
								FStaticMeshElement&	Element = MeshTD->Mesh->LODModels(0).Elements(0);
								if (Element.Material)
								{
									MaterialInst = Element.Material;
								}
							}
						}
					}

					UMaterial* Material = MaterialInst ? MaterialInst->GetMaterial() : NULL;

					// check for unlit distortion
					if (Material &&
						(Material->LightingModel == MLM_Unlit) &&
						IsTranslucentBlendMode((EBlendMode)Material->BlendMode)
						)
					{
						bResult = TRUE;
						break;
					}
				}
			}
		}
	}

	return bResult;
} 

/** Returns true if the prim is using a material with unlit translucency */
UBOOL UParticleSystemComponent::HasUnlitDistortion() const
{
	UBOOL bResult = FALSE;

	// iterate over all emiiters for this primitive
	if (Template)
	{
		for (INT EmitterIndex = 0; EmitterIndex < Template->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter = Template->Emitters(EmitterIndex);
			if (Emitter)
			{
				UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(Emitter);

				if (SpriteEmitter)
				{
					UMaterialInstance* MaterialInst = SpriteEmitter->Material;
					check(SpriteEmitter->LODLevels.Num() >= 1);

					UParticleLODLevel* LODLevel = SpriteEmitter->LODLevels(0);
					if (LODLevel && LODLevel->TypeDataModule)
					{
						UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
						if (MeshTD && MeshTD->Mesh)
						{
							if (MeshTD->Mesh->LODModels(0).Elements.Num())
							{
								FStaticMeshElement&	Element = MeshTD->Mesh->LODModels(0).Elements(0);
								if (Element.Material)
								{
									MaterialInst = Element.Material;
								}
							}
						}
					}

					UMaterial* Material = MaterialInst ? MaterialInst->GetMaterial() : NULL;
					if( Material && 
						Material->LightingModel == MLM_Unlit &&
						Material->HasDistortion() )
					{
						bResult = TRUE;
						break;
					}
				}
			}
		}
	}

	return bResult;
}

/** 
* Returns true if the prim is using a material that samples the scene color texture. 
* If true then these primitives are drawn after all other translucency 
*/
UBOOL UParticleSystemComponent::UsesSceneColor() const
{
	UBOOL bResult = FALSE;

	// iterate over all emiiters for this primitive
	if (Template)
	{
		for (INT EmitterIndex = 0; EmitterIndex < Template->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter = Template->Emitters(EmitterIndex);
			if (Emitter)
			{
				UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(Emitter);

				if (SpriteEmitter)
				{
					UMaterialInstance* MaterialInst = SpriteEmitter->Material;
					check(SpriteEmitter->LODLevels.Num() >= 1);

					UParticleLODLevel* LODLevel = SpriteEmitter->LODLevels(0);
					if (LODLevel && LODLevel->TypeDataModule)
					{
						UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
						if (MeshTD && MeshTD->Mesh)
						{
							if (MeshTD->Mesh->LODModels(0).Elements.Num())
							{
								FStaticMeshElement&	Element = MeshTD->Mesh->LODModels(0).Elements(0);
								if (Element.Material)
								{
									MaterialInst = Element.Material;
								}
							}
						}
					}

					UMaterial* Material = MaterialInst ? MaterialInst->GetMaterial() : NULL;
					if( Material && 
						Material->UsesSceneColor() )
					{
						bResult = TRUE;
						break;
					}
				}
			}
		}
	}
	return bResult;
}

FPrimitiveSceneProxy* UParticleSystemComponent::CreateSceneProxy()
{
	FParticleSystemSceneProxy* NewProxy = NULL;

	if (bIsActive == TRUE)
	{
		NewProxy = ::new FParticleSystemSceneProxy(this);
		check (NewProxy);
	}
	
	// 
	return NewProxy;
}

////////////////////////////////////////////////////////////////////////////////
//	Helper functions
///////////////////////////////////////////////////////////////////////////////
void PS_DumpBeamDataInformation(TCHAR* Message, 
	UParticleSystemComponent* PSysComp, FParticleSystemSceneProxy* Proxy, 
	FParticleDynamicData* NewPSDynamicData, FParticleDynamicData* OldPSDynamicData, 
	FDynamicBeam2EmitterData* NewBeamData, FDynamicBeam2EmitterData* OldBeamData)
{
#if defined(_DEBUG_BEAM_DATA_)
	INT	Spaces = 0;
	if (Message)
	{
		OutputDebugString(Message);
		Spaces = appStrlen(Message);
	}

	while (Spaces < 72)
	{
		OutputDebugString(TEXT(" "));
		Spaces++;
	}
	OutputDebugString(TEXT("    "));

	FString DebugOut = FString::Printf(TEXT("PSysComp     0x%08x - SceneProxy   0x%08x - DynamicData  0x%08x - OldDynData   0x%08x - BeamData     0x%08x - OldBeamData  0x%08x"),
		PSysComp, Proxy, NewPSDynamicData, OldPSDynamicData, NewBeamData, OldBeamData);
	OutputDebugString(*DebugOut);
	OutputDebugString(TEXT("\n"));
#endif
}

void PS_DumpTrailDataInformation(TCHAR* Message, 
	UParticleSystemComponent* PSysComp, FParticleSystemSceneProxy* Proxy, 
	FParticleDynamicData* NewPSDynamicData, FParticleDynamicData* OldPSDynamicData, 
	FDynamicTrail2EmitterData* NewTrailData, FDynamicTrail2EmitterData* OldTrailData)
{
#if defined(_DEBUG_TRAIL_DATA_)
	INT	Spaces = 0;
	if (Message)
	{
		OutputDebugString(Message);
		Spaces = appStrlen(Message);
	}

	while (Spaces < 48)
	{
		OutputDebugString(TEXT(" "));
		Spaces++;
	}
	OutputDebugString(TEXT("    "));

	FString DebugOut = FString::Printf(TEXT("PSysComp     0x%08x - SceneProxy   0x%08x - DynamicData  0x%08x - OldDynData   0x%08x - TrailData     0x%08x - OldTrailData  0x%08x"),
		PSysComp, Proxy, NewPSDynamicData, OldPSDynamicData, NewTrailData, OldTrailData);
	OutputDebugString(*DebugOut);
	OutputDebugString(TEXT("\n"));
#endif
}
