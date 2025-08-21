
// AvaBsp :
// Copyright 2007 Redduck, Inc.

#include "EditorPrivate.h"

#define AVA_INCLUDE_BSP_GLOBALS
#include "UnBsp.cpp"
#undef AVA_INCLUDE_BSP_GLOBALS

#pragma optimize("", off)

void AddSolidToDetailFunc( UModel* Model, INT iNode, FPoly* EdPoly,
						  EPolyNodeFilter Filter, ENodePlace ENodePlace )
{
	switch( Filter )
	{
	case F_OUTSIDE:
	case F_COPLANAR_OUTSIDE:
		// Only affect the world poly if it has been cut.
		if( EdPoly->PolyFlags & PF_EdCut )
			GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, EdPoly );
		break;
	case F_INSIDE:
	case F_COPLANAR_INSIDE:
	case F_COSPATIAL_FACING_IN:
	case F_COSPATIAL_FACING_OUT:
		GDiscarded++;
		if( GModel->Nodes(GNode).NumVertices )
		{
			GModel->Nodes.ModifyItem( GNode );
			GModel->Nodes(GNode).NumVertices = 0;
		}
		break;
	}
}

void AddDetailToSolidFunc( UModel* Model, INT iNode, FPoly* EdPoly,
						  EPolyNodeFilter Filter, ENodePlace ENodePlace )
{
	switch( Filter )
	{
	case F_OUTSIDE:
	case F_COPLANAR_OUTSIDE:
		GEditor->bspAddNode (Model,iNode,ENodePlace,NF_IsNew,EdPoly);
		break;
	case F_COSPATIAL_FACING_OUT:
		GEditor->bspAddNode (Model,iNode,ENodePlace,NF_IsNew,EdPoly);
		break;
	case F_INSIDE:
	case F_COPLANAR_INSIDE:
	case F_COSPATIAL_FACING_IN:
		break;
	}
}

void AddOnlyDetailToSolidFunc( UModel* Model, INT iNode, FPoly* EdPoly,
							  EPolyNodeFilter Filter, ENodePlace ENodePlace )
{
	switch( Filter )
	{
	case F_OUTSIDE:
	case F_COPLANAR_OUTSIDE:
		GEditor->bspAddNode (Model,iNode,ENodePlace,NF_IsNew,EdPoly);
		break;
	case F_INSIDE:
	case F_COPLANAR_INSIDE:
	case F_COSPATIAL_FACING_IN:
	case F_COSPATIAL_FACING_OUT:
		break;
	}
}

void FilterStructuralNode( UModel *Bsp, INT iNode, UModel *Detail, INT Outside )
{
	const INT iOriginalNode = iNode;
	const INT iFront = Bsp->Nodes(iOriginalNode).iFront;
	const INT iBack = Bsp->Nodes(iOriginalNode).iBack;

	// traverse tree
	if( iFront != INDEX_NONE
		&& !(Bsp->Surfs(Bsp->Nodes(iFront).iSurf).PolyFlags & PF_Semisolid))
	{
		FilterStructuralNode( Bsp, iFront, Detail, 0 );
	}

	if( iBack != INDEX_NONE 
		&& !(Bsp->Surfs(Bsp->Nodes(iBack).iSurf).PolyFlags & PF_Semisolid) )
	{
		FilterStructuralNode( Bsp, iBack, Detail, 0 );
	}

	while( iNode != INDEX_NONE )
	{
		FPoly Poly;
		if( !(Bsp->Nodes(iNode).NodeFlags & NF_IsNew)
			&&	!(Bsp->Surfs(Bsp->Nodes(iNode).iSurf).PolyFlags & PF_Semisolid)
			&& 	!(Bsp->Surfs(Bsp->Nodes(iNode).iSurf).PolyFlags & PF_Hint)
			&& 	!(Bsp->Surfs(Bsp->Nodes(iNode).iSurf).PolyFlags & PF_Portal)
			&& 	!(Bsp->Surfs(Bsp->Nodes(iNode).iSurf).PolyFlags & PF_Invisible)
			&& 	!(Bsp->Surfs(Bsp->Nodes(iNode).iSurf).PolyFlags & PF_NotSolid)
			&&	GEditor->bspNodeToFPoly( Bsp, iNode, &Poly ) )
		{
			const INT OriginalNumNodes = Bsp->Nodes.Num();
			GModel			= Bsp;
			GNode			= iNode;
			GDiscarded		= 0;
			GLastCoplanar	= iNode;
			while( Bsp->Nodes(GLastCoplanar).iPlane != INDEX_NONE )
			{
				GLastCoplanar = Bsp->Nodes(GLastCoplanar).iPlane;
			}
			BspFilterFPoly( AddSolidToDetailFunc, Detail, &Poly );
			const INT Count = Bsp->Nodes.Num() - OriginalNumNodes;

			if( GDiscarded == 0 )
			{
				Bsp->Nodes(GLastCoplanar).iPlane = INDEX_NONE;
				Bsp->Nodes.Remove( OriginalNumNodes, Bsp->Nodes.Num() - OriginalNumNodes );
			}
			else
			{
				if( Bsp->Nodes(iNode).NumVertices )
				{
					Bsp->Nodes.ModifyItem(iNode);
					Bsp->Nodes(iNode).NumVertices = 0;
					for( INT i = OriginalNumNodes; i < Bsp->Nodes.Num(); ++i )
					{
						Bsp->Nodes(i).iCluster = Bsp->Nodes(iNode).iCluster;
					}
				}
			}
		}
		iNode = Bsp->Nodes(iNode).iPlane;
	}
}

