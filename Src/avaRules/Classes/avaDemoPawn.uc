//=============================================================================
//  avaDemoPawn
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/13 by OZ
//		4월28일 Prototype Demo 를 위한 시연용 Pawn
//=============================================================================
class avaDemoPawn extends avaCharacter_RifleMan
	placeable;

var(DemoPawn) ETeamType						CurTeamType;
var(DemoPawn) int							ClassType;
var(DemoPawn) class<avaWeaponAttachment>	WeaponAttachmentClass;
var(DemoPawn) bool							bNoDamage;

simulated function ChangeMesh( int nTeam )
{

}

simulated function SetMeshVisibility(bool bVisible)
{

}

simulated function NotifyTeamChanged()
{
	ChangeMesh( CurTeamType );
	SetPhysics( PHYS_Walking );

	//if ( WeaponAttachmentClass != None )
	//{
	//	CurrentWeaponAttachmentClass = WeaponAttachmentClass;
	//	WeaponAttachmentChanged();
	//}
}

function DoDamage( int Damage )
{
	if ( !bNoDamage )	Super.DoDamage( Damage );
}

defaultproperties
{
	//Begin Object Name=WPawnSkeletalMeshComponent
	//	SkeletalMesh=SkeletalMesh'CH_IronGuard.Mesh.SK_CH_IronGuard_FemaleA'
	//	AnimSets(0)=AnimSet'CH_BaseAnim.Anim.K_CH_BaseMale'
	//	AnimTreeTemplate=AnimTree'CH_BaseAnim.Anim.AT_CH_BaseChar'
	//	PhysicsAsset=PhysicsAsset'ch-eu-1.Mesh.ch-ussr-test_Physics'
	//	BlockRigidBody=true
	//	CastShadow=true
	//	//Translation=(X=0.0,Y=0.0,Z=-48.0)
	//	//Rotation=(Yaw=-16384)
	//End Object
}
