//=============================================================================
//  avaPickUp_C4
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/06 by OZ
//		1. 폭탄을 떨어뜨렸거나 주었을때 HUD 에 표시해준다.
//=============================================================================
class avaPickUp_C4 extends avaPickup;

var()	float		ExplodeTime;
var()	float		DefuseTime;

function GiveToEx( Pawn P, Inventory Inv )
{
	Super.GiveToEx( P, Inv );
	if ( Inv == None ) return;
	if ( ExplodeTime != 0.0 )	avaWeap_C4(Inv).ExplodeTime = ExplodeTime;
	if ( DefuseTime  != 0.0 )	avaWeap_C4(Inv).DefuseTime	= DefuseTime;
}

event TakeDamage(int nDamage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> DamageCauser)
{

}

defaultproperties
{
	InventoryClass=class'avaWeap_C4'
	// C4 는 LifeSpan 을 가지면 안된다.
	LifeSpan	= 0
	bDrawInRadar= true
	IconCode	= 16
	LifeTime	= 0
	StaticMeshName = "Wp_New_C4.MS_C4_3p"
	bCollideWorld	= true
}