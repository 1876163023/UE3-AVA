#include "avaGame.h"
#include "UnNet.h"
#include "avaShatterGlass.h"
#include "LevelUtils.h"
#if WITH_NOVODEX
#include "../../engine/src/UnNovodexSupport.h"
#endif // WITH_NOVODEX

#include "EngineMaterialClasses.h"

#include "avaShatterGlassRender.h"


#define WINDOW_PANEL_SIZE			12
#define WINDOW_SMALL_SHARD_SIZE		4
#define WINDOW_LARGE_SHARD_SIZE		7
#define WINDOW_MAX_SUPPORT			6.75
#define WINDOW_BREAK_SUPPORT		0.20
#define WINDOW_PANE_BROKEN			-1

IMPLEMENT_CLASS(UavaShatterGlassComponent);
IMPLEMENT_CLASS( AavaShatterGlassActor )

FPrimitiveSceneProxy* UavaShatterGlassComponent::CreateSceneProxy()
{
	FShatterGlassSceneProxy* NewProxy = ::new FShatterGlassSceneProxy(this);
	check (NewProxy);

	UpdateDynamicData(NewProxy);
	//	NewProxy->UpdateData(CachedDynamicData);

	// 
	return NewProxy;
}

void UavaShatterGlassComponent::UpdateDynamicData( FShatterGlassSceneProxy* Proxy )
{
	if (Proxy)
	{
		FShatterGlassRenderData* RenderData = new FShatterGlassRenderData( RenderContext );

		Proxy->UpdateData( RenderData );
	}
}

void UavaShatterGlassComponent::UpdateDynamicData()
{
	FShatterGlassSceneProxy* SceneProxy = (FShatterGlassSceneProxy*)Scene_GetProxyFromInfo(SceneInfo);
	UpdateDynamicData(SceneProxy);
}

INT* AavaShatterGlassActor::GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel)
{
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);

	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	if (bNetDirty)
	{
		//DOREP(avaShatterGlassActor,AllBroken);
		//DOREP(avaShatterGlassActor,NumEdgeUpdates);
		DOREPARRAY(avaShatterGlassActor,PanelBitmap);		
	}

	return Ptr;
}

void AavaShatterGlassActor::OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData)
{
	if (Info0.Actor == this)
	{
		ShatterGlassComponent->SurfaceTouch( Info1.Actor );
	}
	else
	{
		ShatterGlassComponent->SurfaceTouch( Info0.Actor );
	}
}

UBOOL UavaShatterGlassComponent::HasUnlitDistortion()
{
	UMaterial* Material = this->Material ? this->Material->GetMaterial() : NULL;
	return ( Material && Material->LightingModel == MLM_Unlit && Material->HasDistortion() );	
}

/** Returns true if the prim is using a material with unlit translucency */
UBOOL UavaShatterGlassComponent::HasUnlitTranslucency()
{
	UMaterial* Material = this->Material ? this->Material->GetMaterial() : NULL;
	return (Material && (Material->LightingModel == MLM_Unlit) && IsTranslucentBlendMode((EBlendMode)Material->BlendMode) );
}

void UavaShatterGlassComponent::UpdateBounds()
{
	FBox BoundingBox;
	BoundingBox.Init();

	FVector Origin = LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);
	FVector Z = LocalToWorld.GetAxis(2);		

	BoundingBox += Origin;
	BoundingBox += Origin + X * NumWide;
	BoundingBox += Origin + Z * NumHigh;
	BoundingBox += Origin + X * NumWide + Z * NumHigh;		

	Bounds = FBoxSphereBounds(BoundingBox);	
}



//void UavaShatterGlassComponent::Render(const FSceneContext& Context,FPrimitiveRenderInterface* PRI)
//{
//	PrepareRenderer();
//
//	UpdateSolidRenderList();
//
//	// Try to find a color for level coloration.
//	FColor* LevelColor = NULL;
//	if ( Context.View->ShowFlags & SHOW_LevelColoration )
//	{
//		if ( Owner )
//		{
//			ULevel* Level = Owner->GetLevel();
//			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
//			if ( LevelStreaming )
//			{
//				LevelColor = &LevelStreaming->DrawColor;
//			}
//		}
//	}	
//
//	if (!(Context.View->ViewMode & SVM_WireframeMask))
//		Renderer->Render( Context, PRI, LevelColor );
//
//	if (Context.View->ShowFlags & SHOW_Editor)
//	{
//		for (INT i=0; i<SolidRenderList.Num(); ++i)
//		{
//			const FSolidRenderInfo& sri = SolidRenderList(i);
//			RenderOneBlock( Context, PRI, sri.Base, sri.X, sri.Z );
//		}
//	}
//	
//	
//	//RenderEdges( Context, PRI );
//}
//
//void UavaShatterGlassComponent::RenderEdges(const FSceneContext& Context,FPrimitiveRenderInterface* PRI)
//{
//	FVector Origin = LocalToWorld.GetOrigin();
//	FVector X = LocalToWorld.GetAxis(0);
//	FVector Z = LocalToWorld.GetAxis(2);				
//
//	for (INT i=0; i<EdgeRenderList.Num(); ++i)
//	{		
//		const FEdgeRenderInfo& eri = EdgeRenderList(i);
//		FVector Base = Origin + (eri.x*X) + (eri.z*Z);
//
//		FColor LineColor = FColor( 128, 255, 128 );
//
//		PRI->DrawLine( Base, Base + Z, LineColor, SDPG_Foreground );				
//		PRI->DrawLine( Base, Base + X, LineColor, SDPG_Foreground );
//		PRI->DrawLine( Base + X, Base + X + Z, LineColor, SDPG_Foreground );				
//		PRI->DrawLine( Base + Z, Base + X + Z, LineColor, SDPG_Foreground );		
//	}
//}

