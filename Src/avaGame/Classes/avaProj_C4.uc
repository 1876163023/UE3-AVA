//=============================================================================
//  avaProj_C4
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/02/15
//		1. ��ġ�� bomb volume �� ���� Sequence Event ó��	by OZ
//=============================================================================
class avaProj_C4 extends avaKProjectile
	native	// 20061207 dEAthcURe|HM
	placeable
	config(Game);


/// ������ �ð�.
var float							ExplodeTime;
var float							DefuseTime;
var repnotify hmserialize float		RepRemainForExplode;		// [!] 20070323 dEAthcURe|HM 'hmserialize'
var float							RemainForExplode;
var	repnotify hmserialize float		RepRemainForDefuse;			// [!] 20070323 dEAthcURe|HM 'hmserialize'
var float							RemainForDefuse;

var float							DebugExplode;
var PlayerReplicationInfo			SettedPRI;			/// ���� ��ġ�Ͽ���.
var avaPlayerReplicationInfo		DefusingPRI;		/// ���� ��ġ ���ΰ�.
var float							LastAlertChangeTime;	// On/Off �� Change �� �ð�
var bool							LastAlert;			// ���� Bomb �� On/Off State
var PlayerController				LocalPC;			// Local Player Controller (HUD �� �����ϱ� ���ؼ�...)
var SoundCue						BombAlertSound;
var	TriggerVolume					BombVolume;
var hmserialize repnotify bool		bDefused;			// [!] 20070323 dEAthcURe|HM 'hmserialize'

var	class<avaGunMuzzleFlashLight>	AlertLightClass;
var avaGunMuzzleFlashLight			AlertLight;
var MaterialInstanceConstant		LampMIC;
var MaterialInstanceConstant		LCDMaterialInstanceConstant;

var ParticleSystemComponent			LampPSC;
var ParticleSystem					LampPSCTemplate;
var int								LampCnt;

var int								nPrvDefuseStep;
var LinearColor						Param1, Param2;

var hmserialize int					TeamIndex;	// [!] 20070323 dEAthcURe|HM 'hmserialize'


var avaPlayerController.ViewShakeInfo ExplosionShake;
var float	ExplosionRadius;

replication
{
	if ( bNetDirty && ROLE == ROLE_Authority )
		RepRemainForExplode, RepRemainForDefuse, BombVolume;

	if ( bNetDirty && ROLE == ROLE_Authority )
		SettedPRI, DefusingPRI, bDefused, TeamIndex;
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	RemainForExplode	= ExplodeTime;
	RemainForDefuse		= 0;

	if ( ROLE == ROLE_Authority )
	{
		SettedPRI			= Instigator.PlayerReplicationInfo;
		RepRemainForExplode	= ExplodeTime;
		RepRemainForDefuse	= 0;
		TeamIndex			= Instigator.GetTeamNum();
	}

	AlertLight = new(self) AlertLightClass;
	AttachComponent( AlertLight );
	AlertLight.SetEnabled(FALSE);

	ForEach LocalPlayerControllers(LocalPC){ break; }
	if ( avaHUD(LocalPC.MyHUD) != None )	avaHUD(LocalPC.MyHUD).InstalledBomb( true, self );

	LampMIC							=	StaticMeshComponent.CreateMaterialInstance( 1 );
	LCDMaterialInstanceConstant		=	StaticMeshComponent.CreateMaterialInstance( 2 );

	LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', MakeLinearColor( 0.5, 0.375, 0, 1 ) );
	LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', MakeLinearColor( 0.5, 0.375, 0, 1 ) );

	LampPSC = new(self) class'ParticleSystemComponent';
	LampPSC.SetTemplate( LampPSCTemplate );
	LampPSC.DeactivateSystem();
	AttachComponent( LampPSC );
}

function SetBombVolume( TriggerVolume volume )
{
	BombVolume = volume;
	avaBombVolume( BombVolume ).TriggerByBomb( 'Installed', Controller( SettedPRI.Owner ) );
}

//simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal)
//{
//	if ( Role == ROLE_Authority )
//	{
//		// C4 ���� Effect Play
//		Spawn(class'avaProj_C4ExpFx', ,, HitLocation, Rotation );
//		bSuppressExplosionFX = true;
//	}
//}

