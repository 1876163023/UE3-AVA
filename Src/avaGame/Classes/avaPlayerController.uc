/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// avaPlayerController.
//=============================================================================
class avaPlayerController extends GamePlayerController
	dependson(avaUICharacter)
	dependson(avaPawn)
	dependson(avaColorCorrectionManager)
	config(Game)
	native;

`include(avaGame/avaGame.uci)

var globalconfig	bool	bAutoTaunt;
var localized string		TeamSayPrefix, SayPrefix;
var Pawn					ShadowPawn;	/** Pawn currently controlled in behindview, for which shadow is turned on.  Turn off shadow if player is no longer controlling this pawn or in behindview */
var bool					bEnableVote;			// 

var globalconfig enum EPawnShadowMode
{
	SHADOW_None,
	SHADOW_Self,
	SHADOW_LOD,
	SHADOW_All
} PawnShadowMode;

enum ESpecatatorCamMode
{
	SPECTATORCAMMODE_NORMAL,	//
	SPECTATORCAMMODE_FREE,		//
};

var		bool			bBehindView;			// true �̸� 3��Ī View �Դϴ�.
var		bool			bLockRotation;			// Local Control �� ��� Pawn �� ȸ���� ������Ű��, Camera �� Pawn �� ������ ȸ���մϴ�. bBehindView �� true �� ��쿡�� ���˴ϴ�.
var		bool			bForceBehindView;		// if true, while in the spectating state, behindview will be forced on a player
var		bool			bDefusingBomb;			// bomb ��ü�ϰ� �ִ�.
var		avaProj_C4		DefusingBomb;			//		

/** if true, rotate smoothly to desiredrotation */
var bool				bUsePhysicsRotation;

var globalconfig	avaPawn.EWeaponHand	WeaponHandPreference;
var					avaPawn.EWeaponHand	WeaponHand;

var localized string	MsgPlayerNotFound;

var byte				bDash;

var float				fBlurAmount;
var float				fBlurTime;
var float				fBlurEndTime;

var DSPPreset			 DSP_Muffle;					// ���� �� �ߵ���ų DSPPreset, DSPSlot=Player

var	float				SpectatorSpeedRate;

var	bool				bActivateDeadScene;				// Dead Scene �� ���� Active �������� �� �� �ִ�.
var bool				bEnableCloseDeadScene;			// true �̸� Click �� ���ؼ� Dead Scene �� ���� �� �ִ�.

var float				CurrentBattery;					// ���� ���͸� ��
var float				MaximumBattery;					// �ִ� ���͸� ��
var float				BatteryChargeSpeed;				// ���͸��� ������� ������ �����Ǵ� �ӵ�...
var	float				NVGMinimumPower;				// NVG �� �ѱ� ���� Minimum Power
var float				NVGConsumptionSpeed;			// NVG �� �Ѱ� ������ ���͸� �Ҹ�
var	bool				bInfinityBattery;				// Test �� Flag �̴�... true �̸� ���͸��� ���� �ʴ´�..

/** view shake information */
struct native ViewShakeInfo
{
	/** how far to offset view */
	var() vector OffsetMag;
	/** how fast to offset view */
	var() vector OffsetRate;
	/** total duration of view offset shaking */
	var() float OffsetTime;
	/** @note: the rotation data is in vectors instead of rotators so the same function can be used to manipulate them as the offsets above
		X = Pitch, Y = Yaw, Z = Roll
	*/
	/** how far to rotate view */
	var() vector RotMag;
	/** how fast to rotate view �ʴ� rotate �� */
	var() vector RotRate;
	/** total duration of view rotation shaking */
	var() float RotTime;
};

/** view shaking that is currently being applied */
var ViewShakeInfo CurrentViewShake;
/** base view shake applied when hit (adjusted depending on damage received) */
var ViewShakeInfo BaseDamageShake;
/** current offsets applied to the camera due to CurrentViewShake */
var vector ShakeOffset; // current magnitude to offset camera position from shake
var rotator ShakeRot; // current magnitude to offset camera rotation from shake
var globalconfig	bool	bLandingShake;

var float LastCameraTimeStamp; /** Used during matinee sequences */
var class<Camera> MatineeCameraClass;

var config bool bCenteredWeaponFire;

/** cached result of GetPlayerViewPoint() */
var Actor				CalcViewActor;
var vector				CalcViewActorLocation;
var rotator				CalcViewActorRotation;
var vector				CalcViewLocation;
var rotator				CalcViewRotation;
var float				CalcEyeHeight;
var vector				CalcWalkBob;

var bool				bRequestSwapWeapon;			// �������� Weapon Swap �� ��û���̴�. Weapon �� ���õ� Action ���� ���ƾ� �Ѵ�....

// Sound ���� Properties
var const SoundCue		ChatNotifySound;		//	ä�� �� Notify Sound...
var SoundCue			DamageBreath;			//	Health �� 25 ���Ϸ� ������ ��쿡 Play �Ǵ� SoundCue �̴�...
var AudioComponent		GrenadeEffectSound;		//	����ź�� ������ ��ô�Ǿ��� �� Play ���ش�.
var AudioComponent		HeartbeatSound;			//	DeadScene ���� Play �Ǵ� Sound
var AudioComponent		UseSound;		
var avaDSPPreset		ShockMuffleDSP;

// Input ���� Properties
var bool				bIgnoreFireInput;
var float				MouseSensitivityEx;
var bool				bAlwaysCrouch;			// True �̸� �׻� �ɰ� �ȴ�. (�ڰ����� �򶧵� �̿��ϱ� ���ؼ� �������)
var bool				bIgnoreCrouch;			// True �� ��� Duck & UnDuck Key �� ���õȴ�.
var int					LastAutoMessage;
var int					LastAutoMessageTime;

// ColorCorrectionManager.(��� ColorCorrection ������ Manager�� Tick���� �Ͼ)

var Actor				KillCamTargetActor;
var float				KillCamStartedTime;
var bool				KillCam_bIsKiller;

var Actor				ChatCamActor;
var float				ChatCamStartedTime;


var bool				bForceShoulderCam;
var	bool				bUseOldThirdPersonCam;
var	bool				bUseMeshInterpolating;

// Input ���� ���� �ð�....
var float				NoInputElapsedTime;
var float				NOINPUTKICKTIME;

struct native FadeParameters
{
	var float HoldTime, FadeTime, Alpha;
	var float StartTime;
	var float CurrentAlpha;
};

var array<FadeParameters> ActiveFades;

// Interpolation ViewTarget
var float				LastInterpTime;
var Vector				SavedLoc;
var Rotator				SavedRot;
var Pawn				PrevViewTarget;
var avaGameInfoMessage	gameInfoMsg;
var Actor				LastBase;

struct native KickablePlayer
{
	var avaPlayerReplicationInfo	PRI;
	var string						PlayerName;				// Vote �߿� ���� ����� ���ؼ��� Vote�� ������ �Ǿ�� �ϱ� ������...
	var int							AccountID;
};

var array<KickablePlayer>			KickablePlayerList;

struct native BroadcastData
{
	var	string						BroadcastString;
	var float						BroadcastTime;
};

var BroadcastData					BroadcastHistory[5];

var	int								UIMode;					//	1 �̸� UI �� �����ִ� �����̴�... FreeCam ������ �۵��ؾ� �Ѵ�....
var bool							bCameraCollision;
var bool							bDisableCameraEffect;

/** �׽�Ʈ�� Ragdoll �÷��� */
var	bool					bDisablePlayTakeHitRagdoll;

// Free cam�� ������ non-free cam position���� ����ǵ���
var vector							PendingLocation;
var rotator							PendingRotation;
var bool							bPendingFreeCam;

var	bool							bDisplayBotInfo;	// For Debug, Bot �� ���� ������ Display ���ش�...

cpptext
{
	virtual void HearSound(USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed);
	virtual UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );
	virtual UBOOL CanApplyCullDistance();
	virtual void  OnPreRender();
}

// PlayerController �� NativeReplication ���� �Ǿ��ִµ�, ���� Class �� �׷��� �ʴ�.
// C++ �� Replication Block �� ����� ��� �ұ�?
// �Ŀ� Replication �� �ȵȴٸ� ����� ����!!!
replication
{
	// Things the server should send to the client.
	if ( bNetOwner && Role==ROLE_Authority )
		MouseSensitivityEx, bAlwaysCrouch, bIgnoreCrouch;
}

simulated native function SetClearBackBufferFlag( bool bFlag  );
native final function string GetMapFilename();

/****************************************************************************
 Epic Code - Not Used 
*****************************************************************************/
event ReceiveWarning(Pawn shooter, float projSpeed, vector FireDir){}
function ReceiveProjectileWarning(Projectile proj){}
function PlayWinMessage(bool bWinner){}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( gameInfoMsg == None )
		gameInfoMsg = Spawn( class'avaGameInfoMessage', Self,, );
}

simulated event ReplicatedEvent(name VarName)
{
	Super.ReplicatedEvent(VarName);
	if (VarName == 'PlayerReplicationInfo')
		ClientUpdatePRI();
}

simulated function ClientUpdatePRI()
{
	if ( PlayerReplicationInfo.Team != None )
		SetTeam( PlayerReplicationInfo.Team.TeamIndex );

	if ( !IsInState( 'Spectating' ) && 
		 ( PlayerReplicationInfo.bOnlySpectator == true  || ( PlayerReplicationInfo.bOutOfLives == true && IsInState( 'PlayerWaiting' ) ) ) )
	{
		GotoState( 'Spectating' );
	}
	else
	{
		ServerRestartPlayer();
	}

	if ( PlayerReplicationInfo.bOnlySpectator == true )
	{
		avaHUD( myHUD ).UpdateSpectatorInfo( PlayerReplicationInfo.bOnlySpectator );
		avaGameReplicationInfo( WorldInfo.GRI ).LoadLocalizedTeamPack( 255, self );
	}
}

`devexec function SetAlwaysCrouch( optional bool bFlag )
{
	bAlwaysCrouch = bFlag;
	if ( bFlag == false )	ClientUnDuck();
}

`devexec function IgnoreCrouch ( optional bool bFlag )
{
	bIgnoreCrouch = bFlag;
}

/** LandingShake()
returns true if controller wants landing view shake
*/
simulated function bool LandingShake()
{
	return bLandingShake;
}

// 'E' Key �� ����� ����Դϴ�...
simulated function bool TriggerInteracted()
{
	if ( Pawn != None && avaPawn( Pawn ).TouchedUseVolume != None )
	{
		if ( avaPawn( Pawn ).TouchedUseVolume.ClientIsUseable( Pawn ) )
		{
			avaPawn( Pawn ).TouchedUseVolume.UsedBy( Pawn );
			return true;
		}
	}
	return super.TriggerInteracted();
}

// 'E' Key �� �� ����Դϴ�...
simulated function PerformedUnUseAction()
{
	if ( bDefusingBomb )
	{
		StopDefuseBomb();
		return;
	}
	if ( Role == ROLE_Authority )
	{
		if ( avaPawn( Pawn ).TouchedUseVolume != None  )
			avaPawn( Pawn ).TouchedUseVolume.UnUse( Pawn );
	}
}

simulated event Destroyed()
{
	Super.Destroyed();
	if ( bDefusingBomb )
	{
		StopDefuseBomb();
	}
}

event KickWarning()
{
}

/*
*	������ ���ؼ� ���������� Dash �� Cancel �Ѵ�.
*	1. Pawn �� �׾��� ���
*	2. Server ���� Dash �� ��û�ߴµ� �޾Ƶ鿩���� ���� ���
*/
client reliable function ClientCancelDash()
{
	CancelDash();
}

/* CheckJumpOrDuck()
Called by ProcessMove()
handle jump and duck buttons which are pressed

 Client �� Server Side ���� ���ÿ� ���´�....
*/
function CheckJumpOrDuck()
{
	if ( bPressedJump )
	{
		Pawn.DoJump( bUpdating );
	}
	if ( Pawn.Physics != PHYS_Falling && Pawn.bCanCrouch )
	{
		// crouch if pressing duck
		Pawn.ShouldCrouch(bDuck != 0);
	}
}

simulated `devexec function DoDash()
{
	if ( avaPawn(Pawn) != none )
	{
		avaPawn( Pawn ).DoDash( true );
	}
}

simulated `devexec function CancelDash()
{
	if ( avaPawn(Pawn) != none )
	{
		avaPawn( Pawn ).DoDash( false );
	}
}

event function Server_IgnoreMoveInput( bool bIgnore ) // [!] 20070523 dEAthcURe|HM
{
	Client_IgnoreMoveInput( bIgnore );
	if ( !bIgnore || !IsMoveInputIgnored() )
		IgnoreMoveInput( bIgnore );
}

reliable client function Client_IgnoreMoveInput( bool bIgnore )
{
	if ( !bIgnore || !IsMoveInputIgnored() )
		IgnoreMoveInput( bIgnore );
}

function Server_IgnoreFireInput( bool bIgnore )
{
	bIgnoreFireInput = bIgnore;
	Client_IgnoreFireInput( bIgnore );
}

reliable client function Client_IgnoreFireInput( bool bIgnore )
{
	bIgnoreFireInput = bIgnore;
}

// IgnoreMoveInput���� �Է� �ܿ����� ���� ������ ����
function IgnoreMoveInputEx( bool bNewMoveInput,optional bool bClearInput)
{
	Super.IgnoreMoveInput(bNewMoveInput);
	if(bClearInput)
	{
		PlayerInput.aForward	= 0;
		PlayerInput.aStrafe		= 0;
		PlayerInput.aUp			= 0;
		bPressedJump			= false;
	}
}

// ignoreLookInput���� �Է� �ܿ����� ���� ������ ����
function IgnoreLookInputEx( bool bNewLookInput, optional bool bClearInput)
{
	Super.IgnoreLookInput(bNewLookInput);
	if( bClearInput )
	{
		PlayerInput.aTurn = 0;
		PlayerInput.aLookUp = 0;
	}
}

state PlayerClimbing
{
	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		Super.ProcessMove( DeltaTime, NewAccel, DoubleClickMove, DeltaRot );
		CancelDash();
	}
}

//! Vehicle�� ź ��쿡�� RemoteViewPitch�� �����ϵ��� �Ѵ�.(2007/08/20 ����)
state PlayerDriving
{
	function PlayerMove( float DeltaTime )
	{
		Super.PlayerMove( DeltaTime );

		if( Pawn == None )
		{
			return;
		}

		if( Role == Role_Authority && WorldInfo.NetMode != NM_StandAlone )
		{
			// Update ViewPitch for remote clients
			Pawn.SetRemoteViewPitch( Rotation.Pitch );

			if( Vehicle(Pawn) != None && Vehicle(Pawn).Driver != None )
				Vehicle(Pawn).Driver.SetRemoteViewPitch( Rotation.Pitch );
		}
	}
}

simulated function SetBlur( float amount, optional float fTime )
{
	if ( fTime > 0.0 )
	{
		fBlurTime		= fTime;
		fBlurEndTime	= WorldInfo.TimeSeconds + fTime;
		fBlurAmount		= amount;
	}
	/*@@@@TODO!!! PostProcess effect�� �ٲ߽ô�.
	PlayerCamera.BlurAlpha = FClamp( amount * 100, 0, 100 );
	if (amount > 0)
		PlayerCamera.bOverrideBlurSettings = true;
	else
		PlayerCamera.bOverrideBlurSettings = false;
	*/
}

simulated function UpdateBlur()
{
	if ( fBlurTime > 0.0 )
	{
		if ( fBlurEndTime - WorldInfo.TimeSeconds > 0.0 )
		{
			SetBlur( fBlurAmount * (fBlurEndTime - WorldInfo.TimeSeconds)/fBlurTime );
		}
		else
		{
			fBlurTime		= 0.0;
			fBlurAmount		= 0;
			fBlurEndTime	= 0.0;
			SetBlur( 0 );
		}
	}
	//if (RealViewTarget != none)
	//{
	//	SetBlur( VSize( avaPawn(PlayerController(RealViewTarget.Owner).Pawn).Velocity ) / 14000 );
	//}
	//else
	//{
	//	if ( avaPawn(Pawn) != None )
	//		SetBlur( VSize( avaPawn(Pawn).Velocity ) / 14000 );
	//}
}

event PreRender(Canvas Canvas)
{
	//Blur ��������� �ʴµ� ȣ��������!
	//UpdateBlur();

	if ( avaPawn( Pawn ) != None && avaPawn( Pawn ).TouchedUseVolume != None )
	{
		if ( avaPawn( Pawn ).TouchedUseVolume.ClientIsUseable( Pawn ) )
		{
			ShowGIM( avaPawn( Pawn ).TouchedUseVolume.GameInfoMsgIndex, 2 );
		}
		else
		{
			ClearGIM( 2 );
		}
	}
	else
	{
		ClearGIM( 2 );
	}

	UpdateFades();

	if ( Pawn != None && Pawn.Weapon != None && avaWeapon( Pawn.Weapon ) != None )
	{
		avaWeapon( Pawn.Weapon ).SetPosition( avaPawn( Pawn ) );
	}
}

reliable client simulated function Client_ResetLevel()
{
	local avaKActor		kactor;
	if ( Role == ROLE_Authority )	return;		// Host �� ���⼭ �� �ʿ� ����...

	//// Reset all actors (except controllers, the GameInfo, and any other actors specified by ShouldReset())
	foreach AllActors(class'avaKActor', kactor)
	{
		kactor.ClientReset();
	}
}

// �Ѿ��� �ȳ����� Ȥ�� bIgnoreFireInput �̳� bRequestSwapWeapon �� false �� �� �ֱ� ������ �ʱ�ȭ�� �����ֵ��� ����....
function ServerGivePawn()
{
	`log( "avaPlayerController.ServerGivePawn" );
	Super.ServerGivePawn();
	bIgnoreFireInput = false;
	bRequestSwapWeapon = false;
}

reliable client function GivePawn(Pawn NewPawn)
{
	Super.GivePawn( NewPawn );
	bIgnoreFireInput	=	false;
	bRequestSwapWeapon	=	false;
	CurrentBattery		=	MaximumBattery;
	
}