void UavaShatterGlassComponent::UpdateSolidRenderList()
{	
	int x = 0;
	if (x)
	{
		UpdateEdges();
	}

	SolidRenderList.Empty();	

	NumSolidTrisToRender = 0;	

	FVector Origin = LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);
	FVector Z = LocalToWorld.GetAxis(2);			

	checkSlow(RenderContext != NULL);

	RenderContext->Reset();	

	for (INT i=0; i<EdgeRenderList.Num(); ++i)
	{		
		const FEdgeRenderInfo& eri = EdgeRenderList(i);

		RenderContext->AddEdge( eri.EdgeType, eri.Style, eri.x, eri.z, (WinSide_t)eri.Side );		
	}

	FVector Base = Origin;
	for (int width=0;width<NumWide;width++)
	{
		int height;
		int nHCount = 0;
		for (height=0;height<NumHigh;height++)
		{
			// Keep count of how many panes there are in a row
			if (IsPanelSolid(width,height))
			{
				nHCount++;
			}

			// Drow the strip and start counting again
			else if (nHCount > 0)
			{
				Base = Origin + X * width + Z * (height-nHCount);				

				RenderContext->AddSolidBlock( width, height, nHCount );
				new (SolidRenderList) FSolidRenderInfo( Base, X, Z*nHCount );				
				nHCount = 0;

				NumSolidTrisToRender += 2;
			}
		}
		if (nHCount)
		{
			Base = Origin + X * width + Z * (height-nHCount);							

			RenderContext->AddSolidBlock( width, height, nHCount );			
			new (SolidRenderList) FSolidRenderInfo( Base, X, Z*nHCount );

			NumSolidTrisToRender += 2;
		}
	}
}

//void UavaShatterGlassComponent::RenderOneBlock(const FSceneContext& Context,FPrimitiveRenderInterface* PRI,const FVector& Base,const FVector& X,const FVector& Z)
//{
//	FColor LineColor = FColor( 255, 255, 128 );
//
//	PRI->DrawLine( Base, Base + Z, LineColor, InDepthPriorityGroup );				
//	PRI->DrawLine( Base, Base + X, LineColor, SDPG_Foreground );
//	PRI->DrawLine( Base + X, Base + X + Z, LineColor, SDPG_Foreground );				
//	PRI->DrawLine( Base + Z, Base + X + Z, LineColor, SDPG_Foreground );
//}

void UavaShatterGlassComponent::Precache()
{
	//	check(GRenderDevice);
}

UBOOL UavaShatterGlassComponent::IsValidComponent() const
{
	return NumWide > 0 && NumHigh > 0 && NumWide <= MAX_NUM_PANELS && NumHigh <= MAX_NUM_PANELS;
}

void UavaShatterGlassComponent::BreakAll()
{
	int width;
	for (width=0;width<NumWide;width++)
	{
		for (int height=0;height<NumHigh;height++)
		{
			SetPanelSolid(width,height,false);
		}
	}
	for (width=0;width<NumWide;width++)
	{
		for (int height=0;height<NumHigh;height++)
		{
			UpdateEdgeType(width,height);
		}
	}
}

void UavaShatterGlassComponent::BreakBitmap( INT i, UINT diff )
{
	UINT m = 1;

	for (INT j=0; j<32 && diff != 0; ++j)
	{
		if ((diff & m) != 0)
		{
			Break( j & 15, i * 2 + ((j < 16) ? 0 : 1) );
			diff = diff & (~m);
		}

		m = m << 1;
	}
}

void UavaShatterGlassComponent::Break( int x, int z )
{
	Die();

	if (IsBroken( x, z ))
		return;

	Support[x][z] = WINDOW_PANE_BROKEN;

	Cast<AavaShatterGlassActor>( Owner )->eventClientPlaySound( CoordToWorld( x, z ) );

	//debugf( TEXT("avaShatterGlassComponent breaks %d %d"), x, z );
	checkSlow(InLegalRange(x,z));

	SetPanelSolid(x,z,false);

	// Mark these panels and being stale (need edge type updated)
	// We update them in one fell swoop rather than as each panel
	// is updated, so we don't have to do duplicate operations
	SetPanelStale(x,	z  ,true);
	SetPanelStale(x,   z+1,true);
	SetPanelStale(x,   z-1,true);
	SetPanelStale(x-1, z  ,true);
	SetPanelStale(x+1, z  ,true);
	SetPanelStale(x+1, z+1,true);
	SetPanelStale(x-1, z+1,true);
	SetPanelStale(x+1, z-1,true);
	SetPanelStale(x-1, z-1,true);
}

void UavaShatterGlassComponent::UpdateEdges()
{
	for (int width=0;width<NumWide;width++)
	{
		for (int height=0;height<NumHigh;height++)
		{
			if (IsPanelStale(width,height))
			{
				UpdateEdgeType(width,height);
			}
		}
	}	

	bSolidRenderListUpdated = false;
}

void UavaShatterGlassComponent::UpdateEdgeType(int x, int z, int forceStyle /*=-1*/ )
{
	checkSlow( forceStyle < NUM_EDGE_STYLES );

	// -----------------------------------
	//  Check edge conditions
	// -----------------------------------
	if (!InLegalRange(x,z))
	{
		return;
	}

	// ----------------------------------
	//  If solid has no edges
	// ----------------------------------
	if (IsPanelSolid(x,z))
	{
		return;
	}

	// Panel is no longer stale
	SetPanelStale(x, z,false);

	// ----------------------------------
	//  Set edge type base on neighbors
	// ----------------------------------
	bool bUp		= HavePanel(x,   z+1);
	bool bDown		= HavePanel(x,   z-1);
	bool bLeft		= HavePanel(x-1, z  );
	bool bRight		= HavePanel(x+1, z  );

	bool bUpLeft	= HavePanel(x-1,  z+1);
	bool bUpRight	= HavePanel(x+1,  z+1);
	bool bDownLeft	= HavePanel(x-1,  z-1);
	bool bDownRight	= HavePanel(x+1,  z-1);

	//-------------
	// Top
	//-------------
	if (bUp)
	{
		bool bLeftEdge		= !bLeft  && bUpLeft;
		bool bRightEdge		= !bRight && bUpRight;

		if (bLeftEdge && bRightEdge)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_TOP, EDGE_FULL, forceStyle );
		}
		else if (bLeftEdge)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_TOP, EDGE_LEFT, forceStyle );
		}
		else if (bRightEdge)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_TOP, EDGE_RIGHT, forceStyle );
		}
		else
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_TOP, EDGE_NONE, forceStyle );
		}
	}
	else
	{
		AddToEdgeRenderList(x, z, WIN_SIDE_TOP, EDGE_NOT, forceStyle );
	}
	//-------------
	// Bottom
	//-------------
	if (bDown)
	{
		bool bLeftEdge		= !bLeft  && bDownLeft;
		bool bRightEdge		= !bRight && bDownRight;

		if (bLeftEdge && bRightEdge)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_BOTTOM, EDGE_FULL, forceStyle );
		}
		else if (bLeftEdge)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_BOTTOM, EDGE_RIGHT, forceStyle );
		}
		else if (bRightEdge)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_BOTTOM, EDGE_LEFT, forceStyle );
		}
		else
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_BOTTOM, EDGE_NONE, forceStyle );
		}
	}
	else
	{
		AddToEdgeRenderList(x, z, WIN_SIDE_BOTTOM, EDGE_NOT, forceStyle );
	}
	//-------------
	// Left
	//-------------
	if (bLeft)
	{
		bool bTop		= !bUp 	 && bUpLeft;
		bool bBottom	= !bDown && bDownLeft;

		if (bTop && bBottom)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_LEFT, EDGE_FULL, forceStyle );
		}
		else if (bTop)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_LEFT, EDGE_RIGHT, forceStyle );
		}
		else if (bBottom)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_LEFT, EDGE_LEFT, forceStyle );
		}
		else
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_LEFT, EDGE_NONE, forceStyle );
		}
	}
	else
	{
		AddToEdgeRenderList(x, z, WIN_SIDE_LEFT, EDGE_NOT, forceStyle );
	}
	//-------------
	// Right
	//-------------
	if (bRight)
	{
		bool bTop		= !bUp 	 && bUpRight;
		bool bBottom	= !bDown && bDownRight;

		if (bTop && bBottom)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_RIGHT, EDGE_FULL, forceStyle );
		}
		else if (bTop)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_RIGHT, EDGE_LEFT, forceStyle );
		}
		else if (bBottom)
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_RIGHT, EDGE_RIGHT, forceStyle );
		}
		else
		{
			AddToEdgeRenderList(x, z, WIN_SIDE_RIGHT, EDGE_NONE, forceStyle );
		}
	}
	else
	{
		AddToEdgeRenderList(x, z, WIN_SIDE_RIGHT, EDGE_NOT, forceStyle );
	}
}

