
// Copyright 2007 redduck, Inc

#include "EditorPrivate.h"
#include "SurfaceIterators.h"

#define	AVA_INCLUDE_FSTATICBRUSHITERATOR
#include "UnEdCsg.cpp"
#undef	AVA_INCLUDE_FSTATICBRUSHITERATOR

void UEditorEngine::Ava_bspRepartition( UModel *Bsp, INT iNode, INT Simple )
{
	bspBuildFPolys( Bsp, 1, iNode );
	bspMergeCoplanars( Bsp, 0, 0 );
	bspBuild( Bsp, BSP_Optimal, 12, 70, Simple, iNode );
	bspRefresh( Bsp, 1 );
}

//
// Rebuild the level's Bsp from the level's CSG brushes.
//
void UEditorEngine::Ava_csgRebuild()
{
	if( bSolidOnly )
	{
		GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("BuildSolidGeometryMsg")), 1 );
	}
	else
	{
		if( bFullBuildGeometry )
			GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("FullBuildGeometry")), 1 );
		else
			GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("RebuildingGeometry")), 1 );
	}

	FastRebuild = 1;
	FinishAllSnaps();

	// Empty the model out.
	GWorld->GetModel()->EmptyModel( 1, 1 );

	// Count brushes.
	INT BrushTotal=0, BrushCount=0, StructuralBurshTotal = 0;
	for( FStaticBrushIterator It; It; ++It )
	{
		ABrush* Brush = CastChecked<ABrush>(*It);
		if( !Brush->IsABuilderBrush() )
		{
			if( !(Brush->PolyFlags & PF_Hint) )
			{
				BrushTotal++;
			}

			if( !(Brush->PolyFlags & PF_Semisolid)
			||	Brush->CsgOper != CSG_Add
			||	Brush->PolyFlags & PF_Hint )
			{
				StructuralBurshTotal++;
			}
		}
	}

	// build tree bounds for generating portal.
	FBox StructuralBound;
	StructuralBound.Init();
	for( FStaticBrushIterator It; It; ++It )
	{
		ABrush *BrushActor = CastChecked<ABrush>(*It);
		if( !BrushActor->IsABuilderBrush() )
		{
			if( BrushActor->PolyFlags & PF_Hint )
			{
				continue;
			}
			if( !(BrushActor->PolyFlags & PF_Semisolid)
				|| (BrushActor->CsgOper != CSG_Add)
				|| (BrushActor->PolyFlags & PF_Portal) )
			{
				UModel *Brush = BrushActor->Brush;
				for( INT PolyIndex = 0; PolyIndex < Brush->Polys->Element.Num(); ++PolyIndex )
				{
					FPoly CurrentPoly = Brush->Polys->Element(PolyIndex);
					CurrentPoly.Transform( BrushActor->PrePivot, BrushActor->Location );
					for( INT VertexIndex = 0; VertexIndex < CurrentPoly.Vertices.Num(); ++VertexIndex )
					{
						StructuralBound += CurrentPoly.Vertices(VertexIndex);
					}
				}
			}
		}
	}
	// Compose all structural brushes and portals.
	UModel *StructuralModel = GWorld->PersistentLevel->StructuralModel;
	StructuralModel->EmptyModel(1,1);
	StructuralModel->TreeBounds = StructuralBound;
	if( bFullBuildGeometry )
	{
		for( FStaticBrushIterator It; It; ++It )
		{	
			ABrush* Brush = CastChecked<ABrush>(*It);
			if( !Brush->IsABuilderBrush() )
			{
				if( !(Brush->PolyFlags & PF_Semisolid )
					|| (Brush->CsgOper!=CSG_Add )
					|| (Brush->PolyFlags&PF_Hint) )
				{
					if( Brush->PolyFlags & PF_Hint )
					{
						Brush->PolyFlags = (Brush->PolyFlags & ~PF_Semisolid ) | PF_NotSolid;
					}
					BrushCount++;
					GWarn->StatusUpdatef( BrushCount, StructuralBurshTotal, *LocalizeUnrealEd(TEXT("ApplyingStructuralBrushF")), BrushCount, StructuralBurshTotal );
					bspBrushCSG( Brush, StructuralModel, Brush->PolyFlags, (ECsgOper)Brush->CsgOper, FALSE, TRUE, FALSE );
				}
			}
		}

		// Repartition the structural BSP.
		Ava_bspRepartition( StructuralModel, 0, 0 );

		// Generate Portal and Compute Potentially Visible Sets
		TestVisibility( StructuralModel, 0, 0 );
	}
	else
	{
		// full build 가 아니면, vis 관련 정보를 반드시 초기화해 줍니다.
		StructuralModel->NumClusters = 0;
		StructuralModel->LeafBytes = 0;
		StructuralModel->Leaves.Empty();
		StructuralModel->LeafColors.Empty();
		StructuralModel->StaticMeshLeaves.Empty();
		StructuralModel->LeafInfos.Empty();
		StructuralModel->Portals.Empty();
	}


	// Compose all structural and detail brushes and portals.
	BrushCount = 0;
	for( FStaticBrushIterator It; It; ++It )
	{	
		ABrush* Brush = CastChecked<ABrush>(*It);
		if( !Brush->IsABuilderBrush() && !(Brush->PolyFlags & PF_Hint) )
		{
			if( bSolidOnly && (Brush->PolyFlags & PF_Semisolid) )
			{
				continue;
			}
			// Treat portals as solids for cutting.
			if( Brush->PolyFlags & PF_Portal )
			{
				Brush->PolyFlags = (Brush->PolyFlags & ~PF_Semisolid) | PF_NotSolid;
			}
			// change poly flags
			const UBOOL bIsDetail = Brush->PolyFlags & PF_Semisolid;
			if( bIsDetail )
			{
				Brush->PolyFlags = (Brush->PolyFlags & ~PF_Semisolid );
				for( INT PolyIndex = 0; PolyIndex < Brush->Brush->Polys->Element.Num(); ++PolyIndex )
				{
					FPoly &Poly = Brush->Brush->Polys->Element(PolyIndex);
					Poly.PolyFlags &= ~PF_Semisolid;
				}
			}
			BrushCount++;
			GWarn->StatusUpdatef( BrushCount, BrushTotal, *LocalizeUnrealEd(TEXT("ApplyingBrushF")), BrushCount, BrushTotal );
			bspBrushCSG( Brush, GWorld->GetModel(), Brush->PolyFlags, (ECsgOper)Brush->CsgOper, FALSE, TRUE, FALSE );
			// restore poly flags
			if( bIsDetail )
			{
				Brush->PolyFlags |= PF_Semisolid;
				for( INT PolyIndex = 0; PolyIndex < Brush->Brush->Polys->Element.Num(); ++PolyIndex )
				{
					FPoly &Poly = Brush->Brush->Polys->Element(PolyIndex);
					Poly.PolyFlags |= PF_Semisolid;
				}
			}
		}
	}

	// Repartition World BSP.
	bspRepartition( 0, 0 );

	// Compute World Bsp Nodes's Cluster Number
	UModel *WorldModel = GWorld->GetModel();
	Ava_AssignClusterNumber( StructuralModel, WorldModel );
	bspCleanup( WorldModel );
	bspRefresh( WorldModel, 1 );

	// Build World Bounding Volumes.
	GWarn->StatusUpdatef( 1, 3,  *LocalizeUnrealEd(TEXT("RebuildCSGBuildingBoundingVolumes")) );
	bspOptGeom( GWorld->GetModel() );
	bspBuildBounds( GWorld->GetModel() );

	// Rebuild dynamic brush BSP's.
	GWarn->StatusUpdatef( 2, 3,  *LocalizeUnrealEd(TEXT("RebuildCSGRebuildingDynamicBrushBSPs")) );
	for( FActorIterator It; It; ++It )
	{
		ABrush* B=Cast<ABrush>(*It);
		if(B && B->Brush && !B->IsStaticBrush())
		{
			csgPrepMovingBrush(B);
		}
	}

	// Done.
	FastRebuild = 0;
	GWorld->CurrentLevel->MarkPackageDirty();
	GWarn->EndSlowTask();
}



