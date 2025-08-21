class avaWeap_BaseRifle extends avaWeap_BaseGun
	native;

var(RifleBurst) int BurstMax;				/** How many shots max in burst before going to cool down */
var(RifleBurst) float BurstInterval;				/** How long between burst shots */
var(RifleBurst) float BurstCoolDownTime;			/** How long to cool down */

var int BurstCnt;					/** How many shots have been fired in burst */
var byte CurrentBurstMode;

//var() SkeletalMeshComponent	ScopeComp;
//var string					ScopeMeshName;



var Texture2D	HudMaterial;

var vector	LightDirInWorldSpace;
var vector	LightDirInViewSpace;
var Color	LightColor;
var float	LightBrightness;




replication
{
	if (Role == Role_Authority && bNetDirty)
			CurrentBurstMode;
}

simulated function AttachScope()
{
	local SkeletalMesh			tempMesh;
	local vector				translation;
	if ( ScopeMeshName != "" )
	{
		tempMesh = SkeletalMesh( DynamicLoadObject( ScopeMeshName, class'SkeletalMesh' ) );

		translation		= ScopeComp.Translation;
		Translation.x	= -8.0; 
		translation.y	= -PlayerViewOffset.y;
		translation.z	= -PlayerViewOffset.z;
		ScopeComp.SetTranslation( translation );
		SkeletalMeshComponent(ScopeComp).SetSkeletalMesh( tempMesh );
		ScopeComp.SetShadowParent( Mesh );
		ScopeComp.SetOcclusionGroup( Mesh );		
		ScopeComp.SetHidden( true );
		SkeletalMeshComponent(ScopeComp).PlayAnim( 'Idle',, false );
		ScopeMIC = ScopeComp.CreateMaterialInstance( tempMesh.Materials.Length-1 );
	}
}

simulated function AttachItems()
{
	Super.AttachItems();
	AttachScope();
}

simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	Super.PlayFireEffects( FireModeNum, HitLocation );
	if ( SkeletalMeshComponent( ScopeComp ) != None )
		SkeletalMeshComponent( ScopeComp ).PlayAnim( 'Fire',, false );
}

//simulated function SetPosition( avaPawn Holder )
//{
//	local rotator	NewRotation;
//	local vector	ViewOffset;
//	local vector	SkyDirection;
//	local float		UpperBrightness, LowerBrightness;
//	local color		UpperColor, LowerColor;
//	Super.SetPosition( Holder );
//
//	if ( SightMode >= 1 && ScopeMIC != None )
//	{
//		SkyDirection = vect( 0, 0, -1 );
//		// Get Light Direction in World Space.
//		LightDirInWorldSpace	= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastDirection;
//		LightColor				= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastColor;
//		LightBrightness			= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastBrightness;	
//
//		UpperBrightness			= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastUpperSkyBrightness;
//		LowerBrightness			= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastLowerSkyBrightness;
//		UpperColor				= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastUpperSkyColor;
//		LowerColor				= DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastLowerSkyColor;
//
//		UpperBrightness = FMin( 1.0, UpperBrightness );
//		LowerBrightness = FMin( 1.0, LowerBrightness );
//		
//		// Adjust View Matrix to Light Direction.
//		Holder.Controller.GetPlayerViewPoint( ViewOffset, NewRotation );
//		LightDirInViewSpace = TransformTest( Vect(0,0,0), NewRotation, LightDirInWorldSpace );
//		SkyDirection		= TransformTest( Vect(0,0,0), NewRotation, SkyDirection );
//
//		ScopeMIC.SetVectorParameterValue( 'LightDir', MakeLinearColor( -LightDirInViewSpace.x , LightDirInViewSpace.y , LightDirInViewSpace.z , 0.0 ) );
//		ScopeMIC.SetVectorParameterValue( 'SkyDirection', MakeLinearColor( -SkyDirection.x , SkyDirection.y , SkyDirection.z , 0.0 ) );
//		ScopeMIC.SetVectorParameterValue( 'LightColor', ColorToLinearColor( LightColor ) * LightBrightness );
//		ScopeMIC.SetVectorParameterValue( 'UpperColor', ColorToLinearColor( UpperColor ) * UpperBrightness );
//		ScopeMIC.SetVectorParameterValue( 'LowerColor', ColorToLinearColor( LowerColor ) * LowerBrightness );
//	}
//}

//simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
//{
//	local string	T;
//	local float		UpperBrightness, LowerBrightness;
//	Super.DisplayDebug( HUD, out_YL, out_YPos );
//	T = "LightBrightness " $LightBrightness$ " LightColor " $LightColor.R $LightColor.G $LightColor.B$ " LightDir(World Space) "$LightDirInWorldSpace$ "  LightDir(View Space) " $LightDirInViewSpace;
//	HUD.Canvas.SetPos(24,out_YPos);
//    HUD.Canvas.DrawText(T, false);
//    out_YPos += out_YL;
//
//	UpperBrightness = DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastUpperSkyBrightness;
//	LowerBrightness = DynamicLightEnvironmentComponent( Mesh.LightEnvironment ).LastLowerSkyBrightness;
//
//	T = "SkyUpperBrightness " $UpperBrightness$ " LowerBrightness " $LowerBrightness;
//	HUD.Canvas.SetPos(24,out_YPos);
//    HUD.Canvas.DrawText(T, false);
//    out_YPos += out_YL;
//}