reliable client function ClientRestart(Pawn NewPawn)
{
	local avaPickUp		PickUp;

	`log( "avaPlayerController.ClientRestart" @NewPawn );
	if ( avaPawn( NewPawn ) != None )	
	{
		ForceShoulderCam( false );
		SetMouseSensitivityEx( 1.0 );
	}
	else if ( Vehicle( NewPawn ) != None )
	{
		// ��� Vehicle�� ���� ������� �ʵ��� ����.
		if ( avaVehicleBase( NewPawn ) != None )
		{
			ForceShoulderCam( avaVehicleBase( NewPawn ).bForceShoulderCam );
			SetMouseSensitivityEx( avaVehicleBase( NewPawn ).MouseSensitivity );
		}
		else
		{
			ForceShoulderCam( true );
			SetMouseSensitivityEx( 0.3 );
		}
	}

	Super.ClientRestart(NewPawn);
	ServerPlayerPreferences(WeaponHandPreference, bAutoTaunt, bCenteredWeaponFire);
	ForEach DynamicActors(class'avaPickUp', PickUp)
	{
		PickUp.CheckIndicatorTeam( self );
	}
}

function Possess(Pawn aPawn, bool bVehicleTransition)
{
	local avaGame					GI;
	local avaPlayerModifierInfo		avaPMI;
	local int						i;
	local int						TypeID;

	`log("avaPlayerController.Possess - aPawn:"@aPawn.name @"Self:" @Self.name @bVehicleTransition );

	if ( avaPawn( aPawn ) != None && bVehicleTransition == false  )
	{
		avaPMI = avaPlayerReplicationInfo( PlayerReplicationInfo ).avaPMI;
		 //Server ���� Character Modifier �� �������ش�....
		TypeID = avaPawn( aPawn ).GetTypeID();
		if ( avaPMI != None && avaPMI.ClassTypeInfos[TypeID].CharMod.length > 0 )
		{
			for ( i = 0 ; i < avaPMI.ClassTypeInfos[TypeID].CharMod.length ; ++ i )
				avaPMI.ClassTypeInfos[TypeID].CharMod[i].static.ApplyToCharacter_Server( avaPawn( aPawn ) );
		}
		else
		{
			for ( i = 0 ; i < avaCharacter( aPawn ).DefaultModifier.length ; ++ i )
				avaCharacter( aPawn ).DefaultModifier[i].static.ApplyToCharacter_Server( avaPawn( aPawn ) );
		}

		avaPlayerReplicationInfo( PlayerReplicationInfo ).SpawnStart( TypeID );
		aPawn.Health					= avaPawn(aPawn).HealthMax;
		avaPawn(aPawn).Armor_Stomach	= avaPawn(aPawn).ArmorMax;
		GI = avaGame(WorldInfo.Game);
		if (GI != none && GI.GameStats != none && !PlayerReplicationInfo.bOnlySpectator )
			GI.PlayerEvent( 'NewSpawn', PlayerReplicationInfo );

		CurrentBattery		=	MaximumBattery;

		SetTeam( avaCharacter( aPawn ).DefaultTeam );
	}

	Super.Possess( aPawn, bVehicleTransition );
}

event UnPossess()
{
	Super.UnPossess();
	DefaultFOV = Default.DefaultFOV;
	DesiredFOV = DefaultFOV;
}

function AcknowledgePossession(Pawn P)
{
	Super.AcknowledgePossession(P);

	if ( LocalPlayer(Player) != None )
	{
		ServerPlayerPreferences(WeaponHandPreference, bAutoTaunt, bCenteredWeaponFire);
	}
}

function SetPawnFemale()
{
//@todo steve - only if no valid character	if ( PawnSetupRecord.Species == None )
		PlayerReplicationInfo.bIsFemale = true;
}

function NotifyChangedWeapon( Weapon PreviousWeapon, Weapon NewWeapon )
{
	//@todo steve - should pawn do this directly?
	if ( avaPawn(Pawn) != None )
		avaPawn(Pawn).SetHand(WeaponHand);
}

reliable server function ServerPlayerPreferences( avaPawn.EWeaponHand NewWeaponHand, bool bNewAutoTaunt, bool bNewCenteredWeaponFire)
{
	ServerSetHand(NewWeaponHand);
	bCenteredWeaponFire = bNewCenteredWeaponFire;
}

reliable server function ServerSetHand(avaPawn.EWeaponHand NewWeaponHand)
{
	WeaponHand = NewWeaponHand;
    if ( avaPawn(Pawn) != None )
		avaPawn(Pawn).SetHand(WeaponHand);
}

simulated function SetHand(avaPawn.EWeaponHand NewWeaponHand)
{
	WeaponHandPreference = NewWeaponHand;
    SaveConfig();
    if ( avaPawn(Pawn) != None )
		avaPawn(Pawn).SetHand(WeaponHand);

    ServerSetHand(NewWeaponHand);
}

event ResetCameraMode()
{}

/**
* return whether viewing in first person mode
*/
function bool UsingFirstPersonCamera()
{
	return !bBehindView;
}

// ------------------------------------------------------------------------
exec function ToggleScreenShotMode()
{
	if ( avaHUD(myHUD).bCrosshairShow )
	{
		avaHUD(myHUD).bCrosshairShow = false;
		SetHand(HAND_Hidden);
		if ( myHUD.bShowHUD == true )
			myHUD.ToggleHud();
	}
	else
	{
		// return to normal
		avaHUD(myHUD).bCrosshairShow = true;
		SetHand(HAND_Right);
		if ( myHUD.bShowHUD == false )
			myHUD.ToggleHud();
		myHUD.bShowHUD = true;
	}
}

/** checks if the player has enabled swapped firemodes for this weapon and adjusts FireModeNum accordingly */
function CheckSwappedFire(out byte FireModeNum)
{
	local avaWeapon Weap;

	if (Pawn != None)
	{
		Weap = avaWeapon(Pawn.Weapon);
		if (Weap != None && Weap.bSwapFireModes)
		{
			if (FireModeNum == 0)
			{
				FireModeNum = 1;
			}
			else if (FireModeNum == 1)
			{
				FireModeNum = 0;
			}
		}
	}
}

exec function ToggleNightvision()
{
	if ( Pawn != None && !IsDead() && avaPawn(Pawn).bEnableNightVision == TRUE && !avaPawn(Pawn).bIsDash )
	{
		if ( avaPawn(Pawn).NightvisionActivated == FALSE )
		{
			if ( CurrentBattery > NVGMinimumPower )
			{
				CurrentBattery -= NVGMinimumPower;
				ServerToggleNightvision();
			}
			else
			{
				// Play Low Battery Sound...
			}
		}
		else
		{
			ServerToggleNightvision();
		}
	}
}

reliable server event function ServerTurnOffNightvision()
{
	if ( avaPawn(Pawn).NightvisionActivated == TRUE )
	{
		avaPawn(Pawn).NightvisionActivated = FALSE;
		avaPawn(Pawn).Client_UpdateNightvision();
	}
}

reliable server function ServerToggleNightvision()
{
	avaPawn(Pawn).NightvisionActivated = !avaPawn(Pawn).NightvisionActivated;
	//avaPawn(Pawn).NetUpdateTime = WorldInfo.TimeSeconds - 1;

	avaPawn(Pawn).Client_UpdateNightvision();
}

reliable server function ServerWeaponZoomModeChange()
{
	avaPawn(Pawn).WeaponZoomChange++;
	avaPawn(Pawn).Client_WeaponZoomModeChange();
}

exec function StartFire(optional byte FireModeNum)
{
	if ( bIgnoreFireInput == true )		return;
	if ( bRequestSwapWeapon == true )	return;

	CheckSwappedFire(FireModeNum);

	Super.StartFire(FireModeNum);
}

exec function StopFire(optional byte FireModeNum)
{
	CheckSwappedFire(FireModeNum);

	Super.StopFire(FireModeNum);
}

exec function Reload()
{
	if ( bRequestSwapWeapon == true )	return;
	ServerReload();
}

exec function ToggleSilencer()
{
	if ( avaPawn(Pawn) == None )	return;
	if ( avaWeap_BaseGun( Pawn.Weapon ) == None )	return;
	avaWeap_BaseGun( Pawn.Weapon ).ToggleSilencer();
}

reliable server function ServerReload()
{
	`log( "ServerReload" @Pawn @Pawn.Weapon );
	if ( avaPawn(Pawn) == None && avaWeaponPawn(Pawn) == None )	return;
	if ( avaWeap_BaseGun( Pawn.Weapon ) == None )	return;
	avaWeap_BaseGun( Pawn.Weapon ).DoReload();
}

/// Use �� ���� ��ź�� ��ü�ϱ� ���� override.
/// Use() ���� �� �Լ��� ȣ���Ѵ�.
simulated function bool PerformedUseAction()
{
    if ( Pawn == None || !Pawn.bCanUse )
		return true;

	// below is only on server
	if( Role < Role_Authority )
	{
		return false;
	}

	// ���� ��� �ִ� ��ȭ�Ⱑ �ִٸ� ���´�...
	if ( avaPawn( Pawn ).GripHeavyWeapon != None )
	{
		return avaPawn( Pawn ).GripHeavyWeapon.UserLeave( avaPawn( Pawn ) );
	}

	// �ֺ��� ���� �� �ִ� ��ȭ�Ⱑ �ִ��� Check �ؼ� ���� �� �ִٸ� ��´�.
	if ( TryToUseHeavyWeapon() )
	{
		return true;
	}

	// leave vehicle if currently in one
	if( Vehicle(Pawn) != None )
	{
		return Vehicle(Pawn).DriverLeave(false);
	}

	// try to find a vehicle to drive
	if( FindVehicleToDrive() )
	{
		return true;
	}
	
	//`log("[dEAthcURe|avaPlayerController::PerformedUseAction] before DefuseBomb");

	if ( DefuseBomb())
	{
		bDefusingBomb = true;
		return true;
	}

	// try to interact with triggers
	return TriggerInteracted();
}

/** Tries to find a vehicle to drive within a limited radius. Returns true if successful */
//! Engine���� ������ ���� �ϱ� �Ⱦ ���⼭ overriding.
function bool FindVehicleToDrive()
{
	local Vehicle V, Best;
	local vector ViewDir, PawnLoc2D, VLoc2D;
	local float NewDot, BestDot;

	if (Vehicle(Pawn.Base) != None  && Vehicle(Pawn.Base).TryToDrive(Pawn))
	{
		return true;
	}

	// Pick best nearby vehicle
	PawnLoc2D = Pawn.Location;
	PawnLoc2D.Z = 0;
	ViewDir = vector(Pawn.Rotation);

	ForEach Pawn.OverlappingActors(class'Vehicle', V, Pawn.VehicleCheckRadius)
	{
		// Prefer vehicles that Pawn is facing
		VLoc2D = V.Location;
		Vloc2D.Z = 0;
		NewDot = Normal(VLoc2D-PawnLoc2D) Dot ViewDir;
		if ( (Best == None) || (NewDot > BestDot) )
		{
			// check that vehicle is visible
			if ( FastTrace(V.Location,Pawn.Location) )
			{
				Best = V;
				BestDot = NewDot;
			}
		}
	}

	return (Best != None && Best.TryToDrive(Pawn));
}

function bool TryToUseHeavyWeapon()
{
	local avaFixedHeavyWeapon w;
	ForEach Pawn.OverlappingActors( class'avaFixedHeavyWeapon', w, Pawn.CylinderComponent.CollisionRadius )
	{
		if ( w.TryToEnter( avaPawn( Pawn ) ) )
			return true;
	}
	return false;
}



exec function UnUse()
{
	if( Role < Role_Authority )
	{
		PerformedUnUseAction();
	}
	ServerUnUse();
}

reliable server function ServerUnUse()
{
	PerformedUnUseAction();
}

function bool DefuseBomb()
{
	local avaProj_C4	bomb;
	//local float			incidence;
	//local vector		diff;
	//local vector		viewLoc;
	//local rotator		viewRot;

	local vector HitLocation, HitNormal;
	local vector TraceEnd, TraceStart;
	local Actor	 HitActor;
	
	//`log("[dEAthcURe|avaPlayerController::DefuseBomb] before if ( Pawn == none )");

	// find "avaProj_C4" for defuse.
	if ( Pawn == none )
		return false;

	//GetPlayerViewPoint( viewLoc, viewRot );
	// ���� ������� ������.
	
	//`log("[dEAthcURe|avaPlayerController::DefuseBomb] before Pawn.Trace");

	// Trace �� �ؼ� Defuse Bomb �� ���̴��� Check �Ѵ�...
	TraceStart	=	Pawn.GetPawnViewLocation();
	TraceEnd	=	TraceStart + vector( Pawn.GetViewRotation() ) * 140;
	HitActor	=	Pawn.Trace( HitLocation, HitNormal, TraceEnd, TraceStart, true, vect(0,0,0), ,TRACEFLAG_Bullet );
	
	//`log("[dEAthcURe|avaPlayerController::DefuseBomb] TraceStart=" @ TraceStart @ "TraceEnd=" @ TraceEnd @ "HitActor=" @ HitActor);
	//`log("[dEAthcURe|avaPlayerController::DefuseBomb] before bomb = avaProj_C4( HitActor )" @ HitActor);
	
	/*
	ForEach DynamicActors(class'avaProj_C4', bomb )
	{
		`log( "[dEAthcURe|avaPlayerController::DefuseBomb] Find Bomb" @bomb @bomb.Location @bomb.CollisionComponent.BlockZeroExtent );
	}
	*/
	

	bomb		=	avaProj_C4( HitActor );
	if ( bomb == None )				return false;
	if ( !bomb.CanDefuse( Pawn ) )	return false;
	if ( bomb.BeginDefuse( avaPlayerReplicationInfo( PlayerReplicationInfo ) ) )
	{
		DefusingBomb = bomb;
		return true;
	}
	
	//foreach Pawn.VisibleActors( class'avaProj_C4', bomb, 110 ) // InteractDistance )
	//{
	//	if ( !bomb.CanDefuse( pawn ) )	continue;


	//	`log( "avaPlayerController.DefuseBomb" @bomb );

	//	// Player �� �þ߿� ���Դ��� Check
	//	diff = bomb.Location - viewLoc;
	//	incidence = vector(viewRot) dot normal(diff);
	//	if ( incidence > 0 )
	//	{
	//		if ( incidence > cos( DesiredFOV / 2 * PI / 180 ) )
	//		{
	//			//	we have a bomb for defusing.
	//			// ���� �̹� defusing ���̸� �����ϰԵ�.
	//			if ( bomb.BeginDefuse( avaPlayerReplicationInfo(PlayerReplicationInfo) ) )
	//			{
	//				DefusingBomb = bomb;
	//				return true;
	//			}
	//		}
	//	}
	//}

	//`log( "no bomb insight" );
	return false;
}

function StopDefuseBomb()
{
	if ( bDefusingBomb == true )
	{
		if ( DefusingBomb != none )
		{
			DefusingBomb.EndDefuse( avaPlayerReplicationInfo(PlayerReplicationInfo) );
			DefusingBomb = none;
		}
		bDefusingBomb = false;
	}
}

function NotifyTakeHit(Controller InstigatedBy, vector HitLocation, int Damage, class<DamageType> damageType, vector Momentum)
{
	local int iDam;

	Super.NotifyTakeHit(InstigatedBy,HitLocation,Damage,DamageType,Momentum);

	iDam = Clamp(Damage,0,250);
	if ( (iDam == 0) && !bGodMode )
		return;
	//if ( (WorldInfo.NetMode == NM_DedicatedServer) || (WorldInfo.NetMode == NM_ListenServer) )
	//{
	if ( InstigatedBy != None && InstigatedBy.Pawn != none && class<avaDmgType_Gun>(damageType) != None )
		ClientPlayTakeHit(InstigatedBy.Pawn.Location - Pawn.Location, iDam, damageType);
	else
		ClientPlayTakeHit(hitLocation - Pawn.Location, iDam, damageType);
	//}
	//else if ( avaHUD(MyHud)!=none )
	//{
	//	if ( InstigatedBy != None && InstigatedBy.Pawn != None )
	//		avaHUD(MyHud).DisplayHit(InstigatedBy.Pawn.Location - Pawn.Location, Damage, DamageType);
	//	else
	//		avaHUD(MyHud).DisplayHit(HitLocation - Pawn.Location, Damage, DamageType);
	//}

}

simulated function GrenadeSoundEffectFinished(AudioComponent AC)
{
	GrenadeEffectSound = None;
}

function GrenadeEffect( int Damage )
{
	local float DamageTime;
	if ( GrenadeEffectSound != None )	GrenadeEffectSound.OnAudioFinished = None;
	// @TODO: Damage�� �׻� Max�� 100�ΰ�?
	//DamageRate = float(Damage)/class'Pawn'.default.Health;
	//GrenadeEffectSound = ClientPlaySoundCue(SoundCue'avaPlayerSounds.Damage.HighFreq3000Cue', Self, DamageRate, 1, -1);
	// @TODO: �̷��� �ϸ� Linear�ϰ� ���� �������µ�, `log������ ���ؾ� ���� ������?? �׷��ٸ� AudioComponent�� ���Ӱ� ������ �Ǵµ�,
	// ���Ӱ� ������� �ҽ��� ��ġ�ų�, ��� ������ ������ �� �ؾ� �ҵ�
	DamageTime = 5.f*Damage /100.f;
	//GrenadeEffectSound.FadeOut(DamageTime, 0.f);
	SetBlur( 0.5, DamageTime );
	//GrenadeEffectSound.OnAudioFinished = GrenadeSoundEffectFinished;
}

unreliable client function ClientPlayTakeHit(vector HitLoc, byte Damage, class<DamageType> damageType)
{
	if ( avaHUD(MyHud) != none )
		avaHUD(MyHud).DisplayHit(HitLoc, Damage, DamageType);

	// GrenadeEffect �� ���� ������ �ʽ��ϴ�!!!
	//if ( class<avaDmgType_Explosion>(damageType) != None )
	//{
	//	GrenadeEffect( Damage );
	//}
}

// Player movement.
// Player Standing, walking, running, falling. 
state PlayerWalking
{
ignores SeePlayer, HearNoise, Bump;

	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);
		PlayerReplicationInfo.bIsSpectator = false;

		if (avaHUD(myHUD) != None)
		{
			avaHUD(myHUD).OpenHudScene();
		}
		
		if ( IsLocalPlayerController() )
		{
			SetBehindView( false );
		}
	}

	event bool NotifyLanded(vector HitNormal, Actor FloorActor)
	{
		if (DoubleClickDir == DCLICK_Active)
		{
			DoubleClickDir = DCLICK_Done;
			ClearDoubleClick();
		}
		else
			DoubleClickDir = DCLICK_None;

		if ( Global.NotifyLanded(HitNormal,FloorActor) )
			return true;

		return false;
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		if ( (DoubleClickMove == DCLICK_Active) && (Pawn.Physics == PHYS_Falling) )
			DoubleClickDir = DCLICK_Active;
		else if ( (DoubleClickMove != DCLICK_None) && (DoubleClickMove < DCLICK_Active) )
		{
			/*if ( DoubleClickMove == DCLICK_Forward )
				avaPawn(Pawn).DoDash();*/
			//if ( avaPawn(Pawn).DoDash(DoubleClickMove) )
			//	DoubleClickDir = DCLICK_Active;
		}

		//`log( "ProcessMove " @PlayerInput );
		//if ( PlayerInput.aForward <= 0.0 )
		//	avaPawn(Pawn).CancelDash();
		if( Role == Role_Authority && WorldInfo.NetMode != NM_StandAlone )
			avaPawn( Pawn ).SetRemoteViewYaw( Rotation.Yaw );

		Super.ProcessMove(DeltaTime,NewAccel,DoubleClickMove,DeltaRot);
	}

    function PlayerMove( float DeltaTime )
    {
		local vector X,Y,Z;
		local Rotator DeltaRot, ViewRotation;

		local vector			NewAccel;
		local eDoubleClickDir	DoubleClickMove;
		local rotator			OldRotation;
		local bool				bSaveJump;

`if( `isdefined(FINAL_RELEASE) )
		local float				PrvNoInputElapsedTime;
		local int				ChannelFlag;
	
		ChannelFlag	= class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag();	

		if ( ChannelFlag != EChannelFlag_Match )
		{

			PrvNoInputElapsedTime = NoInputElapsedTime;

			if ( !avaGameReplicationInfo(WorldInfo.GRI).bWarmUpRound )
				NoInputElapsedTime += DeltaTime;

			if ( PrvNoInputElapsedTime < NOINPUTKICKTIME - 10.0 && NoInputElapsedTime >= NOINPUTKICKTIME - 10.0 )
			{
				// No Input Warning 
				avaHUD( myHUD ).AddGameMessage( class'avaLocalizedMessage'.default.NoInputKickWarning );
			}
			
			if ( PrvNoInputElapsedTime < NOINPUTKICKTIME && NoInputElapsedTime >= NOINPUTKICKTIME )
			{
				// Leave Game 
				class'avaNetHandler'.static.GetAvaNetHandler().LeaveRoom( ELR_Idle );
				NoInputElapsedTime = 0.0;
			}
		}