simulated function Destroyed()
{
	local avaPlayerController pc;

	if ( DefusingPRI != none )
	{
		pc = avaPlayerController(DefusingPRI.Owner);
		pc.StopDefuseBomb();
	}
	bSuppressExplosionFX = true;
	if ( avaHUD(LocalPC.MyHUD) != None )	avaHUD(LocalPC.MyHUD).InstalledBomb( false, self );

	DetachComponent( AlertLight );

	super.Destroyed();
}

simulated function Reset()
{
	bSuppressExplosionFX = true;
	Destroy();
}

// ��ġ�� �� �ݴ��� ��ü�� �� �� �ִ�.
simulated function bool CanDefuse( Pawn p )
{
	//`log("[dEAthcURe|avaProj_C4::CanDefuse] bDefused=" @ bDefused @ "TeamIndex=" @ TeamIndex @ "p.GetTeamNum()=" @ p.GetTeamNum()); // 20070309
	if ( bDefused == true )	return false;
	if ( p.GetTeamNum() != TeamIndex )
		return true;
	return false;
}

/// ����.
/// ���� ����, ���� ���ֱ� ��.
simulated function ExplodeBomb()
{
	Explode( Location, Normal( vect(0,0,1) ));
}

simulated function bool BeginDefuse( avaPlayerReplicationInfo pri )
{
	local avaPlayerController PC;

//	`log( "begin defusing" );

	if ( DefusingPRI != none || !IsInState( 'BombSetted' ) )
		return false;

	assert( pri != none );

	DefusingPRI = pri;

	pri.SetGauge( 0, DefuseTime*10 );
	GotoState( 'BombDefusing' );

	if ( Role == Role_Authority )
	{
		PC = avaPlayerController( DefusingPRI.Owner );
		PC.Server_IgnoreMoveInput( true );
		PC.Server_IgnoreFireInput( true );
		PC.Pawn.StopFiring();
	}
	return true;
}

simulated function EndDefuse( avaPlayerReplicationInfo pri )
{
	local avaPlayerController PC;
	pri.SetGauge( 0, 0 );

	if ( pri == DefusingPRI )
	{
		if ( Role == Role_Authority )
		{
			PC = avaPlayerController( DefusingPRI.Owner );
			PC.Server_IgnoreMoveInput( false );
			PC.Server_IgnoreFireInput( false );
		}
		DefusingPRI = none;
		// ��ü�� �Ϸ� �Ǿ����� TickBomb ���� Destroy ��.
		if ( !IsInState( 'BombExploded' ) )
			GotoState( 'BombSetted' );
	}
}

// true �̸� �����ų� ��ü �� ��. UpdateBombHUD () �� ȣ���� �ʿ䰡 ����.
simulated function bool TickBomb( float deltaTime )
{
	local float PrevRemain;
	local int	nPrv,nCur;
	if ( bDefused == true )	return true;
	PrevRemain		  =	RemainForExplode;
	RemainForExplode -= deltaTime;

	if ( Role == Role_Authority )
	{
		nPrv = PrevRemain;
		nCur = RemainForExplode;
		if ( nPrv != nCur )
		{
			RepRemainForExplode = RemainForExplode;
			NetUpdateTime	   = WorldInfo.TimeSeconds - 1;
		}
	}

	if ( RemainForExplode <= 0 )
	{
		if ( !IsInState( 'BombExploded' ) )
		{
			GotoState( 'BombExploded' );
		}
		return true;
	}
	else
		return false;
}