bool UavaShatterGlassComponent::HavePanel(int x, int z)
{
	// If I'm off the edge, always give support
	if (!InLegalRange(x,z))
	{
		return true;
	}
	return (IsPanelSolid(x,z));
}

void UavaShatterGlassComponent::AddToEdgeRenderList(int x, int z, WinSide_t nSide, WinEdge_t nEdgeType, int forceStyle )
{
	// -----------------------------------------------------
	// Try to find old panel
	int nOldPanelIndex = FindRenderPanel(x,z,nSide);

	// -----------------------------------------------------
	// If I have an old panel, get it's style and remove it
	// otherwise randomly chose a style
	int nStyle;
	if (nOldPanelIndex>=0)
	{
		nStyle = EdgeRenderList(nOldPanelIndex).Style;
		EdgeRenderList.Remove(nOldPanelIndex);

		NumEdgeTrisToRender -= 2;
	}
	else
	{
		nStyle = (int)( appFrand() * (NUM_EDGE_STYLES-1) );
	}

	if ( forceStyle != -1 )
	{
		nStyle = forceStyle;
	}

	// -----------------------------------------------------
	// If my new panel has an edge, add it to render list
	if (nEdgeType != EDGE_NOT)
	{
		// EdgeRenderList is sorted by texture type.  Find first element
		// that shares the same texture as the new panel
		INT nTexIndex = FindFirstRenderTexture(nEdgeType, nStyle);

		// If texture was already in list, add after last use
		FEdgeRenderInfo* eri;		
		if (nTexIndex>=0)
		{			
			eri = new (EdgeRenderList, nTexIndex)FEdgeRenderInfo;			
		}
		// Otherwise add to send of render list
		else
		{
			eri = new (EdgeRenderList)FEdgeRenderInfo;
		}

		// Now fill out my data
		eri->z	= z;
		eri->x	= x;
		eri->EdgeType = nEdgeType;
		eri->Side = nSide;
		eri->Style = nStyle;

		checkSlow( nStyle < NUM_EDGE_STYLES );
		SetStyleType( x, z, nStyle );

		NumEdgeTrisToRender += 2;
	}
}

int UavaShatterGlassComponent::FindRenderPanel(int x, int z, WinSide_t nWinSide)
{	
	for (INT i=0; i<EdgeRenderList.Num(); i++)
	{
		if (EdgeRenderList(i).Side == nWinSide && EdgeRenderList(i).x == x && EdgeRenderList(i).z== z)
		{
			return i;
		}
	}
	return -1;
}

//----------------------------------------------------------------------------------
// Purpose : Returns first element in render list with the same edge type and style
// Input   :
// Output  :
//----------------------------------------------------------------------------------
int UavaShatterGlassComponent::FindFirstRenderTexture(WinEdge_t nEdgeType, int nStyle)
{	
	for (INT i=0; i<EdgeRenderList.Num(); ++i)
	{
		if (EdgeRenderList(i).Style == nStyle && EdgeRenderList(i).EdgeType == nEdgeType)
		{
			return i;
		}
	}

	return -1;
}

void UavaShatterGlassComponent::Reset()
{
	for (int width=0; width<NumWide; ++width)
	{
		for (int height=0; height<NumHigh; ++height)
		{
			SetPanelStale( width, height, false );
			SetPanelSolid( width, height, true );
		}
	}

	NumBrokenPanes = 0;
	bIsBroken = false;	
	BlockActors=true;
	bNeedsUpdateSupport = false;	
	bSolidRenderListUpdated = false;

	EdgeRenderList.Empty();

	NumEdgeTrisToRender = NumSolidTrisToRender = 0;

	UpdateSolidRenderList();
	UpdateDynamicData();
}

void UavaShatterGlassComponent::PostEditChange(UProperty* PropertyThatChanged)
{
	__super::PostEditChange(PropertyThatChanged);

	DiscardMaterialInstances();

	Reset();	
}

void UavaShatterGlassComponent::Attach()
{
	Super::Attach();

	Reset();
}

void UavaShatterGlassComponent::PostLoad()
{
	__super::PostLoad();

	DiscardMaterialInstances();

	Reset();	
}

float UavaShatterGlassComponent::GetSupport( int x, int z )
{
	return Max( Support[x][z], 0.0f );
}