void UEditorEngine::FilterStructuralNodes( UModel *Bsp, UModel *Detail )
{
	if( Bsp->Nodes.Num() )
	{
		FilterStructuralNode( Bsp, 0, Detail, Detail->RootOutside );
	}
}

//////////////////// Assign Cluster Number to World Bsp Node //////////////////////////////
void AssignClusterNumberFunc( UModel* Model, INT iNode, FPoly* EdPoly, EPolyNodeFilter Filter, ENodePlace ENodePlace )
{
	switch( Filter )
	{
	case F_OUTSIDE:
	case F_COSPATIAL_FACING_OUT:
		// Only affect the world poly if it has been cut.
		GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, EdPoly, Model->Nodes(iNode).iCluster );
		break;
	case F_COPLANAR_OUTSIDE:
	case F_INSIDE:
	case F_COPLANAR_INSIDE:
	case F_COSPATIAL_FACING_IN:
		break;
	}
}

void FilterThroughStructuralTree( UModel *StructuralBsp, INT iNode, FPoly Poly )
{
	FBspNode &Node = StructuralBsp->Nodes(iNode);
	const INT FrontChild	= Node.iFront;
	const INT BackChild		= Node.iBack;
	const INT FrontLeaf		= Node.iLeaf[1];
	const INT BackLeaf		= Node.iLeaf[0];

	FPoly Front, Back;

	//<@ 2008. 3. 4 정밀도 에러 때문에, Invalid leaf 로 떨어지는 node들이 생긴다.
	//int Split = Poly.SplitWithNode( StructuralBsp, iNode, &Front, &Back, 1 );
	int Split = Poly.SplitWithNode( StructuralBsp, iNode, &Front, &Back, 0 );
	//>@ 

	switch( Split )
	{
	case SP_Coplanar:
		if( (Poly.Normal | Node.Plane) > 0.0f )
		{
			if( FrontChild == INDEX_NONE )
			{
				check( FrontLeaf != INDEX_NONE );
				GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, &Poly, FrontLeaf );
			}
			else
			{
				FilterThroughStructuralTree( StructuralBsp, FrontChild, Poly );
			}
		}
		else
		{
			if( BackChild == INDEX_NONE )
			{
				if( BackLeaf != INDEX_NONE )
				{
					GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, &Poly, BackLeaf );
				}
			}
			else
			{
				FilterThroughStructuralTree( StructuralBsp, BackChild, Poly );
			}
		}
		break;

	case SP_Front:
		if( FrontChild == INDEX_NONE )
		{
			check( FrontLeaf != INDEX_NONE );
			GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, &Poly, FrontLeaf );
		}
		else
		{
			FilterThroughStructuralTree( StructuralBsp, FrontChild, Poly );
		}
		break;

	case SP_Back:
		if( BackChild == INDEX_NONE )
		{
			if( BackLeaf != INDEX_NONE )
			{
				GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, &Poly, BackLeaf );
			}
		}
		else
		{
			FilterThroughStructuralTree( StructuralBsp, BackChild, Poly );
		}
		break;

	case SP_Split:
		if( FrontChild == INDEX_NONE )
		{
			check( FrontLeaf != INDEX_NONE );
			GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, &Front, FrontLeaf );
		}
		else
		{
			FilterThroughStructuralTree( StructuralBsp, FrontChild, Front );
		}
		if( BackChild == INDEX_NONE )
		{
			if( BackLeaf != INDEX_NONE )
			{
				GEditor->bspAddNode( GModel, GLastCoplanar, NODE_Plane, NF_IsNew, &Back, BackLeaf );
			}
		}
		else
		{
			FilterThroughStructuralTree( StructuralBsp, BackChild, Back );
		}
		break;
	}
}