simulated function UpdateBombHUD()
{
	local float remain;
	local float BlinkTime;

	//local avaGameReplicationInfo GRI;
	//GRI = avaGameReplicationInfo(WorldInfo.GRI);

	// update bomb mark flashing.
	if ( LocalPC == None )	return;	// Local PC �� ���°� Dedicate ����?
	if ( bDefused == true )	return;

	remain = Max( RemainForExplode, 0 ) * 100.0 / ExplodeTime;
	if ( remain < 0 )		return;
	else if ( remain < 10 )	BlinkTime = 0.1;
	else if ( remain < 25 ) BlinkTime = 0.2;
	else if ( remain < 50 ) BlinkTime = 0.5;
	else if ( remain < 75 ) BlinkTime = 1.0;
	else					BlinkTime = 1.5;
	
	// {{ 20061219 dEAthcURe|HM time����ġ	�ؼ�
	if(	WorldInfo.TimeSeconds - LastAlertChangeTime < 0) 
	{
		LastAlertChangeTime = WorldInfo.TimeSeconds;
	}
	// }} 20061219 dEAthcURe|HM time����ġ	�ؼ�

	if ( LastAlertChangeTime == 0 )	LastAlertChangeTime = WorldInfo.TimeSeconds;
	if ( WorldInfo.TimeSeconds - LastAlertChangeTime > BlinkTime )
	{
		// Play Sound
		if ( LastAlert == true )	
		{
			PlaySound( BombAlertSound, true,,,Location );		
			AlertLight.ResetLight();
			LampMIC.SetScalarParameterValue( 'Lamp', 1.0 );
			LampPSC.ActivateSystem();

			++LampCnt;
			if ( LampCnt >= 4 )	LampCnt = 0;

			if ( DefusingPRI == None )
			{
				LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', MakeLinearColor( 0.5 + LampCnt * 14.0/256.0, 0.375 , 0, 1 ) );
				LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', MakeLinearColor( 0.5 + LampCnt * 14.0/256.0, 0.375 , 0, 1 ) );
			}
		}
		else
		{
			LampMIC.SetScalarParameterValue( 'Lamp', 0.0 );
		}
		LastAlert = !LastAlert;
		LastAlertChangeTime = WorldInfo.TimeSeconds;
		if ( avaHUD(LocalPC.MyHUD) != None )	avaHUD(LocalPC.MyHUD).UpdateBombAlertBlink( LastAlert );

		
	}
}

// {{ 20061219 dEAthcURe|HM
event function HmUpdateBombHUD()
{
	UpdateBombHUD();
}
// }} 20061219 dEAthcURe|HM

// {{ 20070326 dEAthcURe|HM
event function OnHmRestore()
{
	`log("[dEAthcURe|avaProj_C4::OnHmRestore] goto state 'BombSetted'");
	
	RemainForExplode = RepRemainForExplode; // [+] 20070907
	RemainForDefuse = RepRemainForDefuse; // [+] 20070907
	
	GotoState( 'BombSetted' );	
}
// }} 20070326 dEAthcURe|HM

auto simulated state BombSetted
{
	simulated function BeginState( name prevState )
	{
		UpdateBombHUD();
	}

	simulated function Tick( float deltaTime )
	{
		super.Tick( deltaTime );

		//`log( "tick " $ GetStateName() );
		if ( !TickBomb( deltaTime) )
			UpdateBombHUD();
	}

	//simulated event EndState( Name NextStateName )
	//{
	//	`log( "end bomb SETTED state "$ NextStateName );
	//}
}

simulated state BombDefusing
{
	simulated function BeginState( name prevState )
	{
		RemainForDefuse		= DefuseTime;
		RepRemainForDefuse	= DefuseTime;
		UpdateBombHUD();
	}

	simulated function Tick( float deltaTime )
	{
		super.Tick( deltaTime );

		//`log( "tick " $ GetStateName() );
		if ( !TickBomb( deltaTime ))
			UpdateBombHUD();
	}

	simulated function bool TickBomb( float deltaTime )
	{
		local avaPlayerController PC;
		local vector HitLocation, HitNormal;
		local vector TraceEnd, TraceStart;
		local Actor	 HitActor;
		local int	 nPrv, nCur;
		local float	 PrvRemain;

		if ( global.TickBomb( deltaTime ))
			return true;

		PrvRemain		 = RemainForDefuse;
		RemainForDefuse -= deltaTime;

		if ( Role == Role_Authority )
		{
			nPrv = PrvRemain;
			nCur = RemainForDefuse;
			if ( nPrv != nCur )
			{
				RepRemainForDefuse = RemainForDefuse;
				NetUpdateTime	   = WorldInfo.TimeSeconds - 1;
			}
		}

		if ( RemainForDefuse <= 0 )
		{
			GotoState( 'BombDefused' );
			return true;
		}
		else
		{
			PC = avaPlayerController(DefusingPRI.Owner);
			if ( PC.Pawn != None )
			{
				// Trace �� �ؼ� Defuse Bomb �� ���̴��� Check �Ѵ�...
				TraceStart	=	PC.Pawn.GetPawnViewLocation();
				TraceEnd	=	TraceStart + vector( PC.Pawn.GetViewRotation() ) * 140;
				HitActor	=	PC.Pawn.Trace( HitLocation, HitNormal, TraceEnd, TraceStart, true, vect(0,0,0), ,TRACEFLAG_Bullet );
				if ( avaProj_C4( HitActor ) == Self )
					return false;
			}
			PC.StopDefuseBomb();
			return false;
		}
	}

	simulated function UpdateBombHUD()
	{
		global.UpdateBombHUD();

		if ( DefusingPRI != none )
		{
			DefusingPRI.SetGauge( int( (DefuseTime - RemainForDefuse)*10), int(DefuseTime*10) );
		}
	}
}

