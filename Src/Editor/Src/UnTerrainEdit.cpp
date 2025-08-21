/*=============================================================================
	UnTerrainEdit.cpp: Terrain editing code.
	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"
#include "UnTerrain.h"

/*-----------------------------------------------------------------------------
	FModeTool_Terrain.
-----------------------------------------------------------------------------*/

FModeTool_Terrain::FModeTool_Terrain()
{
	ID = MT_None;
	bUseWidget = 0;
	Settings = new FTerrainToolSettings;

	PaintingViewportClient = NULL;
}

UBOOL FModeTool_Terrain::MouseMove(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,INT x, INT y)
{
	ViewportClient->Viewport->InvalidateDisplay();
	return 1;
}

/**
 * @return		TRUE if the delta was handled by this editor mode tool.
 */
UBOOL FModeTool_Terrain::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	if (PaintingViewportClient == InViewportClient)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @return		TRUE if the key was handled by this editor mode tool.
 */
UBOOL FModeTool_Terrain::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	if ((PaintingViewportClient == NULL || PaintingViewportClient == ViewportClient) && (Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton))
	{
		if (IsCtrlDown(Viewport) && Event == IE_Pressed)
		{
			FSceneViewFamilyContext ViewFamily(ViewportClient->Viewport,ViewportClient->GetScene(),ViewportClient->ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetRealTimeSeconds(),NULL);
			FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);

			FTerrainToolSettings*	Settings = (FTerrainToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
			FViewport*				Viewport = ViewportClient->Viewport;
			FVector					HitLocation, HitNormal;;
			ATerrain*				Terrain = TerrainTrace(Viewport,View,HitLocation, HitNormal, Settings);

			if (Terrain)
			{
				GEditor->Trans->Begin(TEXT("TerrainEdit"));
				Terrain->Modify();
				bIsTransacting = TRUE;

				UBOOL bMirrorX = FALSE;
				UBOOL bMirrorY = FALSE;
				if (SupportsMirroring() == TRUE)
				{
					bMirrorX = (((INT)(Settings->MirrorSetting) & FTerrainToolSettings::TTMirror_X) != 0) ? TRUE : FALSE;
					bMirrorY = (((INT)(Settings->MirrorSetting) & FTerrainToolSettings::TTMirror_Y) != 0) ? TRUE : FALSE;
				}

				if (BeginApplyTool(Terrain,Settings->DecoLayer,
					FindMatchingTerrainLayer(Terrain, Settings),
					Settings->MaterialIndex,Terrain->WorldToLocal().TransformFVector(HitLocation), 
					HitLocation, HitNormal, bMirrorX, bMirrorY
					)
					)
				{
					PaintingViewportClient = ViewportClient;
					Terrain->MarkPackageDirty();
				}
				else
				{
					GEditor->Trans->End();
					bIsTransacting = FALSE;
				}
			}

			return 1;
		}
		else if (PaintingViewportClient)
		{
			EndApplyTool();

			if (bIsTransacting)
			{
				GEditor->Trans->End();
				bIsTransacting = FALSE;
			}

			INT	X	= Viewport->GetMouseX();
			INT	Y	= Viewport->GetMouseY();
			PaintingViewportClient = NULL;
			Viewport->SetMouse(X, Y);
			return 1;
		}
		else
		{
			return 0;
		}
	}

	return 0;

}

void FModeTool_Terrain::Tick(FEditorLevelViewportClient* ViewportClient,FLOAT DeltaTime)
{
	FEdModeTerrainEditing*	EdMode = (FEdModeTerrainEditing*)(GEditorModeTools().GetCurrentMode());
	check(EdMode);
	FTerrainToolSettings*	Settings = (FTerrainToolSettings*)(EdMode->GetSettings());
	FViewport*				Viewport = ViewportClient->Viewport;
	FVector					HitLocation;

	FSceneViewFamilyContext ViewFamily(ViewportClient->Viewport,ViewportClient->GetScene(),ViewportClient->ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetRealTimeSeconds(),NULL);
	FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);
	if (PaintingViewportClient == ViewportClient)
	{
		for (INT i = 0; i < EdMode->ToolTerrains.Num(); i++)
		{
			ATerrain* Terrain = EdMode->ToolTerrains(i);
			if (Terrain)
			{
				FLOAT	Direction = Viewport->KeyState(KEY_LeftMouseButton) ? 1.f : -1.f;
				FVector	LocalPosition = Terrain->WorldToLocal().TransformFVector(EdMode->ToolHitLocation); // In the terrain actor's local space.
				FLOAT	LocalStrength = Settings->Strength * DeltaTime / Terrain->DrawScale3D.Z / TERRAIN_ZSCALE,
						LocalMinRadius = Settings->RadiusMin / Terrain->DrawScale3D.X / Terrain->DrawScale,
						LocalMaxRadius = Settings->RadiusMax / Terrain->DrawScale3D.X / Terrain->DrawScale;

				INT	MinX = Clamp(appFloor(LocalPosition.X - LocalMaxRadius),0,Terrain->NumVerticesX - 1),
					MinY = Clamp(appFloor(LocalPosition.Y - LocalMaxRadius),0,Terrain->NumVerticesY - 1),
					MaxX = Clamp(appCeil(LocalPosition.X + LocalMaxRadius),0,Terrain->NumVerticesX - 1),
					MaxY = Clamp(appCeil(LocalPosition.Y + LocalMaxRadius),0,Terrain->NumVerticesY - 1);

				UBOOL bMirrorX = FALSE;
				UBOOL bMirrorY = FALSE;
				if (SupportsMirroring() == TRUE)
				{
					bMirrorX = (((INT)(Settings->MirrorSetting) & FTerrainToolSettings::TTMirror_X) != 0) ? TRUE : FALSE;
					bMirrorY = (((INT)(Settings->MirrorSetting) & FTerrainToolSettings::TTMirror_Y) != 0) ? TRUE : FALSE;
				}


				FIntRect MirrorValues;
				MirrorValues.Min.X = MinX;
				MirrorValues.Min.Y = MinY;
				MirrorValues.Max.X = MaxX;
				MirrorValues.Max.Y = MaxY;
				INT Temp1, Temp2;
				if (bMirrorY)
				{
					Temp1 = EdMode->GetMirroredValue_X(Terrain, MinX);
					Temp2 = EdMode->GetMirroredValue_X(Terrain, MaxX);
					MirrorValues.Min.X = Min<INT>(Temp1, Temp2);
					MirrorValues.Max.X = Max<INT>(Temp1, Temp2);
				}
				if (bMirrorX)
				{
					Temp1 = EdMode->GetMirroredValue_Y(Terrain, MinY);
					Temp2 = EdMode->GetMirroredValue_Y(Terrain, MaxY);
					MirrorValues.Min.Y = Min<INT>(Temp1, Temp2);
					MirrorValues.Max.Y = Max<INT>(Temp1, Temp2);
				}

				MirrorValues.Min.X = Clamp(MirrorValues.Min.X,0,Terrain->NumVerticesX - 1),
				MirrorValues.Min.Y = Clamp(MirrorValues.Min.Y,0,Terrain->NumVerticesY - 1),
				MirrorValues.Max.X = Clamp(MirrorValues.Max.X,0,Terrain->NumVerticesX - 1),
				MirrorValues.Max.Y = Clamp(MirrorValues.Max.Y,0,Terrain->NumVerticesY - 1);

				INT LayerIndex = FindMatchingTerrainLayer(Terrain, Settings);
				ApplyTool(Terrain, Settings->DecoLayer, LayerIndex, Settings->MaterialIndex, LocalPosition, 
					LocalMinRadius, LocalMaxRadius, Direction, LocalStrength, MinX, MinY, MaxX, MaxY,
					bMirrorX, bMirrorY);
				if (LayerIndex == INDEX_NONE)
				{
					Terrain->UpdatePatchBounds(MinX, MinY, MaxX, MaxY);
					if (bMirrorY || bMirrorX)
					{
						Terrain->UpdatePatchBounds(
							MirrorValues.Min.X,MirrorValues.Min.Y,
							MirrorValues.Max.X,MirrorValues.Max.Y);
					}
				}
				Terrain->UpdateRenderData(MinX,MinY,MaxX,MaxY);
				if (bMirrorY || bMirrorX)
				{
					Terrain->UpdateRenderData(
						MirrorValues.Min.X,MirrorValues.Min.Y,
						MirrorValues.Max.X,MirrorValues.Max.Y);
				}

				// remember that we modified this terrain
				ModifiedTerrains.AddUniqueItem(Terrain);

				// Force viewport update
				Viewport->Invalidate();
			}
		}
	}
}