float UavaShatterGlassComponent::RecalcSupport( int x, int z )
{
	// Always has some support. Zero signifies that it has been broken
	float flSupport = 0.01f;

	// ------------
	// Top support
	// ------------
	if (z == NumHigh-1)
	{
		flSupport += 1.0f;
	}
	else
	{
		flSupport += GetSupport(x,z+1);	 
	}

	// ------------
	// Bottom Support
	// ------------
	if (z == 0)
	{
		flSupport += 1.25f;
	}
	else
	{
		flSupport += 1.25f * GetSupport(x,z-1);
	}

	// ------------
	// Left Support
	// ------------
	if (x == 0)
	{
		flSupport += 1.0f;
	}
	else
	{
		flSupport += GetSupport(x-1,z);
	}

	// --------------
	// Right Support
	// --------------
	if (x == NumWide-1)
	{
		flSupport += 1.0f;
	}
	else
	{
		flSupport += GetSupport(x+1,z);
	}

	// --------------------
	// Bottom Left Support
	// --------------------
	if (z == 0 || x == 0)
	{
		flSupport += 1.0f;
	}
	else
	{
		flSupport += GetSupport(x-1,z-1);
	}

	// ---------------------
	// Bottom Right Support
	// ---------------------
	if (z == 0 || x == NumWide-1)
	{
		flSupport += 1.0f;
	}
	else
	{
		flSupport += GetSupport(x+1,z-1);
	}

	// -----------------
	// Top Right Support
	// -----------------
	if (z == NumHigh-1 || x == NumWide-1)
	{
		flSupport += 0.25f;
	}
	else
	{
		flSupport += 0.25f * GetSupport(x+1,z+1);
	}

	// -----------------
	// Top Left Support
	// -----------------
	if (z == NumHigh-1 || x == 0)
	{
		flSupport += 0.25f;
	}
	else
	{
		flSupport += 0.25f * GetSupport(x-1,z+1);
	}

	return flSupport;

}

void UavaShatterGlassComponent::GlassTick()
{

}

void UavaShatterGlassComponent::Tick( FLOAT DeltaTime )
{
	//if (GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->GRI) 
	//{
	//	INT CurrentRound = Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI )->CurrentRound;

	//	if (CurrentRound != LastRound)
	//	{
	//		Reset();

	//		LastRound = CurrentRound;
	//	}
	//}	

	Super::Tick( DeltaTime );

	UBOOL bDirty = !bSolidRenderListUpdated;

	if (bDirty)
	{
		UpdateSolidRenderList();
		bSolidRenderListUpdated = true;
	}

	if (bIsBroken || bNeedsUpdateSupport) 
	{
		for (INT iTouching=0; iTouching<Owner->Touching.Num(); iTouching++)
		{
			AActor* TestActor = Owner->Touching(iTouching);
			if(	TestActor && !TestActor->bDeleteMe)
			{
				SurfaceTouch( TestActor );
			}
		}

		bNeedsUpdateSupport = false;

		// -----------------------
		// Recalculate all support
		// -----------------------
		int w;
		float flSupport[MAX_NUM_PANELS][MAX_NUM_PANELS];
		for (w=0;w<NumWide;w++)
		{
			for (int h=0;h<NumHigh;h++)
			{
				if (!IsBroken(w,h))
				{
					flSupport[w][h] = RecalcSupport(w,h);
				}
			}
		}

		// ----------------------------------------------------
		//  Set support and break inadequately supported panes
		// ----------------------------------------------------
		float flBreakValue = WINDOW_BREAK_SUPPORT*(Fragility/100.0);
		bool  bNeedUpdate  = false;
		for (w=0;w<NumWide;w++)
		{
			for (int h=0;h<NumHigh;h++)
			{
				if (!IsBroken(w,h))
				{
					Support[w][h] = flSupport[w][h]/WINDOW_MAX_SUPPORT;
					if (Support[w][h] < flBreakValue)
					{					
						// Occasionaly drop a pane
						if (appFrand() > 0.5f)
						{
							DropPane(w,h);
						}
						// Otherwise just shatter the glass
						else
						{
							ShatterPane(w,h,FVector(0,0,0),FVector(0,0,0));
						}					
						bNeedUpdate   = true;
					}
				}
			}
		}

		if (bNeedUpdate)
		{
			bNeedsUpdateSupport = true;
			Cast<AavaShatterGlassActor>( Owner )->eventUpdateEdges();			

			bDirty = TRUE;
		}
	}

	if (bDirty)
		UpdateDynamicData();
}

void UavaShatterGlassComponent::BreakPane(int x, int z)
{
	// Check parameter range
	if (x < 0  || x  >= NumWide) return;
	if (z < 0 || z >= NumHigh) return;

	Die();

	// Count how many panes have been broken or dropped
	Cast<AavaShatterGlassActor>( Owner )->eventBreak( x, z );	

	NumBrokenPanes++;	
}

//------------------------------------------------------------------------------
// Purpose : Drop a window pane entity
// Input   :
// Output  :
//------------------------------------------------------------------------------
void UavaShatterGlassComponent::DropPane(int x, int z)
{
	// Check parameter range
	if (x < 0  || x  >= NumWide) return;
	if (z < 0 || z >= NumHigh) return;

	Die();

	if (!IsBroken(x,z))
	{
		BreakPane(x,z);

		CreateShards( x, z, FVector(0,0,0), FVector(0,0,0) );



		/*		

		CreateShards(vBreakPos, vAngles, vec3_origin, vec3_origin,
		WINDOW_PANEL_SIZE,	WINDOW_PANEL_SIZE,
		WINDOW_SMALL_SHARD_SIZE);

		CWindowPane *pPane = CWindowPane::CreateWindowPane(vBreakPos, vAngles);
		if (pPane)
		{
		pPane->SetLocalAngularVelocity( RandomAngle(-120,120) );
		}*/
	}
}

UBOOL UavaShatterGlassComponent::ShatterPane(int x, int z, const FVector& vForce, const FVector& vForcePos)
{	
	// Check parameter range
	if (x < 0  || x  >= NumWide) return FALSE;
	if (z < 0 || z >= NumHigh) return FALSE;

	if (!IsBroken(x,z))
	{
		BreakPane(x,z);		

		CreateShards( x, z, vForce, vForcePos );

		/*QAngle vAngles;
		VectorAngles(-1*Normal,vAngles);
		FVector vWidthDir,vHeightDir;
		AngleVectors(vAngles,NULL,&vWidthDir,&vHeightDir);
		FVector vBreakPos	= Corner + 
		(x*vWidthDir*m_flPanelWidth) + 
		(z*vHeightDir*m_flPanelHeight);

		CreateShards(vBreakPos, vAngles,vForce,	vForcePos, m_flPanelWidth, m_flPanelHeight, WINDOW_SMALL_SHARD_SIZE);
		*/

		return TRUE;
	}

	return FALSE;
}

FVector UavaShatterGlassComponent::CoordToWorld( INT x, INT z ) const
{
	FVector Origin = LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);		
	FVector Z = LocalToWorld.GetAxis(2);		

	return Origin + X * x + Z * z;
}

void UavaShatterGlassComponent::CreateShards( int x, int z, const FVector& Force, const FVector& ForcePos )
{
	Cast<AavaShatterGlassActor>( Owner )->eventCreateShards( CoordToWorld( x, z ), Force, ForcePos );			
}