`endif

		if ( bBehindView && bLockRotation )
		{
			GetAxes(Rotation,X,Y,Z);
			// Update view rotation.
			ViewRotation = Rotation;
			// Calculate Delta to be applied on ViewRotation
			DeltaRot.Yaw	= PlayerInput.aTurn;
			DeltaRot.Pitch	= PlayerInput.aLookUp;
			ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
			SetRotation(ViewRotation);

			ViewShake(DeltaTime);

			if ( Role < ROLE_Authority ) // then save this move and replicate it
				ReplicateMove(DeltaTime, vect(0,0,0), DCLICK_None, rot(0,0,0));
			bPressedJump = false;


			GetAxes(Pawn.Rotation,X,Y,Z);

			// Update acceleration.
			NewAccel	= PlayerInput.aForward*X + PlayerInput.aStrafe*Y;
			NewAccel.Z	= 0;
			if( VSize(NewAccel) < 1.0 )
			{
				NewAccel = vect(0,0,0);
			}
			DoubleClickMove = PlayerInput.CheckForDoubleClickMove( DeltaTime/WorldInfo.TimeDilation );

			bDoubleJump = false;

			if( bPressedJump && Pawn.CannotJumpNow() )
			{
				bSaveJump = true;
				bPressedJump = false;
			}
			else
			{
				bSaveJump = false;
			}

			if( Role < ROLE_Authority ) // then save this move and replicate it
			{
				ReplicateMove(DeltaTime, NewAccel, DoubleClickMove, OldRotation - Rotation);
			}
			else
			{
				ProcessMove(DeltaTime, NewAccel, DoubleClickMove, OldRotation - Rotation);
			}
			bPressedJump = bSaveJump;
			return;
		}

		Super.PlayerMove(DeltaTime);

		GroundPitch = 0;

	}
}

simulated function ClientSpectate()
{
	GotoState('Spectating');
}

function ServerSpectate()
{
	GotoState('Spectating');
}

state RoundEnded
{
ignores SeePlayer, HearNoise, KilledBy, NotifyBump, HitWall, NotifyHeadVolumeChange, NotifyPhysicsVolumeChange, Falling, TakeDamage, Suicide;

	exec function PrevWeapon() {}
	exec function NextWeapon() {}
	exec function SwitchWeapon(byte T) {}

	function FindGoodView()
	{
		local vector cameraLoc;
		local rotator cameraRot, ViewRotation;
		local int tries, besttry;
		local float bestdist, newdist;
		local int startYaw;

		ViewRotation = Rotation;
		ViewRotation.Pitch = 56000;
		tries = 0;
		besttry = 0;
		bestdist = 0.0;
		startYaw = ViewRotation.Yaw;
		for (tries=0; tries<16; tries++)
		{
			CalcViewActor = None;
			cameraLoc = ViewTarget.Location;
			SetRotation(ViewRotation);
			GetPlayerViewPoint( cameraLoc, cameraRot );
			newdist = VSize(cameraLoc - ViewTarget.Location);
			if (newdist > bestdist)
			{
				bestdist = newdist;
				besttry = tries;
			}
			ViewRotation.Yaw += 4096;
		}
		ViewRotation.Yaw = startYaw + besttry * 4096;
		SetRotation(ViewRotation);
	}

    unreliable client function LongClientAdjustPosition
    (
	float TimeStamp,
	name newState,
	EPhysics newPhysics,
	float NewLocX,
	float NewLocY,
	float NewLocZ,
	float NewVelX,
	float NewVelY,
	float NewVelZ,
	Actor NewBase,
	float NewFloorX,
	float NewFloorY,
	float NewFloorZ
    )
    {
		if ( newState == 'PlayerWaiting' )
			GotoState( newState );
    }

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;
		local Rotator DeltaRot, ViewRotation;

		GetAxes(Rotation,X,Y,Z);
		// Update view rotation.
		ViewRotation = Rotation;
		// Calculate Delta to be applied on ViewRotation
		DeltaRot.Yaw	= PlayerInput.aTurn;
		DeltaRot.Pitch	= PlayerInput.aLookUp;
		ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
		SetRotation(ViewRotation);

		ViewShake(DeltaTime);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, vect(0,0,0), DCLICK_None, rot(0,0,0));
		bPressedJump = false;
	}

	function BeginState(Name PreviousStateName)
	{
	//	local Pawn P;
	//	FOVAngle = DesiredFOV;
	//	bFire = 0;
	//	if ( Pawn != None )
	//	{
	//		Pawn.TurnOff();
	//		Pawn.bSpecialHUD = false;
	//    StopFiring();
	//	}
	//	bFrozen = true;
	//	FindGoodView();
	//SetTimer(5, false);
	//	ForEach DynamicActors(class'Pawn', P)
	//	{
	//		if ( P.Role == ROLE_Authority )
	//			P.RemoteRole = ROLE_SimulatedProxy;
	//		P.TurnOff();
	//	}
	}

	function EndState(name NextStateName)
	{
		Super.EndState(NextStateName);

		if (avaHUD(myHUD) != None)
		{
			avaHUD(myHUD).HideScores();
		}

		SetBehindView(false);
		StopViewShaking();
	}
}

`devexec simulated function ToggleLP()
{
	avaPawn(ViewTarget).bThirdPersonDeathCam = !avaPawn(ViewTarget).bThirdPersonDeathCam;
}

simulated function CloseDeadScene()
{

}