ATerrain* FModeTool_Terrain::TerrainTrace(FViewport* Viewport,
	const FSceneView* View,FVector& Location, FVector& Normal, 
	FTerrainToolSettings* Settings, UBOOL bGetFirstHit)
{
	if (Viewport)
	{
        // Fix from BioWare for the cursor offset problem - Nov-12-2005 - START
        // Broke the transform by the inverse viewprojection matrix into two parts: transform by inverse
        // projection followed by transform by inverse view.  This solves some numerical stability problems
        // when painting on terrain with large drawscale.

        // Calculate World-space position of the raycast towards the mouse cursor.
		FIntPoint	MousePosition;
		Viewport->GetMousePos(MousePosition);
		INT		X = MousePosition.X,
				Y = MousePosition.Y;

        // Get the eye position and direction of the mouse cursor in two stages (inverse transform projection, then inverse transform view).
        // This avoids the numerical instability that occurs when a view matrix with large translation is composed with a projection matrix
        FMatrix InverseProjection = View->ProjectionMatrix.Inverse();
        FMatrix InverseView = View->ViewMatrix.Inverse();

        // The start of the raytrace is defined to be at mousex,mousey,0 in projection space
        // The end of the raytrace is at mousex, mousey, 0.5 in projection space
        FLOAT ScreenSpaceX = (X-Viewport->GetSizeX()/2.0f)/(Viewport->GetSizeX()/2.0f);
        FLOAT ScreenSpaceY = (Y-Viewport->GetSizeY()/2.0f)/-(Viewport->GetSizeY()/2.0f);
        FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY,    0, 1.0f);
        FVector4 RayEndProjectionSpace   = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

        // Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
        // by the projection matrix should use homogenous coordinates (i.e. FPlane).
        FVector4 HGRayStartViewSpace = InverseProjection.TransformFVector4(RayStartProjectionSpace);
        FVector4 HGRayEndViewSpace   = InverseProjection.TransformFVector4(RayEndProjectionSpace);
        FVector RayStartViewSpace(HGRayStartViewSpace.X, HGRayStartViewSpace.Y, HGRayStartViewSpace.Z);
        FVector   RayEndViewSpace(HGRayEndViewSpace.X,   HGRayEndViewSpace.Y,   HGRayEndViewSpace.Z);
        // divide vectors by W to undo any projection and get the 3-space coordinate 
        if (HGRayStartViewSpace.W != 0.0f)
        {
            RayStartViewSpace /= HGRayStartViewSpace.W;
        }
        if (HGRayEndViewSpace.W != 0.0f)
        {
            RayEndViewSpace /= HGRayEndViewSpace.W;
        }
        FVector RayDirViewSpace = RayEndViewSpace - RayStartViewSpace;
        RayDirViewSpace = RayDirViewSpace.SafeNormal();

        // The view transform does not have projection, so we can use the standard functions that deal with vectors and normals (normals
        // are vectors that do not use the translational part of a rotation/translation)
        FVector RayStartWorldSpace = InverseView.TransformFVector(RayStartViewSpace);
        FVector RayDirWorldSpace   = InverseView.TransformNormal(RayDirViewSpace);

        // Finally, store the results in the hitcheck inputs.  The start position is the eye, and the end position
        // is the eye plus a long distance in the direction the mouse is pointing.
        FVector Start = RayStartWorldSpace;
        FVector Dir = RayDirWorldSpace.SafeNormal();
        FVector End = RayStartWorldSpace + Dir*WORLD_MAX;
        // Fix from BioWare for the cursor offset problem - Nov-12-2005 - END

		// Do the trace.
		if (bGetFirstHit)
		{
			FCheckResult	Hit(1);
			GWorld->SingleLineCheck(Hit,NULL,End,Start,TRACE_Terrain|TRACE_TerrainIgnoreHoles);
			ATerrain* HitTerrain = Cast<ATerrain>(Hit.Actor);
			if (HitTerrain)
			{
				if (TerrainIsValid(HitTerrain, Settings))
				{
					Normal = Hit.Normal;
					Location = Hit.Location;
					return HitTerrain;
				}
			}
		}
		else
		{
			FMemMark		Mark(GMem);
			FCheckResult*	FirstHit	= NULL;
			DWORD			TraceFlags	= TRACE_Terrain|TRACE_TerrainIgnoreHoles;

			FirstHit	= GWorld->MultiLineCheck(GMem, End, Start, FVector(0.f,0.f,0.f), TraceFlags, NULL);
			for (FCheckResult* Test = FirstHit; Test; Test = Test->GetNext())
			{
				ATerrain* HitTerrain	= Cast<ATerrain>(Test->Actor);
				if (HitTerrain)
				{
					if (TerrainIsValid(HitTerrain, Settings))
					{
						Normal		= Test->Normal;
						Location	= Test->Location;
						Mark.Pop();
						return HitTerrain;
					}
				}
			}

			Mark.Pop();
		}
	}

	return NULL;

}

/**
 * Determines how strongly the editing circles affect a vertex.
 *
 * @param	ToolCenter		The location being pointed at on the terrain
 * @param	Vertex			The vertex being affected
 * @param	MinRadius		The outer edge of the inner circle
 * @param	MaxRadius		The outer edge of the outer circle
 *
 * @return	A vaue between 0-1, representing how strongly the tool should affect the vertex.
 */

FLOAT FModeTool_Terrain::RadiusStrength(const FVector& ToolCenter,const FVector& Vertex,FLOAT MinRadius,FLOAT MaxRadius)
{
	FLOAT	Distance = (Vertex - ToolCenter).Size2D();

	if (Distance <= MinRadius)
		return 1.0f;
	else if (Distance < MaxRadius)
		return 1.0f - (Distance - MinRadius) / (MaxRadius - MinRadius);
	else
		return 0.0f;
}