void UavaShatterGlassComponent::BreakAllPanes()
{
	Die();

	// Now tell the client all the panes have been broken
	for (int width=0;width<NumWide;width++)
	{
		for (int height=0;height<NumHigh;height++)
		{
			Support[width][height] = WINDOW_PANE_BROKEN;
		}
	}	

	Cast<AavaShatterGlassActor>( Owner )->eventBreakAll();			
	bNeedsUpdateSupport = false;

	NumBrokenPanes = NumWide*NumHigh;
}

bool UavaShatterGlassComponent::IsBroken(int x, int z)
{
	if (!bIsBroken) return false;

	if (x  < 0  || x  >= NumWide) return true;
	if (z < 0 || z >= NumHigh)  return true;

	return (Support[x][z]==WINDOW_PANE_BROKEN);
}

UBOOL UavaShatterGlassComponent::PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags)
{
	if (Extent.IsZero() || bIsBroken && ( GWorld->GetNetMode() == NM_Client )) return TRUE;

	FVector RelPos = Location - LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);
	FVector Y = LocalToWorld.GetAxis(1);
	FVector Z = LocalToWorld.GetAxis(2);		

	Y.Normalize();

	FVector XLocation, XExtent;

	XLocation.X = (RelPos | X) / (X.SizeSquared() * NumWide);
	XLocation.Y = (RelPos | Y);
	XLocation.Z = (RelPos | Z) / (Z.SizeSquared() * NumHigh);

	XExtent.X = fabsf( (Extent | X) / (X.SizeSquared() * NumWide) );
	XExtent.Y = fabsf( (Extent | Y) );
	XExtent.Z = fabsf( (Extent | Z) / (Z.SizeSquared() * NumHigh) );

	/// 위에 있네~~

	FVector XMin, XMax;

	XMin = XLocation - XExtent;
	XMax = XLocation + XExtent;

	if (XMin.X > 1 || XMin.Z > 1 || XMax.X < 0 || XMax.Z < 0)
	{
		return TRUE;
	}

	float depth = fabsf( Extent | Y );

	if (XMin.Y > depth || XMax.Y < -depth)
	{
		return TRUE;
	}

	Result.Actor = Owner;
	Result.Component = this;	
	Result.Normal = XLocation.Y > 0 ? Y : -Y;
	Result.Location = Location;
	Result.Material = Material;

	return FALSE;

}
UBOOL UavaShatterGlassComponent::LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags)
{	
	FVector Origin = LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);
	FVector Y = LocalToWorld.GetAxis(1);
	FVector Z = LocalToWorld.GetAxis(2);

	FVector Dir = (End - Start);

	float Length = Dir.Size();
	Dir /= Length;

	Y.Normalize();	

	float Y_Dir = Y | Dir;	

	float d = - (Y | Origin);

	if (Extent.IsZero())
	{		
		float t = - ( (Y | Start) + d ) / ( Y_Dir );	

		if (t < 0 || t > Length)
		{
			return TRUE;
		}

		Result.Location = Start + Dir * t;

		FVector RelPos = Result.Location - Origin;		

		float w = (RelPos | X);
		if (w < 0 || w > X.SizeSquared() * NumWide) return TRUE;
		float h = (RelPos | Z);
		if (h < 0 || h > Z.SizeSquared() * NumHigh) return TRUE;
	}
	else
	{	
		//@BUGFIX ;  Client에서도 체크제대로 해야함! deif 2007/5/28
		/// 꺠진 경우는 체크할 필요 없음!		
		/*if (bIsBroken && ( GWorld->GetNetMode() == NM_Client ))  
			return TRUE;*/

		/// 처음부터 끼어있는지도 체크해야 함... -_-;
		if (!PointCheck( Result, Start, Extent, TraceFlags ))
		{
			return FALSE;
		}

		FVector Offset( 
			Dir.X > 0 ? Extent.X : -Extent.X,
			Dir.Y > 0 ? Extent.Y : -Extent.Y,
			Dir.Z > 0 ? Extent.Z : -Extent.Z );		

		FVector Farthest = Start - Offset;
		FVector Nearest = Start + Offset;

		float tf = - ( (Y | Farthest) + d ) / ( Y_Dir );	
		float tn = - ( (Y | Nearest) + d ) / ( Y_Dir );	

		/// 이미 지나갔다!!!
		if (tf < 0) return TRUE;

		/// 도달하지 못했다
		if (tn > Length) return TRUE;

		/// 아직 도달하지 않았다
		if (tn > 0)
		{
			Result.Location = Nearest + Dir * tn - Offset;

			FVector RelPos = Result.Location - Origin;

			float w = (RelPos | X);
			if (w < 0 || w > X.SizeSquared() * NumWide) return TRUE;
			float h = (RelPos | Z);
			if (h < 0 || h > Z.SizeSquared() * NumHigh) return TRUE;
		}
		/// 중간에 꼈다!!!!
		else 
		{
			float t = - ( (Y | Start) + d ) / ( Y_Dir );	

			if (t < 0 || t > Length)
			{
				return TRUE;
			}

			Result.Location = Start;

			FVector RelPos = Result.Location - Origin;

			float w = (RelPos | X);
			if (w < 0 || w > X.SizeSquared() * NumWide) return TRUE;
			float h = (RelPos | Z);
			if (h < 0 || h > Z.SizeSquared() * NumHigh) return TRUE;
		}
	}	

	if ( bIsBroken )
	{
		float flWidth,flHeight;
		PanePos(Result.Location,&flWidth,&flHeight);
		int nWidth  = flWidth;
		int nHeight = flHeight;
		if ( IsBroken(nWidth,nHeight) )	return TRUE;
	}

	Result.Actor = Owner;
	Result.Component = this;	
	Result.Normal = (Y_Dir) > 0 ? -Y : Y;	
	Result.Material = Material;
	Result.Time = ((Result.Location - Start) | Dir) / Length;

	return FALSE;
}

void UavaShatterGlassComponent::Die()
{
	if (bIsBroken) return;	

	BlockActors=false;

	bIsBroken = true;	
	bNeedsUpdateSupport = false;

	// Initialize panels
	for (int w=0;w<MAX_NUM_PANELS;w++)
	{ 
		for (int h=0;h<MAX_NUM_PANELS;h++)
		{
			Support[w][h] = 1.0f;
		}
	}

	/// 여기서 이 유리창 위에 서 있던 것이 떨어지도록 처리해야 함
}

