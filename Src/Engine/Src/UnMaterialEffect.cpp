/*=============================================================================
	UMaterialEffect.cpp: Material post process effect implementation.
	Copyright 2003-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "TileRendering.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(UMaterialEffect);

/*-----------------------------------------------------------------------------
FMaterialPostProcessSceneProxy
-----------------------------------------------------------------------------*/

/**
* Render-side data and rendering functionality for a UMaterialEffect
*/
class FMaterialPostProcessSceneProxy : public FPostProcessSceneProxy
{
public:
	/** 
	* Initialization constructor. 
	* @param InEffect - material post process effect to mirror in this proxy
	*/
	FMaterialPostProcessSceneProxy(const UMaterialEffect* InEffect,const FPostProcessSettings* WorldSettings)
		:	FPostProcessSceneProxy(InEffect)
		,	MaterialInstance(InEffect->Material ? InEffect->Material->GetInstanceInterface(FALSE) : GEngine->EmissiveTexturedMaterial->GetInstanceInterface(FALSE))
	{
		if(WorldSettings)
		{
			// The material need to be a material instance constant.
			UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(InEffect->Material);
			if( MaterialInstance )
			{
				// Propagate settings.
				MaterialInstance->SetScalarParameterValue( NAME_Desaturation, WorldSettings->Scene_Desaturation );
				const FVector& HighLights	= WorldSettings->Scene_HighLights;
				MaterialInstance->SetVectorParameterValue( NAME_HighLights, FLinearColor( HighLights.X, HighLights.Y, HighLights.Z, 0.f ) );
				const FVector& MidTones	= WorldSettings->Scene_MidTones;
				MaterialInstance->SetVectorParameterValue( NAME_MidTones, FLinearColor( MidTones.X, MidTones.Y, MidTones.Z, 0.f ) );
				const FVector& Shadows		= WorldSettings->Scene_Shadows;
				MaterialInstance->SetVectorParameterValue( NAME_Shadows, FLinearColor( Shadows.X, Shadows.Y, Shadows.Z, 0.f ) );
			}
		}
	}

	/**
	* Render the post process effect
	* Called by the rendering thread during scene rendering
	* @param InDepthPriorityGroup - scene DPG currently being rendered
	* @param View - current view
	* @param CanvasTransform - same canvas transform used to render the scene
	* @return TRUE if anything was rendered
	*/
	UBOOL Render(FCommandContextRHI* Context, UINT InDepthPriorityGroup,FViewInfo& View,const FMatrix& CanvasTransform)
	{
		SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("MaterialEffect"));

		UBOOL bDirty=TRUE;
		const FMaterial* Material = MaterialInstance->GetMaterial();
		check(Material);

		// distortion materials are not handled
		if( Material->IsDistorted() )
		{
			bDirty = FALSE;
			return bDirty;
		}

		// post process effects are rendered to the scene color
		GSceneRenderTargets.BeginRenderingSceneColor();

		// viewport to match view size
		RHISetViewport(Context,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);

		// Turn off alpha-writes, since we sometimes store the Z-values there.
		RHISetColorWriteMask(Context, CW_RGB);

		FTileRenderer TileRenderer;
		// draw a full-view tile (the TileRenderer uses SizeX, not RenderTargetSizeX)
		check(View.SizeX == View.RenderTargetSizeX);
		TileRenderer.DrawTile(Context, View, MaterialInstance);

		// Turn on alpha-writes again.
		RHISetColorWriteMask(Context, CW_RGBA);

        // scene color is resolved if something was rendered
		GSceneRenderTargets.FinishRenderingSceneColor(bDirty);

		return bDirty;
	}

private:
	/** Material instance used by the effect */
	const FMaterialInstance* MaterialInstance;
};

/*-----------------------------------------------------------------------------
UMaterialEffect
-----------------------------------------------------------------------------*/

/**
* Creates a proxy to represent the render info for a post process effect
* @return The proxy object.
*/
FPostProcessSceneProxy* UMaterialEffect::CreateSceneProxy(const FPostProcessSettings* WorldSettings)
{
	if( Material && (!WorldSettings || WorldSettings->bEnableSceneEffect))
	{
		return new FMaterialPostProcessSceneProxy(this,WorldSettings);
	}
	else
	{
		return NULL;
	}
}