simulated state BombExploded
{
	ignores TickBomb;
	simulated function BeginState( name prevState )
	{
		ExplodeBomb();
		if ( Role == ROLE_Authority )
		{
			// ���� ����.

			avaBombVolume( BombVolume ).TriggerByBomb( 'Exploded', Controller(SettedPRI.Owner) );

			if ( DefusingPRI != none )
			{
				DefusingPRI.SetGauge( 0, 0 );
			}
		}

		//LifeSpan = 1.0;
		//Destroy();
	}
}

simulated state BombDefused
{
	simulated function BeginState( name prevState )
	{
		local avaPlayerController	 PC;
		bSuppressExplosionFX	= true;
		if ( Role == ROLE_Authority )
		{
			// ���� ����.
			avaBombVolume( BombVolume ).TriggerByBomb( 'Defused', Controller(DefusingPRI.Owner) );

			if ( DefusingPRI != none )
			{
				DefusingPRI.SetGauge( 0, 0 );
			}

			PC = avaPlayerController( DefusingPRI.Owner );
			PC.Server_IgnoreMoveInput( false );
			PC.Server_IgnoreFireInput( false );
			bDefused				= true;
			DefuseComplete();
			
			// ���߿��� Alpha �� �����ؼ� ������ ���������....
			if ( avaGameReplicationInfo(WorldInfo.GRI).bReinforcement == true )
				Destroy();
		}
	}
}

simulated function DefuseComplete()
{
	LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', MakeLinearColor( 0.0, 0.375, 0, 1 ) );
	LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', MakeLinearColor( 0.0, 0.375, 0, 1 ) );
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bDefused' )
	{
		if ( bDefused == true )
			DefuseComplete();
	}
	else if ( VarName == 'RepRemainForExplode' )
	{
		RemainForExplode = RepRemainForExplode;
	}
	else if ( VarName == 'RepRemianForDefuse' )
	{
		RemainForDefuse	 = RepRemainForDefuse;
	}
	Super.ReplicatedEvent( VarName );
}

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
}

simulated function Tick( float deltaTime )
{
	local int nDefuseStep;
	Super.Tick( deltaTime );
	if ( bDefused == true )	return;
	if ( DefusingPRI == None )
	{
		if ( nPrvDefuseStep != -1 )
		{
			nPrvDefuseStep = -1;
			LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', MakeLinearColor( 0.5 + LampCnt * 14.0/256.0, 0.375 , 0, 1 ) );
			LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', MakeLinearColor( 0.5 + LampCnt * 14.0/256.0, 0.375 , 0, 1 ) );
		}
		return;
	}
	nDefuseStep = ( DefuseTime - RemainForDefuse ) * 7 / DefuseTime;
	if ( nPrvDefuseStep != nDefuseStep )
	{
		nPrvDefuseStep = nDefuseStep;
		Param1			=	MakeLinearColor( (nDefuseStep%2) * 0.5, 0.5 + 0.125 * ( nDefuseStep / 2 ), 0, 1 );
		LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', Param1 );
		LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', Param1 );
	}
}

simulated event Landed( vector HitNormal, actor FloorActor )
{
	//SetCollision( true, false );	// shut down collision
	//SetPhysics( PHYS_None );
}

simulated singular function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{

}

event TakeDamage(int nDamage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> DamageCauser)
{

}

