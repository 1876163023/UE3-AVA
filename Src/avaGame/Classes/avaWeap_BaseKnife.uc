//=============================================================================
//  avaWeap_BaseKnife
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/03/03 by OZ
//		Class ����	
//	2006/03/15 by OZ
//		Fire �� Interval �� ������ Hit ������ �Ѵ�.
//	2006/07/10 by OZ
//		�꽺���� ���� Animation ��ȭ ���� (Penalty ����)
//
//=============================================================================
class avaWeap_BaseKnife extends avaWeapon;

var	array<float>			HitDecisionTime;	// Fire �� ���� Interval �� ���� �� Hit ������ �� ���ΰ�?

var	bool					bAutoFire;

var(Sounds)	array<SoundCue>	KnifeFireSnd;

var array<float>			KnifeRange;

// Knife �� �Ѿ��� �ʿ����.
simulated function bool HasAnyAmmo()		{	return true;	}

// Knife �� �Ѿ��� �Һ����� �ʴ´�.
function ConsumeAmmo( byte FireModeNum )	{}

simulated function PlayFiringSound()
{
	// play weapon fire sound
	if ( KnifeFireSnd.length > CurrentFireMode )
	{
		WeaponPlaySound( KnifeFireSnd[CurrentFireMode] );
	}
	InvManager.OwnerEvent('WeaponFiringSound');
}

simulated function ProcessInstantHit( byte FiringMode, ImpactInfo Impact )
{
	local float		TotalDamage;
	local avaPawn		AP;
	local DamageData	damageData;
	// cause damage to locally authoritative actors
	TotalDamage  =	InstantHitDamage[CurrentFireMode] * avaPawn(Instigator).WeapTypeAmp[WeaponType].DamageAmp;
	if (Impact.HitActor != None )
	{
		AP = avaPawn(Impact.HitActor);
		// ���� ����� Pawn �̶�� Local ���� �Ǵ��Ѱ��� Host �� �÷� �����ش�...
		if ( AP != None  )
		{
			if ( Instigator.IsLocallyControlled() )
			{
				// client�� ���
				if (WorldInfo.NetMode != NM_ListenServer && WorldInfo.NetMode != NM_DedicatedServer)
				{
					// play clientside effects
					AP.PlayZeroLatencyEffect( Impact, Instigator.Controller );

				}
				damageData.Damage		= TotalDamage;
				damageData.BoneIndex	= AP.Mesh.MatchRefBone( Impact.HitInfo.BoneName );
				AP.Client_RequestTakeGunDamage( damageData, Instigator.Controller );
			}			
		}
		else if ( Impact.HitActor.Role == ROLE_Authority )
		{
			
			Impact.HitActor.TakeDamage( TotalDamage, Instigator.Controller,
							Impact.HitLocation, InstantHitMomentum[FiringMode] * Impact.RayDir,
							InstantHitDamageTypes[FiringMode], Impact.HitInfo, Class );
		}
	}
}

simulated function float GetTraceRange()
{
	return KnifeRange[CurrentFireMode] * avaPawn(Instigator).WeapTypeAmp[WeaponType].RangeAmp;
}

simulated function ImpactInfo CalcWeaponFire(vector StartTrace, vector EndTrace, optional out array<ImpactInfo> ImpactList)
{
	local vector			HitLocation, HitNormal;
	local Actor				HitActor;
	local TraceHitInfo		HitInfo;
	local ImpactInfo		CurrentImpact;
	
	// Perform trace to retrieve hit info
	HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE, vect(0,0,0), HitInfo, TRACEFLAG_Bullet);
	// If we didn't hit anything, then set the HitLocation as being the EndTrace location
	if( HitActor == None )
	{
		HitLocation	= EndTrace;
	}
	
	// Convert Trace Information to ImpactInfo type.
	CurrentImpact.HitActor		= HitActor;
	CurrentImpact.HitLocation	= HitLocation;
	CurrentImpact.HitNormal		= HitNormal;
	CurrentImpact.RayDir		= Normal(EndTrace-StartTrace);
	CurrentImpact.HitInfo		= HitInfo;
	// Add this hit to the ImpactList
	ImpactList[ImpactList.Length] = CurrentImpact;
	return CurrentImpact;
}