// WorldBsp 를 순회하면서, 각 Node를 Structural Bsp로 Filtering하여, ClusterNumber를 준다.
void AssignClusterNumberToNode( UModel *StructuralBsp, INT iNode, UModel *WorldBsp )
{
	const INT iOriginalNode	= iNode;
	const INT iFront		= WorldBsp->Nodes(iOriginalNode).iFront;
	const INT iBack			= WorldBsp->Nodes(iOriginalNode).iBack;

	// traverse tree
	if( iFront != INDEX_NONE )
	{
		AssignClusterNumberToNode( StructuralBsp, iFront, WorldBsp );
	}
	if( iBack != INDEX_NONE )
	{
		AssignClusterNumberToNode( StructuralBsp, iBack, WorldBsp );
	}

	while( iNode != INDEX_NONE )
	{
		FPoly Poly;

		if( !(WorldBsp->Nodes(iNode).NodeFlags & NF_IsNew) && GEditor->bspNodeToFPoly( WorldBsp, iNode, &Poly ) )
		{
			const INT OriginalNumNodes = WorldBsp->Nodes.Num();
			GModel			= WorldBsp;
			GNode			= iNode;
			GDiscarded		= 0;
			GLastCoplanar	= iNode;
			while( WorldBsp->Nodes(GLastCoplanar).iPlane != INDEX_NONE )
			{
				GLastCoplanar = WorldBsp->Nodes(GLastCoplanar).iPlane;
			}

			//BspFilterFPoly( AssignClusterNumberFunc, StructuralBsp, &Poly );
			FilterThroughStructuralTree( StructuralBsp, 0, Poly );

			const INT Count = WorldBsp->Nodes.Num() - OriginalNumNodes;

			// 모든 node가 같은 cluster인지 check
			if( WorldBsp->Nodes.Num() > OriginalNumNodes )
			{
				INT CanMerge = 1, ClusterNumber = INDEX_NONE ;
				for( INT NewNodeIndex = OriginalNumNodes; NewNodeIndex < WorldBsp->Nodes.Num(); ++NewNodeIndex )
				{
					if( WorldBsp->Nodes(NewNodeIndex).iCluster != INDEX_NONE )
					{
						ClusterNumber = WorldBsp->Nodes(NewNodeIndex).iCluster;
					}
				}
				for( INT NewNodeIndex = OriginalNumNodes; NewNodeIndex < WorldBsp->Nodes.Num(); ++NewNodeIndex )
				{
					if( WorldBsp->Nodes(NewNodeIndex).iCluster != INDEX_NONE
					&&	WorldBsp->Nodes(NewNodeIndex).iCluster != ClusterNumber )
					{
						CanMerge = 0;
					}
				}
				if( CanMerge )
				{
					// original node에 cluster number를 할당합니다.
					WorldBsp->Nodes(GLastCoplanar).iPlane = INDEX_NONE;
					WorldBsp->Nodes.Remove( OriginalNumNodes, WorldBsp->Nodes.Num() - OriginalNumNodes );
					check( ClusterNumber != INDEX_NONE );
					WorldBsp->Nodes(iNode).iCluster = ClusterNumber;
				}
				else
				{
					// 한 node가 여러 cluster에 걸쳐 있으니, original node를 지웁니다.
					if( WorldBsp->Nodes(iNode).NumVertices )
					{
						WorldBsp->Nodes.ModifyItem(iNode);
						WorldBsp->Nodes(iNode).NumVertices = 0;
					}
				}
			}
			else
			{
				//check(0);
				WorldBsp->Nodes(iNode).iCluster = INDEX_NONE;
			}
		}

		iNode = WorldBsp->Nodes(iNode).iPlane;
	}
}


