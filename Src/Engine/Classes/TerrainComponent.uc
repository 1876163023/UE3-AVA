/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class TerrainComponent extends PrimitiveComponent
	native(Terrain)
	noexport;

/**	INTERNAL: Array of shadow map data applied to the terrain component.		*/
var private const array<ShadowMap2D> ShadowMaps;
/**	INTERNAL: Array of lights that don't apply to the terrain component.		*/
var const array<Guid>				IrrelevantLights;

var const native transient pointer	TerrainObject;
var const int						SectionBaseX,
									SectionBaseY,
									SectionSizeX,
									SectionSizeY;
									
/** The actual section size in vertices...										*/
var const int						TrueSectionSizeX;
var const int						TrueSectionSizeY;

var native private const pointer	LightMap{FLightMap2D};

var private const native transient array<int>	PatchBounds;
var private const native transient array<int>	PatchBatches;
var private const native transient array<int>	BatchMaterials;
var private const native transient int		FullBatch;

var private const native transient pointer	PatchBatchOffsets;
var private const native transient pointer	WorkingOffsets;
var private const native transient pointer	PatchBatchTriangles;
var private const native transient pointer	PatchCachedTessellationValues;
var private const native transient pointer	TesselationLevels;


/**
 * Place holder structure that mirrors the byte size needed
 * for a kDOP tree
 */
struct TerrainkDOPTree
{
	var private const native array<int> Nodes;
	var private const native array<int> Triangles;
};

/**
 * Used for box checks against terrain
 */
var private const native transient TerrainkDOPTree BoxCheckTree;
/**
 * This is a low poly version of the terrain vertices in local space. The
 * triangle data is created based upon Terrain->CollisionTesselationLevel
 */
var private const native transient array<int>		CollisionVertices;

/** Physics engine version of heightfield data. */
var const native pointer RBHeightfield;

//!{ 2006-08-10	Çã Ã¢ ¹Î
var private native transient const int FirstExportedFaceNumber;
//!} 2006-08-10	Çã Ã¢ ¹Î

defaultproperties
{
	CollideActors=TRUE
	BlockActors=TRUE
	BlockZeroExtent=TRUE
	BlockNonZeroExtent=TRUE
	BlockRigidBody=TRUE
	CastShadow=TRUE
	bAcceptsLights=TRUE
	bAcceptsDecals=TRUE
	bUsePrecomputedShadows=TRUE
	bUseAsOccluder=TRUE
}
