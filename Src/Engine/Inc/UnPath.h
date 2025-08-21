/*=============================================================================
	UnPath.h: Path node creation and ReachSpec creations and management specific classes
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PATH_H
#define __PATH_H

#include "UnOctree.h"

#define MAXPATHDIST			1200 // maximum distance for paths between two nodes
#define MAXPATHDISTSQ		MAXPATHDIST*MAXPATHDIST
#define	PATHPRUNING			1.2f // maximum relative length of indirect path to allow pruning of direct reachspec between two pathnodes
#define MINMOVETHRESHOLD	4.1f // minimum distance to consider an AI predicted move valid
#define SWIMCOSTMULTIPLIER	2.f // cost multiplier for paths which require swimming
#define CROUCHCOSTMULTIPLIER 1.1f // cost multiplier for paths which require crouching
#define MAXTESTMOVESIZE		200.f // maximum size used for test moves

#define VEHICLEADJUSTDIST 400.f // used as threshold for adjusting trajectory of vehicles
#define TESTMINJUMPDIST  128.f	// assume any bJumpCapable pawn can jump this far in walk move test

// magic number distances used by AI/networking
#define CLOSEPROXIMITY		500.f
#define	PROXIMATESOUNDTHRESHOLD 50.f
#define MINSOUNDOVERLAPTIME	0.18f
#define MAXSOUNDOVERLAPTIME	0.2f
#define NEARSIGHTTHRESHOLD	2000.f
#define MEDSIGHTTHRESHOLD	3162.f
#define FARSIGHTTHRESHOLD	8000.f
#define	PROXIMATESOUNDTHRESHOLDSQUARED PROXIMATESOUNDTHRESHOLD*PROXIMATESOUNDTHRESHOLD
#define CLOSEPROXIMITYSQUARED CLOSEPROXIMITY*CLOSEPROXIMITY
#define NEARSIGHTTHRESHOLDSQUARED NEARSIGHTTHRESHOLD*NEARSIGHTTHRESHOLD
#define MEDSIGHTTHRESHOLDSQUARED MEDSIGHTTHRESHOLD*MEDSIGHTTHRESHOLD
#define FARSIGHTTHRESHOLDSQUARED FARSIGHTTHRESHOLD*FARSIGHTTHRESHOLD

//Reachability flags - using bits to save space
enum EReachSpecFlags
{
	R_WALK = 1,	//walking required
	R_FLY = 2,   //flying required 
	R_SWIM = 4,  //swimming required
	R_JUMP = 8,   // jumping required
	R_HIGHJUMP = 16	// higher jump required (not useable without special abilities)
}; 

#define MAXSORTED 32
class FSortedPathList
{
public:
	ANavigationPoint *Path[MAXSORTED];
	INT Dist[MAXSORTED];
	int numPoints;

	FSortedPathList() { numPoints = 0; }
	ANavigationPoint* FindStartAnchor(APawn *Searcher); 
	ANavigationPoint* FindEndAnchor(APawn *Searcher, AActor *GoalActor, FVector EndLocation, UBOOL bAnyVisible, UBOOL bOnlyCheckVisible); 
	void AddPath(ANavigationPoint * node, INT dist);
};

class FPathBuilder
{
public:
	static AScout* GetScout();
	static void DestroyScout();

private:
	static AScout *Scout;

	/**
	 * Builds the per-level nav lists and then assembles the world list.
	 */
	void BuildNavLists();

public:
	/**
	 * Toggles collision on all actors for path building.
	 */
	void SetPathCollision(UBOOL bEnabled);

	/**
	 * Moves all interp actors to the path building position.
	 */
	void UpdateInterpActors(UBOOL &bProblemsMoving, TArray<USeqAct_Interp*> &InterpActs);

	/**
	 * Moves all updated interp actors back to their original position.
	 */
	void RestoreInterpActors(TArray<USeqAct_Interp*> &InterpActs);

	/**
	 * Clears all the paths and rebuilds them.
	 *
	 * @param	bReviewPaths	If TRUE, review paths if any were created.
	 * @param	bShowMapCheck	If TRUE, conditionally show the Map Check dialog.
	 */
	void DefinePaths(UBOOL bReviewPaths, UBOOL bShowMapCheck);

	/**
	 * Clears all pathing information in the level.
	 */
	void UndefinePaths();

	void PrunePaths(INT NumPaths);
	void ReviewPaths();
	void AdjustCover( UBOOL bFromDefinePaths = FALSE );
	void BuildCover(  UBOOL bFromDefinePaths = FALSE );
};