/**
 *	Retreive the mirrored versions of the Min/Max<X/Y> values
 *
 *	@param	Terrain		Pointer to the terrain of interest
 *	@param	InMinX		The 'source' MinX value
 *	@param	InMinY		The 'source' MinY value
 *	@param	InMaxX		The 'source' MaxX value
 *	@param	InMaxY		The 'source' MaxY value
 *	@param	bMirrorX	Whether to mirror about the X axis
 *	@param	bMirrorY	Whether to mirror about the Y axis
 *	@param	OutMinX		The output of the mirrored MinX value
 *	@param	OutMinY		The output of the mirrored MinY value
 *	@param	OutMaxX		The output of the mirrored MaxX value
 *	@param	OutMaxY		The output of the mirrored MaxY value
 */
void FModeTool_Terrain::GetMirroredExtents(ATerrain* Terrain, INT InMinX, INT InMinY, INT InMaxX, INT InMaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY, INT& OutMinX, INT& OutMinY, INT& OutMaxX, INT& OutMaxY)
{
	check(Terrain);

	FLOAT HalfX = (FLOAT)(Terrain->NumPatchesX) / 2.0f;
	FLOAT HalfY = (FLOAT)(Terrain->NumPatchesY) / 2.0f;

	FLOAT Diff;
	FLOAT Temp;

	if (bMirrorX == TRUE)
	{
		// Mirror about the X-axis
		Diff = HalfY - InMinY;
		Temp = HalfY + Diff;
		Diff = HalfY - InMaxY;
		OutMinY = HalfY + Diff;
		OutMaxY = Temp;
	}

	if (bMirrorY == TRUE)
	{
		// Mirror about the Y-axis
		Diff = HalfX - InMinX;
		Temp = HalfX + Diff;
		Diff = HalfX - InMaxX;
		OutMinX = HalfX + Diff;
		OutMaxX = Temp;
	}

	//@todo. Do we want to clamp? What about uneven terrains?
	OutMinX = Clamp(OutMinX,0,Terrain->NumVerticesX - 1);
	OutMinY = Clamp(OutMinY,0,Terrain->NumVerticesY - 1);
	OutMaxX = Clamp(OutMaxX,0,Terrain->NumVerticesX - 1);
	OutMaxY = Clamp(OutMaxY,0,Terrain->NumVerticesY - 1);
}

INT FModeTool_Terrain::FindMatchingTerrainLayer(ATerrain* TestTerrain, FTerrainToolSettings* Settings)
{
	if (Settings->CurrentTerrain == TestTerrain)
	{
		return Settings->LayerIndex;
	}

	if ((Settings->DecoLayer == TRUE) || (Settings->LayerIndex == INDEX_NONE))
	{
		return INDEX_NONE;
	}

	UTerrainLayerSetup* Setup = Settings->CurrentTerrain->Layers(Settings->LayerIndex).Setup;
	for (INT i = 0; i < TestTerrain->Layers.Num(); i++)
	{
		if (TestTerrain->Layers(i).Setup == Setup)
		{
			return i;
		}
	}

	return INDEX_NONE;        
}

UBOOL FModeTool_Terrain::TerrainIsValid(ATerrain* TestTerrain, FTerrainToolSettings* Settings)
{
	// always allow matching terrain or heightmap painting
	if ((Settings->CurrentTerrain == TestTerrain) || (Settings->LayerIndex == INDEX_NONE))
	{
		return TRUE;
	}

	// allow painting layers if there is a matching LayerSetup in the other terrain
	if (FindMatchingTerrainLayer(TestTerrain, Settings) != INDEX_NONE)
	{
		return TRUE;
	}

	return FALSE;
}

UBOOL FModeTool_Terrain::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	ModifiedTerrains.Empty();
	return TRUE;
}