simulated function Bool HurtRadiusEx
(
	float				BaseDamage,
	float				InDamageRadius,
	class<DamageType>	DamageType,
	float				Momentum,
	vector				HurtOrigin,
	optional Actor		IgnoredActor,
	optional Controller InstigatedByController = Instigator != None ? Instigator.Controller : None,
	optional bool       bDoFullDamage
)
{
	local Actor	Victim;
	local bool bCausedDamage;

	// Prevent HurtRadius() from being reentrant.
	if ( bHurtEntry )
		return false;

	bHurtEntry = true;
	bCausedDamage = false;
	foreach CollidingActors( class'Actor', Victim, InDamageRadius, HurtOrigin )
	{
		if ( (Victim != self) && (Victim != IgnoredActor) )
		{
			Victim.TakeRadiusDamageEx( InstigatedByController, BaseDamage, InDamageRadius, FullDamageMinRadius, DamageType, Momentum, HurtOrigin, bDoFullDamage, WeaponBy );
			bCausedDamage = bCausedDamage || Victim.bProjTarget;
		}
	}
	bHurtEntry = false;
	return bCausedDamage;
}

simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal)
{
	local avaPlayerController PC;	
	local vector L;
	local rotator R;
	local avaPlayerController.ViewShakeInfo NewExplosionShake;

	super.SpawnExplosionEffects( HitLocation, HitNormal );

	foreach WorldInfo.AllControllers(class'avaPlayerController', PC)
	{
		PC.GetPlayerViewPoint( L, R );			

		if (VSize(L-HitLocation) < ExplosionRadius)
		{				
			NewExplosionShake = ExplosionShake;

			PC.ShakeView( NewExplosionShake );			
			//PC.ApplyShockMuffleDSP();
			//by killy
		}		
	}
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	`log( "avaProj_C4.FellOutOfWorld!!!!" );
	Super.FellOutOfWorld( dmgType );
}

// {{ 20061207 dEAthcURe|HM
cpptext 
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061207 dEAthcURe|HM
	#endif
}
// }} 20061207 dEAthcURe|HM

defaultproperties
{

	//ProjFlightTemplate=ParticleSystem'WP_RocketLauncher.Particles.P_WP_RocketLauncher_RocketTrail'
	//ProjExplosionTemplate=None
	//ExplosionDecal=None
	
	Damage=800.0
	DamageRadius=800.0

	MomentumTransfer=500000
	MyDamageType=class'avaDmgType_C4'
	LifeSpan=0.0
	//AmbientSound=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Travel_Cue'
	//ExplosionSound=None
	BombAlertSound=SoundCue'avaWeaponSounds.Mission_C4.Mission_C4_alert'
	bCollideWorld=true
	bStatic=false
	bNetTemporary=false
	//bTicked=true
	bAlwaysRelevant=true
	bCollideActors	=	true
	bBlockActors	=	true

	//bWaitForEffects=true

	//Begin Object Class=CylinderComponent NAME=CollisionCylinder
	//	CollisionRadius=+00012.000000
	//	CollisionHeight=+00005.000000
	//	CollideActors=true
	//	BlockZeroExtent=false
	//End Object
	//cylinderComponent=CollisionCylinder
	//Components.Add(CollisionCylinder)
	DrawScale=1.5

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Wp_New_C4.MS_C4_3p'
		Rotation=(Yaw=16384)
		bAcceptsDecals=false
		CollideActors=true
		BlockActors=false
		BlockZeroExtent=true
	End Object	

	CollisionComponent=StaticMeshComponent0

	ExplodeTime			=	40
	DefuseTime			=	7.5

	AlertLightClass		=	class'avaGame.avaBlinkLightComponent'
	LampPSCTemplate		=	ParticleSystem'avaEffect2.C4.PS_C4_Red_Light'
	nPrvDefuseStep		=	-1

	ExplosionSound			=	SoundCue'avaWeaponSounds.Mission_C4.Mission_C4_explosion'
	ProjExplosionTemplate	=	ParticleSystem'avaEffect.Explosion.PS_C4_Explosion'
	ExplosionDecal			=	Material'avaDecal.Crater.M_C4_Crater'
	DecalWidth				=	800
	DecalHeight				=	800
	//RemoteRole				=	ROLE_SimulatedProxy
	ExplosionShake			=	(OffsetMag=(X=-15.0,Y=-15.00,Z=-15.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=10,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	
	ExplosionRadius			=	2560
}
 