/** specialized faster versions of FNavigationOctreeObject Owner accessor for common types in ENavOctreeObjectType enum */
template<> FORCEINLINE UObject* FNavigationOctreeObject::GetOwner<UObject>()
{
	return Owner;
}
template<> FORCEINLINE ANavigationPoint* FNavigationOctreeObject::GetOwner<ANavigationPoint>()
{
	return (OwnerType & NAV_NavigationPoint) ? (ANavigationPoint*)Owner : NULL;
}
template<> FORCEINLINE UReachSpec* FNavigationOctreeObject::GetOwner<UReachSpec>()
{
	return (OwnerType & NAV_ReachSpec) ? (UReachSpec*)Owner : NULL;
}

/** a single node in the navigation octree */
class FNavigationOctreeNode : public FOctreeNodeBase
{
private:
	/** children of this node, either NULL or 8 elements */
	FNavigationOctreeNode* Children;

	/** objects in this node */
	TArray<struct FNavigationOctreeObject*> Objects;

public:
	/** constructor */
	FNavigationOctreeNode()
		: Children(NULL)
	{}
	/** destructor, clears out Objects array and deletes Children if they exist */
	~FNavigationOctreeNode()
	{
		// clear out node pointer in all of the objects
		for (INT i = 0; i < Objects.Num(); i++)
		{
			Objects(i)->OctreeNode = NULL;
		}
		Objects.Empty();
		// delete children
		if (Children != NULL)
		{
			delete [] Children;
			Children = NULL;
		}
	}

	/** filters an object with the given bounding box through this node, either adding it or passing it to its children
	 * if the object is added to this node, the node may also be split if it exceeds the maximum number of objects allowed for a node without children
	 * assumes the bounding box fits in this node and always succeeds
	 * @param Object the object to filter
	 * @param NodeBounds the bounding box for this node
	 */
	void FilterObject(struct FNavigationOctreeObject* Object, const FOctreeNodeBounds& NodeBounds);

	/** searches for an entry for the given object and removes it if found
	 * @param Object the object to remove
	 * @return true if the object was found and removed, false if it wasn't found
	 */
	UBOOL RemoveObject(struct FNavigationOctreeObject* Object)
	{
		return (Objects.RemoveItem(Object) > 0);
	}

	/** returns all objects in this node and all children whose bounding box intersects with the given sphere
	 * @param Point the center of the sphere
	 * @param RadiusSquared squared radius of the sphere
	 * @param Extent bounding box for the sphere
	 * @param OutObjects (out) all objects found in the radius
	 * @param NodeBounds the bounding box for this node
	 */
	void RadiusCheck(const FVector& Point, FLOAT RadiusSquared, const FBox& Extent, TArray<FNavigationOctreeObject*>& OutObjects, const FOctreeNodeBounds& NodeBounds);

	/** checks the given box against the objects in this node and returns all objects found that intersect with it
	 * recurses down to children that intersect the box
	 * @param Box the box to check
	 * @param NodeBounds the bounding box for this node
	 * @param OutObjects (out) all objects found that intersect
	 */
	void OverlapCheck(const FBox& Box, TArray<FNavigationOctreeObject*>& OutObjects, const FOctreeNodeBounds& NodeBounds);

	/** counts the number of nodes and objects there are in the octree
	 * @param NumNodes (out) incremented by the number of nodes (this one plus all children)
	 * @param NumObjects (out) incremented by the total number of objects in this node and its child nodes
	 */
	void CollectStats(INT& NumNodes, INT& NumObjects);

	UBOOL FindObject( UObject* Owner, UBOOL bRecurseChildren );
};

/**
 * Octree containing NavigationPoint/ReachSpec bounding boxes for quickly determining things like whether a given location is on the path network 
 *
 * We deliberatly don't serialize its object references as the octree won't be the only reference and the AddReferencedObject interface doesn't
 * support NULLing out references anyways.
 */
class FNavigationOctree 
{
private:
	/** the root node encompassing the entire world */
	class FNavigationOctreeNode* RootNode;

	/** the bounds of the root node; should be the size of the world */
	static const FOctreeNodeBounds RootNodeBounds;

public:
	/** constructor, creates the root node */
	FNavigationOctree()
	{
		RootNode = new FNavigationOctreeNode;
	}
	/** destructor, deletes the root node */
	~FNavigationOctree()
	{
		delete RootNode;
		RootNode = NULL;
	}