void UavaShatterGlassComponent::TakeDamage( INT Damage, AController* EventInstigator, const FVector& HitLocation, const FVector& Momentum, UClass* DamageType, const FTraceHitInfo& HitInfo, class AActor* DamageCauser )
{
	Die();	

	float flWidth,flHeight;
	PanePos(HitLocation,&flWidth,&flHeight);

	int nWidth  = flWidth;
	int nHeight = flHeight;
	ShatterPane(nWidth, nHeight,Momentum,HitLocation);

	float flWRem = flWidth  - nWidth;
	float flHRem = flHeight - nHeight;

	if (flWRem > 0.8 && nWidth != NumWide-1)
	{
		ShatterPane(nWidth+1, nHeight,Momentum,HitLocation);
	}
	else if (flWRem < 0.2 && nWidth != 0)
	{
		ShatterPane(nWidth-1, nHeight,Momentum,HitLocation);
	}
	if (flHRem > 0.8 && nHeight != NumHigh-1)
	{
		ShatterPane(nWidth, nHeight+1,Momentum,HitLocation);
	}
	else if (flHRem < 0.2 && nHeight != 0)
	{
		ShatterPane(nWidth, nHeight-1,Momentum,HitLocation);
	}

	// Occasionally break the pane above me
	if (appFrand() > 0.5f)
	{
		ShatterPane(nWidth, nHeight+1,Momentum * 2,HitLocation);
		// Occasionally break the pane above that
		if (appFrand() > 0.5f)
		{
			ShatterPane(nWidth, nHeight+2,Momentum * 2,HitLocation);
		}		
	}
}

void UavaShatterGlassComponent::PanePos(const FVector &vPos, float *flWidth, float *flHeight)
{
	FVector RelPos = vPos - LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);	
	FVector Z = LocalToWorld.GetAxis(2);

	float XsizeSq = X.SizeSquared(), ZsizeSq = Z.SizeSquared();

	*flWidth = (RelPos | X) / XsizeSq;
	*flHeight = (RelPos | Z) / ZsizeSq;
}

void UavaShatterGlassComponent::SurfaceTouch( AActor* Actor )
{
	if (!Actor || !Actor->bBlockActors)
		return;	

	Die();

	FVector Origin = LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);	
	FVector Normal = LocalToWorld.GetAxis(1);	
	FVector Z = LocalToWorld.GetAxis(2);
	Normal.Normalize();

	// Find nearest point on plane for max
	FBox Box;
	Actor->GetComponentsBoundingBox( Box );

	FVector vToPlane		= (Box.Max - Origin);
	float  vDistToPlane = Normal | vToPlane;
	FVector vTouchPos	= Box.Max + vDistToPlane * Normal;

	float flMinsWidth,flMinsHeight;
	PanePos(vTouchPos, &flMinsWidth, &flMinsHeight);

	// Find nearest point on plane for mins
	vToPlane		= (Box.Min - Origin);
	vDistToPlane = Normal | vToPlane;
	vTouchPos	= Box.Min + vDistToPlane*Normal;

	float flMaxsWidth,flMaxsHeight;
	PanePos(vTouchPos, &flMaxsWidth, &flMaxsHeight);

	int nMinWidth = appFloor(Max(0.0f,	Min(flMinsWidth,flMaxsWidth)));
	int nMaxWidth = appCeil(Min((float)NumWide,Max(flMinsWidth,flMaxsWidth)));

	int nMinHeight = appFloor(Max(0.0f, Min(flMinsHeight,flMaxsHeight)));
	int nMaxHeight = appCeil(Min((float)NumHigh,Max(flMinsHeight,flMaxsHeight)));

	// Move faster then penetrating object so can see shards
	FVector vHitVel = Actor->Velocity * 5;		

	UBOOL Hit = FALSE;

	for (int height=nMinHeight;height<nMaxHeight;height++)
	{
		// Randomly break the one before so it doesn't look square
		if (appFrand() > 0.5f)
		{
			if (ShatterPane(nMinWidth-1, height,vHitVel,Actor->Location))
				Hit = TRUE;
		}
		for (int width=nMinWidth;width<nMaxWidth;width++)
		{
			if (ShatterPane(width, height,vHitVel,Actor->Location))
				Hit = TRUE;
		}
		// Randomly break the one after so it doesn't look square
		if (appFrand() > 0.5f)
		{
			if (ShatterPane(nMaxWidth+1, height,vHitVel,Actor->Location))
				Hit = TRUE;
		}
	}

	if (Hit)
	{
		AKActor* KActor = Cast<AKActor>( Actor );
		if (KActor)
		{
			NxActor* nActor = KActor->CollisionComponent->GetNxActor();

			nActor->addForce( nActor->getLinearVelocity() * -0.5f, NX_VELOCITY_CHANGE );
		}
	}	

	Cast<AavaShatterGlassActor>( Owner )->eventUpdateEdges();			
}

UavaShatterGlassComponent::UavaShatterGlassComponent()
{
	memset( Style, 0, sizeof(Style) );
	memset( Texture_Style, 0, sizeof(Texture_Style) );	

	RenderContext = new FShatterGlassRenderContext( this );

	if (GIsGame || GIsEditor)
	{
		PrepareMaterialInstances();
	}
}

void UavaShatterGlassComponent::FinishDestroy()
{
	delete RenderContext;

	Super::FinishDestroy();
}

void FShatterGlassRenderContext::Reset()
{
	for (INT i=0; i<13; ++i)
	{
		Vertices[i].Empty();
		Indices[i].Empty();
	}	

	Origin = Component->LocalToWorld.GetOrigin();
	X = Component->LocalToWorld.GetAxis(0);
	UnitZ = Component->LocalToWorld.GetAxis(2);			

	FVector Normal = Component->LocalToWorld.GetAxis(1);
	Normal.Normalize();

	FVector tx, ty;	
	Normal.FindBestAxisVectors(ty,tx);

	TangentX = tx;
	TangentY = ty;
	TangentZ = Normal;		
}

