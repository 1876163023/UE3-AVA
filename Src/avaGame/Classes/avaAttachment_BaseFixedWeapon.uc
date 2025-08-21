/*=============================================================================
  avaAttachment_BaseFixedWeapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/26 by OZ

	고정중화기 Attachment class

		avaWeap_BaseFixedWeapon 과 마찬가지로 이 Class 는 Weapon 의 Logical 한 부분만을 담당하며,
		Visual 적인 부분은 avaFixedHeavyWeapon 에서 담당한다.
	
=============================================================================*/
class avaAttachment_BaseFixedWeapon extends avaWeaponAttachment;


simulated function SkeletalMeshComponent GetSkeletalMeshComp()
{
	if ( avaWeap_BaseFixedHeavyWeapon( LinkedWeapon ) != None &&
		 avaWeap_BaseFixedHeavyWeapon( LinkedWeapon ).BaseWeap != None )
		 return SkeletalMeshComponent( avaWeap_BaseFixedHeavyWeapon( LinkedWeapon ).BaseWeap.WeaponMesh );
	return None;
}

simulated function AttachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	SKMesh = GetSkeletalMeshComp();
	if (SKMesh != none && MuzzleFlashSocket != '')
	{
		// Muzzle Flash mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.AttachComponentToSocket(MuzzleFlashMesh, MuzzleFlashSocket);
		if (MuzzleFlashPSCTemplate != None || MuzzleFlashAltPSCTemplate != None)
		{
			MuzzleFlashPSC = new(Outer) class'avaParticleSystemComponent';
			SKMesh.AttachComponentToSocket(MuzzleFlashPSC, MuzzleFlashSocket);
			MuzzleFlashPSC.DeactivateSystem();
			MuzzleFlashPSC.SetColorParameter('MuzzleFlashColor', MuzzleFlashColor);
		}

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
		    SKMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}
}

simulated function DetachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	SKMesh = GetSkeletalMeshComp();
	if (SKMesh != none)
	{
		// Muzzle Flash Mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.DetachComponent( MuzzleFlashMesh );

		if (MuzzleFlashPSC != none)
			SKMesh.DetachComponent( MuzzleFlashPSC );

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			SKMesh.DetachComponent( MuzzleFlashLight );
	}
}

simulated function CauseMuzzleFlashLight()
{
	local SkeletalMeshComponent SKMesh;
	SKMesh = GetSkeletalMeshComp();
	MuzzleFlashLight = new(Outer) MuzzleFlashLightClass;
	if ( (SKMesh != None) && (SKMesh.GetSocketByName(MuzzleFlashSocket) != None) )
	{
		SKMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}
}

//simulated function PlayTrailEffect( vector HitLocation )
//{
//	local vector	l;
//	local rotator	r;
//	local float		d;
//	local SkeletalMeshComponent SKMesh;
//	SKMesh = GetSkeletalMeshComp();
//	if (SKMesh.GetSocketWorldLocationAndRotation( MuzzleFlashSocket, l, r ) )
//	{
//		d = VSize(HitLocation-l);
//		if (d > 100)
//		{
//			BulletTrailComponent.Fire( l + (HitLocation-l) * 100 / d, HitLocation );
//		}			
//	}
//}

simulated event ThirdPersonFireEffects(vector HitLocation)
{
	// Play Fire Animation Of Character...
	Super.ThirdPersonFireEffects( HitLocation );
	// Play Fire Animation Of Weapon...
	if ( avaWeap_BaseFixedHeavyWeapon( LinkedWeapon ).BaseWeap != None )
		avaWeap_BaseFixedHeavyWeapon( LinkedWeapon ).BaseWeap.PlayAnimatin( LinkedWeapon.default.WeaponFireAnim[0], 0.0 );
}


defaultproperties
{
	DamageCode=Gun

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Rifle_MuzzleFlash_3P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'
}