	/** adds an object with the given bounding box
	 * @param Object the object to add
	 * @note this method assumes the object is not already in the octree
	 */
	void AddObject(struct FNavigationOctreeObject* Object);
	/** removes the given object from the octree
	 * @param Object the object to remove
	 * @return true if the object was in the octree and was removed, false if it wasn't in the octree
	 */
	UBOOL RemoveObject(struct FNavigationOctreeObject* Object);

	/** returns all objects in the octree whose bounding box intersects with the given sphere
	 * @param Point the center of the sphere
	 * @param Radius radius of the sphere
	 * @param OutObjects (out) all objects found in the radius
	 */
	void RadiusCheck(const FVector& Point, FLOAT Radius, TArray<FNavigationOctreeObject*>& OutObjects)
	{
		FVector Extent(Radius, Radius, Radius);
		RootNode->RadiusCheck(Point, Radius * Radius, FBox(Point - Extent, Point + Extent), OutObjects, RootNodeBounds);
	}

	/** checks the given point with the given extent against the octree and returns all objects found that intersect with it
	 * @param Point the origin for the point check
	 * @param Extent extent of the box for the point check
	 * @param OutObjects (out) all objects found that intersect
	 */
	void PointCheck(const FVector& Point, const FVector& Extent, TArray<FNavigationOctreeObject*>& OutObjects)
	{
		RootNode->OverlapCheck(FBox(Point - Extent, Point + Extent), OutObjects, RootNodeBounds);
	}

	/** checks the given box against the octree and returns all objects found that intersect with it
	 * @param Box the box to check
	 * @param OutObjects (out) all objects found that intersect
	 */
	void OverlapCheck(const FBox& Box, TArray<FNavigationOctreeObject*>& OutObjects)
	{
		RootNode->OverlapCheck(Box, OutObjects, RootNodeBounds);
	}

	/** console command handler for implementing debugging commands */
	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);

	/** 
	 * Removes all objects from octree.
	 */
	void RemoveAllObjects();
};

//superville
/////////////////////////////////////////////////////////////
///////////////  A* PATH SEARCHING //////////////////////////
/////////////////////////////////////////////////////////////

#define		ASTAR_MAXPATHLIMIT	10000.f

struct FAStarPath
{
	/** Cost of partial path described by NavList */
	FLOAT PartialPathCost;
	/** Cost estimate == distance of last node in list to goal */
	FLOAT EstimateCost;
	/** List of nodes for this partial path - start -> goal */
	TArray<ANavigationPoint*> NavList;

	// Consturctor 
	FAStarPath() :
		PartialPathCost( 0.f ),
		EstimateCost( 0.f )
	{
	}

	// - used to append node to a previous path and calc new costs
	UBOOL SetPath( FAStarPath* PrevPath, UObject* AddObject, ANavigationPoint* Goal, APawn* Pawn, FLOAT MaxPathLength );

	/** Current path has reached goal */
	UBOOL IsPathTo( ANavigationPoint* Goal )
	{
		return (NavList.Num() && NavList.Top() == Goal);
	}

	/** Partial path cost + distance to goal */
	inline FLOAT GetPathCost()
	{
		return PartialPathCost + EstimateCost;
	}

	/** Returns TRUE if a shorter path exists in list already and this path should be discarded 
	  * Returns FALSE if no matching path exists or if a longer one exists 
	  *		(in which case, it removes the longer path from the list before returning)
	  */
	UBOOL ShorterPathExists( TArray<FAStarPath>& PathList );

	/**		
	  *	Sets route cache for given controller after doing 
	  *	some cleanup on nodes that may have already been reached 
	  */
	void SetRouteCache( AController* C, FLOAT StartDist, FLOAT EndDist );

	/**
	 *	Give game a chance to remove unnecessary nodes from the route
	 */
	void PruneRoute( APawn* Pawn );

	/** DEBUG FUNCTIONS */
	/** Dumps the current paths to log */
	void DumpPath();
	/** Checks for a loop in the path */
	UBOOL HasLoop();
	/** END DEBUG FUNCTIONS */


	static UBOOL AStarPathTo( APawn* Pawn, ANavigationPoint *Start, ANavigationPoint* Goal, FLOAT MaxPathLength, UBOOL bReturnPartial, FAStarPath& Result );
};

/**
 * For use with the templated sort. Sorts by class name then object name
 */
struct FAStarPathCompare
{
	static INT Compare( FAStarPath& A, FAStarPath& B )
	{
		return (A.GetPathCost() < B.GetPathCost());
	}
};

#endif // __PATH_H
