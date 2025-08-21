/**
 * avaDmgType_Bomb
 *
 *
 *
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaDmgType_C4 extends avaDmgType_Explosion;

defaultproperties
{
	KDamageImpulse=1500
	KDeathImpulse=10000
	KDeathUpKick=200
	bKRadialImpulse=true
	bThrowRagdoll=true
	GibPerterbation=0.15
	bForceTeamDamage=true
}