void UEditorEngine::Ava_AssignClusterNumber( UModel *StructuralBsp, UModel *WorldBsp )
{
	if( StructuralBsp->Nodes.Num() )
	{
		AssignClusterNumberToNode( StructuralBsp, 0, WorldBsp );
	}
}

void UEditorEngine::bspAddDetailToStructure( UModel *Structure, UModel *Detail )
{
	bspBuildFPolys( Detail, 1, 0 );
	bspMergeCoplanars( Detail, 0, 0 );

	// 새로운 surface link를 할당한다.
	const INT DetailSurfaceNum = Detail->Surfs.Num();
	for( INT PolyIndex = 0; PolyIndex < Detail->Polys->Element.Num(); ++PolyIndex )
	{
		const INT OldLink = Detail->Polys->Element(PolyIndex).iLink;

		check( OldLink != INDEX_NONE );

		// it's updated
		if( OldLink > Detail->Surfs.Num() )
		{
			continue;
		}

		const INT NewLink = Detail->Surfs.Num() + PolyIndex;
		for( INT iPoly = PolyIndex; iPoly < Detail->Polys->Element.Num(); ++iPoly )
		{
			FPoly &EdPoly = Detail->Polys->Element(iPoly);
			if( EdPoly.iLink == OldLink )
			{
				EdPoly.iLink = NewLink;
			}
		}
	}
	for( INT PolyIndex = 0; PolyIndex < Detail->Polys->Element.Num(); ++PolyIndex )
	{
		FPoly &TempEdPoly = Detail->Polys->Element(PolyIndex);
		if( TempEdPoly.iLink == Detail->Surfs.Num() + PolyIndex )
		{
			TempEdPoly.iLink = Structure->Surfs.Num();
		}
		else
		{
			TempEdPoly.iLink = Detail->Polys->Element( TempEdPoly.iLink - Detail->Surfs.Num() ).iLink;
		}
		FPoly EdPoly = TempEdPoly;
		EdPoly.PolyFlags |= PF_Semisolid;
		if( bFullBuildGeometry )
		{
			BspFilterFPoly( AddDetailToSolidFunc, Structure, &EdPoly );
		}
		else
		{
			BspFilterFPoly( AddOnlyDetailToSolidFunc, Structure, &EdPoly );
		}
	}

	// 가려진 solid face를 제거한다.
	if( bFullBuildGeometry )
	{
		//<@ ava speicifc ; 2007. 1. 11 changmin
		// filtering solid node
		bspBuild( Detail, BSP_Lame, 0, 70, 1, 0 );
		FilterStructuralNodes( GWorld->GetModel(), Detail );
		//>@ava
	}

	// Clean up nodes, reset node flags.
	bspCleanup( Structure );

	// assign cluster number to detail nodes
	AssignDetailClusters( Structure );
}
//>@ ava

//<@ ava specific ; 2008. 3. 6 changmin
/**
* Iterator used to iterate over all static brush actors in the current level.
*/
class AVA_StaticBrushIterator
{
public:
	/**
	* Default constructor, initializing all member variables and iterating to first.
	*/
	AVA_StaticBrushIterator()
		:	ActorIndex( -1 ),
		ReachedEnd( FALSE )
	{
		// Iterate to first.
		++(*this);
	}