state Dead
{
	ignores SeePlayer, HearNoise, KilledBy, NextWeapon, PrevWeapon, StartAltFire;

	exec function SwitchWeapon(byte T){}

	exec function StartFire( optional byte FireModeNum )
	{
		if ( bEnableCloseDeadScene == true && bActivateDeadScene == true )
			CloseDeadScene();
	}

	function Timer()
	{
		if (!bFrozen)
			return;

		// force garbage collection while dead, to avoid GC during gameplay
		if ( (WorldInfo.NetMode == NM_Client) || (WorldInfo.NetMode == NM_Standalone) )
		{
			WorldInfo.ForceGarbageCollection();
		}
		bFrozen = false;
		bUsePhysicsRotation = false;
		bPressedJump = false;
	}

	// Dead State ���� Client ViewTarget �� �ٲ۴�.
	reliable client event ClientSetViewTarget( Actor A, optional float TransitionTime )
	{
		if( A == None )
		{
			ServerVerifyViewTarget();
			return;
		}
		// don't force view to self while dead (since server may be doing it having destroyed the pawn)
		if ( A == self )
			return;
		SetViewTarget( A, TransitionTime );
	}

	function FindGoodView()
	{
		local vector cameraLoc;
		local rotator cameraRot, ViewRotation, RealRotation;
		local int tries, besttry;
		local float bestdist, newdist, RealCameraScale;
		local int startYaw;
		local avaPawn P;

		if ( avaVehicle(ViewTarget) != None )
		{
			DesiredRotation = Rotation;
			bUsePhysicsRotation = true;
			return;
		}

		ViewRotation = Rotation;
		RealRotation = ViewRotation;
		ViewRotation.Pitch = 56000;
		SetRotation(ViewRotation);
		P = avaPawn(ViewTarget);
		if ( P != None )
		{
			RealCameraScale = P.CurrentCameraScale;
			P.CurrentCameraScale = P.CameraScale;
		}

		// use current rotation if possible
		CalcViewActor = None;
		cameraLoc = ViewTarget.Location;
		GetPlayerViewPoint( cameraLoc, cameraRot );
		if ( P != None )
		{
			newdist = VSize(cameraLoc - ViewTarget.Location);
			if (newdist < P.CylinderComponent.CollisionRadius + P.CylinderComponent.CollisionHeight )
			{
				// find alternate camera rotation
				tries = 0;
				besttry = 0;
				bestdist = 0.0;
				startYaw = ViewRotation.Yaw;

				for (tries=1; tries<16; tries++)
				{
					CalcViewActor = None;
					cameraLoc = ViewTarget.Location;
					ViewRotation.Yaw += 4096;
					SetRotation(ViewRotation);
					GetPlayerViewPoint( cameraLoc, cameraRot );
					newdist = VSize(cameraLoc - ViewTarget.Location);
					if (newdist > bestdist)
					{
						bestdist = newdist;
						besttry = tries;
					}
				}
				ViewRotation.Yaw = startYaw + besttry * 4096;
			}
			P.CurrentCameraScale = RealCameraScale;
		}
		SetRotation(RealRotation);
		DesiredRotation = ViewRotation;
		DesiredRotation.Roll = 0;
		bUsePhysicsRotation = true;
	}

	simulated function OpenDeadScene()
	{
		bActivateDeadScene		=	true;
		bEnableCloseDeadScene	=	false;
		if ( avaHUD(MyHud) != None )	avaHUD(MyHud).OpenDeathScene();
		PlayerReplicationInfo.bIsSpectator = true;
		if ( IsLocalPlayerController() )
		{
			SetBehindView(true);
			HeartbeatSound = ClientPlaySoundCue(SoundCue'avaPlayerSounds.HeartbeatCue', Self, 1, 1, -1);
			DSP_Muffle.Apply();
		}
		SetTimer( `DEADVIEWTARGETCHANGETIME, false, 'DeadTimer');
		SetTimer(2.f, false, 'EnableCloseDeadScene' );
	}

	simulated function CloseDeadScene()
	{
		local rotator	R;
		bActivateDeadScene = false;
		if ( avaHUD(MyHud) != None )	avaHUD(MyHud).OpenSpectatorScene();
		StopViewShaking();
		avaPawn(ViewTarget).bThirdPersonDeathCam = true;
		R = Rotation;
		R.Pitch = -16300;
		SetRotation( R );
		SetPhysics(PHYS_None);
		if ( IsLocalPlayerController() )
		{
			if (HeartbeatSound != None) 
			{
				HeartbeatSound.Stop();
				HeartbeatSound = None;
			}
			DSP_Muffle.Stop();
		}
	}

	simulated function EnableCloseDeadScene()
	{	
		bEnableCloseDeadScene	=	true;
	}

	simulated function DeadTimer()
	{
		if( IsInState('Spectate') == false )
			ClientSpectate();
	}

	simulated function BeginState(Name PreviousStateName)
	{
		ActiveFades.Length = 0;	// Flash Bang ȿ���� ������Ų��...
		OpenDeadScene();
		Super.BeginState(PreviousStateName);
	}

	function EndState(name NextStateName)
	{
		bUsePhysicsRotation = false;
		Super.EndState(NextStateName);
		if ( bActivateDeadScene == true )
			CloseDeadScene();
		ClearTimer( 'DeadTimer' );
		ClearTimer( 'EnableCloseDeadScene' );
	}

Begin:
	/*
    Sleep(0.5);
	if ( (ViewTarget == None) || (ViewTarget == self) || (VSize(ViewTarget.Velocity) < 1.0) )
	{
		Sleep(0.5);
		if ( myHUD != None )
			myHUD.bShowScores = true;
	}
	else
		Goto('Begin');*/

	if ( myHUD != None )
	{
		myHUD.bShowScores = true;
	}
}

/**
 * list important avaPlayerController variables on canvas. HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.


 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
}

function Reset()
{
	Super.Reset();
	/*if ( PlayerCamera != None )
	{
		PlayerCamera.Destroy();
	}*/
}


`devexec function SetRadius(float newradius)
{
	Pawn.SetCollisionSize(NewRadius,Pawn.GetCollisionHeight());
}

exec function BehindView()
{
//	if ( WorldInfo.NetMode == NM_Standalone )
	SetBehindView(!bBehindView);
}

reliable client function ClientSetBehindView(bool bNewBehindView)
{
//	`log( "ClientSetBehindView " @bNewBehindView @Self );
	if (LocalPlayer(Player) != None)
	{
		SetBehindView(bNewBehindView);
	}
	// make sure we recalculate camera position for this frame
	LastCameraTimeStamp = WorldInfo.TimeSeconds - 1.0;
}

/**
 * Set new camera mode
 *
 * @param	NewCamMode, new camera mode.
 */
function SetCameraMode( name NewCamMode )
{
	//if ( PlayerCamera != None )
	//{
	//	Super.SetCameraMode(NewCamMode);
	//}
	//else
	if ( NewCamMode == 'ThirdPerson' )
	{
		if ( !bBehindView )
			SetBehindView(true);
	}
	else
	{
		if ( bBehindView )
			SetBehindView(false);
	}

	if ( PlayerCamera != None )
	{
		PlayerCamera.CameraStyle = NewCamMode;
		if ( WorldInfo.NetMode == NM_DedicatedServer )
		{
			ClientSetCameraMode( NewCamMode );
		}
	}
}

/**
 *	Calculate camera view point, when viewing this actor.
 *
 * @param	fDeltaTime	delta time seconds since last update
 * @param	out_CamLoc	Camera Location
 * @param	out_CamRot	Camera Rotation
 * @param	out_FOV		Field of View
 *
 * @return	true if Actor should provide the camera point of view.
 */
simulated function bool CalcCamera( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	out_CamLoc = Location + (CylinderComponent(CollisionComponent).CollisionHeight * Vect(0,0,1));
	out_CamRot = Rotation;
	return false;
}

`devexec simulated function TestViewTarget()
{
}

function SpawnCamera()
{
	local Actor OldViewTarget;

	// Associate Camera with PlayerController
	PlayerCamera = Spawn(MatineeCameraClass, self);
	if (PlayerCamera != None)
	{
		OldViewTarget = ViewTarget;
		PlayerCamera.InitializeFor(self);
		PlayerCamera.SetViewTarget(OldViewTarget);
//		`log( "avaPlayerController.SpawnCamera SetViewTarget" @OldViewTarget );
	}
	else
	{
	}
}

/* GetPlayerViewPoint: Returns Player's Point of View
	For the AI this means the Pawn's Eyes ViewPoint
	For a Human player, this means the Camera's ViewPoint */
simulated event GetPlayerViewPoint( out vector POVLocation, out Rotator POVRotation )
{
	local float DeltaTime;
	local bool bIsFreeCam, bLocalPC;
	local avaPawn P;

	bIsFreeCam = (ViewTarget == None || ViewTarget == Self);
	bLocalPC = IsLocalPlayerController();

	P = IsLocalPlayerController() ? avaPawn(CalcViewActor) : None;
	
	if (LastCameraTimeStamp == WorldInfo.TimeSeconds
		&& CalcViewActor == ViewTarget
		&& CalcViewActor != None
		&& CalcViewActor.Location == CalcViewActorLocation
		&& CalcViewActor.Rotation == CalcViewActorRotation
		)
	{
		if ( (P == None) || ((P.EyeHeight == CalcEyeHeight) && (P.WalkBob == CalcWalkBob)) )
		{
			// use cached result
			POVLocation = CalcViewLocation;
			POVRotation = CalcViewRotation;
			return;
		}
	}

	if ( !UseShoulderView() )
	{
		DeltaTime = WorldInfo.TimeSeconds - LastCameraTimeStamp;

		if ( CameraActor(ViewTarget) != None )
		{
			if ( PlayerCamera == None )
			{
				super.ResetCameraMode();
				SpawnCamera();
			}
			super.GetPlayerViewPoint( POVLocation, POVRotation );
		}
		else
		{
			if ( PlayerCamera == None )
			{
				SpawnCamera();
			}

			if ( ViewTarget != None )
			{
				POVRotation = Rotation;
				ViewTarget.CalcCamera( DeltaTime, POVLocation, POVRotation, FOVAngle );		
			}
			else
			{
				CalcCamera( DeltaTime, POVLocation, POVRotation, FOVAngle );		
			}
		}
	}
	else
	{
		Super.GetPlayerViewPoint( POVLocation, POVRotation );
	}

	// free-cam�̾ƴҶ�
	if (!bIsFreeCam && bLocalPC)
	{
		PendingLocation = POVLocation;
		PendingRotation = POVRotation;
		bPendingFreeCam = true;		 
	}  
	
	// apply view shake
	POVRotation = Normalize(POVRotation + ShakeRot);
	POVLocation += ShakeOffset >> Rotation;

	// cache result
	CalcViewActor			= ViewTarget;
	CalcViewActorLocation	= ViewTarget.Location;
	CalcViewActorRotation	= ViewTarget.Rotation;
	CalcViewLocation		= POVLocation;
	CalcViewRotation		= POVRotation;
	
	if ( P != None )
	{
		CalcEyeHeight = P.EyeHeight;
		CalcWalkBob = P.WalkBob;
	}
	
	LastCameraTimeStamp		= WorldInfo.TimeSeconds;
}

reliable client function ClientSetHUD(class<HUD> newHUDType, class<Scoreboard> newScoringType)
{
	Super.ClientSetHUD(newHUDType, newScoringType);
}

/** Causes a view shake based on the amount of damage
	Should only be called on the owning client */
function DamageShake(int Damage)
{
	local ViewShakeInfo NewViewShake;
	Damage = Min(Damage, 200);
	NewViewShake = BaseDamageShake;
	NewViewShake.RotMag *= Damage;
	NewViewShake.RotTime += 0.005 * Damage;
	NewViewShake.OffsetMag *= Damage;
	ShakeView(NewViewShake);
}

exec function ApplyShockMuffleDSP()
{
	ShockMuffleDSP.Apply();
}

simulated function ApplyFlashEffect( float HoldTime, float FadeTime, float Alpha )
{
	local FadeParameters Fade;

	// 3��Ī Specating �߿��� Flash Effect �� ���� �ʵ��� ����...
	if ( IsSpectating() && bBehindView == true )	return;
	if ( IsDead() )									return;

	//`log( "ApplyFlashEffect "@HoldTime@FadeTime@Alpha );

	Fade.HoldTime = HoldTime;
	Fade.FadeTime = FadeTime;
	Fade.Alpha = Alpha;
	Fade.StartTime = WorldInfo.TimeSeconds;
	ActiveFades[ActiveFades.Length] = Fade;
}

simulated function UpdateFades()
{
	local int i;
	local float ElapsedTime;
	local float Alpha;
	local float LocalAlpha;

	Alpha = 0;

	for (i=ActiveFades.Length-1; i>=0; --i)
	{
		ElapsedTime = WorldInfo.TimeSeconds - ActiveFades[i].StartTime;		

		LocalAlpha = ActiveFades[i].Alpha * ( 1 - FClamp( ( ElapsedTime - ActiveFades[i].HoldTime ) / ActiveFades[i].FadeTime, 0, 1 ) );

		if (LocalAlpha == 0)
		{
			ActiveFades.Remove( i, 1 );
			continue;
		}
		else
			Alpha += LocalAlpha;

		ActiveFades[i].CurrentAlpha = LocalAlpha;
	}	

	avaPlayerCamera(PlayerCamera).UpdateFlashEffect( Alpha );
}

/** Shakes the player's view with the given parameters
	Should only be called on the owning client */
function ShakeView(ViewShakeInfo NewViewShake)
{
	// apply the new rotation shaking only if it is larger than the current amount
	if (VSize(NewViewShake.RotMag) > VSize(CurrentViewShake.RotMag))
	{
		CurrentViewShake.RotMag = NewViewShake.RotMag;
		CurrentViewShake.RotRate = NewViewShake.RotRate;
		CurrentViewShake.RotTime = NewViewShake.RotTime;
	}
	// apply the new offset shaking only if it is larger than the current amount
	if (VSize(NewViewShake.OffsetMag) > VSize(CurrentViewShake.OffsetMag))
	{
		CurrentViewShake.OffsetMag = NewViewShake.OffsetMag;
		CurrentViewShake.OffsetRate = NewViewShake.OffsetRate;
		CurrentViewShake.OffsetTime = NewViewShake.OffsetTime;
	}
}

static function ShakeAll(avaPlayerController.ViewShakeInfo shakeInfo, vector Loc, float radius, WorldInfo WI)
{
	local avaPlayerController PC;
	local float Dist, Scale;
	local ViewShakeInfo ViewShake;

	foreach WI.AllControllers(class'avaPlayerController', PC)
	{
		if (PC.ViewTarget != None)
		{
			Dist = VSize(Loc - PC.ViewTarget.Location);
			if (Dist < Radius * 2.0)
			{
				if (Dist < Radius)
				{
					Scale = 1.0;
				}
				else
				{
					Scale = (Radius * 2.0 - Dist) / Radius;
				}
				ViewShake = shakeInfo;
				ViewShake.OffsetMag *= Scale;
				ViewShake.RotMag *= Scale;

				PC.ShakeView(ViewShake);
			}
		}
	}
}

// 
simulated function OnCameraShake(avaSeqAct_CameraShake ShakeAction)
{
	ShakeView(ShakeAction.CameraShake);
}

/** Turns off any view shaking */
function StopViewShaking()
{
	local ViewShakeInfo EmptyViewShake;
		EmptyViewShake.OffsetTime=0.0f;	// FIXMERON : Yech.
	CurrentViewShake = EmptyViewShake;
}

final native function CheckShake(out float MaxOffset, out float Offset, out float Rate, float Time);

final native function UpdateShakeRotComponent(out float Max, out int Current, out float Rate, float Time, float DeltaTime);

/** Sets ShakeOffset and ShakeRot to the current view shake that should be applied to the camera */
function ViewShake(float DeltaTime)
{
    if (!IsZero(CurrentViewShake.OffsetRate))
    {
	// modify shake offset
	ShakeOffset.X += DeltaTime * CurrentViewShake.OffsetRate.X;
	CheckShake(CurrentViewShake.OffsetMag.X, ShakeOffset.X, CurrentViewShake.OffsetRate.X, CurrentViewShake.OffsetTime);

	ShakeOffset.Y += DeltaTime * CurrentViewShake.OffsetRate.Y;
	CheckShake(CurrentViewShake.OffsetMag.Y, ShakeOffset.Y, CurrentViewShake.OffsetRate.Y, CurrentViewShake.OffsetTime);

	ShakeOffset.Z += DeltaTime * CurrentViewShake.OffsetRate.Z;
	CheckShake(CurrentViewShake.OffsetMag.Z, ShakeOffset.Z, CurrentViewShake.OffsetRate.Z, CurrentViewShake.OffsetTime);

	CurrentViewShake.OffsetTime -= DeltaTime;
    }

    if (!IsZero(CurrentViewShake.RotRate))
    {
	UpdateShakeRotComponent(CurrentViewShake.RotMag.X, ShakeRot.Pitch, CurrentViewShake.RotRate.X, CurrentViewShake.RotTime, DeltaTime);
	UpdateShakeRotComponent(CurrentViewShake.RotMag.Y, ShakeRot.Yaw, CurrentViewShake.RotRate.Y, CurrentViewShake.RotTime, DeltaTime);
	UpdateShakeRotComponent(CurrentViewShake.RotMag.Z, ShakeRot.Roll, CurrentViewShake.RotRate.Z, CurrentViewShake.RotTime, DeltaTime);
	CurrentViewShake.RotTime -= DeltaTime;
    }
}


//=====================================================================
// ava specific implementation of networked player movement functions
//
function MoveAutonomous
(
	float DeltaTime,
	byte CompressedFlags,
	vector newAccel,
	rotator DeltaRot
)
{	
	if (newAccel != vect(0,0,0))
	{
		newAccel = Pawn.AccelRate * Normal(newAccel);
	}

	Super.MoveAutonomous( DeltaTime, CompressedFlags, newaccel, DeltaRot );
}

function CallServerMove
(
	SavedMove NewMove,
    vector ClientLoc,
    byte ClientRoll,
    int View,
    SavedMove OldMove
)
{
	local vector BuildAccel;
	local byte OldAccelX, OldAccelY, OldAccelZ;

	// compress old move if it exists
	if ( OldMove != None )
	{
		// old move important to replicate redundantly
		BuildAccel = 0.05 * OldMove.Acceleration + vect(0.5, 0.5, 0.5);
		OldAccelX = CompressAccel(BuildAccel.X);
		OldAccelY = CompressAccel(BuildAccel.Y);
		OldAccelZ = CompressAccel(BuildAccel.Z);

		OldServerMove(OldMove.TimeStamp,OldAccelX, OldAccelY, OldAccelZ, OldMove.CompressedFlags());
	}

	if ( PendingMove != None )
	{
		DualServerMove
		(
			PendingMove.TimeStamp,
			PendingMove.Acceleration * 10,
			PendingMove.CompressedFlags(),
			((PendingMove.Rotation.Yaw & 65535) << 16) + (PendingMove.Rotation.Pitch & 65535),
			NewMove.TimeStamp,
			NewMove.Acceleration * 10,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View
		);
	}
    else if ( (NewMove.Acceleration * 10 == vect(0,0,0)) && (NewMove.DoubleClickMove == DCLICK_None))
    {
		ShortServerMove
		(
			NewMove.TimeStamp,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View
		);
    }
    else
		ServerMove
	(
	    NewMove.TimeStamp,
	    NewMove.Acceleration * 10,
	    ClientLoc,
			NewMove.CompressedFlags(),
	    ClientRoll,
	    View
	);
}

/* ShortServerMove()
compressed version of server move for bandwidth saving
*/
unreliable server function ShortServerMove
(
	float TimeStamp,
	vector ClientLoc,
	byte NewFlags,
	byte ClientRoll,
	int View
)
{
    ServerMove(TimeStamp,vect(0,0,0),ClientLoc,NewFlags,ClientRoll,View);
}

// {{ 20061215 dEAthcURe|HM
state PlayerHmRestarting
{	
	
Begin:
	super.ServerRestartPlayer();
	WorldInfo.Game.RestartPlayer(self);
}
// }} 20061215 dEAthcURe|HM

simulated function NotifyOutOfLives( bool bOutOfLives )
{
	if ( bOutOfLives == true && IsInState( 'PlayerWaiting') )
	{
		GotoState('Spectating');	
	}
}

`devexec simulated function RequestRestartPlayer()
{
	ServerRequestRestartPlayer();
}

reliable server function ServerRequestRestartPlayer()
{
	local avaWeapon				InvWeapon;
	if ( avaGame( WorldInfo.Game ).IsPracticeMode() == true )
	{
		// ������忡�� ����۽� SpecialInventory�� ������ �ִٸ� �ٴڿ� ����߸��� ������ϵ��� �Ѵ�...
		if ( avaPawn( Pawn ) != None )
		{
			foreach Pawn.InvManager.InventoryActors( class'avaWeapon', InvWeapon )
			{
				if ( InvWeapon.bSpecialInventory == true )
					InvWeapon.ThrowWeapon( true );
			}
		}
		WorldInfo.Game.RestartPlayer( self );
	}
}

auto state PlayerWaiting
{
	exec function StartFire( optional byte FireModeNum )
	{
/* //@todo steve - load skins that haven't been precached (because players joined since game start)
	LoadPlayers();
	if ( !bForcePrecache && (WorldInfo.TimeSeconds > 0.2) )
*/
		if ( PlayerReplicationInfo.bOutOfLives )
			GotoState( 'Spectating' );
		else
		{
            ServerReStartPlayer();
		}
	}

    reliable server function ServerRestartPlayer()
	{
		super.ServerRestartPlayer();

		if ( WorldInfo.Game.bWaitingToStartMatch && avaGame(WorldInfo.Game).bWarmupRound && avaGame(WorldInfo.Game).WarmupTime>1.0 )
		{
			WorldInfo.Game.RestartPlayer(self);
		}
	}
}

state BaseSpectating
{
	ignores ToggleNightvision;

	exec function SwitchWeapon(byte F){}

    function PlayerMove(float DeltaTime)
    {
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local float		VelSize;

		///* smoothly accelerate and decelerate */
		Acceleration = Normal(NewAccel) * class'Pawn'.Default.AirSpeed * SpectatorSpeedRate;

		VelSize = VSize(Velocity);
		if( VelSize > 0 )
		{
			Velocity = Velocity - (Velocity - Normal(Acceleration) * VelSize) * FMin(DeltaTime * 8, 1);
		}

		Velocity = Velocity + Acceleration * DeltaTime;
		if( VSize(Velocity) > class'Pawn'.default.AirSpeed * SpectatorSpeedRate )
		{
			Velocity = Normal(Velocity) * class'Pawn'.default.AirSpeed * SpectatorSpeedRate;
		}

		if( VSize(Velocity) > 0 )
		{
			MoveSmooth( (1+bRun) * Velocity * DeltaTime );
		}
	}

	function ReplicateMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		ProcessMove(DeltaTime, NewAccel, DoubleClickMove, DeltaRot);
		// �̰� ���� �ʿ��ұ��???
		//ServerSetSpectatorLocation(Location);
	}

}

function int BlendRot(float DeltaTime, int BlendC, int NewC)
{
	if ( Abs(BlendC - NewC) > 32767 )
	{
		if ( BlendC > NewC )
			NewC += 65536;
		else
			BlendC += 65536;
	}
	if ( Abs(BlendC - NewC) > 4096 )
		BlendC = NewC;
	else
		BlendC = BlendC + (NewC - BlendC) * FMin(1,24 * DeltaTime);

	return (BlendC & 65535);
}

exec function SwitchWeapon(byte T)
{
	// Swap Weapon �� ��û�� �� ������ �ޱ� ������ Switch Weapon �� �� �� ����
	if ( bRequestSwapWeapon == true )
		return;

	if (avaPawn(Pawn) != None)
		avaPawn(Pawn).SwitchWeapon(t);
}

/**
 * Allow a player's vehicle or weapon to override the fov
 */
event float GetFOVAngle()
{
	local float FOVA;

	FOVA = Super.GetFOVAngle();


	if (Pawn != None && avaPawn(Pawn) != None && avaPawn(Pawn).IsFirstPerson())
	{
		if (Pawn.Weapon != None)
		{
			FOVA = Pawn.Weapon.AdjustFOVAngle(FOVA);
		}
	}
	return FOVA;
}

//{{ Foregrond DPG FOV
//!{ 2006. 3. 6		�� â ��

/**
 * Foreground DPG�� FOV angle�� �������̵� �ϰ� �Ѵ�.
 */
event float GetForegroundFOVAngle()
{
	local float FOVA;

	FOVA = Super.GetFOVAngle();
	if( Pawn != None )
	{
		if( Pawn.Weapon != None )
		{
			FOVA = Pawn.Weapon.AdjustForegroundFOVAngle(FOVA);
		}
	}
	return FOVA;
}
//!} 2006. 3. 6		�� â ��
//}}

unreliable server function ServerViewSelf()
{
	//SetLocation(ViewTarget.Location);
	//SetBehindView(false);
    SetViewTarget( Self );
}


exec function ViewPlayerByName(string PlayerName);

unreliable server function ServerViewPlayerByName(string PlayerName)
{
	local int i;
	for (i=0;i<WorldInfo.GRI.PRIArray.Length;i++)
	{
		if (WorldInfo.GRI.PRIArray[i].PlayerName ~= PlayerName)
		{
			if ( WorldInfo.Game.CanSpectate(self, WorldInfo.GRI.PRIArray[i]) )
			{
				SetViewTarget(WorldInfo.GRI.PRIArray[i]);
			}
			return;
		}
	}

	ClientMessage(MsgPlayerNotFound);
}

exec function PreviousWeapon()
{
	if ( bRequestSwapWeapon == true )	return;
	if ( Pawn.InvManager != None )
		avaInventoryManager(Pawn.InvManager).SwitchToPreviousWeapon();
}

exec function PrevWeapon()
{
	if ( bRequestSwapWeapon == true )	return;
	if (Pawn == None)
	{
		AdjustCameraScale(true);
	}
	Super.PrevWeapon();
}

exec function NextWeapon()
{
	if ( bRequestSwapWeapon == true )	return;
	if (Pawn == None)
	{
		AdjustCameraScale(false);
	}
	Super.NextWeapon();
}

/** moves the camera in or out */
exec function AdjustCameraScale(bool bIn)
{
	if (avaPawn(ViewTarget) != None)
	{
		avaPawn(ViewTarget).AdjustCameraScale(bIn);
	}
}

state Spectating
{
	ignores LongClientAdjustPosition;

	function BeginState(Name PreviousStateName)
	{
		super.BeginState(PreviousStateName);
		
		PlayerReplicationInfo.bIsSpectator = true;
		
		if ( avaHUD(MyHud) != None )
		{
			avaHUD(MyHud).OpenSpectatorScene();		// HUD �� Spectator Scene �� �����ش�...
			avaHUD(MyHud).CheckRespawnScene();		// Game Mode �� Respawn �� �����ϴٸ� Respawn ������ �����ֵ��� �Ѵ�...
		}

		if ( IsLocalPlayerController() )
		{
			ServerInitViewPlayer();
			SetBehindView( true );
		}
	}

	simulated function EndState( name NextState )
	{
		Super.EndState( NextState );
		if ( avaHUD(MyHud) != None )
			avaHUD(MyHud).OpenHudScene();
	}

	exec function ViewPlayerByName(string PlayerName)
	{
		ServerViewPlayerByName(PlayerName);
	}

	exec function BehindView()
	{
		bForceBehindView = !bForceBehindview;
	}

	exec function PrevWeapon()
	{
		if ( bUseFreeCam == true )
		{
			DecreaseCameraFOV();
		}
		else
		{
			if ( avaHUD(myHUD).HasBroadcastAuthority() )
			{
				ServerViewPrevSlotPlayer();
			}
			else
			{
				ServerViewPrevPlayer();
			}
		}
	}

	exec function NextWeapon()
	{
		if ( bUseFreeCam == true )
		{
			IncreaseCameraFOV();
		}
		else
		{
			if ( avaHUD(myHUD).HasBroadcastAuthority() )
			{
				ServerViewNextSlotPlayer();
			}
			else
			{
				ServerViewNextPlayer();
			}
		}
	}

	exec function CheckSpectatorType()
	{
		if ( bPressedJump )
		{
			if ( avaGameReplicationInfo(WorldInfo.GRI).bAllowThirdPersonCam && bUseFreeCam == false )
			{
				ToggleOldView();
			}
			bPressedJump = false;
		}
	}

	exec function StartFire( optional byte FireModeNum )
	{
		if ( bUseFreeCam == false )
		{
			if ( avaHUD(myHUD).HasBroadcastAuthority() )
			{
				ServerViewNextSlotPlayer();
			}
			else
			{
				ServerViewNextPlayer();
			}
		}
	}

	exec function StartAltFire( optional byte FireModeNum )
	{
		if ( bUseFreeCam == false )
		{
			if ( avaHUD(myHUD).HasBroadcastAuthority() )
			{
				ServerViewChangeTeam();
			}
			else
			{
				ServerViewPrevPlayer();
			}
		}
	}

	reliable client event ClientSetViewTarget( Actor A, optional float TransitionTime )
	{
		if( A == None )
		{
			ServerVerifyViewTarget();
		}
		SetViewTarget( A, TransitionTime );
	}

	// ��û�� ���ؼ��� Restart �� �����ϵ��� �ϱ� ���ؼ� PlayerController ������ Spectating State ����
	// ClientRestart�� Ignore ���״�.... Auto Respawn �� �Ǳ� ���ؼ��� �ʿ����� ������?
	// 2006/04/25 by OZ
	reliable client function ClientRestart(Pawn NewPawn)
	{
		global.ClientRestart(NewPawn);
		ServerPlayerPreferences(WeaponHandPreference, bAutoTaunt, bCenteredWeaponFire);
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local float		VelSize;
		///* smoothly accelerate and decelerate */
		Acceleration = Normal(NewAccel) * class'Pawn'.Default.AirSpeed * SpectatorSpeedRate;
		VelSize = VSize(Velocity);
		if( VelSize > 0 )
		{
			Velocity = Velocity - (Velocity - Normal(Acceleration) * VelSize) * FMin(DeltaTime * 8, 1);
		}
		Velocity = Velocity + Acceleration * DeltaTime;
		if( VSize(Velocity) > class'Pawn'.default.AirSpeed * SpectatorSpeedRate )
		{
			Velocity = Normal(Velocity) * class'Pawn'.default.AirSpeed * SpectatorSpeedRate;
		}
		if( VSize(Velocity) > 0 )
		{
			MoveSmooth( (1+bRun) * Velocity * DeltaTime );
		}
		CheckSpectatorType();
	}
}

event BecomeViewTarget( PlayerController PC )
{
	if ( Role == ROLE_Authority )
	{
		if ( PC == self && !bUseFreeCam )
			ServerViewAPlayer( 1 );
	}
}

/**
 * This state is used when the player is out of the match waiting to be brought back in
 */

state InQueue extends Spectating
{
	simulated function EndState(Name NextStateName)
	{
		super.EndState(NextStateName);
		ServerViewSelf();
	}

}

exec function OpenMenu(string Which)
{
	if(LocalPlayer(Player) != None)
	{
		avaGameViewportClient(LocalPlayer(Player).ViewportClient).OpenMapMenu();
	}
}

// UI Kismet ���� �Է¹޾� ����
//exec function Talk()
//{
//	avaHUD(MyHUD).OpenChatScene();
//}
//
//exec function TeamTalk()
//{
//	avaHUD(MyHUD).OpenTeamChatScene();
//}

client reliable function ShowHudMap()
{
	if ( avaTeamMapHUD(MyHud) != none )
	{
		avaTeamMapHud(MyHud).ShowMap();
	}
}

/* epic ===============================================
* ::ClientGameEnded
*
* Replicated equivalent to GameHasEnded().
*
 * @param	EndGameFocus - actor to view with camera
 * @param	bIsWinner - true if this controller is on winning team
* =====================================================
*/
reliable client function ClientGameEnded(optional Actor EndGameFocus, optional bool bIsWinner)
{
	if( EndGameFocus == None )
		ServerVerifyViewTarget();
	else
		SetViewTarget(EndGameFocus);
	SetBehindView(true);
	if ( !PlayerReplicationInfo.bOnlySpectator )
		PlayWinMessage( bIsWinner );
	if ( !PlayerReplicationInfo.bOnlySpectator )	
	{
		GotoState('RoundEnded');
	}
	else
	{
		if (avaHUD(myHUD) != None)
		{
			avaHUD(myHUD).HideScores();
		}
	}
}

/* epic ===============================================
* ::RoundHasEnded
*
 * @param	EndRoundFocus - actor to view with camera
* =====================================================
*/
function RoundHasEnded(optional Actor EndRoundFocus)
{
	ClientRoundEnded(EndRoundFocus);
	//SetViewTarget(EndRoundFocus);
	// OnlySpectator �� ��� RoundEnded State �� ���� Spectating State �� �����ִ� code �� ����...
	if ( !PlayerReplicationInfo.bOnlySpectator )	GotoState('RoundEnded');
	if ( Pawn != None )
	{
		PawnDied( Pawn );
		UnPossess();
	}
}

/* epic ===============================================
* ::ClientRoundEnded
*
 * @param	EndRoundFocus - actor to view with camera
* =====================================================
*/
reliable client function ClientRoundEnded(Actor EndRoundFocus)
{
	// 2006/3/13 by OZ
	if ( EndRoundFocus != self )		SetBehindView(true);
	if ( avaHUD(myHUD) != None )		avaHUD(myHUD).RoundEnded();
	if ( GrenadeEffectSound != None )	GrenadeEffectSound.Stop();		// ����ź ȿ������ ������Ų��.
	ActiveFades.Length = 0;	// Flash Bang ȿ���� ������Ų��...
	if ( !PlayerReplicationInfo.bOnlySpectator )
	{
		GotoState('RoundEnded');
	}
	else
	{
		if (avaHUD(myHUD) != None)
			avaHUD(myHUD).HideScores();
	}
	if ( Pawn != None )
		Pawn = None;
}

/* epic ===============================================
* ::CheckBulletWhip
*
 * @param	BulletWhip - whip sound to play
 * @param	FireLocation - where shot was fired
 * @param	FireDir	- direction shot was fired
 * @param	HitLocation - impact location of shot
* =====================================================
*/
function CheckBulletWhip(soundcue BulletWhip, vector FireLocation, vector FireDir, vector HitLocation)
{
	local vector PlayerDir;
	local float Dist, PawnDist;

	if ( ViewTarget != None  )
	{
		// if bullet passed by close enough, play sound
		// first check if bullet passed by at all
		PlayerDir = ViewTarget.Location - FireLocation;
		Dist = PlayerDir Dot FireDir;
		if ( (Dist > 0) && ((FireDir Dot (HitLocation - ViewTarget.Location)) > 0) )
		{
			// check distance from bullet to vector
			PawnDist = VSize(PlayerDir);
			if ( Square(PawnDist) - Square(Dist) < 40000 )
			{
				// check line of sight
				if ( FastTrace(ViewTarget.Location + class'avaPawn'.default.BaseEyeheight*vect(0,0,1), FireLocation + Dist*FireDir) )
					PlaySound(BulletWhip, true,,,HitLocation);
			}
		}
	}
}
/* epic ===============================================
* ::PawnDied - Called when a pawn dies
*
 * @param	P - The pawn that died
* =====================================================
*/

function PawnDied(Pawn P)
{
	Super.PawnDied(P);
	ClientUnDuck();
	ClientCancelDash();

	ClientNotifyPawnDied();

	// Pawn �� �׾����� Progress Bar �� Reset �ؾ� ���� �ʳ�?
	avaPlayerReplicationInfo( PlayerReplicationInfo ).SetGauge(0,0);
	avaPlayerReplicationInfo( PlayerReplicationInfo ).SetWeaponStatusStr( "" );
}

reliable client function ClientNotifyPawnDied()
{
	if( LocalPlayer(Player) != None && avaGameViewportClient(LocalPlayer(Player).ViewportClient) != None )
		avaGameViewportClient(LocalPlayer(Player).ViewportClient).NotifyPawnDied();
	avaPlayerReplicationInfo( PlayerReplicationInfo ).SetWeaponStatusStr( "" );
}

/* epic ===============================================
* ::PawnDied - Stop ducking
*
*/
client reliable function ClientUnDuck()
{
	if (avaPlayerInput(PlayerInput) != none)
	{
		avaPlayerInput(PlayerInput).bDuck = 0;
	}
}

`devexec function WeaponTest()
{
	ServerWeaponTest();
}

server reliable function ServerWeaponTest()
{
}

// The player wants to alternate-fire.
exec function StartAltFire( optional Byte FireModeNum )
{
	if ( bIgnoreFireInput == true )		return;
	if ( bRequestSwapWeapon == true || Pawn == None )	return;
	StartFire( 1 );
}

exec function StopAltFire( optional byte FireModeNum )
{
	if (Pawn == None )	return;
	StopFire( 1 );
}

exec function ToggleBurstMode()
{
	local avaWeap_BaseRifle g;
	g = avaWeap_BaseRifle(Pawn.Weapon);
	if (g != none)
		g.SwitchToNextBurstMode();
}

//********************************************************************************************
// ���� Swap ����
//********************************************************************************************
exec function SwapDroppedWeapon()
{
	if ( avaPawn(Pawn).bIsDash )	return;		// Dash �߿��� Swap �� �� ����....
	// Weapon �� ���õ� Client �� Action �� ���ƾ� �Ѵ�...
	if ( bRequestSwapWeapon == true )
		return;
	bRequestSwapWeapon = true;
	ServerSwapDroppedWeapon();
}

reliable client function NotifySwapWeapon( bool bResult, optional avaWeapon ItemToRemove )
{
	bRequestSwapWeapon = false;

	if ( ItemToRemove != None && Pawn != None && Pawn.InvManager != None )
	{
		avaInventoryManager(Pawn.InvManager).ReserveRemovedItem( ItemToRemove );
	}
}

reliable server function ServerSwapDroppedWeapon()
{
	local avaPawn				avaPawn;
	local avaWeapon				InvWeapon;
	local int					i;
	local avaPickupProvider		avaPP;
	local avaWeapon				DropWeapon;

	avaPawn = avaPawn(Pawn);
	
	// PickUp Provider�� ���� Weapon Swap...
	foreach Pawn.OverlappingActors( class'avaPickupProvider', avaPP, Pawn.GetCollisionRadius() )
	{
		if ( avaPP.InventoryClass == None )		continue;
		if ( avaPP.bSwap == false )				continue;
		if ( avaPP.CanUse( Pawn ) == false )	continue;

		foreach avaPawn.InvManager.InventoryActors( class'avaWeapon', InvWeapon )
		{
			if( InvWeapon.InventoryGroup == avaPP.InventoryClass.default.InventoryGroup )
			{
				if ( avaPawn.CurrentWeapon.InventoryGroup == InvWeapon.InventoryGroup &&
					 avaPawn.CurrentWeapon != InvWeapon )
					 continue;
				if ( InvWeapon.CanThrow() )
				{
					if ( InvWeapon.ThrowWeapon( !avaPP.bDoNotSwitch ) == true )
					{
						avaPP.GiveTo( Pawn );
						NotifySwapWeapon( true, InvWeapon );
						return;
					}	
				}
			}
		}
	}


	for ( i = 0 ; i < avaPawn.TouchedPickUp.length ; ++ i )
	{
		ForEach avaPawn.InvManager.InventoryActors( class'avaWeapon', InvWeapon )
		{
			if( InvWeapon.InventoryGroup == avaPawn.TouchedPickUp[i].InventoryClass.default.InventoryGroup )
			{
				if ( avaPawn.CurrentWeapon.InventoryGroup == InvWeapon.InventoryGroup &&
					 avaPawn.CurrentWeapon != InvWeapon )
					 continue;
				if ( InvWeapon.CanThrow() )
				{
					if ( InvWeapon.ThrowWeapon( !avaPawn.TouchedPickUp[i].bDoNotSwitch ) == true )
					{
						avaPawn.TouchedPickUp[i].GiveTo( Pawn );
						NotifySwapWeapon( true, InvWeapon );
						return;
					}
				}
			}
		}
	}

	// ������ �ִ� ���⸦ ������ ����̴�....
	if ( avaPawn.CurrentWeapon.PickUpClass != class'avaSwappedPickUp' )
	{
		DropWeapon = avaPawn.CurrentWeapon;
		if ( avaPawn.CurrentWeapon.AbandonWeapon() == true )
		{
			NotifySwapWeapon( true, DropWeapon );
			return;
		}
	}

	NotifySwapWeapon( false );
}

reliable client function Client_BeginRound()
{
	local actor a;

	if ( Role != ROLE_Authority )
	{
		foreach dynamicActors(class'Actor', a)
		{
			if ( avaKActor(a) != none && avaKActor_Debris(a) == none )
			{
				if ( a.RemoteRole == ROLE_None )
					a.Reset();
			}
			else if (avaPawn(a)!=none)
			{
				avaPawn(a).Client_Reset();
			}
		}
	}
}

function bool FilterChat( out string Msg )
{
	local float				TimeLeft;
	local avaNetHandler		NetHandler;
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	TimeLeft = NetHandler.GetChatOffDue() - GetGlobalSeconds();
	if ( TimeLeft >= 0 )
	{
		if ( avaHUD(myHUD) != None )	
			avaHUD(myHUD).AddChatOffMessage( TimeLeft );
		return FALSE;
	}
	else
	{
		// ��Ģ�� ��ü
		NetHandler.FilterChatMsg(Msg);

		// �Ӹ��� ä�� �������ξ� �˻�.
		if ( !NetHandler.ParseChatCommand(Msg) )
		{
			Msg = Left(Msg,128);
			return TRUE;
		}
	}
	return FALSE;
}

exec function Say( string Msg )
{
	if ( FilterChat( Msg ) )
	{
		if ( AllowTextMessage(Msg) )
			ServerSay(Msg);
	}
}

exec function TeamSay( string Msg )
{
	if ( FilterChat( Msg ) )
	{
		if ( AllowTextMessage(Msg) )
			ServerTeamSay(Msg);
	}
}

unreliable server function ServerSay( string Msg )
{
	WorldInfo.Game.Broadcast(self, Msg, 'Say');
}

unreliable server function ServerTeamSay( string Msg )
{
	LastActiveTime = WorldInfo.TimeSeconds;
    WorldInfo.Game.BroadcastTeam( self, Msg, 'TeamSay');
}

// ���� ���� Rule....
function bool AllowTextMessage(string Msg)
{
	local int i;
	if ( PlayerReplicationInfo.bAdmin ||
		 avaPlayerReplicationInfo(PlayerReplicationInfo).bSilentLogin )
		 return true;

	// 1�ʿ� 4ȸ �̻� Chatting �� �� ����....
	if ( WorldInfo.TimeSeconds - BroadcastHistory[4].BroadcastTime < 1.0 )
		return false;

	BroadcastHistory[0].BroadcastString = Msg;
	BroadcastHistory[0].BroadcastTime   = WorldInfo.TimeSeconds;
	for ( i = 4 ; i > 0 ; i-- )
	{
		BroadcastHistory[i] = BroadcastHistory[i-1];
	}
	return true;
}

function NotifyChatOff()
{
	Local float TimeLeft;

	TimeLeft = class'avaNetHandler'.static.GetAvaNetHandler().GetChatOffDue() - GetGlobalSeconds();
	if( avaHUD(MyHUD) != None )
		avaHUD(MyHUD).AddChatOffMessage( TimeLeft ,  true );
}

reliable client event TeamMessage( PlayerReplicationInfo PRI, coerce string S, name Type, optional float MsgLifeTime, optional int TypeIdx  )
{
	Local AudioComponent AC;

	if ( Type == 'GameInfo' )
	{
		avaHUD(myHUD).GameInfoMessage( S, MsgLifeTime, , TypeIdx );
	}
	else
	{
		// �� �޼������� �޼����� �����ߴٸ� Ư����ɾ�(�Ӹ�, Ŭ��ê)�� �ƴѰ����� �����Ѵ�.
		// ���� FilterChat���� ��Ģ� ��ü�ȴ�.
		FilterChat(S);
		Super.TeamMessage( PRI, S, Type, MsgLifeTime );

		if( Type == 'Say' || Type == 'TeamSay' )
		{
			AC = WorldInfo.CreateAudioComponent( ChatNotifySound , false, true );
			AC.bAllowSpatialization = false;
			AC.bAutoDestroy = true;
			AC.Play();
		}
	}
}

reliable client function PlayDamageBreathSound()
{
	Local AudioComponent AC;
	AC = WorldInfo.CreateAudioComponent( DamageBreath , false, true );
	AC.bAllowSpatialization = false;
	AC.bAutoDestroy = true;
	AC.Play();
}

reliable client function ShowScores( bool bResult, bool bShow )
{
	if ( bShow )
	{
		avaHUD(myHUD).ShowScores();
		// ��ȸ�� Channel ���� �ڵ� ScreenShot ����� ���� �߰���.... 2008/01/18
		if ( avaHUD(myHUD).HasBroadcastAuthority() )
		{
			ConsoleCommand( "Shot" );
		}
	}
	else
	{
		avaHUD(myHUD).HideScores();
	}
}

unreliable client function AudioComponent ClientPlaySoundCue(SoundCue ASound, Actor SourceActor, float VolumeMultiplier, float PitchMultiplier, float FadeInTime)
{
	local AudioComponent AC;
	if (SourceActor != None)
	{
		AC = SourceActor.CreateAudioComponent(ASound,FALSE,TRUE,,,,AudioChannel_Body);
		AC.VolumeMultiplier = VolumeMultiplier;
		AC.PitchMultiplier = PitchMultiplier;
		AC.FadeIn(FadeInTime,1.f);
		AC.bAutoDestroy = TRUE;
	}
	return AC;
}

/**
	Spectator ���� �Լ���

	1. ViewTarget �� �׻� �ڽ��� Pawn �� �ƴ� ���� �ִ�.
	2. Server �� ��� SetBehindView �� ȣ���� �Ǿ��� ��� �����ؼ� ó���ϵ��� �Ѵ�.
**/

function SetBehindView(bool bNewBehindView)
{
	bBehindView = bNewBehindView;
	if ( avaPawn(ViewTarget) != None )
	{
		avaPawn(ViewTarget).CurrentCameraScale = 1.0;
		if ( IsLocalPlayerController() )
			avaPawn(ViewTarget).SetMeshVisibility(bBehindView);
	}

	if (LocalPlayer(Player) == None)
	{
		ClientSetBehindView(bNewBehindView);
	}
	// make sure we recalculate camera position for this frame
	LastCameraTimeStamp = WorldInfo.TimeSeconds - 1.0;
}

reliable client event ClientSetViewTarget( Actor A, optional float TransitionTime )
{
	Super.ClientSetViewTarget( A, TransitionTime );
	if ( A != None && Pawn != None && A != Pawn )
	{
		`warn( "********* avaPlayerController.ClientSetViewTarget PlayerController already has valid Pawn" @Pawn @A @Self );
	}
}

// Yaw ������ �����Ѵ�.
function Rotator LimitViewYawRotation( Rotator ViewRotation, float ViewYawMin, float ViewYawMax )
{
	if ( ViewRotation.Yaw > ViewYawMax )		ViewRotation.Yaw = ViewYawMax;
	else if ( ViewRotation.Yaw < ViewYawMin )	ViewRotation.Yaw = ViewYawMin;
	return ViewRotation;
}

function Rotator LimitViewPitchRotation( Rotator ViewRotation, float ViewPitchMin, float ViewPitchMax )
{
	if ( ViewPitchMin < 0 )
	{
		if( ViewRotation.Pitch > ViewPitchMax && ViewRotation.Pitch < (65535+ViewPitchMin) )
		{
			if( ViewRotation.Pitch < 32768 )
				ViewRotation.Pitch = ViewPitchMax;
			else
				ViewRotation.Pitch = 65535 + ViewPitchMin;
		}
	}
	else
	{
		if ( ViewRotation.Pitch > 32768 )	ViewRotation.Pitch -= 65535;

		if ( ViewRotation.Pitch > ViewPitchMax )		ViewRotation.Pitch = ViewPitchMax;
		else if ( ViewRotation.Pitch < ViewPitchMin )	ViewRotation.Pitch = ViewPitchMin;

	}
	return ViewRotation;
}


unreliable client function LongClientAdjustPosition( float TimeStamp, name NewState, EPhysics NewPhysics,
					float NewLocX, float NewLocY, float NewLocZ,
					float NewVelX, float NewVelY, float NewVelZ, Actor NewBase,
					float NewFloorX, float NewFloorY, float NewFloorZ )
{
	local avaPawn P;

	Super.LongClientAdjustPosition( TimeStamp, NewState, NewPhysics, NewLocX, NewLocY, NewLocZ,
					NewVelX, NewVelY, NewVelZ, NewBase, NewFloorX, NewFloorY, NewFloorZ );

	// allow changing location of rigid body pawn if feigning death
	P = avaPawn(Pawn);
	if (P != None && P.Physics == PHYS_RigidBody)
	{
		// the actor's location (and thus the mesh) were moved in the Super call, so we just need
		// to tell the physics system to do the same
		P.Mesh.SetRBPosition(P.Mesh.GetPosition());
	}
}

/* BroadCast functions for MessageClasses ( VoteMessage, WaypointMessage, LeaderMessage, ... ) */
function BroadcastTeamParam(class<avaLocalMessage> MessageClassName ,int Switch, avaMsgParam Param)
{

	if ( Param == None )
		ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None );
	else
		ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None ,
							Param.IntParam[0], Param.IntParam[1], Param.FloatParam[0], Param.FloatParam[1],
							Param.StringParam[0], Param.StringParam[1], Param.BoolParam1, Param.BoolParam2 );

	if( Param == None )
		BroadCastLocalizedTeamParam(MessageClassName, Switch);
	else
		BroadcastLocalizedTeamParam(MessageClassName, Switch,
								Param.IntParam[0], Param.IntParam[1], Param.FloatParam[0], Param.FloatParam[1],
								Param.StringParam[0], Param.StringParam[1], Param.BoolParam1, Param.BoolParam2);
}

function BroadcastLocalizedAll(class<avaLocalMessage> MessageClassName ,optional int Switch, optional avaMsgParam Param)
{
	if ( Param == None )
		ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None );
	else
		ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None ,
							Param.IntParam[0], Param.IntParam[1], Param.FloatParam[0], Param.FloatParam[1],
							Param.StringParam[0], Param.StringParam[1], Param.BoolParam1, Param.BoolParam2 );

	if ( Param != None )
	{
		ServerBroadcastLocalizedParam( MessageClassName ,Switch, Param.IntParam[0], Param.IntParam[1], Param.FloatParam[0], Param.FloatParam[1],
				  					   Param.StringParam[0], Param.StringParam[1], Param.BoolParam1, Param.BoolParam2 );
	}
	else
	{
		ServerBroadcastLocalizedParam( MessageClassName ,Switch );
	}
}

reliable server function ServerBroadcastLocalizedParam(class<avaLocalMessage> MessageClassName ,optional int Switch,
													   optional int n1,optional int n2, optional float f1, optional float f2,
													   optional string s1, optional string s2, optional bool b1, optional bool b2)
{
	local PlayerController PC;
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		// Sender ���״� ���״� ������ �ʴ´�.
		if ( PC.PlayerReplicationInfo != PlayerReplicationInfo )
			avaPlayerController(PC).ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None ,n1,n2,f1,f2,s1,s2,b1,b2);
	}
}

reliable server function ServerBroadcastLocalizedAll(class<avaLocalMessage> MessageClassName ,optional int Switch, optional avaMsgParam Param)
{
	local PlayerController PC;
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		// Sender ���״� ���״� ������ �ʴ´�.
		if ( PC.PlayerReplicationInfo != PlayerReplicationInfo )
			avaPlayerController(PC).ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None ,Param.IntParam[0], Param.IntParam[1], Param.FloatParam[0], Param.FloatParam[1],
							Param.StringParam[0], Param.StringParam[1], Param.BoolParam1, Param.BoolParam2);
	}
}

reliable server function BroadcastLocalizedTeamParam(class<avaLocalMessage> MessageClassName ,optional int Switch,
													 optional int n1,optional int n2, optional float f1, optional float f2,
													 optional string s1, optional string s2, optional bool b1, optional bool b2)
{
	local PlayerController PC;
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		// Sender ���״� ���״� ������ �ʴ´�.
		if ( PC.PlayerReplicationInfo != PlayerReplicationInfo && PC.PlayerReplicationInfo.Team == PlayerReplicationInfo.Team )
			avaPlayerController(PC).ReceiveLocalizedParam(MessageClassName, Switch, PlayerReplicationInfo, None , n1,n2,f1,f2,s1,s2,b1,b2);
	}
}

function ReceiveParam(class<avaLocalMessage> MessageClassName, int Switch, avaMsgParam Param, optional PlayerReplicationInfo PRI)
{
	if( Param == None )
		ReceiveLocalizedParam(MessageClassName, Switch, PRI != None ? PRI : PlayerReplicationInfo, None);
	else
		ReceiveLocalizedParam(MessageClassName, Switch, PRI != None ? PRI : PlayerReplicationInfo, None,
							Param.IntParam[0], Param.IntParam[1], Param.FloatParam[0], Param.FloatParam[1],
							Param.StringParam[0], Param.StringParam[1], Param.BoolParam1, Param.BoolParam2);

}

reliable client event ReceiveLocalizedParam( class<LocalMessage> Message, int Switch, PlayerReplicationInfo RelatedPRI_1, PlayerReplicationInfo RelatedPRI_2,
											optional int n1, optional int n2, optional float f1, optional float f2, optional string s1, optional string s2, optional bool b1, optional bool b2)
{
	Local avaMsgParam Param;
	// Wait for player to be up to date with replication when joining a server, before stacking up messages
	if ( WorldInfo.NetMode == NM_DedicatedServer || WorldInfo.GRI == None )
		return;

	Param = class'avaMsgParam'.static.Create();
	Param.SetInt(n1,n2);
	Param.SetFloat(f1,f2);
	Param.SetString(s1,s2);
	Param.SetBool(b1,b2);

	Message.Static.ClientReceive( Self, Switch, RelatedPRI_1, RelatedPRI_2, Param);
}

function RaiseAutoMessage(int MessageNum, bool bTeamCast = true)
{
	ServerRaiseAutoMessage( MessageNum , bTeamCast);
}

reliable server function ServerRaiseAutoMessage( int MessageNum , bool bTeamCast = true)
{
	local avaGame avaGame;
	avaGame = avaGame(WorldInfo.Game);
	// 1�ʾȿ� ���� AutoMessage �� Raise �Ǹ� ������ �ʴ´�.
	if ( LastAutoMessage == MessageNum && WorldInfo.TimeSeconds - LastAutoMessageTime < 1.0 )
		return;
	LastAutoMessage		= MessageNum;
	LastAutoMessageTime = WorldInfo.TimeSeconds;
	if(bTeamCast)
	{
		avaGame.BroadcastLocalizedTeam(Self, class'avaRadioAutoMessage', MessageNum, PlayerReplicationInfo );
	}
	else
	{
		avaGame.BroadcastAll(Self, class'avaRadioAutoMessage', MessageNum, PlayerReplicationInfo);
	}
}

/* Vote�� ���� function (Console Command, Server function - Ű�Է¿��� ������ , Client function - �������� ȭ��������� )*/

// Vote �� ���۵Ǿ���....
reliable client simulated function Client_StartVote( int Subject, avaPlayerReplicationInfo proposer, avaPlayerReplicationInfo targetPRI, float TimeLeft, int Param )
{
	if ( proposer == PlayerReplicationInfo )	ResponseKickVote();	
	avaGameReplicationInfo(WorldInfo.GRI).StartVote( Subject, proposer, targetPRI, TimeLeft, Param, targetPRI == PlayerReplicationInfo );
}

reliable client simulated function Client_EndVote( bool bResult )
{
	avaGameReplicationInfo(WorldInfo.GRI).EndVote( bResult );
}

reliable client simulated function Client_VoteAccepted( avaPlayerReplicationInfo voter, bool bResult )
{
	avaGameReplicationInfo(WorldInfo.GRI).VoteAccept( voter, bResult, voter == PlayerReplicationInfo );
}

// Vote�� ����ǥ�� �����ϴ�.
exec function AcceptVote()
{
	local avaNetHandler				NetHandler;
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	if ( WorldInfo.NetMode != NM_Standalone && NetHandler.IsVoteAvailable() == false )
	{
		avaHUD( myHUD ).AddGameMessage( class'avaVoteMessage'.default.MSG_VOTE_INSUFFICIENTMONEY );
		return;
	}

	if ( avaGameReplicationInfo( WorldInfo.GRI ).bVoting == true )
		Server_Vote( true );
}

// Vote�� �ݴ�ǥ�� �����ϴ�.
exec function DenyVote()
{
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bVoting == true )
		Server_Vote( false );
}

// Client �� ���� �ݴ�ǥ�� �޾ҽ��ϴ�.
reliable server function Server_Vote( bool bFlag )
{
	avaGameReplicationInfo(WorldInfo.GRI).Vote.PlayerVote( self, bFlag );
}

// Host�� ��û�� Kick Vote �� �����ϵ��� �մϴ�. 
simulated function ResponseKickVote()
{
	bEnableVote = false;
}

function ProcessViewRotation( float DeltaTime, out Rotator out_ViewRotation, Rotator DeltaRot )
{
	if( PlayerCamera != None )
	{
		PlayerCamera.ProcessViewRotation( DeltaTime, out_ViewRotation, DeltaRot );
	}

	if ( Pawn != None )
	{	// Give the Pawn a chance to modify DeltaRot (limit view for ex.)
		Pawn.ProcessViewRotation( DeltaTime, out_ViewRotation, DeltaRot );
	}
	else
	{
		// Add Delta Rotation
		out_ViewRotation	+= DeltaRot;
		out_ViewRotation	 = LimitViewRotation(out_ViewRotation, -16300, 16300 );
	}
}

reliable server function ServerVerifyViewTarget()
{
	local Actor TheViewTarget;

	TheViewTarget = GetViewTarget();

	if( TheViewTarget == Self )
	{
		return;
	}
	ClientSetViewTarget( TheViewTarget );
}

// �д������� ���� �Ǿ���....
unreliable client function NotifySquadLeader()
{
	ShowGIM( 21, 1 );
}

exec simulated function RequestInvManager()
{
	Server_RequestInvManager();
}

reliable server function Server_RequestInvManager()
{
	if ( Pawn != None )
		SetInvManager( Pawn.InvManager );
}

reliable client function SetInvManager( InventoryManager InvMan )
{
	Pawn.InvManager = InvMan;
}

`devexec simulated function ToggleFreeCam()
{
	if ( bUseFreeCam )	ChangeCameraMode( SPECTATORCAMMODE_NORMAL );
	else				ChangeCameraMode( SPECTATORCAMMODE_FREE );
}

reliable server function RequestChangeClass( int nClass )
{
	if ( nClass >= 0 && nClass < `MAX_PLAYER_CLASS )
		avaPlayerReplicationInfo( PlayerReplicationInfo ).SetPlayerClassID( nClass );
}

simulated function CameraModeChanged( int nMode )
{
	switch( nMode )
	{
	case 0 :	bUseFreeCam	= false;	
				Acceleration	= Vect(0.0,0.0,0.0);
		break;	// �Ϲ� Camera Mode �̴�...
	case 1 :	bUseFreeCam = true;	break;	// Free Camera Mode �̴�...
	case 2 :	break;							// ��ۿ� Camera reserved...
	}
	AdjustCameraCollision();
}

reliable server function ServerChangeCameraMode( int nMode )
{
	CameraModeChanged( nMode );
	switch( nMode )
	{
	case 0 :	ServerViewNextPlayer();	break;
	case 1 :	ServerViewSelf();		break;
	case 2 :	break;
	}	
}

simulated function ChangeCameraMode( int nMode )
{
	ServerChangeCameraMode( nMode );
	if ( Role < ROLE_Authority )
		CameraModeChanged( nMode );

	SetClearBackBufferFlag( bUseFreeCam  );
	avaHUD( myHUD ).ChangeCameraMode( nMode );

	if ( nMode == 1 && bPendingFreeCam == true )
	{
		PendingRotation.Roll = Rotation.Roll;
		SetLocation( PendingLocation - (CylinderComponent(CollisionComponent).CollisionHeight * Vect(0,0,1)) );
		SetRotation( PendingRotation );
		bPendingFreeCam = false;
	}
}

simulated function bool CanAvailableFreeCam()
{
	// FreeCam �� MyClan Channel ������ �����ϵ��� �Ѵ�...
`if( `isdefined(FINAL_RELEASE)  )
	local int ChannelFlag;
	local int Authority;
	ChannelFlag	=	class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag();	
	Authority = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelMaskLevel();
	if ( Authority >=2 || ( ( ChannelFlag == EChannelFlag_MyClan ||
						   ChannelFlag == EChannelFlag_Trainee || 
						   ChannelFlag == EChannelFlag_Clan ) && PlayerReplicationInfo.bOnlySpectator == true ) )
		return true;
	return false;
`endif
	return true;
}

//	Spectator Mode �϶��� Camera Mode Select ������ ���δ�.	2007/10/11
exec simulated function ChangeClass( int nClass )
{
	if ( avaPlayerReplicationInfo( PlayerReplicationInfo ).bOnlySpectator == true )
	{
		if ( CanAvailableFreeCam() == true )
			ChangeCameraMode( nClass );
	}
	else
	{
		if ( avaPlayerReplicationInfo( PlayerReplicationInfo ).PlayerClassID == nClass )
		{
			avaHUD(MyHud).ActivateClassSelectScene();
		}
		else
		{
			RequestChangeClass( nClass );
		}
	}
}

exec simulated function ToggleUIMode()
{
	if ( CanAvailableFreeCam() == FALSE )
		return;

	UIMode = ++UIMode%2;
	if ( UIMode == 0 )
	{
		if ( myHUD.bShowHUD == false )
			myHUD.ToggleHUD();
	}
	else if ( UIMode == 1 )	// UI �� �׸��� �ʴ´�...
	{
		if ( myHUD.bShowHUD == true )
		{
			avaHUD( myHUD ).NotifyHide();
			myHUD.ToggleHUD();
		}
	}
}

exec simulated function Weapon GiveWeapon( String WeaponClassStr )
{
	Local Weapon		Weap;
	local class<Weapon> WeaponClass;
	WeaponClass = class<Weapon>(DynamicLoadObject(WeaponClassStr, class'Class'));
	if ( WeaponClass == None )
		WeaponClass = class<Weapon>(DynamicLoadObject("avaRules.avaWeap_" $WeaponClassStr, class'Class'));
	if ( WeaponClass == None )
		WeaponClass = class<Weapon>(DynamicLoadObject("avaGame.avaWeap_" $WeaponClassStr, class'Class'));
	Weap		= Weapon(Pawn.FindInventoryType(WeaponClass));
	if( Weap != None )
		return Weap;
	Weap = Weapon(Pawn.CreateInventory( WeaponClass, true ));
	avaWeapon( Weap ).Loaded( true );
	return Weap;
}

exec function ChangeCharacterItem( string ModName )
{
	local avaUICharacter				Chr;
	local class<avaCharacterModifier>	Mod;
	local Pawn							P;

	`log("#### ChangeMod ####");

	foreach DynamicActors(class'Pawn', P)
	{
		if( P.IsA('avaUICharacter') )
		{
			Chr = avaUICharacter(P);
			if ( Chr != None )
			{
				Mod = class<avaCharacterModifier>(DynamicLoadObject( ModName, class'class' ));
				Chr.ChangeCharacterItem( Mod );
			}
		}
	}
}

exec function ChangeCharacter( string ModName )
{
	local avaUICharacter		Chr;
	local class<avaCharacter>	NewCharacter;
	local Pawn					P;

	`log("#### ChangeCharacter ####");

	foreach DynamicActors(class'Pawn', P)
	{
		if( P.IsA('avaUICharacter') )
		{
			Chr = avaUICharacter(P);
			if ( Chr != None )
			{
				NewCharacter = class<avaCharacter>(DynamicLoadObject( ModName, class'class' ));
				Chr.ChangeCharacter( NewCharacter );
			}
		}
	}
}

exec function ChangeWeapon( string ModName )
{
	local avaUICharacter		Chr;
	local class<avaMod_Weapon>	NewWeapon;
	local Pawn					P;

	`log("#### ChangeCharacter ####");

	foreach DynamicActors(class'Pawn', P)
	{
		if( P.IsA('avaUICharacter') )
		{
			Chr = avaUICharacter(P);
			if ( Chr != None )
			{
				NewWeapon = class<avaMod_Weapon>(DynamicLoadObject( ModName, class'class' ));
				Chr.ChangeWeapon( NewWeapon );
			}
		}
	}
}

exec function PlayTurnAnim()
{
	local avaUICharacter		Chr;
	local Pawn					P;

	`log("#### PlayTurnAnim ####");

	foreach DynamicActors(class'Pawn', P)
	{
		if( P.IsA('avaUICharacter') )
		{
			Chr = avaUICharacter(P);
			if ( Chr != None )
				Chr.PlayTurnAnim();
		}
	}
}

simulated function bool CanRestart()
{
	//`log("[dEAthcURe|avaPlayerController::CanRestart] IsInState( 'Dead' )"@IsInState( 'Dead' )@"IsDead()"@IsDead()@"IsInState( 'BaseSpectating' )"@IsInState( 'BaseSpectating' ));
	if ( !IsInState( 'Dead' ) && 
		 ( IsDead() || IsInState( 'BaseSpectating' ) ) &&
		 ( PlayerReplicationInfo != None && !PlayerReplicationInfo.bOnlySpectator ) )
		 return true;
	return false;
}

exec function DynamicMuzzleFlashes()
{
	local avaWeapon Weap;
	foreach DynamicActors(class'avaWeapon',Weap)
	{
		Weap.bDynamicMuzzleFlashes = TRUE;
		if( Weap.MuzzleFlashLight != none )
		{
			Weap.MuzzleFlashLight.CastDynamicShadows = TRUE;
		}
	}
}

reliable client function ClientEndGame()
{
	if ( Role < ROLE_Authority )
		avaGameReplicationInfo(WorldInfo.GRI).EndGame();
}

/** when spectating, tells server where the client is (client is authoritative on location when spectating) */
unreliable server function ServerSetSpectatorLocation(vector NewLoc)
{
}

// ��ũ��Ʈ�� appSeconds. ChatOffDue�� appSeconds�� ���� ������ 
// ChatOff�� ���� ������ �ð��� ���Ϸ��� �ʿ��ϴ�
native final function float GetGlobalSeconds();

// {{ dEAthcURe|HM
event Actor HmSpawnActor(String ClassName, vector Loc, rotator Rot)
{
	local Actor A;
	local class<actor> NewClass;
	
	NewClass = class<actor>( DynamicLoadObject( ClassName, class'Class' ) );
	if( NewClass!=None )
	{		
		A = Spawn(NewClass,,,Loc, Rot,,true); //[+] 20070302 no collision fail
	}
	
	return A;
}

// {{ 20070207 dEAthcURe|HM
event function HmSetCurrentWeapon(string weaponClassName)
{
	local class<avaWeapon>	weaponClass;
	local avaInventoryManager avaInvMan;
	
	weaponClass = class<avaWeapon>(DynamicLoadObject(weaponClassName, class'Class'));	
	
	`log("[dEAthcURe|HmSetCurrentWeapon] before ClientSetWeapon"  @weaponClass);
	Pawn.InvManager.ClientSetWeapon(weaponClass);
	
	avaInvMan = avaInventoryManager(Pawn.InvManager);
	avaInvMan.bHostMigrationComplete = true;
}
// }} 20070207 dEAthcURe|HM
// }} dEAthcURe|HM

function UpdateSignalPos( vector pos )
{
	local PlayerController	PC;

	foreach WorldInfo.AllControllers(class'PlayerController', PC)	
	{
		if ( PC.GetTeamNum() == GetTeamNum() )
		{
			avaPlayerController( PC ).ClientUpdateSignalPos( PlayerReplicationInfo, pos );
		}
	}
}

unreliable client simulated function ClientUpdateSignalPos( PlayerReplicationInfo pri, vector pos )
{
	if ( avaHUD(myHUD) != None )
	{
		avaHUD( myHUD ).SetSignalPos( Pos );
		ReceiveLocalizedMessage( class'avaRadioAutoMessage', AUTOMESSAGE_SelectTargetPoint, pri, PlayerReplicationInfo );
	}
}

/* ClientHearSound()
Replicated function from server for replicating audible sounds played on server
*/
simulated function PlaySoundEx(SoundCue ASound, Actor SourceActor, vector SourceLocation, bool bStopWhenOwnerDestroyed, optional bool bIsOccluded, optional EAudioChannel Channel )
{
	local AudioComponent AC;

//    `log("### ClientHearSound:"@ASound@SourceActor@SourceLocation@bStopWhenOwnerDestroyed@VSize(SourceLocation - Pawn.Location));

	if ( SourceActor == None )
	{
		AC = GetPooledAudioComponent(ASound, SourceActor, bStopWhenOwnerDestroyed, true, SourceLocation);
		if (AC == None)
		{
			return;
		}
		AC.bUseOwnerLocation = false;
		AC.Location = SourceLocation;
	}
	else if ( (SourceActor == GetViewTarget()) || (SourceActor == self) )
	{
		AC = GetPooledAudioComponent(ASound, None, bStopWhenOwnerDestroyed,,, Channel);
		if (AC == None)
		{
			return;
		}
		AC.bAllowSpatialization = false;
	}
	else
	{
		AC = GetPooledAudioComponent(ASound, SourceActor, bStopWhenOwnerDestroyed,,, Channel);
		if (AC == None)
		{
			return;
		}
		if (!IsZero(SourceLocation) && SourceLocation != SourceActor.Location)
		{
			AC.bUseOwnerLocation = false;
			AC.Location = SourceLocation;
		}
	}
	if ( bIsOccluded )
	{
		// if occluded reduce volume: @FIXME do something better
		AC.VolumeMultiplier *= 0.5;
	}
	AC.Play();
}

function StartKillCam( Actor InTarget, bool bIsKiller )
{
	ClientStartKillCam( InTarget, bIsKiller );
}

reliable client function ClientStartKillCam( Actor InTarget, bool bIsKiller )
{
	SetKillCam( InTarget, bIsKiller );
}

function SetKillCam( Actor targetActor, bool bIsKiller )
{
	if (class'avaOptionSettings'.Default.UseHUDCamera == true )
	{
		KillCamTargetActor = targetActor;
		KillCamStartedTime = WorldInfo.TimeSeconds;
		KillCam_bIsKiller = bIsKiller;
	}
}

function SetChatCam( Actor targetActor )
{
	if (class'avaOptionSettings'.Default.UseHUDCamera == true )
	{
		ChatCamActor = targetActor;
		ChatCamStartedTime = WorldInfo.TimeSeconds;
	}
}

exec function QuickChat( int ChatType )
{
	Local avaQuickChatUI QuickChatUI;
	Local LocalPlayer P;

	P = LocalPlayer(Player);
	if( P == None ) 
		return;
	QuickChatUI = avaGameViewportClient(P.ViewportClient).QuickChatUI;
	if( QuickChatUI == None )
		return;

	switch( ChatType )
	{
	case 1: QuickChatUI.OnInputKey(INDEX_NONE, 'QuickChatCommonCommand',0,IE_Pressed);	break;
	case 2:	QuickChatUI.OnInputKey(INDEX_NONE, 'QuickChatCommonResponse',0,IE_Pressed);	break;
	case 3:	QuickChatUI.OnInputKey(INDEX_NONE, 'QuickMemberCommand',0,IE_Pressed);	break;
	default: assert(false);	break;
	}
}

exec simulated function ToggleOldView()
{
	bUseOldThirdPersonCam = !bUseOldThirdPersonCam;
}

simulated function RestorePawnOffset()
{
	if ( PrevViewTarget	!= None )
	{
		SetViewtargetTempLocation( PrevViewTarget, SavedLoc, SavedRot );
	}
}

simulated function SetViewtargetTempLocation( Pawn P, vector loc, rotator rot )
{
	P.SetTempLocation( loc, rot );
}

`devexec simulated function ToggleInterp()
{
	bUseMeshInterpolating = !bUseMeshInterpolating;
}

`devexec simulated function ForceShoulderCam( bool bForce)
{
	bForceShoulderCam = bForce;
}

simulated function UpdatePawnOffset()
{
	local Pawn		P;
	local float		CurrentTime;
	local float		DeltaTime;
	local Actor		PawnBase;
	CurrentTime		= WorldInfo.TimeSeconds;
	DeltaTime		= CurrentTime - LastInterpTime;
	LastInterpTime	= CurrentTime;

	if ( avaPawn( ViewTarget ) != None )
		P = Pawn( ViewTarget );
	else if ( avaWeaponPawn( ViewTarget ) != None )
		P = avaWeaponPawn( ViewTarget ).MyVehicle;

	// �ڱ� �ڽ��̸� Interpolating �� �ʿ� ����...
	if ( P != None && ( bBehindView == true || bForceShoulderCam == true ) && bUseMeshInterpolating == true ) //&& ( P.Controller != self || WorldInfo.NetMode == NM_StandAlone ) )
	{
		// (Jump������)Base�� ���� ��쿣 WorldInfo���.
		if ( P.Base == None || P.Base.bStatic )
			PawnBase = WorldInfo;
		else
			PawnBase = P.Base;

		if ( PrevViewTarget != P )	// first rendering
		{
			// Base�� WorldInfo or InterpActor ���� �� �� �ִ�.(2007/10/09 ����)
			P.LastLoc = P.Location - PawnBase.Location;

//			P.LastLoc = P.Location;		//! ���� �ڵ�.
			P.LastRot = P.Rotation;
		}
		else
		{
			// Base�� �ٲ�� �����ǥ�� �ٽ� ����� �ش�.
			if ( PawnBase != LastBase )
				P.LastLoc = SavedLoc - PawnBase.Location;

			P.LastLoc = VInterpTo( P.LastLoc, P.Location - PawnBase.Location, DeltaTime, P.MeshInterpSpeedT );

//			P.LastLoc = VInterpTo( P.LastLoc, P.Location, DeltaTime, P.MeshInterpSpeedT );	//! ���� �ڵ�.
			P.LastRot = RInterpTo( P.LastRot, P.Rotation, DeltaTime, P.MeshInterpSpeedR );
		}
		SavedLoc = P.Location;
		SavedRot = P.Rotation;
		SetViewtargetTempLocation( P, PawnBase.Location + P.LastLoc, P.LastRot );
//		SetViewtargetTempLocation( P, P.LastLoc, P.LastRot );	//! ���� �ڵ�.
		PrevViewTarget = P;
		LastBase = PawnBase;
	}
	else
	{
		PrevViewTarget = None;
	}
}

simulated function bool UseShoulderView()
{
	local avaCharacter	TargetCharacter;
	TargetCharacter		=	avaCharacter( ViewTarget );
	if ( bForceShoulderCam )						return true;
	if ( TargetCharacter == None )					return false;
	else if ( TargetCharacter.bFixedView )			return false;
	else if ( TargetCharacter.IsFirstPerson() )		return false;
	else if ( bUseOldThirdPersonCam )				return false;
	else if ( TargetCharacter.IsInState('Dying')  )	return false;
	else if ( IsInState( 'Dead' ) )					return false;
	return true;
}

exec simulated function UseLocalSound( bool bUse)
{
	avaGameReplicationInfo(WorldInfo.GRI).UseLocalSound = bUse;
}	

function HandlePickup(Inventory Inv)
{
	ReceiveLocalizedMessage(Inv.MessageClass,,,,Inv.class);
	if ( avaWeapon(Inv) != None && avaWeapon(Inv).GIMIndexWhenPickUp >= 0 )
		ShowGIM( avaWeapon(Inv).GIMIndexWhenPickUp, 1 );
}

reliable client function GetDogTag( optional bool bPack )
{
	if ( bPack == true )
	{
		ClientPlaySound( SoundCue'avaItemSounds.dogTag.DogTag_PackCount' );
		ShowGIM( 103, 1 );
	}
	else
	{
		ClientPlaySound( SoundCue'avaItemSounds.dogTag.DogTag_Get' );
		ShowGIM( 104, 1 );
	}
}

reliable client simulated function ShowGIM( int nIndex, optional int nType = -1, optional float fTime = 3.0f )
{
	gameInfoMsg.ShowGameInfoMessage( nIndex, self, nType, fTime );
}

simulated function ClearGIM( int nType )
{
	if ( avaHUD( myHUD ) != None )
		avaHUD( myHUD ).ClearGameInfoMessage( nType );
}

reliable client function SetTeam( int nTeam )	
{
	if ( IsLocalPlayerController() )
		avaGameReplicationInfo( WorldInfo.GRI ).LoadLocalizedTeamPack( nTeam, self );
}

simulated event NotifyTakeScreenShot( optional string ScreenShotPath )
{
	Local PlayerController PC;
	foreach LocalPlayerControllers(PC)
	{
		avaHUD(PC.MyHUD).AddGameMessage( ScreenShotPath$ " " $class'avaLocalizedMessage'.default.TakeScreenShot );
		PC.ClientPlaySound( SoundCue'avaGameSound.Screen_shot_sound' );
	}
}

exec function AddTypedChat( string Msg, int MsgType )
{
	avaHUD(MyHUD).AddTypedMessage(Msg, EChatMsgType(MsgType));
}

reliable client simulated function ReportEndGameToServer()
{
	if ( Role < ROLE_Authority )
		avaGameReplicationInfo( WorldInfo.GRI ).ReportEndGameToServer();
}

// Kick �� ������ Player List �� ���´�...
simulated function InitKickablePlayerList( int MinimumVoters )
{
	local avaPlayerReplicationInfo	avaPRI;
	local int						i;
	local int						index;
	local int						LocalTeam;
	local avaGameReplicationInfo	avaGRI;
`if( `isdefined(FINAL_RELEASE) )
	local avaNetHandler				NetHandler;
`endif
	
	LocalTeam = GetTeamNum();
	KickablePlayerList.length = 0;

	if ( PlayerReplicationInfo.bOnlySpectator == true )	
		return;	// Spectator �� ���� ��ǥ��....

	if ( bEnableVote == false )
	{
		avaHUD( myHUD ).AddGameMessage( class'avaVoteMessage'.default.MSG_VOTE_DISABLE );
		return;
	}

`if( `isdefined(FINAL_RELEASE) )

	// ��ǥ�� �������� ������� Check �Ͽ� �����ϴٸ� Message �� ����ֵ��� �Ѵ�...
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	if ( WorldInfo.NetMode != NM_Standalone && NetHandler.IsVoteAvailable() == false )
	{
		avaHUD( myHUD ).AddGameMessage( class'avaVoteMessage'.default.MSG_VOTE_INSUFFICIENTMONEY_FOR_PROPOSAL );
		return;
	}
`endif

	avaGRI = avaGameReplicationInfo( WorldInfo.GRI );
	if ( avaGRI.bVoting == true )
	{
		avaHUD( myHUD ).AddGameMessage( class'avaVoteMessage'.default.MSG_VOTE_PROGRESS );
		return;	// Voting ���̸� ��ǥ �� �� ����...
	}
	for ( i = 0 ; i < WorldInfo.GRI.PRIArray.length ; ++ i )
	{
		avaPRI = avaPlayerReplicationInfo( WorldInfo.GRI.PRIArray[i] );
		if ( avaPRI == None )						continue;		// PRI �� ���� ��鵵 �����Ѵ�.
		if ( avaPRI == PlayerReplicationInfo )		continue;		// �ڽ��� �����Ѵ�.
		if ( avaPRI.bSilentLogIn == true )			continue;		// ��ڵ� �����Ѵ�..
		if ( avaPRI.bBot == true )					continue;		// A.I�� �����Ѵ�.. 

		if ( WorldInfo.GRI.GameClass.default.bTeamGame == true )
		{
			if ( avaPRI.Team.TeamIndex >= 0 && avaPRI.Team.TeamIndex <= 1 && avaPri.Team.TeamIndex != LocalTeam )
				continue;
		}
		index = KickablePlayerList.Length;
		KickablePlayerList.length = index + 1;
		KickablePlayerList[index].PRI			=	avaPRI;
		if ( avaPRI.bOnlySpectator )	KickablePlayerList[index].PlayerName = "["$class'avaGameReplicationInfo'.default.TeamNames[2]$"]"$avaPRI.PlayerName;
		else							KickablePlayerList[index].PlayerName =	avaPRI.PlayerName;
		KickablePlayerList[index].AccountID		=	avaPRI.AccountID;
	}
	if ( KickablePlayerList.length <= MinimumVoters )
	{
		avaHUD( myHUD ).AddGameMessage( class'avaVoteMessage'.default.MSV_VOTE_INSUFFICIENTVOTERS );
		KickablePlayerList.length = 0;
		return;	// ������ Vote �ο� �� ����...
	}
}

exec simulated function SetSSR( float fRate )
{
	SpectatorSpeedRate = fRate;
}

exec simulated function SetMS( float fRate )
{
	MouseSensitivityEx = fRate;
}

simulated function SetMouseSensitivityEx( float sensitivity = 1.0 )
{
	MouseSensitivityEx = sensitivity;
}

reliable client simulated function NotifyStartUse( avaUseVolume vol )
{
	if ( UseSound != None )	
		UseSound.Stop();
	if ( vol.UseSoundCue != None )
	{
		UseSound = ClientPlaySoundCue( vol.UseSoundCue, Self, 1, 1, -1);
	}
}

reliable client simulated function NotifyStopUse( avaUseVolume vol )
{
	if ( UseSound != None )	
		UseSound.Stop();
	UseSound = None;
}

/* ClientHearSound()
Replicated function from server for replicating audible sounds played on server
*/
unreliable client event ClientHearSound(SoundCue ASound, Actor SourceActor, vector SourceLocation, bool bStopWhenOwnerDestroyed, optional bool bIsOccluded, optional EAudioChannel Channel )
{
	local AudioComponent AC;

    //`log("### ClientHearSound:"@ASound@SourceActor@SourceLocation@bStopWhenOwnerDestroyed@VSize(SourceLocation - Pawn.Location));

	if ( SourceActor == None )
	{
		AC = GetPooledAudioComponent(ASound, SourceActor, bStopWhenOwnerDestroyed, true, SourceLocation);
		if (AC == None)
		{
			return;
		}
		AC.bUseOwnerLocation = false;
		AC.Location = SourceLocation;
	}
	else if ( (SourceActor == GetViewTarget()) || (SourceActor == self) || (SourceActor == Pawn) )	
	{
		AC = GetPooledAudioComponent(ASound, None, bStopWhenOwnerDestroyed,,, Channel);
		if (AC == None)
		{
			return;
		}
		AC.bAllowSpatialization = false;
	}
	else
	{
		AC = GetPooledAudioComponent(ASound, SourceActor, bStopWhenOwnerDestroyed,,, Channel);
		if (AC == None)
		{
			return;
		}
		if (!IsZero(SourceLocation) && SourceLocation != SourceActor.Location)
		{
			AC.bUseOwnerLocation = false;
			AC.Location = SourceLocation;
		}
	}
	if ( bIsOccluded )
	{
		// if occluded reduce volume: @FIXME do something better
		AC.VolumeMultiplier *= 0.5;
	}
	AC.Play();
}

`devexec function OpenSceneByTag( string SceneTag )
{
	Local UIScene SceneInst;

	SceneInst = UIScene(DynamicLoadObject( SceneTag , class'UIScene' ));
	if( SceneInst == none )
	{
		`warn("there's no scene named "$SceneTag );
		return;
	}
	
	class'avaNetHandler'.static.GetAvaNetHandler().OpenSceneManaged( SceneInst );
}

// Test �� Console ��ɾ�....


`devexec function GenAvaNetMsg( BYTE Cat, BYTE Msg, string s1, string s2, int n1, int n2 )
{
	Local AvaNetHandler NetHandler;
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	switch( Cat )
	{
	case 0:	NetHandler.ProcClientMessage(EMsgClient(Msg), s1, s2, n1, n2);			break;
	case 1:	break;
	case 2:	NetHandler.ProcRoomMessage(EMsgRoom(Msg), s1, s2, n1, n2);				break;
	case 3:	NetHandler.ProcGameMessage(EMsgGame(Msg), s1, s2, n1, n2);				break;
	case 4:	NetHandler.ProcInventoryMessage(EMsgInventory(Msg), s1, s2, n1, n2);	break;
	case 5:	NetHandler.ProcGuildMessage(EMsgGuild(Msg), s1, s2, n1, n2);			break;
	case 6:	break;
	case 7:	break;
	case 8:	break;
	}
}

exec function DecreaseCameraFOV()
{
	ModifyCameraFOV( -5 );
}

exec function IncreaseCameraFOV()
{
	ModifyCameraFOV( 5 );
}

exec function ModifyCameraFOV( float ModFOV )
{
	local string	Msg;
	local float		PrvFOV;
	if ( !bUseFreeCam )
		return;
	PrvFOV = GetFOVAngle();
	ClientFOV( Clamp( GetFOVAngle() + ModFOV, 30, 150 ) );
	if ( PrvFOV != GetFOVAngle() )
	{
		Msg = class'avaStringHelper'.static.Replace( ""$PrvFOV , "%d", class'avaLocalizedMessage'.default.strChangeFOV );
		MyHUD.Message( PlayerReplicationInfo, Msg, '' );
	}
}

exec function DecreaseCameraSpeed()
{
	ModifyCameraSpeed( -0.05 );
}

exec function IncreaseCameraSpeed()
{
	ModifyCameraSpeed( 0.05 );
}

function ModifyCameraSpeed( float ModSpeed )
{
	local string Msg;
	if ( !CanAvailableFreeCam() )
		return;
	SpectatorSpeedRate = Clamp( SpectatorSpeedRate + ModSpeed, 0.0, 10.0 );
	Msg =  class'avaStringHelper'.static.Replace( ""$SpectatorSpeedRate , "%d", class'avaLocalizedMessage'.default.strChangeSpectatorSpeed );
	MyHUD.Message( PlayerReplicationInfo, Msg, '' );
}

// ���� ���� Team �ΰ��� Check �Ѵ�...
// ��۽� Spectator �� � Team �ΰ��� Check �ϱ� ���ؼ��� ���ȴ�...
simulated event function bool IsSameTeam( Pawn TargetPawn )
{
	if ( Pawn == TargetPawn )	return TRUE;		// �ڱ� Pawn �̸�...
	return IsSameTeamByIndex( TargetPawn.GetTeamNum() );
}

simulated event function bool IsSameTeamByIndex( int nTeam )
{
	local int TeamNum;

	// Team Game ���� Team �� 0 �̳� 1�� ������ �Ǿ� ���� �ʴٸ� ���� Team ���� ����....
	if ( WorldInfo.GRI.GameClass.default.bTeamGame == true && nTeam != 0 && nTeam != 1 )
		return true;

	if ( PlayerReplicationInfo.bOnlySpectator == false )
	{
		TeamNum	=	GetTeamNum();
		if ( TeamNum == 255 )	// TeamNum �� 255 �̸� DeathMatch �̴�... ���� ��ΰ� �� ���̴�....
			return false;
		else
			return ( TeamNum == nTeam );
	}

	return true;
}

// Save & Load Camera �� Free Cam �� ���� �����մϴ�....
exec event function SaveCamera( int SlotNum )
{
	local vector CamLoc;
	local rotator CamRot;

	if ( !bUseFreeCam )
		return;

	PlayerCamera.GetCameraViewPoint(CamLoc,CamRot);
	class'avaSavedCam'.static.SaveCamData( GetMapFileName(), SlotNum, CamLoc, CamRot );
	avaHUD(myHUD).AddGameMessage( SlotNum$class'avaLocalizedMessage'.default.NotifySavedCamPos );
}

exec event function LoadCamera( int SlotNum )
{
	local vector	CamLoc;
	local rotator	CamRot;

	if ( !bUseFreeCam )
	{
		ServerViewSlotPlayer( SlotNum );
		return;
	}

	if ( class'avaSavedCam'.static.GetCamData( GetMapFileName(), SlotNum, CamLoc, CamRot ) == false )
		return;

	Velocity = Vect(0,0,0);
	SetLocation( CamLoc );
	SetRotation( CamRot );
}

function AdjustCameraCollision()
{
	if ( bUseFreeCam )
	{
		bCollideWorld	=	bCameraCollision;
	}
	else
	{
		bCollideWorld	=	true;
	}
}

exec function ToggleCameraCollision()
{
	bCameraCollision	=	!bCameraCollision;
	AdjustCameraCollision();

	if ( bCameraCollision )
		avaHUD(myHUD).AddGameMessage( class'avaLocalizedMessage'.default.NotifyCameraCollisionOn );
	else
		avaHUD(myHUD).AddGameMessage( class'avaLocalizedMessage'.default.NotifyCameraCollisionOff );
}

// Night Vision ���� ȿ���� �������� �ʴ´�...
exec function ToggleCameraEffect()
{
	bDisableCameraEffect	=	!bDisableCameraEffect;

	if ( bDisableCameraEffect )
	{
		avaGameViewportClient(LocalPlayer(Player).ViewportClient).NightVisionEffect.bShowInGame = false;
		avaPlayerCamera( PlayerCamera ).bEnableColorScaling = false;
		avaHUD(myHUD).AddGameMessage( class'avaLocalizedMessage'.default.NotifyDisableCameraEffect );
	}
	else
		avaHUD(myHUD).AddGameMessage( class'avaLocalizedMessage'.default.NotifyEnableCameraEffect );
}

exec event simulated function ToggleConsoleUI()
{
	avaHUD(myHUD).ToggleConsoleUI();
}

function GetViewtargetInfo( out int nSlotNum, out int nTeam )
{
	local avaPlayerReplicationInfo	ViewtargetPRI;
	if ( RealViewTarget != None )
	{
		ViewtargetPRI = avaPlayerReplicationInfo( RealViewtarget );
	}
	else
	{
		 if ( avaPawn( ViewTarget ).PlayerReplicationInfo != None )
			 ViewtargetPRI = avaPlayerReplicationInfo( avaPawn( ViewTarget ).PlayerReplicationInfo );
		 else
			 ViewtargetPRI = avaPlayerReplicationInfo( avaPawn( ViewTarget ).DrivenVehicle.PlayerReplicationInfo );
	}

	nSlotNum = ViewtargetPRI.SlotNum;
	if ( ViewtargetPRI.Team != None )	nTeam = ViewtargetPRI.Team.TeamIndex;
	else								nTeam = 255;
}

function ChangeViewtarget( Pawn TargetPawn )
{
	if ( avaWeaponPawn( TargetPawn ) != None )
		TargetPawn = avaWeaponPawn( TargetPawn ).Driver;
	SetViewTarget( TargetPawn );
	if ( Role == ROLE_Authority && RemoteRole == ROLE_SimulatedProxy )
		SetBehindView( true );
}

reliable server function ServerInitViewPlayer()
{
	ServerViewAPlayer(+1);
}

unreliable server function ServerViewNextPlayer()
{
	ServerViewAPlayer(+1);
}

unreliable server function ServerViewPrevPlayer()
{
	ServerViewAPlayer(-1);
}

// Get all PRI's that are viewable, then sort by PlayerID
function bool GetViewablePlayerList( out array<PlayerReplicationInfo> PRIList, optional bool bSort )
{
	local Controller			PC;
	local PlayerReplicationInfo	PRI;
	local int					i;

	foreach WorldInfo.AllControllers( class'Controller', PC )
	{
		if ( PC == self )														continue;
		if ( !WorldInfo.Game.CanSpectate( self, PC.PlayerReplicationInfo ) )	continue;
		if ( PC.IsDead() )														continue;
		if ( PC.Pawn == None )													continue;
		if ( PC.IsSpectating() )												continue;
		PRI = PC.PlayerReplicationInfo;
		if ( PRI.bBot == true && bDisplayBotInfo == false )						continue;

		if ( bSort == true )
		{
			for ( i = 0 ; i < PRIList.Length ; ++i )
			{
				if ( PRIList[i].PlayerID > PRI.PlayerID )
					break;
			}
			if ( i < PRIList.Length )
			{
				PRIList.Insert( i, 1 );
				PRIList[i] = PRI;
			}
			else
			{
				PRIList.Length = i+1;
				PRIList[i] = PRI;
			}
		}
		else
		{
			PRIList[PRIList.Length] = PRI;
		}
	}
	return ( PRIList.Length > 0 );
}

function ServerViewAPlayer(int dir)
{
	local array<PlayerReplicationInfo>	PRIList;
    local int							i,index, SpectatePlayerID;

	if ( Pawn != None && Pawn.Health > 0 )
		return;

	if ( !GetViewablePlayerList( PRIList, true ) )
		return;

	if( RealViewTarget != none )	SpectatePlayerID = RealViewTarget.PlayerID;
	else							SpectatePlayerID = -1;

	index = -1;
	if( dir >=  0)
	{
		for(i=0; i<PRIList.Length; i++)
		{
			if( PRIList[i].PlayerID > SpectatePlayerID )
			{
				index = i;
				break;
			}
		}
		if( index == -1 && PRIList.Length > 0 )
			index = 0;
	}
	else
	{
		for(i= PRIList.Length-1 ; i >= 0; i--)
		{
			if( PRIList[i].PlayerID < SpectatePlayerID )
			{
				index = i;
				break;
			}
		}
		if( index == -1 && PRIList.Length > 0 )
			index = PRIList.Length-1;
	}

	if( index != -1 )
	{
		ChangeViewtarget( Controller(PRIList[Index].Owner).Pawn );
	}
}

// ���� Viewtarget �� ���� ���� Ư�� Slot Player �� ������ �Ѵ�...
unreliable server function ServerViewSlotPlayer( int nSlot )
{
	local array<avaPlayerReplicationInfo>	PRIList;
	local int								nSlotNum, nTeamNum;
	local int								i, index;

	if ( Pawn != None && Pawn.Health > 0 )
		return;

	GetViewtargetInfo( nSlotNum, nTeamNum );
	// Slot �� Zero Base �̱� ������...
	nSlot -= 1;
	GetViewablePlayerListBySlotNum( PRIList, nTeamNum );

	index = -1;
	for ( i = 0 ; i < PRIList.length ; ++ i )
	{
		if ( PRIList[i].SlotNum == nSlot || ( PRIList[i].SlotNum - `MAX_TEAM_SLOT ) == nSlot )
		{
			index = i;
			break;
		}
	}

	if( index != -1 )
	{
		ChangeViewtarget( Controller(PRIList[Index].Owner).Pawn );
	}
}

