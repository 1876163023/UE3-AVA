/*=============================================================================
	UnVisi.cpp: Unreal visibility computation
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.

Description:
	Visibility and zoining code.
=============================================================================*/

#include "EditorPrivate.h"

//<@ ava specific ; 2006. 12. 18 changmin
#include "EnginePhysicsClasses.h"
//>@ ava

/*-----------------------------------------------------------------------------
	Temporary.
-----------------------------------------------------------------------------*/

// Options.
#define DEBUG_PORTALS	0	/* Debugging hull code by generating hull brush */
#define DEBUG_WRAPS		0	/* Debugging sheet wrapping code */
#define DEBUG_BADSHEETS 0   /* Debugging sheet discrepancies */
#define DEBUG_LVS       0   /* Debugging light volumes */
FPoly BuildInfiniteFPoly( UModel *Model, INT iNode );

#define AVA_DEBUG_COLLISION 0	/* Debugging Collision Bounds */

// Thresholds.
#define VALID_SIDE         0.1   /* A normal must be at laest this long to be valid */
#define VALID_CROSS        0.001 /* A cross product can be safely normalized if this big */

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

//
// Debugging.
//
#if DEBUG_PORTALS || DEBUG_WRAPS || DEBUG_BADSHEETS || DEBUG_LVS
	static ABrush *DEBUG_Brush;
#endif

#pragma optimize( "", off )

//
// A portal.
//
class FPortal : public FPoly
{
public:
	// Variables.
	INT	    iFrontLeaf, iBackLeaf, iNode;
	FPortal *GlobalNext, *FrontLeafNext, *BackLeafNext, *NodeNext;
	BYTE	IsTesting, ShouldTest;
	INT		FragmentCount;
	INT	    iZonePortalSurf;
	//<@ ava specific ; 2006. 11. 27 changmin
	UBOOL	IsHint;
	//>@ ava

	// Constructor.
	FPortal( FPoly &InPoly, INT iInFrontLeaf, INT iInBackLeaf, INT iInNode, FPortal *InGlobalNext, FPortal *InNodeNext, FPortal *InFrontLeafNext, FPortal *InBackLeafNext, UBOOL InHint )
	:	FPoly			(InPoly),
		iFrontLeaf		(iInFrontLeaf),
		iBackLeaf		(iInBackLeaf),
		iNode			(iInNode),
		GlobalNext		(InGlobalNext),
		NodeNext		(InNodeNext),
		FrontLeafNext	(InFrontLeafNext),
		BackLeafNext	(InBackLeafNext),
		IsTesting		(0),
		ShouldTest		(0),
		FragmentCount	(0),
		iZonePortalSurf (INDEX_NONE),
		//<@ ava specific ; 2006. 11. 27 changmin
		IsHint( InHint )
		//># ava
	{}
	
	// Get the leaf on the opposite side of the specified leaf.
	INT GetNeighborLeafOf( INT iLeaf )
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		if     ( iFrontLeaf == iLeaf )	return iBackLeaf;
		else							return iFrontLeaf;
	}

	// Get the next portal for this leaf in the linked list of portals.
	FLOAT Area()
	{
		FVector Cross(0,0,0);
		for( INT i=2; i<Vertices.Num(); i++ )
			Cross += (Vertices(i-1)-Vertices(0)) ^ (Vertices(i)-Vertices(0));
		return Cross.Size();
	}
	FPortal* Next( INT iLeaf )
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		if     ( iFrontLeaf == iLeaf )	return FrontLeafNext;
		else							return BackLeafNext;
	}

	// Return this portal polygon, facing outward from leaf iLeaf.
	void GetPolyFacingOutOf( INT iLeaf, FPoly &Poly )
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		Poly = *(FPoly*)this;
		if( iLeaf == iFrontLeaf ) Poly.Reverse();
	}

	// Return this portal polygon, facing inward to leaf iLeaf.
	void GetPolyFacingInto( INT iLeaf, FPoly &Poly)
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		Poly = *(FPoly*)this;
		if( iLeaf == iBackLeaf ) Poly.Reverse();
	}
};

//
// The visibility calculator class.
//
class FEditorVisibility
{
public:
	// Constants.
	enum {MAX_CLIPS=16384};
	enum {CLIP_BACK_FLAG=0x40000000};

	// Types.
	typedef void (FEditorVisibility::*PORTAL_FUNC)(FPoly&,INT,INT,INT,INT);

	// Variables.
	FMemMark		Mark;
	UModel*			Model;
	INT				Clips[MAX_CLIPS];
	INT				NumPortals, NumLogicalLeaves;
	INT				NumClips, NumClipTests, NumPassedClips, NumUnclipped;
	INT				NumBspPortals, MaxFragments, NumZonePortals, NumZoneFragments;
	INT				Extra;
	INT				iZonePortalSurf;
	FPortal*		FirstPortal;
	FPortal**		NodePortals;
	FPortal**		LeafPortals;

	//<@ ava specific ; 2006. 11. 6 changmin
	FPortal*		SolidPortals;
	//>@ ava

	// Constructor.
	FEditorVisibility( UModel* InModel, INT InDebug );

	// Destructor.
	~FEditorVisibility();