	/**
	* Iterates to next suitable actor.
	*/
	void operator++()
	{
		UBOOL FoundSuitableActor = FALSE;
		while( !ReachedEnd && !FoundSuitableActor )
		{
			if( ++ActorIndex >= GWorld->CurrentLevel->Actors.Num() )
			{
				ReachedEnd = TRUE;
			}
			else
			{
				AActor* Actor = GWorld->CurrentLevel->Actors(ActorIndex);
				FoundSuitableActor = Actor && Actor->IsStaticBrush();
			}
		}
	}

	/**
	* Returns the current suitable actor pointed at by the Iterator
	*
	* @return	Current suitable actor
	*/
	AActor* operator*()
	{
		check(ActorIndex<=GWorld->CurrentLevel->Actors.Num());
		check(!ReachedEnd);
		AActor* Actor = GWorld->CurrentLevel->Actors(ActorIndex);
		return Actor;
	}

	/**
	* Returns the current suitable actor pointed at by the Iterator
	*
	* @return	Current suitable actor
	*/
	AActor* operator->()
	{
		check(ActorIndex<=GWorld->CurrentLevel->Actors.Num());
		check(!ReachedEnd);
		AActor* Actor = GWorld->CurrentLevel->Actors(ActorIndex);
		return Actor;
	}

	/**
	* Returns whether the iterator has reached the end and no longer points
	* to a suitable actor.
	*
	* @return TRUE if iterator points to a suitable actor, FALSE if it has reached the end
	*/
	operator UBOOL()
	{
		return !ReachedEnd;
	}

protected:
	/** Current index into actors array							*/
	INT		ActorIndex;
	/** Whether we already reached the end						*/
	UBOOL	ReachedEnd;
};
void AVA_IntersectBrushWithWorldFunc( UModel* Model, INT iNode, FPoly *EdPoly, EPolyNodeFilter Filter,ENodePlace ENodePlace )
{
	switch( Filter )
	{
	case F_OUTSIDE:
	case F_COPLANAR_OUTSIDE:
	case F_COSPATIAL_FACING_IN:
	case F_COSPATIAL_FACING_OUT:
	case F_COPLANAR_INSIDE:
		// Ignore.
		break;
	case F_INSIDE:
		if( EdPoly->Fix()>=3 )
			new(GModel->Polys->Element)FPoly(*EdPoly);
		break;
	}
}

void CreateModelFromBrush( ABrush* SrcBrushActor, UModel* Model )
{
	UModel *Brush = SrcBrushActor->Brush;

	Model->EmptyModel(1,1);
	
	for(INT i=0; i<Brush->Polys->Element.Num();++i)
	{
		FPoly& CurrentPoly = Brush->Polys->Element(i);

		// Get the brush poly.
		FPoly DestEdPoly = CurrentPoly;

		// Set its backward brush link.
		DestEdPoly.Actor = SrcBrushActor;
		DestEdPoly.iBrushPoly = i;

		// set its interal link
		if( DestEdPoly.iLink == INDEX_NONE )
		{
			DestEdPoly.iLink = i;
		}

		// Transform it.
		DestEdPoly.Transform( SrcBrushActor->PrePivot, SrcBrushActor->Location );

		// Add poly to the temp model.
		new( Model->Polys->Element) FPoly( DestEdPoly );
	}
}