void FShatterGlassRenderContext::AddSolidBlock( int width, int height, int nHCount )
{	
	FVector2D NormalTransform0(0,0), NormalTransform1(0,0);

	INT V = Vertices[0].Num();

	FVector Base = Origin + X * width + UnitZ * (height-nHCount);
	FVector Z = UnitZ * nHCount;

	FVector2D UV( (float)width / Component->NumWide, (float)(height-nHCount) / Component->NumHigh );
	FVector2D UVSize( (float)1 / Component->NumWide, (float)nHCount / Component->NumHigh );	

	FVector2D D0( 0, 0 ), D1( 1, 1 );

	Vertices[0].AddItem( FShatterGlassVertex( Base, TangentX, TangentY, TangentZ, UV, FVector2D( D0.X, D0.Y ) ) );
	Vertices[0].AddItem( FShatterGlassVertex( Base + X, TangentX, TangentY, TangentZ, UV + FVector2D( UVSize.X, 0 ), FVector2D( D1.X, D0.Y ) ) );
	Vertices[0].AddItem( FShatterGlassVertex( Base + X + Z, TangentX, TangentY, TangentZ, UV + UVSize, FVector2D( D1.X, D1.Y ) ) );
	Vertices[0].AddItem( FShatterGlassVertex( Base + Z, TangentX, TangentY, TangentZ, UV + FVector2D( 0, UVSize.Y ), FVector2D( D0.X, D1.Y ) ) );

	Indices[0].AddItem( V );
	Indices[0].AddItem( V + 1 );
	Indices[0].AddItem( V + 2 );

	Indices[0].AddItem( V );
	Indices[0].AddItem( V + 2 );
	Indices[0].AddItem( V + 3 );
}

