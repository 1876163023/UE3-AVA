/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaShatterGlassComponent extends PrimitiveComponent
	native
	noexport;

struct SupportRow
{
	var float Value[16];
};

struct StateRow
{
	var byte Value[16];
};

var() const MaterialInstance	Material;
var() const MaterialInstance	BrokenMaterial;
var() const int NumWide;
var() const int NumHigh;
var() const int Fragility;
var() bool bIsBroken;
var private bool bNeedsUpdateSupport;
var private bool bSolidRenderListUpdated;
var private SupportRow Support[16];
var private StateRow State[16];
var private int NumBrokenPanes;
var private int NumEdgeTrisToRender, NumSolidTrisToRender;
var private const	pointer	Renderer;		

var MaterialInstanceConstant Style[12];
var name DetailParameter;
var Texture2D Texture_Style[12];

struct EdgeInfo
{
	var private 	byte			x;
	var private 	byte			z;
	var private 	byte			Side;
	var private 	byte			EdgeType;
	var private 	byte			Style;	
};

struct SolidInfo
{
	var private vector Base, X, Z;
};

var private native transient array<EdgeInfo> EdgeRenderList;
var private native transient array<SolidInfo> SolidRenderList;

var int LastRound;

native simulated function BreakBitmap( int i, int diff );
native simulated function Break( int x, int z );
native simulated function BreakAll();
native simulated function UpdateEdges();
native simulated function Reset();
native function GlassTick();
native function TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor weapon );

defaultproperties
{
	CastShadow=FALSE
	bAcceptsLights=TRUE
	
	CollideActors=True
	BlockActors=True
	BlockZeroExtent=True
	BlockNonZeroExtent=True
	BlockRigidBody=True	

	DetailParameter=Detail
	bNotifyRigidBodyCollision=True

	Material=Material'avaGlass.test'
	BrokenMaterial=Material'avaGlass.testBroken'
	Fragility=50

	NumHigh=16
	NumWide=16

	RBChannel=RBCC_GameplayPhysics
	RBCollideWithChannels=(Default=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE)

	
}