void UEditorEngine::AVA_CheckBrushIntersection( UBOOL bSolidBrushOnly )
{
	FinishAllSnaps();

	// Filtered model은 이 함수가 끝나면 GC 될 겁니다..
	//UModel* FilteredBrush = new UModel(NULL, 1);
	UModel* FilteredBrush = GWorld->GetBrush()->Brush;

	UModel* OtherModel = new UModel( NULL, 1 );

	ABrush *IntersectedBrush1 = NULL;
	ABrush *IntersectedBrush2 = NULL;

	INT NumPolysFromBrush = 0;

	for( AVA_StaticBrushIterator It; It; ++It )
	{
		ABrush *BrushActor = CastChecked<ABrush>(*It);
		if( !BrushActor->IsABuilderBrush() )
		{
			if( BrushActor->PolyFlags & PF_Hint
				|| BrushActor->PolyFlags & PF_Portal
				|| BrushActor->PolyFlags & PF_NotSolid
				|| BrushActor->CsgOper == CSG_Subtract)
			{
				continue;
			}

			if( bSolidBrushOnly && (BrushActor->PolyFlags & PF_Semisolid) )
			{
				continue;
			}

			// copy test brush.
			CreateModelFromBrush( BrushActor, TempModel );
			bspBuild( TempModel, BSP_Lame, 0, 70, 1, 0 );
			
			for( AVA_StaticBrushIterator it2; it2; ++it2 )
			{
				ABrush *OtherBrushActor = CastChecked<ABrush>(*it2);
				if( !OtherBrushActor->IsABuilderBrush() )
				{
					if( OtherBrushActor == BrushActor
					|| OtherBrushActor->PolyFlags & PF_Hint
					|| OtherBrushActor->PolyFlags & PF_Portal
					|| OtherBrushActor->PolyFlags & PF_NotSolid
					|| OtherBrushActor->CsgOper == CSG_Subtract)
					{
						continue;
					}

					if( bSolidBrushOnly && (OtherBrushActor->PolyFlags & PF_Semisolid) )
					{
						continue;
					}

					CreateModelFromBrush( OtherBrushActor, OtherModel );

					// other brush를 현재 brush의 bsp에 넣어본다.
					FilteredBrush->EmptyModel(1,1);
					for( INT i = 0; i < OtherModel->Polys->Element.Num(); ++ i )
					{
						FPoly EdPoly = OtherModel->Polys->Element(i);
						GModel = FilteredBrush;
						BspFilterFPoly( AVA_IntersectBrushWithWorldFunc, TempModel, &EdPoly );
					}

					// find??
					NumPolysFromBrush = FilteredBrush->Polys->Element.Num();
					if( NumPolysFromBrush > 0 )
					{
						IntersectedBrush2 = OtherBrushActor;
						break;
					}
				}
			}

			// find??
			if( NumPolysFromBrush > 0 )
			{
				IntersectedBrush1 = BrushActor;
				break;
			}
		}
	}

	if( IntersectedBrush1 != NULL )
	{
		debugf(NAME_Warning, TEXT("Find Intersected Brush : %s, %s"), *IntersectedBrush1->GetName(), *IntersectedBrush2->GetName());

		SelectNone( TRUE, TRUE );
		SelectActor( IntersectedBrush1, TRUE, NULL, FALSE );
		SelectActor( IntersectedBrush2, TRUE, NULL, FALSE );
		NoteSelectionChange();

		//
		INT i, j;
		for( i = NumPolysFromBrush-1; i >=0 ; i-- )
		{
			FPoly *DestEdPoly = &FilteredBrush->Polys->Element(i);
			for( j = 0; j < i; ++j )
			{
				if (DestEdPoly->iLink == FilteredBrush->Polys->Element(j).iLink)
				{
					DestEdPoly->iLink = j;
					break;
				}
			}
			if( j >= i ) DestEdPoly->iLink = i;
		}

		FilteredBrush->Linked = 1;

		for( i = 0; i < FilteredBrush->Polys->Element.Num(); ++i )
		{
			FPoly *DestEdPoly = &FilteredBrush->Polys->Element(i);
			DestEdPoly->Transform(GWorld->GetBrush()->Location, GWorld->GetBrush()->PrePivot);
			DestEdPoly->Fix();
			DestEdPoly->Actor = NULL;
			DestEdPoly->iBrushPoly = i;
		}

		GWorld->GetBrush()->ClearComponents();
		GWorld->GetBrush()->ConditionalUpdateComponents();
		GEditorModeTools().GetCurrentMode()->MapChangeNotify();
		RedrawLevelEditingViewports();

	}

	TempModel->EmptyModel(1,1);
	OtherModel->EmptyModel(1,1);
	//FilteredBrush->EmptyModel(1,1);
}
//<@ ava
#pragma optimize("", on)