simulated state WeaponFiring
{
	simulated event bool IsFiring()
	{
		return true;
	}

	simulated function BeginState( Name PreviousStateName )
	{
		PlayFire();
		TimeWeaponFiring( CurrentFireMode );
	}

	// Fire Animation �� ������ �� Timer �� Setting �Ѵ�.
	// HitDecisionTime �� FireInterval ���� ���� �ȵȴ�.
	simulated function PlayFire()
	{
		if ( Instigator.IsLocallyControlled() )
		{
			PlayFireEffects( CurrentFireMode );
		}

		IncrementFlashCount();

		if ( HitDecisionTime.length > CurrentFireMode && HitDecisionTime[CurrentFireMode] > 0 )
		{
			SetTimer( HitDecisionTime[CurrentFireMode], false, 'Fire' );
		}
		else
		{
			Fire();
		}
	}

	simulated function RefireCheckTimer()
	{
		if( bWeaponPutDown )
		{
			PutDownWeapon();
			return;
		}

		if( ShouldRefire() )
		{
			PlayFire();
			return;
		}

		// Otherwise we're done firing, so go back to active state.
		GotoState('Active');
	}

	simulated function Fire()
	{
		FireAmmunition();
	}

	simulated function FireAmmunition()
	{
		CustomFire();

		if ( !bAutoFire )
			ClearPendingFire(0);
	}

	// Hit ������ �Ѵ�.
	simulated function CustomFire()
	{
		local vector			StartTrace, EndTrace;
		local Array<ImpactInfo>	ImpactList;
		local int				Idx;
		local ImpactInfo		RealImpact;

		Super.CustomFire();

		// define range to use for CalcWeaponFire()
		StartTrace = Instigator.GetPawnViewLocation();
		EndTrace = StartTrace + vector(Instigator.GetViewRotation()) * GetTraceRange();
		// Perform shot
		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);

		if ( RealImpact.HitActor != None )
		{	
			//SetFlashLocation(RealImpact.HitLocation);
			if ( !RealImpact.HitActor.IsA( 'avaShatterGlassActor' ) )
				SetImpactLocation(RealImpact.HitLocation);
			
		}

		// cass �� ��� ���� �ε��ļ� �Ҹ��� ���ٸ� �⺻ �Ҹ��� Play ���� �ʴ´�.
		PlayFiringSound();

		// Process all Instant Hits on local player and server (gives damage, spawns any effects).
		for (Idx = 0; Idx < ImpactList.Length; Idx++)
		{
			ProcessInstantHit(CurrentFireMode, ImpactList[Idx]);
		}
	}
}

defaultproperties
{
	BaseSkelMeshName	=	"Wp_Knife01.MS_Knife01"
	BaseAnimSetName		=	"Wp_Knife01.Ani_Knife01"

	InventoryGroup				=	3
	GroupWeight					=	0.5

	WeaponFireTypes(0)			=	EWFT_Custom
	WeaponFireTypes(1)			=	EWFT_Custom

	FiringStatesArray(0)		=	WeaponFiring	
	FiringStatesArray(1)		=	WeaponFiring	

	/** Animation Naming & �ӵ� ���� **/
	WeaponFireAnim(0)			=	Fire1
	WeaponAltFireAnim(0)		=	Fire2

 	WeaponPutDownAnim			=	Down
	WeaponEquipAnim				=	BringUp
	WeaponIdleAnims(0)			=	Idle
	EquipTime					=	0.74
	PutDownTime					=	0.0333

	/** Sounds **/
	KnifeFireSnd(0)				=	SoundCue'avaWeaponSounds.Knife.Knife_swing'
	KnifeFireSnd(1)				=	SoundCue'avaWeaponSounds.Knife.Knife_stab'

	/** Damage **/
	InstantHitDamage(0)			=	20.0
	InstantHitDamage(1)			=	65.0
	InstantHitMomentum(0)		=	0.0
	InstantHitMomentum(1)		=	0.0
	InstantHitDamageTypes(0)	=	class'avaDmgType_Melee'
	InstantHitDamageTypes(1)	=	class'avaDmgType_Melee'

	/** Knife **/
	HitDecisionTime(0)			=	0.1832
	HitDecisionTime(1)			=	0.5931

	FireInterval(0)				=	0.35
	FireInterval(1)				=	0.75

	KnifeRange(0)				=	70
	KnifeRange(1)				=	85

	bAutoFire					=	true
	bMeleeWeapon				=	true;	// AI hint

	/** Attachment **/
	

	// Weapon �� ���� Pawn �� �ӵ� ����
	BaseSpeed			= 285	// �⺻�ӵ�
	AimSpeedPct			= 0.8	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct= 1	// �ɾƼ� ������Ʈ�� ����ġ

	WeaponType			= WEAPON_KNIFE

	CrossHairMtrl		=	Texture2D'avaUI.HUD.CrossImage'	
}