	// Portal functions.
	void AddPortal( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void BlockPortal( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void TagZonePortalFragment( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void FilterThroughSubtree( INT Pass, INT iGeneratingNode, INT iGeneratingBase, INT iParentLeaf, INT iNode, FPoly Poly, PORTAL_FUNC Func, INT iBackLeaf );
	void MakePortalsClip( INT iNode, FPoly Poly, INT Clip, PORTAL_FUNC Func );
	void MakePortals( INT iNode );
	void AssignLeaves( INT iNode, INT Outside );

	// Zone functions.
	void FormZonesFromLeaves();
	void AssignAllZones( INT iNode, int Outside );
	void BuildConnectivity();

	// Visibility functions.
	void TestVisibility();

	//<@ ava specific ; 2006. 12. 14 changmin.
	UBOOL SavePortalsToFile();
	UBOOL LoadVisFile();
	void AssignAllClusters( INT iNode, INT Outside );
	void TagNodeFragment( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void FilterSolidNodes( INT iNode, INT Outside );
	void ComputeStaticMeshActorLeaves();
	void ClearStaticMeshLeafInformation();
	void AssignDetailCluster( INT iNode );
	//>@ ava
};

/*-----------------------------------------------------------------------------
	Portal building, a simple recursive hierarchy of functions.
-----------------------------------------------------------------------------*/

//
// Tag a zone portal fragment.
//
void FEditorVisibility::TagZonePortalFragment
(
	FPoly&	Poly,
	INT	    iFrontLeaf,
	INT		iBackLeaf,
	INT		iGeneratingNode,
	INT		iGeneratingBase
)
{
	// Add this node to the bsp as a coplanar to its generator.
	INT iNewNode = GEditor->bspAddNode( Model, iGeneratingNode, NODE_Plane, Model->Nodes(iGeneratingNode).NodeFlags | NF_IsNew, &Poly );

	// Set the node's zones.
	int Backward = (Poly.Normal | Model->Nodes(iGeneratingBase).Plane) < 0.0;
	Model->Nodes(iNewNode).iZone[Backward^0] = iBackLeaf ==INDEX_NONE ? 0 : Model->Leaves(iBackLeaf ).iZone;
	Model->Nodes(iNewNode).iZone[Backward^1] = iFrontLeaf==INDEX_NONE ? 0 : Model->Leaves(iFrontLeaf).iZone;
}

//
// Mark a portal as blocked.
//
void FEditorVisibility::BlockPortal
(
	FPoly&	Poly,
	INT		iFrontLeaf,
	INT		iBackLeaf,
	INT		iGeneratingNode,
	INT		iGeneratingBase
)
{
	if( iFrontLeaf!=INDEX_NONE && iBackLeaf!=INDEX_NONE )
	{
		for( FPortal* Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
		{
			if
			(	(Portal->iFrontLeaf==iFrontLeaf && Portal->iBackLeaf==iBackLeaf )
			||	(Portal->iFrontLeaf==iBackLeaf  && Portal->iBackLeaf==iFrontLeaf) )
			{
				Portal->iZonePortalSurf = iZonePortalSurf;
				NumZoneFragments++;
			}
		}
	}
}

//
// Add a portal to the portal list.
//
void FEditorVisibility::AddPortal
(
	FPoly&	Poly,
	INT		iFrontLeaf,
	INT		iBackLeaf,
	INT		iGeneratingNode,
	INT		iGeneratingBase
)
{
	if( iFrontLeaf!=INDEX_NONE && iBackLeaf!=INDEX_NONE )
	{
		//<@ ava specific ; 2006. 11. 27 changmin
		UBOOL IsHint = Model->Nodes(iGeneratingNode).NodeFlags & NF_Hint;
		//>@ ava
		// Add to linked list of all portals.
		FirstPortal						= 
		LeafPortals[iFrontLeaf]			= 
		LeafPortals[iBackLeaf]			= 
		NodePortals[iGeneratingNode]	= 
			new(GMem)FPortal
			(
				Poly,
				iFrontLeaf,
				iBackLeaf,
				iGeneratingNode,
				FirstPortal,
				NodePortals[iGeneratingNode],
				LeafPortals[iFrontLeaf],
				LeafPortals[iBackLeaf]
				//<@ ava specific ; 2006. 11. 27 changmin
				,IsHint
				//>@ ava
			);
		NumPortals++;

#if DEBUG_PORTALS
		//debugf("AddPortal: %i verts",Poly.NumVertices);
		Poly.PolyFlags |= PF_NotSolid;
		new(DEBUG_Brush->Brush->Polys->Element)FPoly(Poly);
#endif
	}
	//<@ ava specific ; 2006. 11. 7 changmin
	// solid portal list
	else if( iFrontLeaf != INDEX_NONE || iBackLeaf != INDEX_NONE )
	{
		SolidPortals =
			new(GMem)FPortal
			(
				Poly,
				iFrontLeaf,
				iBackLeaf,
				iGeneratingNode,
				SolidPortals,
				NULL,	// node portals 
				NULL,	// front leaf portals
				NULL	// back leaf portals
				, FALSE
			);

	
#if DEBUG_PORTALS
		//debugf("AddPortal: %i verts",Poly.NumVertices);
		//if( iFrontLeaf != INDEX_NONE /*|| iBackLeaf != INDEX_NONE*/ )
		//{
		//	Poly.PolyFlags |= PF_NotSolid;
		//	new(DEBUG_Brush->Brush->Polys->Element)FPoly(Poly);
		//}
#endif
	}
	//>@ ava
}

//
// Filter a portal through a front or back subtree.
//
void FEditorVisibility::FilterThroughSubtree
(
	INT			Pass,
	INT			iGeneratingNode,
	INT			iGeneratingBase,
	INT			iParentLeaf,
	INT			iNode,
	FPoly		Poly,
	PORTAL_FUNC Func,
	INT			iBackLeaf
)
{
	while( iNode != INDEX_NONE )
	{
		// Test split.
		FPoly Front,Back;
		int Split = Poly.SplitWithNode( Model, iNode, &Front, &Back, 1 );

		// Recurse with front.
		if( Split==SP_Front || Split==SP_Split )
			FilterThroughSubtree
			(
				Pass,
				iGeneratingNode,
				iGeneratingBase,
				Model->Nodes(iNode).iLeaf[1],
				Model->Nodes(iNode).iFront,
				Split==SP_Front ? Poly : Front,
				Func,
				iBackLeaf
			);

		// Consider back.
		if( Split!=SP_Back && Split!=SP_Split )
			return;

		// Loop with back.
		if( Split == SP_Split )
			Poly = Back;
		iParentLeaf = Model->Nodes(iNode).iLeaf[0];
		iNode       = Model->Nodes(iNode).iBack;
	}

	// We reached a leaf in this subtree.
	if( Pass == 0 ) FilterThroughSubtree
	(
		1,
		iGeneratingNode,
		iGeneratingBase,
		Model->Nodes(iGeneratingBase).iLeaf[1],
		Model->Nodes(iGeneratingBase).iFront,
		Poly,
		Func,
		iParentLeaf
	);
	else (this->*Func)( Poly, iParentLeaf, iBackLeaf, iGeneratingNode, iGeneratingBase );
}

//
// Clip a portal by all parent nodes above it.
//
void FEditorVisibility::MakePortalsClip
(
	INT			iNode,
	FPoly		Poly,
	INT			Clip,
	PORTAL_FUNC Func
)
{
	// Clip by all parents.
	while( Clip < NumClips )
	{
		INT iClipNode = Clips[Clip] & ~CLIP_BACK_FLAG;

		// Split by parent.
		FPoly Front,Back;
		int Split = Poly.SplitWithNode(Model,iClipNode,&Front,&Back,1);

		// Make sure we generated a useful fragment.
		if(	(Split==SP_Front &&  (Clips[Clip] & CLIP_BACK_FLAG) )
		||	(Split==SP_Back  && !(Clips[Clip] & CLIP_BACK_FLAG) )
		||	(Split==SP_Coplanar))
		{
			// Clipped to oblivion, or useless coplanar.
			return;
		}

		if( Split==SP_Split )
		{
			// Keep the appropriate piece.
			Poly = (Clips[Clip] & CLIP_BACK_FLAG) ? Back : Front;
		}

		// Clip by next parent.
		Clip++;
	}

	// Filter poly down the back subtree.
	FilterThroughSubtree
	(
		0,
		iNode,
		iNode,
		Model->Nodes(iNode).iLeaf[0],
		Model->Nodes(iNode).iBack,
		Poly,
		Func,
		INDEX_NONE
	);
}

//
// Make all portals.
//
void FEditorVisibility::MakePortals( INT iNode )
{
	INT iOriginalNode = iNode;

	// Make an infinite edpoly for this node.
	FPoly Poly = BuildInfiniteFPoly( Model, iNode );

	//<@ ava specific ; 2006. 12.12 changmin
	// world bounds로 쪼갠다.
#define SIDESPACE 8
	FBox TreeBound = Model->TreeBounds;
	FVector bounds[2];
	for( int i = 0; i < 3; ++i )
	{
		bounds[0][i] = TreeBound.Min[i] - SIDESPACE;
		bounds[1][i] = TreeBound.Max[i] + SIDESPACE;
	}

	FPlane plane[6];
	for( int i = 0; i < 3; ++i )
	{
		for( int  j = 0; j < 2; ++j )
		{
			int n = j * 3 + i;
			FPlane *pl = &plane[n];
			memset( pl, 0, sizeof(*pl) );
			if( j )
			{
				(*pl)[i] = -1;
				pl->W = -bounds[j][i];
			}
			else
			{
				(*pl)[i] = 1;
				pl->W = bounds[j][i];
			}
		}
	}

	FPoly FrontPoly, BackPoly;
	for( int i =0 ; i < 6; ++i )
	{
		FVector Base = plane[i] * plane[i].W;
		switch( Poly.SplitWithPlane(Base,(FVector)plane[i],&FrontPoly,&BackPoly,0) )
		{
		case SP_Split:
			Poly = FrontPoly;
			break;
		default:
			break;
		}
	}
	//>@ ava

	// Filter the portal through this subtree.
	MakePortalsClip( iNode, Poly, 0, &FEditorVisibility::AddPortal );

	// Make portals for front.
	if( Model->Nodes(iNode).iFront != INDEX_NONE )
	{
		Clips[NumClips++] = iNode;
		MakePortals( Model->Nodes(iNode).iFront );
		NumClips--;
	}

	// Make portals for back.
	if( Model->Nodes(iNode).iBack != INDEX_NONE )
	{
		Clips[NumClips++] = iNode | CLIP_BACK_FLAG;
		MakePortals( Model->Nodes(iNode).iBack );
		NumClips--;
	}

	// For all zone portals at this node, mark the matching FPortals as blocked.
	while( iNode != INDEX_NONE )
	{
		FBspNode& Node = Model->Nodes( iNode      );
		FBspSurf& Surf = Model->Surfs( Node.iSurf );
		if( (Surf.PolyFlags & PF_Portal) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
		{
			Model->PortalNodes.AddItem(iNode);
			NumZonePortals++;
			iZonePortalSurf = Node.iSurf;
			FilterThroughSubtree
			(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes(iOriginalNode).iLeaf[0],
				Model->Nodes(iOriginalNode).iBack,
				Poly,
				&FEditorVisibility::BlockPortal,
				INDEX_NONE
			);
		}
		iNode = Node.iPlane;
	}
}

/*-----------------------------------------------------------------------------
	Assign leaves.
-----------------------------------------------------------------------------*/

//
// Assign contiguous unique numbers to all front and back leaves in the BSP.
// Stores the leaf numbers in FBspNode::iLeaf[2].
//
void FEditorVisibility::AssignLeaves( INT iNode, INT Outside )
{
	FBspNode &Node = Model->Nodes(iNode);
	for( int IsFront=0; IsFront<2; IsFront++ )
	{
		if( Node.iChild[IsFront] != INDEX_NONE )
		{
			AssignLeaves( Node.iChild[IsFront], Node.ChildOutside( IsFront, Outside, NF_NotVisBlocking ) );
		}
		else if( Node.ChildOutside( IsFront, Outside, NF_NotVisBlocking ) )
		{
			Node.iLeaf[IsFront] = Model->Leaves.AddItem(FLeaf(Model->Leaves.Num()));
		}
	}
}

/*-----------------------------------------------------------------------------
	Zoning.
-----------------------------------------------------------------------------*/

//
// Form zones from the leaves.
//
void FEditorVisibility::FormZonesFromLeaves()
{
	FMemMark Mark(GMem);

	// Go through all portals and merge the adjoining zones.
	for( FPortal* Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
	{
		if( Portal->iZonePortalSurf==INDEX_NONE )//!!&& Abs(Portal->Area())>10.0 )
		{
			INT Original = Model->Leaves(Portal->iFrontLeaf).iZone;
			INT New      = Model->Leaves(Portal->iBackLeaf ).iZone;
			for( INT i=0; i<Model->Leaves.Num(); i++ )
			{
				if( Model->Leaves(i).iZone == Original )
					Model->Leaves(i).iZone = New;
			}
		}
	}
	// Renumber the leaves.
	INT NumZones=0;
	for( INT i=0; i<Model->Leaves.Num(); i++ )
	{
		if( Model->Leaves(i).iZone >= NumZones )
		{
			for( int j=i+1; j<Model->Leaves.Num(); j++ )
				if( Model->Leaves(j).iZone == Model->Leaves(i).iZone )
					Model->Leaves(j).iZone = NumZones;
			Model->Leaves(i).iZone = NumZones++;
		}
	}
	debugf( NAME_Log, TEXT("Found %i zones"), NumZones );

	// Confine the zones to 1-63.
	for( INT i=0; i<Model->Leaves.Num(); i++ )
		Model->Leaves(i).iZone = (Model->Leaves(i).iZone % 63) + 1;

	// Set official zone count.
	Model->NumZones = Clamp(NumZones+1,1,64);

	Mark.Pop();
}

/*-----------------------------------------------------------------------------
	Assigning zone numbers.
-----------------------------------------------------------------------------*/

//
// Go through the Bsp and assign zone numbers to all nodes.  Prior to this
// function call, only leaves have zone numbers.  The zone numbers for the entire
// Bsp can be determined from leaf zone numbers.
//
void FEditorVisibility::AssignAllZones( INT iNode, int Outside )
{
	INT iOriginalNode = iNode;

	// Recursively assign zone numbers to children.
	if( Model->Nodes(iOriginalNode).iFront != INDEX_NONE )
		AssignAllZones( Model->Nodes(iOriginalNode).iFront, Outside || Model->Nodes(iOriginalNode).IsCsg(NF_NotVisBlocking) );
	
	if( Model->Nodes(iOriginalNode).iBack != INDEX_NONE )
		AssignAllZones( Model->Nodes(iOriginalNode).iBack, Outside && !Model->Nodes(iOriginalNode).IsCsg(NF_NotVisBlocking) );

	// Make sure this node's polygon resides in a single zone.  In other words,
	// find all of the zones belonging to outside Bsp leaves and make sure their
	// zone number is the same, and assign that zone number to this node.
	while( iNode != INDEX_NONE )
	{
		FPoly Poly;
		if( !(Model->Nodes(iNode).NodeFlags & NF_IsNew) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
		{
			// Make sure this node is added to the BSP properly.
			int OriginalNumNodes = Model->Nodes.Num();
			FilterThroughSubtree
			(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes(iOriginalNode).iLeaf [0],
				Model->Nodes(iOriginalNode).iChild[0],
				Poly,
				&FEditorVisibility::TagZonePortalFragment,
				INDEX_NONE
			);

			// See if all of all non-interior added fragments are in the same zone.
			if( Model->Nodes.Num() > OriginalNumNodes )
			{
				int CanMerge=1, iZone[2]={0,0};
				for( int i=OriginalNumNodes; i<Model->Nodes.Num(); i++ )
					for( int j=0; j<2; j++ )
						if( Model->Nodes(i).iZone[j] != 0 )
							iZone[j] = Model->Nodes(i).iZone[j];
				for( int i=OriginalNumNodes; i<Model->Nodes.Num(); i++ )
					for( int j=0; j<2; j++ )
						if( Model->Nodes(i).iZone[j]!=0 && Model->Nodes(i).iZone[j]!=iZone[j] )
							CanMerge=0;
				if( CanMerge )
				{
					// All fragments were in the same zone, so keep the original and discard the new fragments.
					for( int i=OriginalNumNodes; i<Model->Nodes.Num(); i++ )
						Model->Nodes(i).NumVertices = 0;
					for( int i=0; i<2; i++ )
						Model->Nodes(iNode).iZone[i] = iZone[i];
				}
				else
				{
					// Keep the multi-zone fragments and remove the original plus any interior unnecessary polys.
					Model->Nodes(iNode).NumVertices = 0;
					for( int i=OriginalNumNodes; i<Model->Nodes.Num(); i++ )
						if( Model->Nodes(i).iZone[0]==0 && Model->Nodes(i).iZone[1]==0 )
							Model->Nodes(i).NumVertices = 0;
				}
			}
		}
		iNode = Model->Nodes(iNode).iPlane;
	}
}

/*-----------------------------------------------------------------------------
	Bsp zone structure building.
-----------------------------------------------------------------------------*/

//
// Build a 64-bit zone mask for each node, with a bit set for every
// zone that's referenced by the node and its children.  This is used
// during rendering to reject entire sections of the tree when it's known
// that none of the zones in that section are active.
//
FZoneSet BuildZoneMasks( UModel* Model, INT iNode )
{
	FBspNode& Node = Model->Nodes(iNode);
	FZoneSet ZoneMask = FZoneSet::NoZones();

	if( Node.iZone[0]!=0 ) ZoneMask.AddZone(Node.iZone[0]);
	if( Node.iZone[1]!=0 ) ZoneMask.AddZone(Node.iZone[1]);

	if( Node.iFront != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node.iFront );
	if( Node.iBack  != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node.iBack );
	if( Node.iPlane != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node.iPlane );

	Node.ZoneMask = ZoneMask;

	return ZoneMask;
}

//
// Build 64x64 zone connectivity matrix.  Entry(i,j) is set if node i is connected
// to node j.  Entry(i,i) is always set by definition.  This structure is built by
// analyzing all portals in the world and tagging the two zones they connect.
//
// Called by: TestVisibility.
//
void FEditorVisibility::BuildConnectivity()
{
	for( int i=0; i<64; i++ )
	{
		// Init to identity.
		Model->Zones[i].Connectivity = FZoneSet::IndividualZone(i);
	}
	for( int i=0; i<Model->Nodes.Num(); i++ )
	{
		// Process zones connected by portals.
		FBspNode &Node = Model->Nodes(i);
		FBspSurf &Surf = Model->Surfs(Node.iSurf);

		if( Surf.PolyFlags & PF_Portal )
		{
			Model->Zones[Node.iZone[1]].Connectivity |= FZoneSet::IndividualZone(Node.iZone[0]);
			Model->Zones[Node.iZone[0]].Connectivity |= FZoneSet::IndividualZone(Node.iZone[1]);
		}
	}
}

/*-----------------------------------------------------------------------------
	Volume visibility test.
-----------------------------------------------------------------------------*/

//
// Test visibility.
//
//< ava specific ; 2008. 2. 21 changmin
typedef struct {
	int			visDataSize;
	int			portalClusters;
	float		computePvsTime;
	int			averageClustersVisible;
} dpvsresult_t;
dpvsresult_t GPvsResult;
//>@ ava
void FEditorVisibility::TestVisibility()
{
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("Zoning")), TRUE );

	// Init Bsp info.
	for( int i=0; i<Model->Nodes.Num(); i++ )
	{
		for( int j=0; j<2; j++ )
		{
			Model->Nodes(i).iLeaf [j] = INDEX_NONE;
			Model->Nodes(i).iZone [j] = 0;
		}

		//<@ ava specific ; 2006. 11. 17 changmin
		Model->Nodes(i).iCluster = INDEX_NONE;
		//>@ ava
	}

	// Allocate objects.
	Model->Leaves.Empty();

	// Assign leaf numbers to convex outside volumes.
	AssignLeaves( 0, Model->RootOutside );

	// Allocate leaf info.
	LeafPortals  = new( GMem, MEM_Zeroed, Model->Leaves.Num()      )FPortal*;
	NodePortals  = new( GMem, MEM_Zeroed, Model->Nodes.Num()*2+256)FPortal*; // Allow for 2X expansion from zone portal fragments!!

	// Build all portals, with references to their front and back leaves.
	MakePortals( 0 );

	//<@ ava specific : 2006. 11 .6 changmin
	// Output Portals
	SavePortalsToFile();

	Model->NumClusters	= 0;
	Model->LeafBytes	= 0;
	
	const UBOOL bComputePvs = GEditor->bFullBuildGeometry && (GEditor->PvsComputeOptions != NULL);
	const UBOOL bSaveLeafInfo = GEditor->bFullBuildGeometry;	// bFullBuildGeometry : solid / solid visibility / full build
	UBOOL bLogPVSResult = FALSE;
	if( bComputePvs )
	{
		AvaPVSCalculator PvsCalculator;
		UBOOL Result = PvsCalculator.LoadLauncher();
		if( Result )
		{
			UBOOL Result = PvsCalculator.Compute( *GEditor->PvsComputeOptions );
			PvsCalculator.ShutDown();
			if( Result )
			{
				LoadVisFile();
				extern dpvsresult_t GPvsResult;
				debugf(NAME_Log, TEXT("computing pvs succeed!!!") );
				bLogPVSResult = TRUE;
			}
			else
			{
				debugf(NAME_Warning, TEXT("failed to compute pvs!!!") );
			}
		}
		else
		{
			debugf(NAME_Warning, TEXT("failed to load process launcher!!!") );
		}
	}
	//>@ ava

	// Form zones.
	FormZonesFromLeaves();
	AssignAllZones( 0, Model->RootOutside );
	GEditor->bspCleanup( Model );

	//<@ ava specific ; 2007. 1. 12
	AssignAllClusters( 0, Model->RootOutside );
	Model->LeafColors.Empty();
	for( INT ClusterIndex = 0; ClusterIndex < Model->Leaves.Num(); ++ClusterIndex )
	{
		Model->LeafColors.AddItem( FColor::MakeRandomColor() );
	}

	Model->StaticMeshLeaves.Empty();
	if( bComputePvs )
	{
		ComputeStaticMeshActorLeaves();
	}
	//else
	//{
	//	ClearStaticMeshLeafInformation();
	//}
	//>@ ava

	// Cleanup the bsp.
	//!!unsafe: screws up the node portals required for visibility checking.
	//!!but necessary for proper rendering.
	GEditor->bspCleanup( Model );
	GEditor->bspRefresh( Model, 1 );
	GEditor->bspBuildBounds( Model );

	// Build zone interconnectivity info.
	BuildZoneMasks( Model, 0 );
	BuildConnectivity();

	debugf( NAME_Log, TEXT("Portalized: %i portals, %i zone portals (%i fragments), %i leaves, %i nodes"), NumPortals, NumZonePortals, NumZoneFragments, Model->Leaves.Num(), Model->Nodes.Num() );

#if DEBUG_PORTALS || DEBUG_WRAPS || DEBUG_BADSHEETS || DEBUG_LVS
	GEditor->bspMergeCoplanars( GWorld->GetBrush()->Brush, 0, 1 );
#endif
	GWarn->EndSlowTask();

	//<@ ava specific ; 2008. 2. 21 changmin
	if( bLogPVSResult )
	{
		if( GWorld )
		{
			AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
			if( WorldInfo )
			{
				WorldInfo->VisdataSize  =GPvsResult.visDataSize;
				WorldInfo->PortalClusters = GPvsResult.portalClusters;
				WorldInfo->ComputePvsTime = GPvsResult.computePvsTime;
				WorldInfo->AverageClustersVisible = GPvsResult.averageClustersVisible;
				WorldInfo->UsePvs = TRUE;
			}
		}
		debugf(NAME_Log, TEXT("visdata size               : %d byte( %fk)"),GPvsResult.visDataSize, (float) GPvsResult.visDataSize/ 1000.0f );
		debugf(NAME_Log, TEXT("portal clusters            : %d"),GPvsResult.portalClusters );
		debugf(NAME_Log, TEXT("compute pvs time           : %.2f sec ( %d min %d sec)"),GPvsResult.computePvsTime, (int)GPvsResult.computePvsTime/60, (int)fmod(GPvsResult.computePvsTime, 60.0f));
		debugf(NAME_Log, TEXT("average clusters visible   : %d"),GPvsResult.averageClustersVisible );
	}
	else
	{
		if( GWorld )
		{
			AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
			if( WorldInfo )
			{
				WorldInfo->VisdataSize  = 0;
				WorldInfo->PortalClusters = 0;
				WorldInfo->ComputePvsTime = 0;
				WorldInfo->AverageClustersVisible = 0;
				WorldInfo->UsePvs = FALSE;
			}
		}
	}

	if( bSaveLeafInfo )
	{
		//<@ ava specific ; 2008. 2. 25 changmin
		// save leaf portals for visualize
		Model->Portals.Empty(NumPortals);
		Model->LeafInfos.Empty(Model->Leaves.Num());
		for( INT LeafIndex = 0; LeafIndex < Model->Leaves.Num(); ++LeafIndex )
		{
			AvaLeafInfo LeafInfo;
			LeafInfo.iFirstPortal = Model->Portals.Num();
			LeafInfo.NumPortals = 0;
			LeafInfo.PortalBounds.Init();

			FPortal* LeafPortal = LeafPortals[LeafIndex];
			while( LeafPortal )
			{
				FPoly PortalPoly;
				LeafPortal->GetPolyFacingOutOf(LeafIndex, PortalPoly );

				AvaPortal Portal;
				Portal.Vertices = PortalPoly.Vertices;
				Portal.Normal = PortalPoly.Normal;

				for( INT VertexIndex = 0; VertexIndex < Portal.Vertices.Num(); ++VertexIndex )
				{
					LeafInfo.PortalBounds += Portal.Vertices(VertexIndex);
				}
				Model->Portals.AddItem( Portal );
				LeafInfo.NumPortals++;
				LeafPortal = LeafPortal->Next( LeafIndex );
			}
			Model->LeafInfos.AddItem( LeafInfo );
		}
		//>@ ava
	}
	else
	{
		Model->Portals.Empty();
		Model->LeafInfos.Empty();
	}
	//>@ ava
}

/*-----------------------------------------------------------------------------
	Visibility constructor/destructor.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
FEditorVisibility::FEditorVisibility( UModel* InModel, INT InExtra )
:	Mark			(GMem),
	Model			(InModel),
	NumPortals		(0),
	NumClips		(0),
	NumClipTests	(0),
	NumPassedClips	(0),
	NumUnclipped	(0),
	NumBspPortals	(0),
	MaxFragments	(0),
	NumZonePortals	(0),
	NumZoneFragments(0),
	Extra			(InExtra),
	FirstPortal		(NULL),
	NodePortals		(NULL),
	LeafPortals		(NULL)
	//<@ ava specific ; 2006. 11. 7 changmin
	,SolidPortals	(NULL)
	//>@ ava
{
#if DEBUG_PORTALS || DEBUG_WRAPS || DEBUG_BADSHEETS || DEBUG_LVS
	// Init brush for debugging.
	DEBUG_Brush=GWorld->GetBrush();
	DEBUG_Brush->Brush->Polys->Element.Empty();
	DEBUG_Brush->Location=DEBUG_Brush->PrePivot=FVector(0,0,0);
	DEBUG_Brush->Rotation = FRotator(0,0,0);
#endif

}

//
// Destructor.
//
FEditorVisibility::~FEditorVisibility()
{
	Mark.Pop();
	//!!Visibility->~FSymmetricBitArray;
}

/*-----------------------------------------------------------------------------
	Main function.
-----------------------------------------------------------------------------*/

//
// Perform visibility testing within the level.
//
void UEditorEngine::TestVisibility( UModel* Model, int A, int B )
{
	if( Model->Nodes.Num() )
	{
		// Test visibility.
		FEditorVisibility Visi( Model, A );
		Visi.TestVisibility();
	}
}

//<@ ava specific ; 2007. 1. 5 changmin
void UEditorEngine::AssignDetailClusters( UModel *Bsp )
{
	if( Bsp->Nodes.Num() )
	{
		FEditorVisibility Visi( Bsp, 0 );
		Visi.AssignAllClusters( 0, Bsp->RootOutside );
		GEditor->bspCleanup( Bsp );
		//for( INT NodeIndex = 0; NodeIndex < Bsp->Nodes.Num(); ++NodeIndex )
		//{
		//	FBspNode &Node = Bsp->Nodes(NodeIndex);
		//	FBspSurf &Surf = Bsp->Surfs(Node.iSurf);
		//	if( Surf.PolyFlags & PF_Semisolid )
		//	{
		//		Visi.AssignDetailCluster( NodeIndex );
		//	}
		//}
	}
}
//>@ ava


/*-----------------------------------------------------------------------------
	Bsp node bounding volumes.
-----------------------------------------------------------------------------*/

#if DEBUG_HULLS
	UModel *DEBUG_Brush;
#endif

//
// Update a bounding volume by expanding it to enclose a list of polys.
//
void UpdateBoundWithPolys( FBox& Bound, FPoly** PolyList, INT nPolys )
{
	for( INT i=0; i<nPolys; i++ )
		for( INT j=0; j<PolyList[i]->Vertices.Num(); j++ )
			Bound += PolyList[i]->Vertices(j);
}

//
// Update a convolution hull with a list of polys.
//
void UpdateConvolutionWithPolys( UModel *Model, INT iNode, FPoly **PolyList, int nPolys )
{
	//<@ ava specific ; 2007. 1. 30 changmin
	// debugging invalid bsp collision
#if AVA_DEBUG_COLLISION
	if( Model == GWorld->GetModel() )
	{
		ABrush *Brush = GWorld->GetBrush();
		for( int i = 0; i < nPolys; ++i )
		{
			if( PolyList[i]->iBrushPoly != INDEX_NONE )
			{
				UBOOL bFindSamePoly = FALSE;
				for( int j = 0 ; j < i; ++j )
				{
					if( PolyList[j]->iBrushPoly == PolyList[i]->iBrushPoly )
					{
						bFindSamePoly = TRUE;
						break;
					}
				}
				if( bFindSamePoly == FALSE )
				{
					new(Brush->Brush->Polys->Element)FPoly(*PolyList[i]);
				}
			}
		}

	}
#endif
	//>@ ava

	FBox Box(0);

	FBspNode &Node = Model->Nodes(iNode);
	Node.iCollisionBound = Model->LeafHulls.Num();
	for( int i=0; i<nPolys; i++ )
	{
		if( PolyList[i]->iBrushPoly != INDEX_NONE )
		{
			int j;
			for( j=0; j<i; j++ )
				if( PolyList[j]->iBrushPoly == PolyList[i]->iBrushPoly )
					break;
			if( j >= i )
				Model->LeafHulls.AddItem(PolyList[i]->iBrushPoly);
		}
		for( int j=0; j<PolyList[i]->Vertices.Num(); j++ )
			Box += PolyList[i]->Vertices(j);
	}
	Model->LeafHulls.AddItem(INDEX_NONE);

	// Add bounds.
	Model->LeafHulls.AddItem( *(INT*)&Box.Min.X );
	Model->LeafHulls.AddItem( *(INT*)&Box.Min.Y );
	Model->LeafHulls.AddItem( *(INT*)&Box.Min.Z );
	Model->LeafHulls.AddItem( *(INT*)&Box.Max.X );
	Model->LeafHulls.AddItem( *(INT*)&Box.Max.Y );
	Model->LeafHulls.AddItem( *(INT*)&Box.Max.Z );

}

//
// Cut a partitioning poly by a list of polys, and add the resulting inside pieces to the
// front list and back list.
//
void SplitPartitioner
(
	UModel*	Model,
	FPoly**	PolyList,
	FPoly**	FrontList,
	FPoly**	BackList,
	INT		n,
	INT		nPolys,
	INT&	nFront, 
	INT&	nBack, 
	FPoly	InfiniteEdPoly,
	TArray<FPoly*>& AllocatedFPolys
)
{
	FPoly FrontPoly,BackPoly;
	while( n < nPolys )
	{
		FPoly* Poly = PolyList[n];
		switch( InfiniteEdPoly.SplitWithPlane(Poly->Vertices(0),Poly->Normal,&FrontPoly,&BackPoly,0) )
		{
			case SP_Coplanar:
				// May occasionally happen.
				debugf( NAME_Log, TEXT("FilterBound: Got inficoplanar") );
				break;

			case SP_Front:
				// Shouldn't happen if hull is correct.
				debugf( NAME_Log, TEXT("FilterBound: Got infifront") );
				return;

			case SP_Split:
				InfiniteEdPoly = BackPoly;
				break;

			case SP_Back:
				break;
		}
		n++;
	}

	FPoly* New = new FPoly;
	*New = InfiniteEdPoly;
	New->Reverse();
	New->iBrushPoly |= 0x40000000;
	FrontList[nFront++] = New;
	AllocatedFPolys.AddItem( New );
	
	New = new FPoly;
	*New = InfiniteEdPoly;
	BackList[nBack++] = New;
	AllocatedFPolys.AddItem( New );
}

//
// Recursively filter a set of polys defining a convex hull down the Bsp,
// splitting it into two halves at each node and adding in the appropriate
// face polys at splits.
//
void FilterBound
(
	UModel*			Model,
	FBox*			ParentBound,
	INT				iNode,
	FPoly**			PolyList,
	INT				nPolys,
	INT				Outside
)
{
	FMemMark Mark(GMem);
	FBspNode&	Node	= Model->Nodes  (iNode);
	FBspSurf&	Surf	= Model->Surfs  (Node.iSurf);
	FVector		Base = Surf.Plane * Surf.Plane.W;
	FVector&	Normal	= Model->Vectors(Surf.vNormal);
	FBox		Bound(0);

	Bound.Min.X = Bound.Min.Y = Bound.Min.Z = +WORLD_MAX;
	Bound.Max.X = Bound.Max.Y = Bound.Max.Z = -WORLD_MAX;

	// Split bound into front half and back half.
	FPoly** FrontList = new(GMem,nPolys*2+16)FPoly*; int nFront=0;
	FPoly** BackList  = new(GMem,nPolys*2+16)FPoly*; int nBack=0;

	// Keeping track of allocated FPoly structures to delete later on.
	TArray<FPoly*> AllocatedFPolys;

	FPoly* FrontPoly  = new FPoly;
	FPoly* BackPoly   = new FPoly;

	// Keep track of allocations.
	AllocatedFPolys.AddItem( FrontPoly );
	AllocatedFPolys.AddItem( BackPoly );

	for( INT i=0; i<nPolys; i++ )
	{
		FPoly *Poly = PolyList[i];
		//switch( Poly->SplitWithPlane( Base, Normal, FrontPoly, BackPoly, 0 ) )
		switch( Poly->SplitWithPlane( Base, Normal, FrontPoly, BackPoly, 1 ) )
		{
			case SP_Coplanar:
				debugf( NAME_Log, TEXT("FilterBound: Got coplanar") );
				FrontList[nFront++] = Poly;
				BackList[nBack++] = Poly;
				break;
			
			case SP_Front:
				FrontList[nFront++] = Poly;
				break;
			
			case SP_Back:
				BackList[nBack++] = Poly;
				break;
			
			case SP_Split:
				FrontList[nFront++] = FrontPoly;
				BackList [nBack++] = BackPoly;
				FrontPoly = new FPoly;
				BackPoly  = new FPoly;
				// Keep track of allocations.
				AllocatedFPolys.AddItem( FrontPoly );
				AllocatedFPolys.AddItem( BackPoly );
				break;

			default:
				appErrorf( TEXT("FZoneFilter::FilterToLeaf: Unknown split code") );
		}
	}
	if( nFront && nBack )
	{
		// Add partitioner plane to front and back.
		FPoly InfiniteEdPoly = BuildInfiniteFPoly( Model, iNode );
		InfiniteEdPoly.iBrushPoly = iNode;
		SplitPartitioner(Model,PolyList,FrontList,BackList,0,nPolys,nFront,nBack,InfiniteEdPoly,AllocatedFPolys);
	}
	else
	{
		if( !nFront ) debugf( NAME_Log, TEXT("FilterBound: Empty fronthull") );
		if( !nBack  ) debugf( NAME_Log, TEXT("FilterBound: Empty backhull") );
	}

	// Recursively update all our childrens' bounding volumes.
	if( nFront > 0 )
	{
		if( Node.iFront != INDEX_NONE )
		{
			FilterBound( Model, &Bound, Node.iFront, FrontList, nFront, Outside || Node.IsCsg() );
		}
		else if( Outside || Node.IsCsg() )
		{
			UpdateBoundWithPolys( Bound, FrontList, nFront );
		}
		else
		{
			UpdateConvolutionWithPolys( Model, iNode, FrontList, nFront );
		}
	}
	if( nBack > 0 )
	{
		if( Node.iBack != INDEX_NONE)
		{
			FilterBound( Model, &Bound,Node.iBack, BackList, nBack, Outside && !Node.IsCsg() );
		}
		else if( Outside && !Node.IsCsg() )
		{
			UpdateBoundWithPolys( Bound, BackList, nBack );
		}
		else
		{
			UpdateConvolutionWithPolys( Model, iNode, BackList, nBack );
		}
	}

	// Update parent bound to enclose this bound.
	if( ParentBound )
	{
		*ParentBound += Bound;
	}

	// Delete FPolys allocated above. We cannot use GMem for FPoly as the array data FPoly contains will be allocated in regular memory.
	for( INT i=0; i<AllocatedFPolys.Num(); i++ )
	{
		FPoly* AllocatedFPoly = AllocatedFPolys(i);
		delete AllocatedFPoly;
	}

	Mark.Pop();
}

//
// Build bounding volumes for all Bsp nodes.  The bounding volume of the node
// completely encloses the "outside" space occupied by the nodes.  Note that 
// this is not the same as representing the bounding volume of all of the 
// polygons within the node.
//
// We start with a practically-infinite cube and filter it down the Bsp,
// whittling it away until all of its convex volume fragments land in leaves.
//
void UEditorEngine::bspBuildBounds( UModel* Model )
{
	if( Model->Nodes.Num()==0 )
		return;

	BuildZoneMasks( Model, 0 );

	FPoly Polys[6], *PolyList[6];
	for( int i=0; i<6; i++ )
	{
		PolyList[i] = &Polys[i];
		PolyList[i]->Init();
		PolyList[i]->iBrushPoly = INDEX_NONE;
	}

	new(Polys[0].Vertices)FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX,HALF_WORLD_MAX);
	new(Polys[0].Vertices)FVector( HALF_WORLD_MAX,-HALF_WORLD_MAX,HALF_WORLD_MAX);
	new(Polys[0].Vertices)FVector( HALF_WORLD_MAX, HALF_WORLD_MAX,HALF_WORLD_MAX);
	new(Polys[0].Vertices)FVector(-HALF_WORLD_MAX, HALF_WORLD_MAX,HALF_WORLD_MAX);
	Polys[0].Normal   =FVector( 0.000000,  0.000000,  1.000000 );
	Polys[0].Base     =Polys[0].Vertices(0);

	new(Polys[1].Vertices)FVector(-HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[1].Vertices)FVector( HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[1].Vertices)FVector( HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[1].Vertices)FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[1].Normal   =FVector( 0.000000,  0.000000, -1.000000 );
	Polys[1].Base     =Polys[1].Vertices(0);

	new(Polys[2].Vertices)FVector(-HALF_WORLD_MAX,HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[2].Vertices)FVector(-HALF_WORLD_MAX,HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[2].Vertices)FVector( HALF_WORLD_MAX,HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[2].Vertices)FVector( HALF_WORLD_MAX,HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[2].Normal   =FVector( 0.000000,  1.000000,  0.000000 );
	Polys[2].Base     =Polys[2].Vertices(0);

	new(Polys[3].Vertices)FVector( HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[3].Vertices)FVector( HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[3].Vertices)FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[3].Vertices)FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[3].Normal   =FVector( 0.000000, -1.000000,  0.000000 );
	Polys[3].Base     =Polys[3].Vertices(0);

	new(Polys[4].Vertices)FVector(HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[4].Vertices)FVector(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[4].Vertices)FVector(HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[4].Vertices)FVector(HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[4].Normal   =FVector( 1.000000,  0.000000,  0.000000 );
	Polys[4].Base     =Polys[4].Vertices(0);

	new(Polys[5].Vertices)FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	new(Polys[5].Vertices)FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[5].Vertices)FVector(-HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	new(Polys[5].Vertices)FVector(-HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[5].Normal   =FVector(-1.000000,  0.000000,  0.000000 );
	Polys[5].Base     =Polys[5].Vertices(0);
	// Empty hulls.
	Model->LeafHulls.Empty();
	for( int i=0; i<Model->Nodes.Num(); i++ )
		Model->Nodes(i).iCollisionBound  = INDEX_NONE;

	//<@ ava specific ; 2007. 1. 30 changmin
	// debugging invalid bsp collision
#if AVA_DEBUG_COLLISION
	if( Model == GWorld->GetModel() )
	{
		ABrush *CollisionBrush = GWorld->GetBrush();
		CollisionBrush->Brush->Polys->Element.Empty();
		CollisionBrush->Location = CollisionBrush->PrePivot = FVector(0,0,0);
		CollisionBrush->Rotation = FRotator(0,0,0);
	}
#endif
	//>@ ava

	FilterBound( Model, NULL, 0, PolyList, 6, Model->RootOutside );
	debugf( NAME_Log, TEXT("bspBuildBounds: Generated %i hulls"), Model->LeafHulls.Num() );
}

/*-----------------------------------------------------------------------------
	Non-class functions.
-----------------------------------------------------------------------------*/

//
// Build an FPoly representing an "infinite" plane (which exceeds the maximum
// dimensions of the world in all directions) for a particular Bsp node.
//
FPoly BuildInfiniteFPoly( UModel* Model, INT iNode )
{
	FBspNode &Node   = Model->Nodes  (iNode       );
	FBspSurf &Poly   = Model->Surfs  (Node.iSurf  );
	FVector  Base    = Poly.Plane * Poly.Plane.W;
	FVector  Normal  = Poly.Plane;
	FVector	 Axis1,Axis2;

	// Find two non-problematic axis vectors.
	Normal.FindBestAxisVectors( Axis1, Axis2 );

	// Set up the FPoly.
	FPoly EdPoly;
	EdPoly.Init();
	EdPoly.Normal      = Normal;
	EdPoly.Base        = Base;
	new(EdPoly.Vertices) FVector(Base + Axis1*WORLD_MAX + Axis2*WORLD_MAX);
	new(EdPoly.Vertices) FVector(Base - Axis1*WORLD_MAX + Axis2*WORLD_MAX);
	new(EdPoly.Vertices) FVector(Base - Axis1*WORLD_MAX - Axis2*WORLD_MAX);
	new(EdPoly.Vertices) FVector(Base + Axis1*WORLD_MAX - Axis2*WORLD_MAX);

	return EdPoly;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/


//<@ ava specific ; 2006. 11. 7 changmin
// Vis program에서 poly(winding)에 대한 plane을 계산하는 방법을 따른다.
void PolyPlaneForVis( FPoly& Poly, FVector* Normal, FLOAT* Dist )
{
	FVector V1, V2;

	V1 = Poly.Vertices(1) - Poly.Vertices(0);
	V2 = Poly.Vertices(2) - Poly.Vertices(0);

	*Normal = V2 ^ V1;
	Normal->Normalize();

	*Dist = V1 | (*Normal);
}

FLOAT Q_rint (FLOAT In)
{
	return floor (In + 0.5);
}

void WriteFloat( FILE* F, FLOAT V )
{
	if( fabs( V - Q_rint(V) ) < 0.001 )
	{
		fprintf( F, "%i ", (INT) Q_rint(V));
	}
	else
	{
		fprintf( F, "%f ", V);
	}
}


#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
// little-endian "IBSP"

#define BSP_VERSION			46

#define	LUMP_ENTITIES		0
#define	LUMP_SHADERS		1
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
#define	LUMP_LEAFSURFACES	5
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_FOGS			12
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17

typedef struct {
	int		fileofs, filelen;
} lump_t;

typedef struct {
	int			ident;
	int			version;

	lump_t		lumps[HEADER_LUMPS];
} dheader_t;


int    LittleLong (int l)
{
	return l;
}
float	LittleFloat (float l)
{
	return l;
}

/*
=============
SwapBlock

If all values are 32 bits, this can be used to swap everything
=============
*/
void SwapBlock( int *block, int sizeOfBlock ) {
	int		i;

	sizeOfBlock >>= 2;
	for ( i = 0 ; i < sizeOfBlock ; i++ ) {
		block[i] = LittleLong( block[i] );
	}
}

FILE *SafeOpenWrite (const char *filename)
{
	FILE	*f;

	f = fopen(filename, "wb");

	if (!f)
	{
		//GError->Log( TEXT ("Error opening %s: %s"),filename,strerror(errno) );
	}

	return f;
}

void SafeWrite (FILE *f, const void *buffer, int count)
{
	if (fwrite (buffer, 1, count, f) != (size_t)count)
	{
		GError->Log(TEXT("File write failure"));
	}

}

/*
=============
AddLump
=============
*/
void AddLump( FILE *bspfile, dheader_t *header, int lumpnum, const void *data, int len )
{
	lump_t *lump;

	lump = &header->lumps[lumpnum];

	lump->fileofs = LittleLong( ftell(bspfile) );
	lump->filelen = LittleLong( len );
	SafeWrite( bspfile, data, (len+3)&~3 );
}

static const FString VisFilePath(TEXT("c:\\"));
static const FString VisFileName(TEXT("avavis"));
/*
=============
WriteBSPFile

Swaps the bsp file in place, so it should not be referenced again
=============
*/
void	WriteBSPFile( const char *filename )
{		
	dheader_t	outheader, *header;
	FILE		*bspfile;

	header = &outheader;
	memset( header, 0, sizeof(dheader_t) );

	//SwapBSPFile();

	header->ident = LittleLong( BSP_IDENT );
	header->version = LittleLong( BSP_VERSION );

	bspfile = SafeOpenWrite( filename );
	SafeWrite( bspfile, header, sizeof(dheader_t) );	// overwritten later

	AddLump( bspfile, header, LUMP_SHADERS, NULL, 0 );
	AddLump( bspfile, header, LUMP_PLANES, NULL, 0 );
	AddLump( bspfile, header, LUMP_LEAFS, NULL, 0 );
	AddLump( bspfile, header, LUMP_NODES, NULL, 0 );
	AddLump( bspfile, header, LUMP_BRUSHES, NULL, 0 );
	AddLump( bspfile, header, LUMP_BRUSHSIDES, NULL, 0 );
	AddLump( bspfile, header, LUMP_LEAFSURFACES, NULL, 0 );
	AddLump( bspfile, header, LUMP_LEAFBRUSHES, NULL, 0 );
	AddLump( bspfile, header, LUMP_MODELS, NULL, 0 );
	AddLump( bspfile, header, LUMP_DRAWVERTS, NULL, 0 );
	AddLump( bspfile, header, LUMP_SURFACES, NULL, 0 );
	AddLump( bspfile, header, LUMP_VISIBILITY, NULL, 0 );
	AddLump( bspfile, header, LUMP_LIGHTMAPS, NULL, 0 );
	AddLump( bspfile, header, LUMP_LIGHTGRID, NULL, 0 );
	AddLump( bspfile, header, LUMP_ENTITIES, NULL, 0 );
	AddLump( bspfile, header, LUMP_FOGS, NULL, 0 );
	AddLump( bspfile, header, LUMP_DRAWINDEXES, NULL, 0 );

	fseek (bspfile, 0, SEEK_SET);
	SafeWrite (bspfile, header, sizeof(dheader_t));
	fclose (bspfile);	
}


UBOOL FEditorVisibility::SavePortalsToFile()
{
	const INT NumVisClusters	= Model->Leaves.Num();

	// compute vis portal count
	INT NumVisPortals = 0;
	for( INT iLeaf = 0; iLeaf < NumVisClusters; ++iLeaf)
	{
		FPortal* LeafPortal = LeafPortals[iLeaf];

		for( ; LeafPortal; LeafPortal = LeafPortal->Next( iLeaf ) )
		{
			// front portal
			if( LeafPortal->iFrontLeaf == iLeaf )
			{
				++NumVisPortals;
			}
		}
	}

	// count solid face
	INT NumSolidFaces = 0;
	FPortal* SolidPortal = SolidPortals;
	for( ; SolidPortal; SolidPortal = SolidPortal->GlobalNext )
	{
		if( SolidPortal->iFrontLeaf != INDEX_NONE || SolidPortal->iBackLeaf != INDEX_NONE )
		{
			++NumSolidFaces;
		}
	}

	GWarn->Log( TEXT("================= Compute Portals ====================") );

	GWarn->Logf( TEXT("%5i visclusters"),	NumVisClusters );
	GWarn->Logf( TEXT("%5i visportals"),	NumVisPortals );
	GWarn->Logf( TEXT("%5i solidfaces"),	NumSolidFaces );


	GWarn->Log( TEXT("================= Save Portals ====================") );

	// Create File

	// save bsp file
	FString BspFileName = VisFilePath + VisFileName + TEXT(".bsp");
	WriteBSPFile( TCHAR_TO_ANSI(*BspFileName) );

	// save portal file
	const FString PORTALFILE(TEXT("PRT1"));
	FString FileName = VisFilePath + VisFileName + TEXT(".prt");
	FILE* Stream = fopen( TCHAR_TO_ANSI(*FileName), "w" );

	if( Stream )
	{
		// output header
		fprintf( Stream, "%s\n", TCHAR_TO_ANSI(*PORTALFILE) );
		fprintf( Stream, "%i\n", NumVisClusters );
		fprintf( Stream, "%i\n", NumVisPortals );
		fprintf( Stream, "%i\n", NumSolidFaces );

		// output portals
		FVector	Normal;
		FLOAT	Dist;

		for( INT iLeaf = 0; iLeaf < NumVisClusters; ++iLeaf)
		{
			FPortal* Portal = LeafPortals[iLeaf];

			for( ; Portal; Portal = Portal->Next( iLeaf ) )
			{
				if( Portal->iFrontLeaf != iLeaf )
					continue;

				// q3 winding은 normal이 winding방향의 반대로 형성됩니다.
				// ue3 poly의 normal은 winding 방향으로 형성됩니다.
				// ue3의 front	= q3 back
				// ue3의 back	= q3 front
				PolyPlaneForVis( *Portal, &Normal, &Dist);

				// quake 3 vis

				// ue3 portal
				//			|
				//	front <-|	back
				//			|

				// the leaf[0] has portal like this
				//			|
				//			|->(-normal) neighbor leaf
				//			|

				// the leaf[1] has portal like this
				//							|
				//	neighbor leaf(normal) <-|
				//							|

				if( ( Portal->Normal | Normal ) < 0.99 )
				{
					// leaf[0]	= normal direction		== -portal->normal = back leaf
					// leaf[1]	= -normal direction		== portal->normal = front leaf
					fprintf( Stream, "%i %i %i ", Portal->Vertices.Num(), Portal->iBackLeaf, Portal->iFrontLeaf );
				}
				else
				{
					// 두 개의 방향이 같으므로,
					// leaf[0] = normal direction == portal->normal = front leaf
					// leaf[1] = -normal direction == -portal->normal =  back leaf
					fprintf( Stream, "%i %i %i ", Portal->Vertices.Num(), Portal->iFrontLeaf, Portal->iBackLeaf );
				}

				// hint
				if( Portal->IsHint )
					fprintf( Stream, "1 " );
				else
					fprintf( Stream, "0 " );

				for( INT VertexIndex = 0; VertexIndex < Portal->Vertices.Num(); ++VertexIndex )
				{
					fprintf( Stream, "(");
					WriteFloat( Stream, Portal->Vertices(VertexIndex).X );
					WriteFloat( Stream, Portal->Vertices(VertexIndex).Y );
					WriteFloat( Stream, Portal->Vertices(VertexIndex).Z );
					fprintf( Stream, ") ");
				}

				fprintf( Stream, "\n");
			}
		}

		
		// output faces
		FPortal* Face = SolidPortals;
		for( ; Face; Face = Face->GlobalNext )
		{
			// q3는 반대로 winding을 형성합니다.
			if( Face->iFrontLeaf != INDEX_NONE )
			{
				fprintf( Stream, "%i %i ", Face->Vertices.Num(), Face->iFrontLeaf );

				for( INT VertexIndex = Face->Vertices.Num() -1 ; VertexIndex >= 0; --VertexIndex )
				{
					fprintf( Stream, "(");
					WriteFloat( Stream, Face->Vertices(VertexIndex).X );
					WriteFloat( Stream, Face->Vertices(VertexIndex).Y );
					WriteFloat( Stream, Face->Vertices(VertexIndex).Z );
					fprintf( Stream, ") ");
				}
				fprintf( Stream, "\n" );
			}
			else if( Face->iBackLeaf != INDEX_NONE )
			{
				fprintf( Stream, "%i %i ", Face->Vertices.Num(), Face->iBackLeaf );

				for( INT VertexIndex = 0; VertexIndex < Face->Vertices.Num(); ++VertexIndex )
				{
					fprintf( Stream, "(");
					WriteFloat( Stream, Face->Vertices(VertexIndex).X );
					WriteFloat( Stream, Face->Vertices(VertexIndex).Y );
					WriteFloat( Stream, Face->Vertices(VertexIndex).Z );
					fprintf( Stream, ") ");
				}
				fprintf( Stream, "\n" );
			}
		}

		// end...
		fclose(Stream);
	}

	GWarn->Log( TEXT("----------------- End of Saving Portals -------------") );
	
	return TRUE;
}

/*
=============
CopyLump
=============
*/
int CopyLump( dheader_t	*header, int lump, void *dest, int size ) {
	int		length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;

	if ( length % size ) 
	{
		GError->Log( TEXT("LoadBSPFile: odd lump size") );
	}

	memcpy( dest, (byte *)header + ofs, length );

	return length / size;
}

FILE *SafeOpenRead (const char *filename)
{
	FILE	*f;

	f = fopen(filename, "rb");

	if (!f)
	{
		//GWarn->Log( TEXT("Error opening %s: %s"),filename,strerror(errno) );
	}

	return f;
}

void SafeRead (FILE *f, void *buffer, int count)
{
	if ( fread (buffer, 1, count, f) != (size_t)count)
	{
		GError->Log( TEXT("File read failure") );
	}
}

/*
================
Q_filelength
================
*/
int Q_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}



/*
==============
LoadFile
==============
*/
int    LoadFile( const char *filename, void **bufferptr )
{
	FILE	*f;
	int    length;
	void    *buffer;

	f = SafeOpenRead (filename);
	if( f )
	{
		length = Q_filelength (f);
		buffer = malloc (length+1);
		((char *)buffer)[length] = 0;
		SafeRead (f, buffer, length);
		fclose (f);
		*bufferptr = buffer;
	}
	else
	{
		length = 0;
	}
	
	return length;
}

#define	MAX_MAP_VISIBILITY	0x200000
int			numVisBytes;
byte		visBytes[MAX_MAP_VISIBILITY];
/*
=============
LoadBSPFile
=============
*/
void	LoadBSPFile( const char *filename )
{
	dheader_t	*header;
	dpvsresult_t *pvsResult;

	numVisBytes = 0;
	memset(&GPvsResult, 0, sizeof(dpvsresult_t));

	// load the file header
	if( LoadFile (filename, (void **)&header) == 0 )
		return;

	// swap the header
	SwapBlock( (int *)header, sizeof(*header) );

	if ( header->ident != BSP_IDENT )
	{
		//GError->Logf(TEXT( "%s is not a IBSP file", filename ) );
	}
	if ( header->version != BSP_VERSION )
	{
		//GError->Logf(TEXT( "%s is version %i, not %i", filename, header->version, BSP_VERSION ) );
	}

	pvsResult = (dpvsresult_t*)(header+1);
	GPvsResult.visDataSize = LittleLong(pvsResult->visDataSize);
	GPvsResult.portalClusters = LittleLong(pvsResult->portalClusters);
	GPvsResult.computePvsTime = LittleFloat(pvsResult->computePvsTime);
	GPvsResult.averageClustersVisible = LittleLong(pvsResult->averageClustersVisible);

	numVisBytes = CopyLump( header, LUMP_VISIBILITY, visBytes, 1 );
	free( header );		// everything has been copied out
	// swap everything
	//SwapBSPFile();
	{
		((int *)&visBytes)[0] = LittleLong( ((int *)&visBytes)[0] );
		((int *)&visBytes)[1] = LittleLong( ((int *)&visBytes)[1] );
	}
}



UBOOL FEditorVisibility::LoadVisFile()
{
	const INT VIS_HEADER_SIZE = 8;
	FString BspFileName = VisFilePath + VisFileName + TEXT("_out.bsp");
	LoadBSPFile( TCHAR_TO_ANSI( *BspFileName ) );
	if( numVisBytes )
	{
		Model->NumClusters	= ((int*)&visBytes)[0];
		Model->LeafBytes	= ((int*)&visBytes)[1];
		Model->VisBytes.Empty( numVisBytes - VIS_HEADER_SIZE );
		Model->VisBytes.AddZeroed( numVisBytes - VIS_HEADER_SIZE );
		memcpy( &(Model->VisBytes(0)), &visBytes[VIS_HEADER_SIZE], numVisBytes - VIS_HEADER_SIZE );
	}
	return TRUE;
}
//>@ ava


//<@ ava specific ; 2006. 11. 16 changmin

//
// Tag a node fragment.
//
void FEditorVisibility::TagNodeFragment
(
 FPoly&	Poly,
 INT	    iFrontLeaf,
 INT		iBackLeaf,
 INT		iGeneratingNode,
 INT		iGeneratingBase
 )
{
	// Add this node to the bsp as a coplanar to its generator.
	INT iNewNode = GEditor->bspAddNode( Model, iGeneratingNode, NODE_Plane, Model->Nodes(iGeneratingNode).NodeFlags | NF_IsNew, &Poly );
	// set cluster number
	UBOOL Backward = (Poly.Normal | Model->Nodes(iGeneratingBase).Plane) < 0.0;
	INT ClusterNumber = (Backward) ? iBackLeaf : iFrontLeaf;
	Model->Nodes(iNewNode).iCluster = ClusterNumber;
}




void FEditorVisibility::AssignDetailCluster( INT iNode )
{
	INT iOriginalNode = iNode;
	FPoly Poly;
	if( !(Model->Nodes(iNode).NodeFlags & NF_IsNew) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
	{
		FBspNode &Node = Model->Nodes(iNode);
		if( Node.iLeaf[0] == INDEX_NONE && Node.iLeaf[1] == INDEX_NONE )
		{
			for( INT iParentNode = 0; iParentNode < Model->Nodes.Num(); ++iParentNode )
			{
				if( iParentNode == iNode )
					continue;

				FBspNode *ParentNode = &(Model->Nodes(iParentNode));
				while ( ParentNode->iPlane != INDEX_NONE )
				{
					if( ParentNode->iPlane == iNode )
						break;

					ParentNode = &Model->Nodes( ParentNode->iPlane );
				}
				if( ParentNode->iPlane != INDEX_NONE )
				{
					// find
					iOriginalNode = iParentNode;
					break;
				}
			}
		}
		// Make sure this node is added to the BSP properly.
		int OriginalNumNodes = Model->Nodes.Num();
		FilterThroughSubtree
			(
			0,
			iNode,
			iOriginalNode,
			Model->Nodes(iOriginalNode).iLeaf[0],
			Model->Nodes(iOriginalNode).iChild[0],
			Poly,
			&FEditorVisibility::TagNodeFragment,
			INDEX_NONE
			);
		// See if all of all non-interior added fragments are in the same zone.
		if( Model->Nodes.Num() > OriginalNumNodes )
		{
			int CanMerge = 1, iCluster = INDEX_NONE;
			for( int i = OriginalNumNodes; i < Model->Nodes.Num(); i++ )
			{
				if( Model->Nodes(i).iCluster != INDEX_NONE )
				{
					iCluster = Model->Nodes(i).iCluster;
				}
			}
			for( int i = OriginalNumNodes; i < Model->Nodes.Num(); i++ )
			{
				if( Model->Nodes(i).iCluster != INDEX_NONE && Model->Nodes(i).iCluster != iCluster )
				{
					CanMerge = 0;
				}
			}
			if( CanMerge )
			{
				// All fragments were in the same zone, so keep the original and discard the new fragments.
				for( int i = OriginalNumNodes; i < Model->Nodes.Num(); i++ )
				{
					Model->Nodes(i).NumVertices = 0;
				}
				// 이런 경우도 있네.. ^^;
				//check( iCluster != INDEX_NONE );
				if( iCluster == INDEX_NONE )
					Model->Nodes(iNode).NumVertices = 0;

				Model->Nodes(iNode).iCluster = iCluster;
			}
			else
			{
				// Keep the multi-zone fragments and remove the original plus any interior unnecessary polys.
				Model->Nodes(iNode).NumVertices = 0;
				for( int i=OriginalNumNodes; i<Model->Nodes.Num(); i++ )
				{
					if( Model->Nodes(i).iCluster == INDEX_NONE )
					{
						Model->Nodes(i).NumVertices = 0;
					}
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
Assigning cluster number(leaf number) to nodes.
-----------------------------------------------------------------------------*/

//
// Go through the Bsp and assign cluster numbers to all nodes.  Prior to this
// function call, leaves have cluster numbers.
//
void FEditorVisibility::AssignAllClusters( INT iNode, INT Outside )
{
	INT iOriginalNode = iNode;
	// Recursively assign cluster numbers to children.
	if( Model->Nodes(iOriginalNode).iFront != INDEX_NONE )
	{
		AssignAllClusters( Model->Nodes(iOriginalNode).iFront, Outside || Model->Nodes(iOriginalNode).IsCsg(NF_NotVisBlocking) );
	}
	if( Model->Nodes(iOriginalNode).iBack != INDEX_NONE )
	{
		AssignAllClusters( Model->Nodes(iOriginalNode).iBack, Outside && !Model->Nodes(iOriginalNode).IsCsg(NF_NotVisBlocking) );
	}
	// Make sure this node's polygon resides in a single clusters.
	// In other words, find all of the clusters belonging to outside Bsp leaves and make sure their
	// cluster number is the same, and assign that cluster number to this node.
	while( iNode != INDEX_NONE )
	{
		FPoly Poly;
		if( !(Model->Nodes(iNode).NodeFlags & NF_IsNew) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
		{
			// Make sure this node is added to the BSP properly.
			int OriginalNumNodes = Model->Nodes.Num();
			FilterThroughSubtree
				(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes(iOriginalNode).iLeaf [0],
				Model->Nodes(iOriginalNode).iChild[0],
				Poly,
				&FEditorVisibility::TagNodeFragment,
				INDEX_NONE
				);
			// See if all of all non-interior added fragments are in the same zone.
			if( Model->Nodes.Num() > OriginalNumNodes )
			{
				int CanMerge = 1, iCluster = INDEX_NONE;
				for( int i = OriginalNumNodes; i < Model->Nodes.Num(); i++ )
				{
					if( Model->Nodes(i).iCluster != INDEX_NONE )
					{
						iCluster = Model->Nodes(i).iCluster;
					}
				}
				for( int i = OriginalNumNodes; i < Model->Nodes.Num(); i++ )
				{
					if( Model->Nodes(i).iCluster != INDEX_NONE && Model->Nodes(i).iCluster != iCluster )
					{
						CanMerge = 0;
					}
				}
				if( CanMerge )
				{
					// All fragments were in the same zone, so keep the original and discard the new fragments.
					for( int i = OriginalNumNodes; i < Model->Nodes.Num(); i++ )
					{
						Model->Nodes(i).NumVertices = 0;
					}
					// 이런 경우도 있네.. ^^;
					//check( iCluster != INDEX_NONE );
					if( iCluster == INDEX_NONE )
						Model->Nodes(iNode).NumVertices = 0;

					Model->Nodes(iNode).iCluster = iCluster;
				}
				else
				{
					// Keep the multi-zone fragments and remove the original plus any interior unnecessary polys.
					Model->Nodes(iNode).NumVertices = 0;
					for( int i=OriginalNumNodes; i<Model->Nodes.Num(); i++ )
					{
						if( Model->Nodes(i).iCluster == INDEX_NONE )
						{
							Model->Nodes(i).NumVertices = 0;
						}
					}
				}
			}
		}
		iNode = Model->Nodes(iNode).iPlane;
	}
}
//>@ ava

//<@ ava specific ; 2006. 12. 14 changmin
// 모든 level의 staticmesh actor를 persistent level의 bsp에 놓는다.

//
// Recursively filter a set of polys defining a convex hull down the Bsp,
// splitting it into two halves at each node and adding in the appropriate
// face polys at splits.
//
void FilterConvex
(
 UModel*			Model,
 INT				iNode,
 FPoly				**PolyList,
 INT				nPolys,
 INT				Outside,
 TArray<INT>		*Leaves
 )
{
	FMemMark Mark(GMem);
	FBspNode&	Node	= Model->Nodes(iNode);
	FBspSurf&	Surf	= Model->Surfs(Node.iSurf);
	FVector		Base	= Surf.Plane * Surf.Plane.W;
	FVector&	Normal	= Model->Vectors(Surf.vNormal);
	// Split bound into front half and back half.
	FPoly** FrontList = new(GMem,nPolys*2+16)FPoly*; int nFront=0;
	FPoly** BackList  = new(GMem,nPolys*2+16)FPoly*; int nBack=0;
	// Keeping track of allocated FPoly structures to delete later on.
	TArray<FPoly*>		AllocatedFPolys;
	FPoly* FrontPoly  = new FPoly;
	FPoly* BackPoly   = new FPoly;
	// Keep track of allocations.
	AllocatedFPolys.AddItem( FrontPoly );
	AllocatedFPolys.AddItem( BackPoly );
	for( INT i=0; i<nPolys; i++ )
	{
		FPoly *Poly = PolyList[i];
		switch( Poly->SplitWithPlane( Base, Normal, FrontPoly, BackPoly, 0 ) )
		{
		case SP_Coplanar:
			debugf( NAME_Log, TEXT("FilterBound: Got coplanar") );
			FrontList[nFront++] = Poly;
			BackList[nBack++] = Poly;
			break;
		case SP_Front:
			FrontList[nFront++] = Poly;
			break;
		case SP_Back:
			BackList[nBack++] = Poly;
			break;
		case SP_Split:
			FrontList[nFront++] = FrontPoly;
			BackList [nBack++] = BackPoly;
			FrontPoly = new FPoly;
			BackPoly  = new FPoly;
			// Keep track of allocations.
			AllocatedFPolys.AddItem( FrontPoly );
			AllocatedFPolys.AddItem( BackPoly );
			break;
		default:
			appErrorf( TEXT("FZoneFilter::FilterToLeaf: Unknown split code") );
		}
	}
	if( nFront && nBack )
	{
		// Add partitioner plane to front and back.
		FPoly InfiniteEdPoly = BuildInfiniteFPoly( Model, iNode );
		SplitPartitioner(Model,PolyList,FrontList,BackList,0,nPolys,nFront,nBack,InfiniteEdPoly,AllocatedFPolys);
	}
	else
	{
		//if( !nFront ) debugf( NAME_Log, TEXT("FilterBound: Empty fronthull") );
		//if( !nBack  ) debugf( NAME_Log, TEXT("FilterBound: Empty backhull") );
	}

	// Recursively update all our childrens' bounding volumes.
	if( nFront > 0 )
	{
		if( Node.iFront != INDEX_NONE )
		{
			FilterConvex( Model, Node.iFront, FrontList, nFront, Outside || Node.IsCsg(), Leaves );
		}
		else if( Outside || Node.IsCsg() )
		{
			// empty leaf
			if( Model->Leaves.Num() )
			{
				const INT LeafNumber = Node.iLeaf[1];
				//check( LeafNumber != INDEX_NONE);
				//check( !Leaves->ContainsItem( LeafNumber ) );
				if( LeafNumber != INDEX_NONE )
				{
					Leaves->AddUniqueItem( LeafNumber );
				}
				else
				{
					debugf(NAME_Warning, TEXT("a actor in invalid leaf."));
				}
			}
		}
		else
		{
			// solid leaf
		}
	}
	if( nBack > 0 )
	{
		if( Node.iBack != INDEX_NONE)
		{
			FilterConvex( Model, Node.iBack, BackList, nBack, Outside && !Node.IsCsg(), Leaves );
		}
		else if( Outside && !Node.IsCsg() )
		{
			// empty leaf
			if( Model->Leaves.Num() )
			{
				const INT LeafNumber = Node.iLeaf[0];
				//check( LeafNumber != INDEX_NONE );
				//check( !Leaves->ContainsItem( LeafNumber ) );
				if( LeafNumber != INDEX_NONE )
				{
					Leaves->AddUniqueItem( LeafNumber );
				}
				else
				{
					debugf(NAME_Warning, TEXT("a actor in invalid leaf."));
				}
			}
		}
		else
		{
			//solid leaf
		}
	}
	// Delete FPolys allocated above. We cannot use GMem for FPoly as the array data FPoly contains will be allocated in regular memory.
	for( INT i=0; i<AllocatedFPolys.Num(); i++ )
	{
		FPoly* AllocatedFPoly = AllocatedFPolys(i);
		delete AllocatedFPoly;
	}
	Mark.Pop();
}

void FEditorVisibility::ClearStaticMeshLeafInformation()
{
	//if( GWorld->PersistentLevel->Model != Model )
	//{
	//	return;
	//}
	// count actors in all levels.
	INT ActorCount = 0;
	for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); ++LevelIndex )
	{
		ULevel *Level = GWorld->Levels(LevelIndex);
		ActorCount += Level->Actors.Num();
	}

	// compute staticmeshactor's leaves
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("ClearStaticMeshActorLeafInformation")), TRUE );
	GWarn->StatusUpdatef(0, ActorCount, *LocalizeUnrealEd(TEXT("ClearStaticMeshActorLeafInformationF")), 0, ActorCount);
	INT GlobalActorIndex = 0;
	for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); ++LevelIndex )
	{
		ULevel *Level = GWorld->Levels(LevelIndex);
		for( INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ++ActorIndex )
		{
			AStaticMeshActor		*StaticMeshActor	= Cast<AStaticMeshActor>( Level->Actors(ActorIndex) );
			UStaticMeshComponent	*Component			= StaticMeshActor ? StaticMeshActor->StaticMeshComponent : NULL;

			if( Component && Component->DepthPriorityGroup != SDPG_World )
			{
				Component->FirstLeaf = -1;
				Component->LeafCount = -1;
			}
			++GlobalActorIndex;
			GWarn->StatusUpdatef(GlobalActorIndex, ActorCount, *LocalizeUnrealEd(TEXT("ClearStaticMeshActorLeafInformationF")), GlobalActorIndex, ActorCount);
		}
	}
	GWarn->EndSlowTask();
}

void FEditorVisibility::ComputeStaticMeshActorLeaves()
{
	//if( GWorld->PersistentLevel->Model != Model )
	//{
	//	return;
	//}
	// Model은 하나 임을~~

	// count actors in all levels.
	INT ActorCount = 0;
	for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); ++LevelIndex )
	{
		ULevel *Level = GWorld->Levels(LevelIndex);
		ActorCount += Level->Actors.Num();
	}

	// compute staticmeshactor's leaves
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("PlacingStaticMeshActors")), TRUE );
	GWarn->StatusUpdatef(0, ActorCount, *LocalizeUnrealEd(TEXT("PlacingStaticMeshActorsF")), 0, ActorCount);
	INT GlobalActorIndex = 0;
	for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); ++LevelIndex )
	{
		ULevel *Level = GWorld->Levels(LevelIndex);
		for( INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ++ActorIndex )
		{
			AStaticMeshActor		*StaticMeshActor	= Cast<AStaticMeshActor>( Level->Actors(ActorIndex) );
			UStaticMeshComponent	*Component			= StaticMeshActor ? StaticMeshActor->StaticMeshComponent : NULL;
			UStaticMesh				*Mesh				= Component ? Component->StaticMesh : NULL;
			URB_BodySetup			*BodySetup			= Mesh ? Mesh->BodySetup : NULL;

			if( Component && Component->DepthPriorityGroup != SDPG_World )
			{
				Component->FirstLeaf = -1;
				Component->LeafCount = -1;
				continue;
			}

			if( StaticMeshActor && Component && Mesh )
			{
				TArray<INT> Leaves;
				if( BodySetup && BodySetup->AggGeom.ConvexElems.Num() )
				{
					FMatrix Transform;
					FVector Scale3D;
					StaticMeshActor->StaticMeshComponent->GetTransformAndScale(Transform, Scale3D);
					const FMatrix LocalToWorld = FScaleMatrix(Scale3D) * Transform;
					for( INT ConvexIndex = 0; ConvexIndex < BodySetup->AggGeom.ConvexElems.Num(); ++ConvexIndex )
					{
						FKConvexElem& Convex = BodySetup->AggGeom.ConvexElems(ConvexIndex);
						const INT NumTriangles = Convex.FaceTriData.Num() / 3;
						TArray<FPoly> Faces;
						TArray<FPoly*> Polys;
						FPoly Poly;
						Faces.AddZeroed( NumTriangles );
						Polys.AddZeroed( NumTriangles );
						for( INT TriangleIndex = 0; TriangleIndex < NumTriangles; ++TriangleIndex )
						{
							FPoly &Poly = Faces( TriangleIndex );
							Polys( TriangleIndex ) = &Faces(TriangleIndex);
							Poly.Init();
							new(Poly.Vertices) FVector(LocalToWorld.TransformFVector(Convex.VertexData(Convex.FaceTriData(TriangleIndex * 3))));
							new(Poly.Vertices) FVector(LocalToWorld.TransformFVector(Convex.VertexData(Convex.FaceTriData(TriangleIndex * 3 + 1))));
							new(Poly.Vertices) FVector(LocalToWorld.TransformFVector(Convex.VertexData(Convex.FaceTriData(TriangleIndex * 3 + 2))));
							FPlane Plane( Poly.Vertices(0), Poly.Vertices(1), Poly.Vertices(2) );
							Poly.Base	= Poly.Vertices(0);
							Poly.Normal = Plane;
                        }
						FilterConvex(Model, 0, &Polys(0), NumTriangles,  Model->RootOutside, &Leaves );
					}
				}
				else if( Component )
				{
					debugf( NAME_Warning, TEXT("ComputeStaticMeshActorLeaves::static mesh has no body setup" ));
					
					// use bounding box to place.
					FBox Bound = Component->Bounds.GetBox();
					FPoly Polys[6], *PolyList[6];
					for( int i=0; i<6; i++ )
					{
						PolyList[i] = &Polys[i];
						PolyList[i]->Init();
						PolyList[i]->iBrushPoly = INDEX_NONE;
					}
					new(Polys[0].Vertices)FVector(Bound.Min.X, Bound.Min.Y, Bound.Max.Z);
					new(Polys[0].Vertices)FVector(Bound.Max.X, Bound.Min.Y, Bound.Max.Z);
					new(Polys[0].Vertices)FVector(Bound.Max.X, Bound.Max.Y, Bound.Max.Z);
					new(Polys[0].Vertices)FVector(Bound.Min.X, Bound.Max.Y, Bound.Max.Z);
					Polys[0].Normal   =FVector( 0.000000,  0.000000,  1.000000 );
					Polys[0].Base     =Polys[0].Vertices(0);

					new(Polys[1].Vertices)FVector(Bound.Min.X, Bound.Max.Y, Bound.Min.Z);
					new(Polys[1].Vertices)FVector(Bound.Max.X, Bound.Max.Y, Bound.Min.Z);
					new(Polys[1].Vertices)FVector(Bound.Max.X, Bound.Min.Y, Bound.Min.Z);
					new(Polys[1].Vertices)FVector(Bound.Min.X, Bound.Min.Y, Bound.Min.Z);
					Polys[1].Normal   =FVector( 0.000000,  0.000000, -1.000000 );
					Polys[1].Base     =Polys[1].Vertices(0);

					new(Polys[2].Vertices)FVector(Bound.Min.X, Bound.Max.Y, Bound.Min.Z);
					new(Polys[2].Vertices)FVector(Bound.Min.X, Bound.Max.Y, Bound.Max.Z);
					new(Polys[2].Vertices)FVector(Bound.Max.X, Bound.Max.Y, Bound.Max.Z);
					new(Polys[2].Vertices)FVector(Bound.Max.X, Bound.Max.Y, Bound.Min.Z);
					Polys[2].Normal   =FVector( 0.000000,  1.000000,  0.000000 );
					Polys[2].Base     =Polys[2].Vertices(0);

					new(Polys[3].Vertices)FVector(Bound.Max.X, Bound.Min.Y, Bound.Min.Z);
					new(Polys[3].Vertices)FVector(Bound.Max.X, Bound.Min.Y, Bound.Max.Z);
					new(Polys[3].Vertices)FVector(Bound.Min.X, Bound.Min.Y, Bound.Max.Z);
					new(Polys[3].Vertices)FVector(Bound.Min.X, Bound.Min.Y, Bound.Min.Z);
					Polys[3].Normal   =FVector( 0.000000, -1.000000,  0.000000 );
					Polys[3].Base     =Polys[3].Vertices(0);

					new(Polys[4].Vertices)FVector(Bound.Max.X, Bound.Max.Y, Bound.Min.Z);
					new(Polys[4].Vertices)FVector(Bound.Max.X, Bound.Max.Y, Bound.Max.Z);
					new(Polys[4].Vertices)FVector(Bound.Max.X, Bound.Min.Y, Bound.Max.Z);
					new(Polys[4].Vertices)FVector(Bound.Max.X, Bound.Min.Y, Bound.Min.Z);
					Polys[4].Normal   =FVector( 1.000000,  0.000000,  0.000000 );
					Polys[4].Base     =Polys[4].Vertices(0);

					new(Polys[5].Vertices)FVector(Bound.Min.X, Bound.Min.Y, Bound.Min.Z);
					new(Polys[5].Vertices)FVector(Bound.Min.X, Bound.Min.Y, Bound.Max.Z);
					new(Polys[5].Vertices)FVector(Bound.Min.X, Bound.Max.Y, Bound.Max.Z);
					new(Polys[5].Vertices)FVector(Bound.Min.X, Bound.Max.Y, Bound.Min.Z);
					Polys[5].Normal   =FVector(-1.000000,  0.000000,  0.000000 );
					Polys[5].Base     =Polys[5].Vertices(0);

					FilterConvex(Model, 0, PolyList, 6,  Model->RootOutside, &Leaves );
				}
				Component->FirstLeaf = Model->StaticMeshLeaves.Num();
				Component->LeafCount = Leaves.Num();
				Model->StaticMeshLeaves.Append(Leaves);
				debugf(NAME_Log, TEXT("Actor(%s) is on %d leaves"), *StaticMeshActor->GetName(), Leaves.Num() );
			}
			++GlobalActorIndex;
			GWarn->StatusUpdatef(GlobalActorIndex, ActorCount, *LocalizeUnrealEd(TEXT("PlacingStaticMeshActorsF")), GlobalActorIndex, ActorCount);
		}
	}
	GWarn->EndSlowTask();
}
//>@ ava
#pragma optimize( "", on )