/**
*	LensFlare.h: LensFlare and helper class definitions.
*	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
*/
#ifndef _LENSFLARE_HEADER_
#define _LENSFLARE_HEADER_

class FLensFlareSceneProxy;

#include "EngineLensFlareClasses.h"

class ULensFlare;
class ULensFlareComponent;
class ALensFlareSource;

/** Vertex definition for lens flare elements */
struct FLensFlareVertex
{
	FVector4		Position;
	FVector4		OldPosition;
	FVector4		Size;
	FLOAT			Rotation;
	FVector2D		TexCoord;
	FLinearColor	Color;
	FVector4		RadialDist_SourceRatio;
};

/** Helper structure for looking up element values */
struct FLensFlareElementValues
{
	/** The lookup values. */
	FLOAT RadialDistance;
	FLOAT SourceDistance;
	/** The material(s) to use for the flare element. */
	UMaterialInstance*	LFMaterial;
	/**	Global scaling.	 */
	FLOAT Scaling;
	/**	Anamorphic scaling.	*/
	FVector AxisScaling;
	/**	Rotation.	 */
	FLOAT Rotation;
	/** Color. */
	FLinearColor Color;
	/** Offset. */
	FVector Offset;
};

/*
*	Sorting Helper
*/
struct FLensFlareElementOrder
{
	INT		ElementIndex;
	FLOAT	RayDistance;

	FLensFlareElementOrder(INT InElementIndex, FLOAT InRayDistance):
	ElementIndex(InElementIndex),
		RayDistance(InRayDistance)
	{}
};

/** Dynamic data for a lens flare */
struct FLensFlareDynamicData
{
	FLensFlareDynamicData(const ULensFlareComponent* InLensFlareComp, FLensFlareSceneProxy* InProxy);
	~FLensFlareDynamicData();

	DWORD GetMemoryFootprint( void ) const;

	/**
	* @return TRUE if the particle system uses FMeshElement.UseDynamicData=TRUE
	*/
	virtual UBOOL HasDynamicMeshElementData()
	{
		// by default all emitters assumed to use dynamic vertex data
		return TRUE;
	}

	/**
	*/
	void InitializeRenderResources(const ULensFlareComponent* InLensFlareComp, FLensFlareSceneProxy* InProxy);
	void RenderThread_InitializeRenderResources(FLensFlareSceneProxy* InProxy);
	void ReleaseRenderResources(const ULensFlareComponent* InLensFlareComp, FLensFlareSceneProxy* InProxy);
	void RenderThread_ReleaseRenderResources();

	/**
	*	Render thread only draw call
	*	
	*	@param	Proxy		The scene proxy for the lens flare
	*	@param	PDI			The PrimitiveDrawInterface
	*	@param	View		The SceneView that is being rendered
	*	@param	DPGIndex	The DrawPrimitiveGroup being rendered
	*/
	virtual void Render(FLensFlareSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, DWORD Flags);

	/** Render the source element. */
	virtual void RenderSource(FLensFlareSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, DWORD Flags);
	/** Render the reflection elements. */
	virtual void RenderReflections(FLensFlareSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, DWORD Flags);

protected:
	/**
	*	Retrieve the various values for the given element
	*
	*	@param	ScreenPosition		The position of the element in screen space
	*	@param	SourcePosition		The position of the source in screen space
	*	@param	Element				The element of interest
	*	@param	Values				The values to fill in
	*
	*	@return	UBOOL				TRUE if successful
	*/
	UBOOL GetElementValues(FVector ScreenPosition, FVector SourcePosition, const FSceneView* View, FLensFlareElement* Element, FLensFlareElementValues& Values);

	/**
	*	Sort the contained elements along the ray
	*/
	void SortElements();

	struct FLensFlareElement* SourceElement;
	TArrayNoInit<struct FLensFlareElement*> Reflections;

	FLensFlareVertexFactory* VertexFactory;
	FLensFlareVertex* VertexData;

	TArray<FLensFlareElementOrder>	ElementOrder;
};

//
//	Scene Proxies
//
class FLensFlareSceneProxy : public FPrimitiveSceneProxy
{
public:
	/** Initialization constructor. */
	FLensFlareSceneProxy(const ULensFlareComponent* Component);
	virtual ~FLensFlareSceneProxy();

	// FPrimitiveSceneProxy interface.

	/**
	* Draws the primitive's static elements.  This is called from the game thread once when the scene proxy is created.
	* The static elements will only be rendered if GetViewRelevance declares static relevance.
	* Called in the game thread.
	* @param PDI - The interface which receives the primitive elements.
	*/
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI);

	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);

	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const class FLightSceneInfo* Light);	
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	*	Called when the rendering thread adds the proxy to the scene.
	*	This function allows for generating renderer-side resources.
	*/
	virtual UBOOL CreateRenderThreadResources();

	/**
	*	Called when the rendering thread removes the dynamic data from the scene.
	*/
	virtual UBOOL ReleaseRenderThreadResources();

	void UpdateData();
	void UpdateData_RenderThread();

	FLensFlareDynamicData* GetDynamicData()
	{
		return DynamicData;
	}

	FLensFlareDynamicData* GetLastDynamicData()
	{
		return LastDynamicData;
	}

	void SetLastDynamicData(FLensFlareDynamicData* InLastDynamicData)
	{
		LastDynamicData  = InLastDynamicData;
	}

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocLFSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const
	{ 
		DWORD AdditionalSize = FPrimitiveSceneProxy::GetAllocatedSize();
		if (DynamicData)
		{
			AdditionalSize = DynamicData->GetMemoryFootprint();
		}

		return AdditionalSize; 
	}


public:
	// While this isn't good OO design, access to everything is made public.
	// This is to allow custom emitter instances to easily be written when extending the engine.

	FPrimitiveSceneInfo* GetPrimitiveSceneInfo() const	{	return PrimitiveSceneInfo;	}

	FMatrix& GetLocalToWorld()			{	return LocalToWorld;			}
	FMatrix GetWorldToLocal()			{	return LocalToWorld.Inverse();	}	
	FLOAT GetLocalToWorldDeterminant()	{	return LocalToWorldDeterminant;	}
	AActor* GetOwner()					{	return Owner;					}
	UBOOL GetSelected()					{	return bSelected;				}	
	UBOOL GetCastShadow()				{	return bCastShadow;				}
	UBOOL GetHasTranslucency()			{	return bHasTranslucency;		}
	UBOOL GetHasDistortion()			{	return bHasDistortion;			}
	UBOOL GetUsesSceneColor()			{	return bUsesSceneColor;			}
	UBOOL GetRenderDebug()				{	return bRenderDebug;			}	

protected:
	AActor* Owner;

	UBOOL bSelected;	

	BITFIELD bCastShadow : 1;
	BITFIELD bHasTranslucency : 1;
	BITFIELD bHasDistortion : 1;
	BITFIELD bUsesSceneColor : 1;
	BITFIELD bRenderDebug : 1;

	/** The scene depth priority group to draw the source primitive in. */
	BYTE	SourceDPG;
	/** The scene depth priority group to draw the reflection primitive(s) in. */
	BYTE	ReflectionsDPG;

	FLensFlareDynamicData* DynamicData;
	FLensFlareDynamicData* LastDynamicData;	
};

#endif	//#ifndef _LENSFLARE_HEADER_