void FModeTool_Terrain::EndApplyTool()
{
	for (INT i = 0; i < ModifiedTerrains.Num(); i++)
	{
		ATerrain* Terrain = ModifiedTerrains(i);
		Terrain->MarkPackageDirty();
		Terrain->WeldEdgesToOtherTerrains();
	}
	ModifiedTerrains.Empty();
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainPaint.
-----------------------------------------------------------------------------*/

FModeTool_TerrainPaint::FModeTool_TerrainPaint()
{
	ID = MT_TerrainPaint;
}

void FModeTool_TerrainPaint::ApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,FLOAT LocalMinRadius,FLOAT LocalMaxRadius,
	FLOAT InDirection,FLOAT LocalStrength,INT MinX,INT MinY,INT MaxX,INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	FEdModeTerrainEditing* EdMode = (FEdModeTerrainEditing*)(GEditorModeTools().GetCurrentMode());
	check(EdMode);

	INT MirrorX;
	INT MirrorY;
	for (INT Y = MinY;Y <= MaxY;Y++)
	{
		MirrorY = Y;
		for (INT X = MinX;X <= MaxX;X++)
		{
			MirrorX = X;
			if (bMirrorY)
			{
				MirrorX = EdMode->GetMirroredValue_X(Terrain, X);
			}
			if (bMirrorX)
			{
				MirrorY = EdMode->GetMirroredValue_Y(Terrain, Y);
			}

			if (LayerIndex == INDEX_NONE)
			{
				FVector	Vertex = Terrain->GetLocalVertex(X,Y);
				INT HeightValue = Terrain->Height(X,Y) + (LocalStrength*InDirection) * 10.0f * RadiusStrength(LocalPosition,Vertex,LocalMinRadius,LocalMaxRadius);
				WORD NewHeight = 
					(WORD)Clamp<INT>(
						HeightValue,
						0,
						MAXWORD
						);
				Terrain->Height(X,Y) = NewHeight;

				// Mirror it
				if (bMirrorY || bMirrorX)
				{
					Terrain->Height(MirrorX,MirrorY) =
						(WORD)Clamp<INT>(
							HeightValue,
							0,
							MAXWORD
							);
				}
			}
			else
			{
				FVector	Vertex = Terrain->GetLocalVertex(X,Y);
				BYTE&	Alpha = Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,X,Y);
				Alpha = (BYTE)Clamp<INT>(
							appFloor(Alpha + (LocalStrength*InDirection) * (255.0f / 100.0f) * 10.0f * RadiusStrength(LocalPosition,Vertex,LocalMinRadius,LocalMaxRadius)),
							0,
							MAXBYTE
							);

				// Mirror it
				if (bMirrorY || bMirrorX)
				{
					BYTE&	MirrorAlpha = Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,MirrorX,MirrorY);
					MirrorAlpha = Alpha;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainNoise.
-----------------------------------------------------------------------------*/

FModeTool_TerrainNoise::FModeTool_TerrainNoise()
{
	ID = MT_TerrainNoise;
}

void FModeTool_TerrainNoise::ApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,FLOAT LocalMinRadius,FLOAT LocalMaxRadius,
	FLOAT InDirection,FLOAT LocalStrength,INT MinX,INT MinY,INT MaxX,INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	FEdModeTerrainEditing*  EdMode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	check(EdMode);

	INT MirrorX;
	INT MirrorY;
	for (INT Y = MinY;Y <= MaxY;Y++)
	{
		MirrorY = Y;
		for (INT X = MinX;X <= MaxX;X++)
		{
			MirrorX = X;
			if (bMirrorY)
			{
				MirrorX = EdMode->GetMirroredValue_X(Terrain, X);
			}
			if (bMirrorX)
			{
				MirrorY = EdMode->GetMirroredValue_Y(Terrain, Y);
			}

			if (LayerIndex == INDEX_NONE)
			{
				FVector	Vertex = Terrain->GetLocalVertex(X,Y);
				WORD NewHeight = 
					(WORD)Clamp<INT>(
						appFloor(Terrain->Height(X,Y) + ((32.f-(appFrand()*64.f)) * LocalStrength) * RadiusStrength(LocalPosition,Vertex,LocalMinRadius,LocalMaxRadius)),
						0,
						MAXWORD
						);
				Terrain->Height(X,Y) = NewHeight;
				if (bMirrorX || bMirrorY)
				{
					Terrain->Height(MirrorX, MirrorY) = NewHeight;
				}
			}
			else
			{
				FVector	Vertex = Terrain->GetLocalVertex(X,Y);
				BYTE&	Alpha = Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,X,Y);
				Alpha = (BYTE)Clamp<INT>(
							appFloor(Alpha + (LocalStrength*InDirection) * (255.0f / 100.0f) * (8.f-(appFrand()*16.f)) * RadiusStrength(LocalPosition,Vertex,LocalMinRadius,LocalMaxRadius)),
							0,
							MAXBYTE
							);

				// Mirror it
				if (bMirrorY || bMirrorX)
				{
					BYTE&	MirrorAlpha = Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,MirrorX,MirrorY);
					MirrorAlpha = Alpha;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainSmooth.
-----------------------------------------------------------------------------*/

FModeTool_TerrainSmooth::FModeTool_TerrainSmooth()
{
	ID = MT_TerrainSmooth;
}

void FModeTool_TerrainSmooth::ApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,FLOAT LocalMinRadius,FLOAT LocalMaxRadius,
	FLOAT InDirection,FLOAT LocalStrength,INT MinX,INT MinY,INT MaxX,INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	FEdModeTerrainEditing*  EdMode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	check(EdMode);

	FLOAT	Filter[3][3] =
	{
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 }
	};
	FLOAT	FilterSum = 0;
	for (UINT Y = 0;Y < 3;Y++)
	{
		for (UINT X = 0;X < 3;X++)
		{
			FilterSum += Filter[X][Y];
		}
	}
	FLOAT	InvFilterSum = 1.0f / FilterSum;

	INT*	AlphaMapIndex	= NULL;
	
	if (LayerIndex != INDEX_NONE)
	{
		AlphaMapIndex = DecoLayer ? &(Terrain->DecoLayers(LayerIndex).AlphaMapIndex) : &(Terrain->Layers(LayerIndex).AlphaMapIndex);
	}

	INT MirrorX;
	INT MirrorY;
	for (INT Y = MinY;Y <= MaxY;Y++)
	{
		MirrorY = Y;
		for (INT X = MinX;X <= MaxX;X++)
		{
			MirrorX = X;
			if (bMirrorY)
			{
				MirrorX = EdMode->GetMirroredValue_X(Terrain, X);
			}
			if (bMirrorX)
			{
				MirrorY = EdMode->GetMirroredValue_Y(Terrain, Y);
			}

			if (LayerIndex == INDEX_NONE)
			{
				FVector	Vertex = Terrain->GetLocalVertex(X,Y);
				FLOAT	Height = (FLOAT)Terrain->Height(X,Y),
						SmoothHeight = 0.0f;

				for (INT AdjacentY = 0;AdjacentY < 3;AdjacentY++)
					for (INT AdjacentX = 0;AdjacentX < 3;AdjacentX++)
						SmoothHeight += Terrain->Height(X - 1 + AdjacentX,Y - 1 + AdjacentY) * Filter[AdjacentX][AdjacentY];
				SmoothHeight *= InvFilterSum;

				WORD NewHeight = 
					(WORD)Clamp<INT>(
						appFloor(Lerp(Height,SmoothHeight,Min(((LocalStrength*InDirection) / 100.0f) * RadiusStrength(LocalPosition,Vertex,LocalMinRadius,LocalMaxRadius),1.0f))),
						0,
						MAXWORD
						);
				Terrain->Height(X,Y) = NewHeight;
				if (bMirrorX || bMirrorY)
				{
					Terrain->Height(MirrorX,MirrorY) = NewHeight;
				}
			}
			else
			{
				FVector	Vertex = Terrain->GetLocalVertex(X,Y);
				FLOAT	Alpha = (FLOAT)Terrain->Alpha(*AlphaMapIndex,X,Y),
						SmoothAlpha = 0.0f;

				for (INT AdjacentY = 0;AdjacentY < 3;AdjacentY++)
					for (INT AdjacentX = 0;AdjacentX < 3;AdjacentX++)
						SmoothAlpha += Terrain->Alpha(*AlphaMapIndex,X - 1 + AdjacentX,Y - 1 + AdjacentY) * Filter[AdjacentX][AdjacentY];
				SmoothAlpha *= InvFilterSum;

				WORD NewAlpha = 
					(WORD)Clamp<INT>(
						appFloor(Lerp(Alpha,SmoothAlpha,Min(((LocalStrength*InDirection) / 100.0f) * RadiusStrength(LocalPosition,Vertex,LocalMinRadius,LocalMaxRadius),1.0f))),
						0,
						MAXBYTE
						);
				Terrain->Alpha(*AlphaMapIndex,X,Y) = NewAlpha;
				if (bMirrorX || bMirrorY)
				{
					Terrain->Alpha(*AlphaMapIndex,MirrorX,MirrorY) = NewAlpha;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainAverage.
-----------------------------------------------------------------------------*/

FModeTool_TerrainAverage::FModeTool_TerrainAverage()
{
	ID = MT_TerrainAverage;
}

void FModeTool_TerrainAverage::ApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,FLOAT LocalMinRadius,FLOAT LocalMaxRadius,
	FLOAT InDirection,FLOAT LocalStrength,INT MinX,INT MinY,INT MaxX,INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	FEdModeTerrainEditing*  EdMode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	check(EdMode);

	FLOAT	Numerator = 0.0f;
	FLOAT	Denominator = 0.0f;

	for (INT Y = MinY;Y <= MaxY;Y++)
	{
		for (INT X = MinX;X <= MaxX;X++)
		{
			FLOAT	Strength = RadiusStrength(LocalPosition,Terrain->GetLocalVertex(X,Y),LocalMinRadius,LocalMaxRadius);
			if (LayerIndex == INDEX_NONE)
			{
				Numerator += (FLOAT)Terrain->Height(X,Y) * Strength;
			}
			else
			{
				Numerator += (FLOAT)Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,X,Y) * Strength;
			}
			Denominator += Strength;
		}
	}

	FLOAT	Average = Numerator / Denominator;

	INT MirrorX;
	INT MirrorY;
	for (INT Y = MinY;Y <= MaxY;Y++)
	{
		MirrorY = Y;
		for (INT X = MinX;X <= MaxX;X++)
		{
			MirrorX = X;
			if (bMirrorY)
			{
				MirrorX = EdMode->GetMirroredValue_X(Terrain, X);
			}
			if (bMirrorX)
			{
				MirrorY = EdMode->GetMirroredValue_Y(Terrain, Y);
			}

			if (LayerIndex == INDEX_NONE)
			{
				FLOAT	Height = (FLOAT)Terrain->Height(X,Y);
				WORD NewHeight = 
					(WORD)Clamp<INT>(
						appFloor(Lerp(Height,Average,Min(1.0f,(LocalStrength*InDirection) * RadiusStrength(LocalPosition,Terrain->GetLocalVertex(X,Y),LocalMinRadius,LocalMaxRadius)))),
						0,
						MAXWORD
						);
				Terrain->Height(X,Y) = NewHeight;
				if (bMirrorX || bMirrorY)
				{
					Terrain->Height(MirrorX,MirrorY) = NewHeight;
				}
			}
			else
			{
				BYTE&	Alpha = Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,X,Y);
				Alpha =
					(BYTE)Clamp<INT>(
						appFloor(Lerp((FLOAT)Alpha,Average,Min(1.0f,(LocalStrength*InDirection) * RadiusStrength(LocalPosition,Terrain->GetLocalVertex(X,Y),LocalMinRadius,LocalMaxRadius)))),
						0,
						MAXBYTE
						);
				if (bMirrorX || bMirrorY)
				{
					Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,MirrorX,MirrorY) = Alpha;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainFlatten.
-----------------------------------------------------------------------------*/

FModeTool_TerrainFlatten::FModeTool_TerrainFlatten()
{
	ID = MT_TerrainFlatten;
}

UBOOL FModeTool_TerrainFlatten::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return FALSE;
	}

	if (LayerIndex == INDEX_NONE)
	{
		FlatValue = Terrain->Height(appTrunc(LocalPosition.X),appTrunc(LocalPosition.Y));
		FlatWorldPosition = WorldPosition;
		FlatWorldNormal = WorldNormal;
	}
	else
	{
		FlatValue = Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,appTrunc(LocalPosition.X),appTrunc(LocalPosition.Y));
		FlatWorldPosition = WorldPosition;
		FlatWorldNormal = WorldNormal;
	}

	return FModeTool_Terrain::BeginApplyTool(Terrain,
		DecoLayer,LayerIndex,MaterialIndex,
		LocalPosition,WorldPosition,WorldNormal,
		bMirrorX, bMirrorY);
}

void FModeTool_TerrainFlatten::ApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,FLOAT LocalMinRadius,FLOAT LocalMaxRadius,
	FLOAT InDirection,FLOAT LocalStrength,INT MinX,INT MinY,INT MaxX,INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	FEdModeTerrainEditing*  EdMode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	check(EdMode);
	FTerrainToolSettings*	Settings = (FTerrainToolSettings*)(EdMode->GetSettings());

	INT MirrorX;
	INT MirrorY;
	for (INT Y = MinY;Y <= MaxY;Y++)
	{
		MirrorY = Y;
		for (INT X = MinX;X <= MaxX;X++)
		{
			MirrorX = X;
			if (bMirrorY)
			{
				MirrorX = EdMode->GetMirroredValue_X(Terrain, X);
			}
			if (bMirrorX)
			{
				MirrorY = EdMode->GetMirroredValue_Y(Terrain, Y);
			}

			if (RadiusStrength(LocalPosition,Terrain->GetLocalVertex(X,Y),LocalMinRadius,LocalMaxRadius) > 0.0f)
			{
				if (LayerIndex == INDEX_NONE)
				{
					if (Settings->FlattenAngle)
					{
						FVector CurWorldPosition = Terrain->GetWorldVertex(X,Y);
						// P is local vertex projected down
						FVector P = FVector(CurWorldPosition.X, CurWorldPosition.Y, FlatWorldPosition.Z);
						FVector PW = (FlatWorldPosition-P);
						FVector PWn = PW.SafeNormal();
						FLOAT Alpha = appAcos(FlatWorldNormal | PWn);
						FLOAT Theta = (0.5f * PI) - Alpha;
						FVector NewWorldPosition = FVector(CurWorldPosition.X, CurWorldPosition.Y, appTan(Theta) * PW.Size() + FlatWorldPosition.Z);
						FLOAT NewHeight = (Terrain->WorldToLocal().TransformFVector(NewWorldPosition).Z / TERRAIN_ZSCALE) + 32768.0f;
						WORD ClampedHeight = (WORD)Clamp<INT>(appFloor(NewHeight), 0, MAXWORD);
						Terrain->Height(X,Y) = ClampedHeight;
						if (bMirrorX || bMirrorY)
						{
							Terrain->Height(MirrorX,MirrorY) = ClampedHeight;
						}
					}
					else
					if (Settings->UseFlattenHeight)
					{
						FVector CurWorldPosition = Terrain->GetWorldVertex(X,Y);
						FVector NewWorldPosition = FVector(CurWorldPosition.X, CurWorldPosition.Y, Settings->FlattenHeight);
						FLOAT NewHeight = (Terrain->WorldToLocal().TransformFVector(NewWorldPosition).Z / TERRAIN_ZSCALE) + 32768.0f;
						WORD ClampedHeight = (WORD)Clamp<INT>(appFloor(NewHeight), 0, MAXWORD);
						Terrain->Height(X,Y) = ClampedHeight;
						if (bMirrorX || bMirrorY)
						{
							Terrain->Height(MirrorX,MirrorY) = ClampedHeight;
						}
					}
					else
					{
						WORD ClampedHeight = (WORD)Clamp<INT>(appFloor(FlatValue), 0, MAXWORD);
						Terrain->Height(X,Y) = ClampedHeight;
						if (bMirrorX || bMirrorY)
						{
							Terrain->Height(MirrorX,MirrorY) = ClampedHeight;
						}
					}
				}
				else
				{
					BYTE Alpha = (BYTE)Clamp<INT>(appFloor(FlatValue), 0, MAXBYTE);
					Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,X,Y) = Alpha;
					if (bMirrorX || bMirrorY)
					{
						Terrain->Alpha(DecoLayer ? Terrain->DecoLayers(LayerIndex).AlphaMapIndex : Terrain->Layers(LayerIndex).AlphaMapIndex,MirrorX,MirrorY) = Alpha;
					}
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainVisibility.
-----------------------------------------------------------------------------*/
FModeTool_TerrainVisibility::FModeTool_TerrainVisibility()
{
	ID = MT_TerrainVisibility;
}

void FModeTool_TerrainVisibility::ApplyTool(ATerrain* Terrain, UBOOL DecoLayer, INT LayerIndex, 
	INT MaterialIndex, const FVector& LocalPosition, FLOAT LocalMinRadius, FLOAT LocalMaxRadius, 
	FLOAT InDirection, FLOAT LocalStrength, INT MinX, INT MinY, INT MaxX, INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	UBOOL bVisible = (InDirection < 0.0f);

	// Tag the 'quad' the tool is in
	INT StartX = MinX - (MinX % Terrain->MaxTesselationLevel);
	INT StartY = MinY - (MinY % Terrain->MaxTesselationLevel);
	INT EndX = MaxX - (MaxX % Terrain->MaxTesselationLevel);
	INT EndY = MaxY - (MaxY % Terrain->MaxTesselationLevel);

	if (EndX < StartX + Terrain->MaxTesselationLevel)
	{
		EndX += Terrain->MaxTesselationLevel;
	}
	if (EndY < StartY + Terrain->MaxTesselationLevel)
	{
		EndY += Terrain->MaxTesselationLevel;
	}

	if (bMirrorX || bMirrorY)
	{
		debugf(TEXT("\n"));
		debugf(TEXT("Visibility: MinX,Y   = %4d,%4d - MaxX,Y = %4d,%4d"), MinX, MinY, MaxX, MaxY);
		debugf(TEXT("            StartX,Y = %4d,%4d - EndX,Y = %4d,%4d"), StartX, StartY, EndX, EndY);
	}

	MinX = StartX;
	MinY = StartY;
	MaxX = EndX;
	MaxY = EndY;

	FEdModeTerrainEditing*  EdMode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	check(EdMode);

	INT MirrorX;
	INT MirrorY;
	for (INT Y = MinY;Y < MaxY;Y++)
	{
		MirrorY = Y;
		for (INT X = MinX;X < MaxX;X++)
		{
			MirrorX = X;
			if (bMirrorY)
			{
				MirrorX = EdMode->GetMirroredValue_X(Terrain, X, TRUE);
			}
			if (bMirrorX)
			{
				MirrorY = EdMode->GetMirroredValue_Y(Terrain, Y, TRUE);
			}

			FTerrainInfoData* InfoData = Terrain->GetInfoData(X, Y);
			if (InfoData)
			{
				InfoData->SetIsVisible(bVisible);
			}
			if (bMirrorX || bMirrorY)
			{
				InfoData = Terrain->GetInfoData(MirrorX, MirrorY);
				if (InfoData)
				{
					InfoData->SetIsVisible(bVisible);
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainTexturePan.
-----------------------------------------------------------------------------*/
FModeTool_TerrainTexturePan::FModeTool_TerrainTexturePan()
{
	ID = MT_TerrainTexturePan;
}

UBOOL FModeTool_TerrainTexturePan::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if ((Terrain->bLocked == TRUE) || (LayerIndex == INDEX_NONE))
	{
		return FALSE;
	}

	// Store off the local position
	LastPosition = LocalPosition;

	return FModeTool_Terrain::BeginApplyTool(Terrain,
		DecoLayer,LayerIndex,MaterialIndex,
		LocalPosition,WorldPosition,WorldNormal,
		bMirrorX, bMirrorY);
}

void FModeTool_TerrainTexturePan::ApplyTool(ATerrain* Terrain, UBOOL DecoLayer, INT LayerIndex,
	INT MaterialIndex, const FVector& LocalPosition, FLOAT LocalMinRadius, FLOAT LocalMaxRadius, 
	FLOAT InDirection, FLOAT LocalStrength, INT MinX, INT MinY, INT MaxX, INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	if ((LayerIndex == INDEX_NONE) || (DecoLayer == TRUE))
	{
		// Can't pan the height map or the decoration layers...
		return;
	}

	if (LayerIndex >= Terrain->Layers.Num())
	{
		// Invalid layer index.
		return;
	}

	// Determine the selected layer and/or material
	FTerrainLayer* Layer = &(Terrain->Layers(LayerIndex));
	FTerrainFilteredMaterial* Material;
	if ((Layer == NULL) || (Layer->Setup == NULL))
	{
		return;
	}

	FVector Diff = LocalPosition - LastPosition;

	if (MaterialIndex == -1)
	{
		// Pan each material in the layer
		for (INT MtlIndex = 0; MtlIndex < Layer->Setup->Materials.Num(); MtlIndex++)
		{
			Material = &(Layer->Setup->Materials(MtlIndex));
			if (Material && Material->Material)
			{
				UTerrainMaterial* TerrainMat = Material->Material;

				TerrainMat->MappingPanU += (Diff.X / 10.0f) * LocalStrength;
				TerrainMat->MappingPanV += (Diff.Y / 10.0f) * LocalStrength;
			}
		}

		//@todo. Determine a cheaper way to do this... 
		Terrain->PostEditChange(NULL);
	}
	else
	{
		if (MaterialIndex < Layer->Setup->Materials.Num())
		{
			// Pan only this material
			Material = &(Layer->Setup->Materials(MaterialIndex));
			if (Material && Material->Material)
			{
				UTerrainMaterial* TerrainMat = Material->Material;

				TerrainMat->MappingPanU += (Diff.X / 10.0f) * LocalStrength;
				TerrainMat->MappingPanV += (Diff.Y / 10.0f) * LocalStrength;

				//@todo. Determine a cheaper way to do this... 
				Terrain->PostEditChange(NULL);
			}
		}
	}

	LastPosition = LocalPosition;
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainTextureRotate.
-----------------------------------------------------------------------------*/
FModeTool_TerrainTextureRotate::FModeTool_TerrainTextureRotate()
{
	ID = MT_TerrainTextureRotate;
}

UBOOL FModeTool_TerrainTextureRotate::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	// Store off the local position
	LastPosition = LocalPosition;

	return FModeTool_Terrain::BeginApplyTool(Terrain,
		DecoLayer,LayerIndex,MaterialIndex,
		LocalPosition,WorldPosition,WorldNormal,
		bMirrorX, bMirrorY);
}

void FModeTool_TerrainTextureRotate::ApplyTool(ATerrain* Terrain, UBOOL DecoLayer, INT LayerIndex,
	INT MaterialIndex, const FVector& LocalPosition, FLOAT LocalMinRadius, FLOAT LocalMaxRadius, 
	FLOAT InDirection, FLOAT LocalStrength, INT MinX, INT MinY, INT MaxX, INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	if ((LayerIndex == INDEX_NONE) || (DecoLayer == TRUE))
	{
		// Can't rotate the height map or the decoration layers...
		return;
	}

	if (LayerIndex >= Terrain->Layers.Num())
	{
		// Invalid layer index.
		return;
	}

	// Determine the selected layer and/or material
	FTerrainLayer* Layer = &(Terrain->Layers(LayerIndex));
	FTerrainFilteredMaterial* Material;
	if ((Layer == NULL) || (Layer->Setup == NULL))
	{
		return;
	}

	FVector Diff = LocalPosition - LastPosition;

	if (MaterialIndex == -1)
	{
		// Rotate each material in the layer
		for (INT MtlIndex = 0; MtlIndex < Layer->Setup->Materials.Num(); MtlIndex++)
		{
			Material = &(Layer->Setup->Materials(MtlIndex));
			if (Material && Material->Material)
			{
				UTerrainMaterial* TerrainMat = Material->Material;

				TerrainMat->MappingRotation += (Diff.X / 10.0f) * LocalStrength;
			}
		}
		//@todo. Determine a cheaper way to do this... 
		Terrain->PostEditChange(NULL);
	}
	else
	{
		if (MaterialIndex < Layer->Setup->Materials.Num())
		{
			// Rotate only this material
			Material = &(Layer->Setup->Materials(MaterialIndex));
			if (Material && Material->Material)
			{
				UTerrainMaterial* TerrainMat = Material->Material;

				TerrainMat->MappingRotation += (Diff.X / 10.0f) * LocalStrength;

				//@todo. Determine a cheaper way to do this... 
				Terrain->PostEditChange(NULL);
			}
		}
	}

	LastPosition = LocalPosition;
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainTextureScale.
-----------------------------------------------------------------------------*/
FModeTool_TerrainTextureScale::FModeTool_TerrainTextureScale()
{
	ID = MT_TerrainTextureScale;
}

UBOOL FModeTool_TerrainTextureScale::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	// Store off the local position
	LastPosition = LocalPosition;

	return FModeTool_Terrain::BeginApplyTool(Terrain,
		DecoLayer,LayerIndex,MaterialIndex,
		LocalPosition,WorldPosition,WorldNormal,
		bMirrorX, bMirrorY);
}

void FModeTool_TerrainTextureScale::ApplyTool(ATerrain* Terrain, UBOOL DecoLayer, INT LayerIndex,
	INT MaterialIndex, const FVector& LocalPosition, FLOAT LocalMinRadius, FLOAT LocalMaxRadius, 
	FLOAT InDirection, FLOAT LocalStrength, INT MinX, INT MinY, INT MaxX, INT MaxY, 
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return;
	}

	if ((LayerIndex == INDEX_NONE) || (DecoLayer == TRUE))
	{
		// Can't scale the height map or the decoration layers...
		return;
	}

	if (LayerIndex >= Terrain->Layers.Num())
	{
		// Invalid layer index.
		return;
	}

	// Determine the selected layer and/or material
	FTerrainLayer* Layer = &(Terrain->Layers(LayerIndex));
	FTerrainFilteredMaterial* Material;
	if ((Layer == NULL) || (Layer->Setup == NULL))
	{
		return;
	}

	FVector Diff = LocalPosition - LastPosition;

	if (MaterialIndex == -1)
	{
		// Scale each material in the layer
		for (INT MtlIndex = 0; MtlIndex < Layer->Setup->Materials.Num(); MtlIndex++)
		{
			Material = &(Layer->Setup->Materials(MtlIndex));
			if (Material && Material->Material)
			{
				UTerrainMaterial* TerrainMat = Material->Material;

				TerrainMat->MappingScale += (Diff.X / 10.0f) * LocalStrength;
			}
		}
		//@todo. Determine a cheaper way to do this... 
		Terrain->PostEditChange(NULL);
	}
	else
	{
		if (MaterialIndex < Layer->Setup->Materials.Num())
		{
			// Scale only this material
			Material = &(Layer->Setup->Materials(MaterialIndex));
			if (Material && Material->Material)
			{
				UTerrainMaterial* TerrainMat = Material->Material;

				TerrainMat->MappingScale += (Diff.X / 10.0f) * LocalStrength;

		//@todo. Determine a cheaper way to do this... 
				Terrain->PostEditChange(NULL);
			}
		}
	}

	LastPosition = LocalPosition;
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainSplitX.
-----------------------------------------------------------------------------*/
UBOOL FModeTool_TerrainSplitX::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (appMsgf(AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_SplitTerrain"))))
	{
		INT X = appRound(LocalPosition.X);
		X -= (X % Terrain->MaxTesselationLevel);
		Terrain->SplitTerrain(TRUE, X);
	}

	return FALSE;
}

UBOOL FModeTool_TerrainSplitX::Render(ATerrain* Terrain, const FVector HitLocation,FPrimitiveDrawInterface* PDI)
{
	INT X = appRound(Terrain->WorldToLocal().TransformFVector(HitLocation).X);
	X -= (X % Terrain->MaxTesselationLevel);
	if (X <= 0 || X >= Terrain->NumPatchesX-1)
	{
		return TRUE;
	}

	// Clamp it to the MaxTessellationLevel to prevent invalid sizes
	Terrain->SplitTerrainPreview(PDI, TRUE, X);
	return FALSE;
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainSplitY.
-----------------------------------------------------------------------------*/
UBOOL FModeTool_TerrainSplitY::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (appMsgf(AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_SplitTerrain"))))
	{
		INT Y = appRound(LocalPosition.Y);
		Y -= (Y % Terrain->MaxTesselationLevel);
		Terrain->SplitTerrain(FALSE, Y);
	}

	return FALSE;
}

UBOOL FModeTool_TerrainSplitY::Render(ATerrain* Terrain, const FVector HitLocation,FPrimitiveDrawInterface* PDI)
{
	INT Y = appRound(Terrain->WorldToLocal().TransformFVector(HitLocation).Y);
	Y -= (Y % Terrain->MaxTesselationLevel);
	if (Y <= 0 || Y >= Terrain->NumPatchesY-1)
	{
		return TRUE;
	}
	Terrain->SplitTerrainPreview(PDI, FALSE, Y);
	return FALSE;
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainMerge.
-----------------------------------------------------------------------------*/
UBOOL FModeTool_TerrainMerge::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	FEdModeTerrainEditing*  Mode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	for (INT i=0;i<Mode->ToolTerrains.Num();i++)
	{
		for (INT j=0;j<Mode->ToolTerrains.Num();j++)
		{
			if (i != j)
			{
				ATerrain* Terrain1 = Mode->ToolTerrains(i);
				ATerrain* Terrain2 = Mode->ToolTerrains(j);
				if (Terrain1 && Terrain2)
				{
					if (Terrain1->MergeTerrainPreview(NULL, Terrain2))
					{
						if (appMsgf(AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_MergeTerrain"))))
						{
							Terrain1->MergeTerrain(Terrain2);
						}
						return FALSE;
					}
				}
			}
		}
	}


	return FALSE;
}

UBOOL FModeTool_TerrainMerge::Render(ATerrain* Terrain, const FVector HitLocation,FPrimitiveDrawInterface* PDI)
{
	FEdModeTerrainEditing*  Mode = (FEdModeTerrainEditing*)GEditorModeTools().GetCurrentMode();
	for (INT i=0;i<Mode->ToolTerrains.Num();i++)
	{
		for (INT j=0;j<Mode->ToolTerrains.Num();j++)
		{
			if (i != j)
			{
				ATerrain* Terrain1 = Mode->ToolTerrains(i);
				ATerrain* Terrain2 = Mode->ToolTerrains(j);
				if (Terrain1 && Terrain2)
				{
					if (Terrain1 == Terrain2)
					{
						debugf(TEXT("Why is the same terrain in here 2x???"));
					}
					if (Terrain1->MergeTerrainPreview(PDI, Terrain2))
						return TRUE;
				}
			}
		}
	}

	return TRUE;
}

/*-----------------------------------------------------------------------------
	FModeTool_TerrainAddRemoveSectors.
-----------------------------------------------------------------------------*/
/**
 * For adding/removing Sectors to existing terrain
 */
UBOOL FModeTool_TerrainAddRemoveSectors::BeginApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,const FVector& WorldPosition, const FVector& WorldNormal,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	if (Terrain->bLocked == TRUE)
	{
		return FALSE;
	}

	CurrentTerrain = Terrain;
	// Store off the local position and direction
	StartPosition = LocalPosition;
	CurrPosition = LocalPosition;

	return FModeTool_Terrain::BeginApplyTool(Terrain,
		DecoLayer,LayerIndex,MaterialIndex,
		LocalPosition,WorldPosition,WorldNormal,
		bMirrorX, bMirrorY);
}

void FModeTool_TerrainAddRemoveSectors::ApplyTool(ATerrain* Terrain,
	UBOOL DecoLayer,INT LayerIndex,INT MaterialIndex,
	const FVector& LocalPosition,FLOAT LocalMinRadius,FLOAT LocalMaxRadius,
	FLOAT InDirection,FLOAT LocalStrength,INT MinX,INT MinY,INT MaxX,INT MaxY,
	UBOOL bMirrorX, UBOOL bMirrorY)
{
	CurrentTerrain = Terrain;
	// Store off the local position and direction
	CurrPosition = LocalPosition;
	Direction = InDirection;
}

void FModeTool_TerrainAddRemoveSectors::EndApplyTool()
{
	if (abs(Direction) < KINDA_SMALL_NUMBER)
	{
		return;
	}
	
	FVector PosDiff = CurrPosition - StartPosition;
	PosDiff.X = (FLOAT)(appTrunc(PosDiff.X) - (appTrunc(PosDiff.X) % CurrentTerrain->MaxTesselationLevel));
	PosDiff.Y = (FLOAT)(appTrunc(PosDiff.Y) - (appTrunc(PosDiff.Y) % CurrentTerrain->MaxTesselationLevel));

	FTerrainToolSettings* TTSettings = (FTerrainToolSettings*)Settings;
	if (TTSettings->MirrorSetting == FTerrainToolSettings::TTMirror_X)
	{
		PosDiff.Y = 0.0f;
	}
	if (TTSettings->MirrorSetting == FTerrainToolSettings::TTMirror_Y)
	{
		PosDiff.X = 0.0f;
	}

	UBOOL bRemoving = Direction < 0.0f;
	UBOOL bAddBottom = PosDiff.X < 0.0f ? TRUE : FALSE;
	UBOOL bAddTop = PosDiff.X > 0.0f ? TRUE : FALSE;
	UBOOL bAddLeft = PosDiff.Y < 0.0f ? TRUE : FALSE;
	UBOOL bAddRight = PosDiff.Y > 0.0f ? TRUE : FALSE;

	INT CountX = 0;
	INT CountY = 0;
	if (bAddLeft || bAddRight)
	{
		CountY = appTrunc(PosDiff.Y) / CurrentTerrain->MaxTesselationLevel;
		if (bRemoving)
		{
			CountY *= -1;
		}
	}
	if (bAddTop || bAddBottom)
	{
		CountX = appTrunc(PosDiff.X) / CurrentTerrain->MaxTesselationLevel;
		if (bRemoving)
		{
			CountX *= -1;
		}
	}

	CurrentTerrain->AddRemoveSectors(CountX, CountY, bRemoving);

	Direction = 0.0f;
}

UBOOL FModeTool_TerrainAddRemoveSectors::Render(ATerrain* Terrain, const FVector HitLocation,FPrimitiveDrawInterface* PDI )
{
	if (abs(Direction) < KINDA_SMALL_NUMBER)
	{
		return FALSE;
	}

	FVector PosDiff = CurrPosition - StartPosition;
	PosDiff.X = (FLOAT)(appTrunc(PosDiff.X) - (appTrunc(PosDiff.X) % Terrain->MaxTesselationLevel));
	PosDiff.Y = (FLOAT)(appTrunc(PosDiff.Y) - (appTrunc(PosDiff.Y) % Terrain->MaxTesselationLevel));

	FTerrainToolSettings* TTSettings = (FTerrainToolSettings*)Settings;
	if (TTSettings->MirrorSetting == FTerrainToolSettings::TTMirror_X)
	{
		PosDiff.Y = 0.0f;
	}
	if (TTSettings->MirrorSetting == FTerrainToolSettings::TTMirror_Y)
	{
		PosDiff.X = 0.0f;
	}

	UBOOL bRemoving = Direction < 0.0f;
	UBOOL bAddBottom = PosDiff.X < 0.0f ? TRUE : FALSE;
	UBOOL bAddTop	 = PosDiff.X > 0.0f ? TRUE : FALSE;
	UBOOL bAddLeft	 = PosDiff.Y < 0.0f ? TRUE : FALSE;
	UBOOL bAddRight  = PosDiff.Y > 0.0f ? TRUE : FALSE;

	FVector Left, Right, Top, Bottom;

	if ((bAddLeft && !bRemoving) || (bAddRight && bRemoving))
	{
		Left = Terrain->GetWorldVertex(0, 0);
		Right = Terrain->GetWorldVertex(Terrain->NumVerticesX - 1, 0);
	}
	else
	if ((bAddRight && !bRemoving) || (bAddLeft && bRemoving))
	{
		Left = Terrain->GetWorldVertex(0, Terrain->NumVerticesY - 1);
		Right = Terrain->GetWorldVertex(Terrain->NumVerticesX - 1, Terrain->NumVerticesY - 1);
	}

	if ((bAddTop && !bRemoving) || (bAddBottom && bRemoving))
	{
		Top = Terrain->GetWorldVertex(Terrain->NumVerticesX - 1, Terrain->NumVerticesY - 1);
		Bottom = Terrain->GetWorldVertex(Terrain->NumVerticesX - 1, 0);
	}
	else
	if ((bAddBottom && !bRemoving) || (bAddTop && bRemoving))
	{
		Top = Terrain->GetWorldVertex(0, Terrain->NumVerticesY - 1);
		Bottom = Terrain->GetWorldVertex(0, 0);
	}

	FLinearColor LineColor = bRemoving ? FLinearColor(1.0f, 0.0f, 0.0f) : FLinearColor(0.0f, 1.0f, 0.0f);
	FLOAT ScaleX = Terrain->DrawScale * Terrain->DrawScale3D.X;
	FLOAT ScaleY = Terrain->DrawScale * Terrain->DrawScale3D.Y;
	FVector AddX = FVector(appTrunc(PosDiff.X) * ScaleX, 0.0f, 0.0f);
	FVector AddY = FVector(0.0f, appTrunc(PosDiff.Y) * ScaleY, 0.0f);
	FVector StepX = FVector(Terrain->MaxTesselationLevel * ScaleX, 0.0f, 0.0f);
	if (PosDiff.X < 0.0f)
	{
		StepX.X *= -1.0f;
	}
	FVector StepY = FVector(0.0f, Terrain->MaxTesselationLevel * ScaleY, 0.0f);
	if (PosDiff.Y < 0.0f)
	{
		StepY.Y *= -1.0f;
	}

	if (bAddLeft || bAddRight)
	{
		PDI->DrawLine(Left, Left + AddY, LineColor, SDPG_Foreground);
		PDI->DrawLine(Right, Right + AddY, LineColor, SDPG_Foreground);
		for (INT Step = 0; Step <= (appTrunc(abs(PosDiff.Y)) / Terrain->MaxTesselationLevel); Step++)
		{
			PDI->DrawLine(Left + StepY * Step, Right + StepY * Step, LineColor, SDPG_Foreground);
		}
	}

	if (bAddTop || bAddBottom)
	{
		PDI->DrawLine(Top, Top + AddX, LineColor, SDPG_Foreground);
		PDI->DrawLine(Bottom, Bottom + AddX, LineColor, SDPG_Foreground);
		for (INT Step = 0; Step <= (appTrunc(abs(PosDiff.X)) / Terrain->MaxTesselationLevel); Step++)
		{
			PDI->DrawLine(Top + StepX * Step, Bottom + StepX * Step, LineColor, SDPG_Foreground);
		}
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
	UTerrainMaterialFactoryNew implementation.
------------------------------------------------------------------------------*/

//
//	UTerrainMaterialFactoryNew::StaticConstructor
//

void UTerrainMaterialFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(TEXT("Object"));
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTerrainMaterialFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UTerrainMaterial::StaticClass();
	bCreateNew			= 1;
	Description			= TEXT("TerrainMaterial");
}
//
//	UTerrainMaterialFactoryNew::FactoryCreateNew
//

UObject* UTerrainMaterialFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UTerrainMaterialFactoryNew);

/*------------------------------------------------------------------------------
	UTerrainLayerSetupFactoryNew implementation.
------------------------------------------------------------------------------*/

//
//	UTerrainLayerSetupFactoryNew::StaticConstructor
//

void UTerrainLayerSetupFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(TEXT("Object"));
}


/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTerrainLayerSetupFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UTerrainLayerSetup::StaticClass();
	bCreateNew			= 1;
	Description			= TEXT("TerrainLayerSetup");
}

//
//	UTerrainLayerSetupFactoryNew::FactoryCreateNew
//

UObject* UTerrainLayerSetupFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UTerrainLayerSetupFactoryNew);