void FShatterGlassRenderContext::AddEdge( int edgeType, int style, int width, int height, WinSide_t nEdge )
{	
	FVector2D NormalTransform0(0,0), NormalTransform1(0,0);

	INT Style = 1 + style * 4 + edgeType;

	INT V = Vertices[Style].Num();

	FVector Base = Origin + X * width + UnitZ * height;
	FVector Z = UnitZ;

	FVector2D UV( (float)width / Component->NumWide, (float)height / Component->NumHigh );
	FVector2D UVSize( (float)1 / Component->NumWide, (float)1 / Component->NumHigh );	

	FVector2D D0, D1;

	switch (nEdge)
	{
	case WIN_SIDE_LEFT :	
		Vertices[Style].AddItem( FShatterGlassVertex( Base, TangentX, TangentY, TangentZ, UV, FVector2D( 0, 1 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X, TangentX, TangentY, TangentZ, UV + FVector2D( UVSize.X, 0 ), FVector2D( 0, 0 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X + Z, TangentX, TangentY, TangentZ, UV + UVSize, FVector2D( 1, 0 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + Z, TangentX, TangentY, TangentZ, UV + FVector2D( 0, UVSize.Y ), FVector2D( 1, 1 ) ) );
		break;
	case WIN_SIDE_TOP :	
		Vertices[Style].AddItem( FShatterGlassVertex( Base, TangentX, TangentY, TangentZ, UV, FVector2D( 0, 0 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X, TangentX, TangentY, TangentZ, UV + FVector2D( UVSize.X, 0 ), FVector2D( 1, 0 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X + Z, TangentX, TangentY, TangentZ, UV + UVSize, FVector2D( 1, 1 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + Z, TangentX, TangentY, TangentZ, UV + FVector2D( 0, UVSize.Y ), FVector2D( 0, 1 ) ) );
		break;
	case WIN_SIDE_RIGHT :	
		Vertices[Style].AddItem( FShatterGlassVertex( Base, TangentX, TangentY, TangentZ, UV, FVector2D( 0, 0 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X, TangentX, TangentY, TangentZ, UV + FVector2D( UVSize.X, 0 ), FVector2D( 0, 1 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X + Z, TangentX, TangentY, TangentZ, UV + UVSize, FVector2D( 1, 1 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + Z, TangentX, TangentY, TangentZ, UV + FVector2D( 0, UVSize.Y ), FVector2D( 1, 0 ) ) );
		break;
	case WIN_SIDE_BOTTOM :		
		Vertices[Style].AddItem( FShatterGlassVertex( Base, TangentX, TangentY, TangentZ, UV, FVector2D( 0, 1 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X, TangentX, TangentY, TangentZ, UV + FVector2D( UVSize.X, 0 ), FVector2D( 1, 1 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + X + Z, TangentX, TangentY, TangentZ, UV + UVSize, FVector2D( 1, 0 ) ) );
		Vertices[Style].AddItem( FShatterGlassVertex( Base + Z, TangentX, TangentY, TangentZ, UV + FVector2D( 0, UVSize.Y ), FVector2D( 0, 0 ) ) );
		break;
	default :
		checkSlow( false );
		break;
	}



	Indices[Style].AddItem( V );
	Indices[Style].AddItem( V + 1 );
	Indices[Style].AddItem( V + 2 );

	Indices[Style].AddItem( V );
	Indices[Style].AddItem( V + 2 );
	Indices[Style].AddItem( V + 3 );
}

void UavaShatterGlassComponent::UpdateTransform()
{
	Super::UpdateTransform();

	bSolidRenderListUpdated = false;
}

void UavaShatterGlassComponent::InitComponentRBPhys(UBOOL bFixed)
{
#if WITH_NOVODEX
	// If no Brush, or no physics scene, or we already have a BodyInstance, do nothing.
	if(!GWorld->RBPhysScene || BodyInstance)
	{
		return;
	}

	check(GEngine->DefaultPhysMaterial);
	UPhysicalMaterial* PhysMat = GEngine->DefaultPhysMaterial;
	if( PhysMaterialOverride )
	{
		PhysMat = PhysMaterialOverride;
	}
	NxMaterialIndex MatIndex = GWorld->RBPhysScene->FindPhysMaterialIndex( PhysMat );


	/// 유리창의 크기!
	FVector FullScale(1.f);
	if(Owner)
	{
		FullScale = Owner->DrawScale * Owner->DrawScale3D;
	}
	FullScale.X *= NumWide;
	FullScale.Z *= NumHigh;

	FKAggregateGeom geoms;
	memset( &geoms, 0, sizeof(geoms) );

	const int ex = geoms.ConvexElems.AddZeroed();
	FKConvexElem* c = &geoms.ConvexElems(ex);
	
	for (INT i=0; i<8; ++i)
		c->VertexData.AddItem( FVector( 
		i & 1 ? 0 : FullScale.X,
		i & 2 ? -5 : 5,
		i & 4 ? 0 : FullScale.Z ) );

	UBOOL bHullIsGood = c->GenerateHullData();	

	NxActorDesc* BrushShapeDesc = geoms.InstanceNovodexGeom( FVector(1.f), NULL, FALSE, *GetFullName() );
	if(BrushShapeDesc)
	{
		NxArray<NxShapeDesc*>* Aggregate = &BrushShapeDesc->shapes;

		// Make transform for this static mesh component
		FMatrix CompTM = LocalToWorld;
		CompTM.RemoveScaling();
		NxMat34 nCompTM = U2NTransform(CompTM);

		NxGroupsMask GroupsMask = CreateGroupsMask(RBChannel, &RBCollideWithChannels);

		// Create actor description and instance it.
		NxActorDesc BrushActorDesc;
		BrushActorDesc.globalPose = nCompTM;
		BrushActorDesc.density = 1.f;

		for(UINT i=0; i<Aggregate->size(); i++)
		{
			BrushActorDesc.shapes.push_back( (*Aggregate)[i] );

			// Set the material to the one specified in the PhysicalMaterial before creating this NxActor instance.
			BrushActorDesc.shapes[i]->materialIndex = MatIndex;
			BrushActorDesc.shapes[i]->groupsMask = GroupsMask;
		}

		// Is possible for brushes to be kinematic (eg dynamic blocking volume).
		NxBodyDesc BodyDesc;
		if(!Owner || !Owner->bStatic)
		{
			BodyDesc.flags |= NX_BF_KINEMATIC;
			BrushActorDesc.body = &BodyDesc;
		}

		/// Collision 정보는 사용하나, response는 하지 않도록!
		BrushActorDesc.flags = NX_AF_DISABLE_RESPONSE;		

		// Create the actual NxActor using the mesh collision shape.
		NxScene* NovodexScene = GWorld->RBPhysScene->GetNovodexPrimaryScene();
		check(NovodexScene);
		NxActor* nBrushActor = NovodexScene->createActor(BrushActorDesc);

		// Then create an RB_BodyInstance for this terrain component and store a pointer to the NxActor in it.
		BodyInstance = ConstructObject<URB_BodyInstance>( URB_BodyInstance::StaticClass(), GWorld, NAME_None, RF_Transactional );
		BodyInstance->BodyData = (FPointer)nBrushActor;
		BodyInstance->OwnerComponent = this;		

		nBrushActor->userData = BodyInstance;

		BodyInstance->SceneIndex = GWorld->RBPhysScene->NovodexSceneIndex;		

		// Put the NxActor into the 'notify on collision' group if desired.
		nBrushActor->setGroup(UNX_GROUP_NOTIFYCOLLIDE);		
	}

	/*SetNotifyRigidBodyCollision( TRUE );

	SetRBChannel(RBCC_GameplayPhysics);
	SetRBCollidesWithChannel(RBCC_GameplayPhysics,TRUE);
	SetRBCollidesWithChannel(RBCC_Default,TRUE);
	SetRBCollidesWithChannel(RBCC_Pawn,TRUE);*/

	// free the shape we returned from InstanceNovodexGeom
	delete BrushShapeDesc;	

#endif // WITH_NOVODEX
}

void UavaShatterGlassComponent::TermComponentRBPhys(FRBPhysScene* Scene)
{
	Super::TermComponentRBPhys(Scene);
}

void UavaShatterGlassComponent::PrepareMaterialInstances()
{
	Texture_Style[0] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_01a"), NULL, LOAD_None, NULL);
	Texture_Style[1] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_01b"), NULL, LOAD_None, NULL);
	Texture_Style[2] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_01c"), NULL, LOAD_None, NULL);
	Texture_Style[3] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_01d"), NULL, LOAD_None, NULL);

	Texture_Style[4] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_02a"), NULL, LOAD_None, NULL);
	Texture_Style[5] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_02b"), NULL, LOAD_None, NULL);
	Texture_Style[6] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_02c"), NULL, LOAD_None, NULL);
	Texture_Style[7] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_02d"), NULL, LOAD_None, NULL);

	Texture_Style[8] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_03a"), NULL, LOAD_None, NULL);
	Texture_Style[9] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_03b"), NULL, LOAD_None, NULL);
	Texture_Style[10] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_03c"), NULL, LOAD_None, NULL);
	Texture_Style[11] = LoadObject<UTexture2D>(NULL, TEXT("avaGlass.glassbroken_03d"), NULL, LOAD_None, NULL);

	for (INT i=0; i<12; ++i)		
		PrepareMaterialInstance( Style[i], Texture_Style[i] );	
}

void UavaShatterGlassComponent::PrepareMaterialInstance( UMaterialInstanceConstant*& Result, UTexture2D* Texture )
{
	if (!Result)
	{
		UMaterialInstanceConstant* NewMaterialInstanceConstant = CastChecked<UMaterialInstanceConstant>( UObject::StaticConstructObject(UMaterialInstanceConstant::StaticClass(), this ) );
		NewMaterialInstanceConstant->SetParent(BrokenMaterial);	
		Result = NewMaterialInstanceConstant;
	}

	Result->SetTextureParameterValue( DetailParameter, Texture );
}

void UavaShatterGlassComponent::DiscardMaterialInstances()
{
	for (INT i=0; i<12; ++i)
	if (Style[i])
	{			
		Style[i] = NULL;
	}

	PrepareMaterialInstances();
}

void UavaShatterGlassComponent::RenderEdges(FPrimitiveDrawInterface* PDI,UINT InDepthPriorityGroup)
{
	FVector Origin = LocalToWorld.GetOrigin();
	FVector X = LocalToWorld.GetAxis(0);
	FVector Z = LocalToWorld.GetAxis(2);				

	for (INT i=0; i<EdgeRenderList.Num(); ++i)
	{		
		const FEdgeRenderInfo& eri = EdgeRenderList(i);
		FVector Base = Origin + (eri.x*X) + (eri.z*Z);

		FColor LineColor = FColor( 128, 255, 128 );

		PDI->DrawLine( Base, Base + Z, LineColor, InDepthPriorityGroup );				
		PDI->DrawLine( Base, Base + X, LineColor, InDepthPriorityGroup );
		PDI->DrawLine( Base + X, Base + X + Z, LineColor, InDepthPriorityGroup );				
		PDI->DrawLine( Base + Z, Base + X + Z, LineColor, InDepthPriorityGroup );		
	}
}

void UavaShatterGlassComponent::RenderOneBlock(FPrimitiveDrawInterface* PDI,UINT InDepthPriorityGroup,const FVector& Base,const FVector& X,const FVector& Z)
{
	FColor LineColor = FColor( 255, 255, 128 );

	PDI->DrawLine( Base, Base + Z, LineColor, InDepthPriorityGroup );				
	PDI->DrawLine( Base, Base + X, LineColor, InDepthPriorityGroup );
	PDI->DrawLine( Base + X, Base + X + Z, LineColor, InDepthPriorityGroup );				
	PDI->DrawLine( Base + Z, Base + X + Z, LineColor, InDepthPriorityGroup );
}