//<@ ava specific ; 2007. 8. 24 changmin
/**
* Selects surfaces whose material matches selected materials in generic browser.
*
* @param	bCurrentLevelOnly		If TRUE, consider onl surfaces in the current level.
*/
void UEditorEngine::Ava_polySelectMatchingSelectedMaterial(UBOOL bCurrentLevelOnly)
{
	Exec( TEXT("SELECT NONE") );

	// TRUE if at least one surface was selected.
	UBOOL bSurfaceWasSelected = FALSE;

	// TRUE if default material representations have already been added to the materials list.
	UBOOL bDefaultMaterialAdded = FALSE;

	TArray<UMaterialInstance*> Materials;

	UMaterialInstance* SelectedMaterial = GEditor->GetSelectedObjects()->GetTop<UMaterialInstance>();

	if( SelectedMaterial )
	{
		if( bCurrentLevelOnly )
		{
			// Select all surfaces with matching materials.
			for ( TSurfaceIterator<FCurrentLevelSurfaceLevelFilter> It ; It ; ++It )
			{
				if( SelectedMaterial == It->Material )
				{
					UModel* Model = It.GetModel();
					const INT SurfaceIndex = It.GetSurfaceIndex();
					Model->ModifySurf( SurfaceIndex, 0 );
					GEditor->SelectBSPSurf( Model, SurfaceIndex, TRUE, FALSE );
					bSurfaceWasSelected = TRUE;
				}
			}
		}
		else
		{
			// Select all surfaces with matching materials.
			for ( TSurfaceIterator<> It ; It ; ++It )
			{
				// Map the default material to NULL, so that NULL assignments match manual default material assignments.
				if( SelectedMaterial ==  It->Material )
				{
					UModel* Model = It.GetModel();
					const INT SurfaceIndex = It.GetSurfaceIndex();
					Model->ModifySurf( SurfaceIndex, 0 );
					GEditor->SelectBSPSurf( Model, SurfaceIndex, TRUE, FALSE );
					bSurfaceWasSelected = TRUE;
				}
			}

		}
	}

	if ( bSurfaceWasSelected )
	{
		NoteSelectionChange();
	}
}
//>@ ava