// ���� viewtarget �� �ٸ� team �� player �� ������ �Ѵ�...
unreliable server function ServerViewChangeTeam()
{
	local array<avaPlayerReplicationInfo>	PRIList;
	local int								nSlotNum, nTeamNum;
	local int								index;

	if ( Pawn != None && Pawn.Health > 0 )
		return;

	GetViewtargetInfo( nSlotNum, nTeamNum );
	
	if ( nTeamNum == 0 )		nTeamNum = 1;
	else if ( nTeamNum == 1 )	nTeamNum = 0;

	GetViewablePlayerListBySlotNum( PRIList, nTeamNum );

	index = 0;

	if( index != -1 )
	{
		ChangeViewtarget( Controller(PRIList[Index].Owner).Pawn );
	}
}

function ServerViewASlotPlayer( int dir )
{
	local array<avaPlayerReplicationInfo>	PRIList;
	local int								nSlotNum, nTeamNum;
	local int								index,i;
	local int								DstSlotNum;
	if ( Pawn != None && Pawn.Health > 0 )
		return;

	GetViewtargetInfo( nSlotNum, nTeamNum );
	GetViewablePlayerListBySlotNum( PRIList, nTeamNum );

	if ( nTeamNum != 255 )
		nSlotNum += nTeamNum * `MAX_TEAM_SLOT;

	if ( dir > 0 )
	{
		index = 0;
		for ( i = 0 ; i < PRIList.length ; ++ i )
		{
			DstSlotNum = PRIList[i].SlotNum;
			if ( PRIList[i].Team != None )
				DstSlotNum += PRIList[i].Team.TeamIndex * `MAX_TEAM_SLOT;

			if ( DstSlotNum > nSlotNum )
			{
				index = i;
				break;
			}
		}
	}
	else
	{
		index = PRIList.length - 1;
		for ( i = PRIList.length - 1 ; i >= 0  ; -- i )
		{
			DstSlotNum = PRIList[i].SlotNum;
			if ( PRIList[i].Team != None )
				DstSlotNum += PRIList[i].Team.TeamIndex * `MAX_TEAM_SLOT;
			if ( DstSlotNum < nSlotNum )
			{
				index = i;
				break;
			}
		}
	}

	if( index != -1 )
	{
		ChangeViewtarget( Controller(PRIList[Index].Owner).Pawn );
	}
}

// ���� viewtarget �� ���� Slot Player �� ������ �Ѵ�..
unreliable server function ServerViewNextSlotPlayer()
{
	ServerViewASlotPlayer( 1 );
}

unreliable server function ServerViewPrevSlotPlayer()
{
	ServerViewASlotPlayer( -1 );
}

function bool GetViewablePlayerListBySlotNum( out array<avaPlayerReplicationInfo> PRIList, optional BYTE nTeam = 255 )
{
	local PlayerController			PC;
	local avaPlayerReplicationInfo	PRI;
	local int						i;

	local int						SrcSlotNum;
	local int						DstSlotNum;

	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		if ( PC == self )														continue;
		if ( !WorldInfo.Game.CanSpectate( self, PC.PlayerReplicationInfo ) )	continue;
		if ( PC.IsDead() )														continue;
		if ( PC.Pawn == None )													continue;
		if ( PC.IsSpectating() )												continue;
		if ( nTeam != 255 && PC.GetTeamNum() != nTeam )							continue;
		PRI = avaPlayerReplicationInfo( PC.PlayerReplicationInfo );

		SrcSlotNum = PRI.SlotNum;

		if ( PRI.Team != None )
			SrcSlotNum += PRI.Team.TeamIndex * `MAX_TEAM_SLOT;

		for ( i = 0 ; i < PRIList.Length ; ++i )
		{
			DstSlotNum = PRIList[i].SlotNum;
			if ( PRIList[i].Team != None )
			{
				DstSlotNum += PRIList[i].Team.TeamIndex * `MAX_TEAM_SLOT;
			}
			if ( DstSlotNum > SrcSlotNum )
				break;
		}
		if ( i < PRIList.Length )
		{
			PRIList.Insert( i, 1 );
			PRIList[i] = PRI;
		}
		else
		{
			PRIList.Length = i+1;
			PRIList[i] = PRI;
		}
	}
	return ( PRIList.Length > 0 );
}

/**************************************************************************************************
	Functons For Testing
**************************************************************************************************/
`devexec function ToggleDisplayBotInfo()
{
	bDisplayBotInfo = !bDisplayBotInfo;
}

`devexec function TestAllTargetted()
{
	local avaPawn	P;
	foreach DynamicActors(class'avaPawn', P)
		P.SetTargetted( true );
}

`devexec simulated function TestEndGame()
{
	TestServerEndGame();
}

reliable server function TestServerEndGame()
{
	avaGame( WorldInfo.Game ).EndGameEx( None, "", WINTYPE_Annihilation );
}

`devexec simulated function TestEndRound()
{
	TestServerEndRound();
}

reliable server function TestServerEndRound()
{
	avaGame( WorldInfo.Game ).EndRoundEx( None, "", 0 );
}

`devexec function TestChatCam()
{
	SetChatCam( Pawn );
}

`devexec function TestKillCam1()
{
	ClientStartKillCam( Pawn, true );
}

`devexec function TestKillCam2()
{
	ClientStartKillCam( Pawn, false );
}

`devexec function LockRotation()
{
	bLockRotation = !bLockRotation;
}

`devexec function Energizer()
{
	if ( WorldInfo.NetMode == NM_Standalone )
		bInfinityBattery = !bInfinityBattery;
}

`devexec function TestWarfare()
{
	avaGameReplicationInfo(WorldInfo.GRI).ReadyForWarfare();
}

`devexec simulated function TestGIM( int nIndex, int nType )
{
	ShowGIM( nIndex, nType );
}

reliable server function debug_gamespeed( float speed )
{
	avaGame( WorldInfo.Game ).bAllowMPGameSpeed = true;
	avaGame( WorldInfo.Game ).SetGameSpeed( speed );
}

exec function sv_gamespeed( float speed )
{
	debug_gamespeed( speed );
}

`devexec simulated function TeamChange( int nTeam )
{
	ServerChangeTeam( nTeam );
}

reliable server function ServerNoDamage()
{
	bGodMode = !bGodMode;
	if (bGodMode)
		ServerSay( PlayerReplicationInfo.PlayerName @ " turns on GodMode cheat, he becomes a cheater!!!" );
	else
		ServerSay( PlayerReplicationInfo.PlayerName @ " turns off GodMode cheat. no cheat anymore" );
}

`devexec simulated function TestSwapTeam()
{
	avaGame( WorldInfo.Game ).SwapTeam();
}

`devexec simulated event function TestAddScore( int nScore )
{
	avaPlayerReplicationInfo( PlayerReplicationInfo ).AddScore( nScore );
}

`devexec simulated function TestAddPoint( int nScore )
{
	avaPlayerReplicationInfo( PlayerReplicationInfo ).AddPoint( PointType_Attack, nScore );
}

`devexec simulated function TestScoreTeam( int nScore )
{
	PlayerReplicationInfo.Team.Score += nScore;
	avaGameReplicationInfo(WorldInfo.GRI).nWinTeam = PlayerReplicationInfo.Team.TeamIndex;
}

`devexec simulated function ToggleRagdoll()
{
	bDisableRagdoll = !bDisableRagdoll;
	MyHUD.Message(PlayerReplicationInfo, "Ragdoll : "$(bDisableRagdoll ? "off" : "on"), '');
}

`devexec simulated function TogglePlayTakeHitRagdoll()
{
	bDisablePlayTakeHitRagdoll = !bDisablePlayTakeHitRagdoll;
	MyHUD.Message(PlayerReplicationInfo, "PlayTakeHitRagdoll : "$(bDisablePlayTakeHitRagdoll ? "off" : "on"), '');
}

`devexec simulated function NoDamage()
{
	ServerNoDamage();
}

`devexec function StopCountDown( bool bFlag )
{
	avaGame(WorldInfo.Game).bStopCountDown = bFlag;
}

`devexec simulated function TestReportEndGame()
{
	avaGameReplicationInfo( WorldInfo.GRI ).ReportEndGameToServer();
}

`devexec simulated function ToggleSquadLeader()
{
	avaPlayerReplicationInfo( PlayerReplicationInfo ).bSquadLeader = !avaPlayerReplicationInfo( PlayerReplicationInfo ).bSquadLeader;
}

`devexec simulated function ExplodeAllKActor()
{
	local avaKActor	kactor;
	foreach AllActors(class'avaKActor', kactor)
	{
		kactor.TakeDamage( 10000, self, vect(0,0,0), vect(1,1,1), class'avaDmgType_Rifle' );
	}
}

`devexec simulated function TestAllBotDead()
{
	local PlayerController PC;
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		`log( "TestAllBotDead" @PC @PC.PlayerReplicationInfo.bBot );
		if ( PC.PlayerReplicationInfo.bBot == true || PC.PlayerReplicationInfo == NONE )
		{
			PC.Pawn.TakeDamage( 250, self, Vect(0,0,0), Vect(0,0,0), None );
		}
	}
}

defaultproperties
{
	DesiredFOV					=	90.000000
	DefaultFOV					=	90.000000
	FOVAngle					=	90.000
	CameraClass					=	class'avaPlayerCamera'
	CheatClass					=	class'avaCheatManager'
	InputClass					=	class'avaGame.avaPlayerInput'
	bForceBehindview			=	true
	BaseDamageShake				=	(RotMag=(X=30),RotRate=(X=120000),RotTime=0.15,OffsetMag=(Z=0.03),OffsetRate=(X=1,Y=1,Z=1),OffsetTime=0.2)
	MatineeCameraClass			=	class'avaPlayerCamera'
	SavedMoveClass				=	class'avaSavedMove';
	ShockMuffleDSP				=	avaDSPPreset'avaDSP.ShockMuffle1';
	MouseSensitivityEx			=	1.0
	DSP_Muffle					=	DSPPreset'avaSoundDSP.Muffled'
	SpectatorSpeedRate			=	1.0
	LastAutoMessage				=	-1
	bCollideActors				=	true

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0017.000000
		CollisionHeight=+0045.000000
		BlockActors=false
		BlockZeroExtent=true
		BlockNonZeroExtent=true
		CollideActors=true
	End Object
	CylinderComponent=CollisionCylinder
	Components.Add(CollisionCylinder)

	CurrentBattery				=	30				// NVG �� ���� Power
	MaximumBattery				=	30				// NVG �� Maximum Power
	BatteryChargeSpeed			=	0.25			// NVG �� ���� ������ Increase Power Speed Per Seconds
	NVGMinimumPower				=	2				// NVG �� �ѱ� ���� Minimum Power
	NVGConsumptionSpeed			=	1				// NVG �� �Ѱ� ������ Decrease Power Speed Per Seconds

	ChatNotifySound				=	SoundCue'avaUISounds.ChatMessageNotificationCue'
	DamageBreath				=	SoundCue'avaPlayerSounds.Damage.Damage_breath_1'	
	bUseMeshInterpolating		=	true
	bEnableVote					=	true

	NOINPUTKICKTIME				=	120.0
	bCameraCollision			=	true
	bDisableCameraEffect		=	false
	bDisplayBotInfo				=	false
	bUseOldThirdPersonCam		=	true
}