simulated function bool BurstModeSupported()
{
	return BurstMax > 0;
}

simulated function SwitchToNextBurstMode()
{
//	`log( "SwitchToNextBurstMode" );

	/// Burst mode가 지원되는 화기인가?
	if (BurstMax > 0)
	{
		SwitchBurstMode( (CurrentBurstMode + 1) % 2 );	
	}
}

simulated function SwitchBurstMode( byte BurstModeNum )
{
//	`log( "SwitchBurstMode "@BurstModeNum );

	ChangeBurstMode( BurstModeNum );

	// Notify the server
	if( Role < Role_Authority )
	{
		ServerSwitchBurstMode( BurstModeNum );
	}
}

reliable server function ServerSwitchBurstMode(byte BurstModeNum)
{
//	`log( "ServerSwitchBurstMode "@BurstModeNum );

	ChangeBurstMode( BurstModeNum );
}

simulated function ChangeBurstMode( byte BurstModeNum )
{
	ServerStopFire(0);

	CurrentBurstMode = BurstModeNum;
}

simulated function HandleAutofire()
{
	/// burst mode인 경우 auto!

	if (CurrentBurstMode == 0)
		super.HandleAutofire();
}

simulated function int GetMaxBurstCnt()
{
	return BurstMax;
}

simulated state WeaponBursting extends WeaponFiring
{
	simulated function float GetFireInterval( byte FireModeNum )
	{
		return BurstInterval;
	}

	simulated function BeginState(name PrevStateName)
	{
//		`log( "WeaponBursting" );

		BurstCnt = 0;
		Super.BeginState(PrevStateName);
	}

	simulated function EndState(name NextStateName)
	{
		Super.EndState(NextStateName);
		Cleartimer('RefireCheckTimer');
	}
	
	simulated function RefireCheckTimer()
	{
		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			PutDownWeapon();
			return;
		}		
		
		// Check to see if we have shot our burst
		
		if ( BurstCnt == GetMaxBurstCnt() )
		{
			GotoState('WeaponBurstCoolDown');
			return;
		}

		BurstCnt++;
		
		// If weapon should keep on firing, then do not leave state and fire again.
		if( ShouldRefire() )
		{
			FireAmmunition();
			return;
		}

		GotoState('Active');

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			WeaponEmpty();
		}
	}
}

simulated State WeaponBurstCoolDown
{
	simulated function BeginState(name PrevStateName)
	{
//		`log( "WeaponBurstCoolDown" );

		ServerStopFire( 0 );

		SetTimer(BurstCoolDowntime,false,'WeaponReady');
	}
	
	simulated function WeaponReady()
	{
		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			PutDownWeapon();
			return;
		}

		if (ReloadCnt<=0)
		{
			GotoState('WeaponReloading');
			ClientReload();
		}
		else
			GotoState('Active');
	}
}

simulated function SendToFiringState(byte FireModeNum)
{
	if (CurrentBurstMode==1)
	{
		GotoState('WeaponBursting');
	}
	else
	{
		GotoState('WeaponFiring');
	}
}



defaultproperties
{
	InventoryGroup=1
	BurstMax=3
	BurstCoolDownTime=0.6
	BurstInterval=0.09

	bAutoFire=True

	InstantHitDamageTypes(0)=class'avaDmgType_Gun'

	// Weapon SkeletalMesh
	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=0.25,OtherShotRate=0.25)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.25,OtherShotRate=0.25)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.25,OtherShotRate=0.25)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.25,OtherShotRate=0.25)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

 	WeaponPutDownAnim	=Down
	WeaponEquipAnim		=BringUp
	WeaponReloadAnim	=Reload
	EquipTime			=1.3
	PutDownTime			=0.0333
	ReloadTime			=2.533
	WeaponIdleAnims(0)	=Idle

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Rifle_MuzzleFlash_1P'
	MuzzleFlashDuration=0.05	

	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.01
		Intensity	=	41.40
		Size		=	1.45
		Speed		=	6315.85
	End Object
	TrailInterval	=	2

	// Weapon 에 의한 Pawn 의 속도 제한
	BaseSpeed			= 260	// 기본속도
	AimSpeedPct			= 0.7	// 조준시 보정치
	WalkSpeedPct		= 0.4	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	//HudMaterial		= Texture2D'avaUI.Texture.DotSight'
	//HudMaterialSide = Texture2D'avaUI.Texture.dotsightSide'

	BulletTemplete		= ParticleSystem'avaEffect.Gun_Effect.Ps_Wp_Rifle_cartridge'

	WeaponType			= WEAPON_RIFLE

	InSightMuzzleFlashPSCTemplate =	ParticleSystem'avaEffect.Particles.P_WP_DotSight_MuzzleFlash_1P'

	Begin Object class=AnimNodeSequence Name=SequenceA
	End Object

	Begin Object Class=SkeletalMeshComponent Name=ScopeComponent0
		bOnlyOwnerSee=true
		DepthPriorityGroup=SDPG_Foreground
		Rotation=(Yaw=-16384)
		Translation=(X=-8.0,Y=4.0,Z=-1.0)
		Animations=SequenceA
		PhysicsAsset=None
		AnimSets(0)=AnimSet'Wp_Scope.Com_Scope.dot_ani'
		bCastDynamicShadow=false
	End Object
	ScopeComp=ScopeComponent0
	Components.Add(ScopeComponent0)

	bAvailableInstantZoomFire = true
}