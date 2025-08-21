/**  
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaPawn extends GamePawn
	dependson(avaCharacterModifier)
	config(Game)
	native
	nativereplication
	abstract
	notplaceable;

`include(avaGame/avaGame.uci)

enum EShotInfo
{
	SI_Generic,
	SI_Head,
	SI_Stomach,
	SI_Chest,
	SI_LeftArm,
	SI_RightArm,
	SI_LeftLeg,
	SI_RightLeg,
};

var		int						Helmet_DamageThreshold;								// Helmet�� �������� �Ǵ� damage �Ӱ谪
var()	hmserialize cipher BYTE	Armor_Stomach;
var()	int						Armor_Head;											// [!] 20070323 dEAthcURe|HM 'hmserialize'
var()	float					Absorb_Stomach, Absorb_Head;
var		float					CurrentArmorParam;

var		bool				bFixedView;
var		bool				bUpdateEyeheight;									/** if true, UpdateEyeheight will get called every tick */

var		globalconfig bool	bWeaponBob;
var		bool				bJustLanded;										/** used by eyeheight adjustment.  True if pawn recently landed from a fall */
var		bool				bLandRecovery;										/** used by eyeheight adjustment. True if pawn recovering (eyeheight increasing) after lowering from a landing */

var		bool				bComponentDebug;

var		vector				FixedViewLoc;
var		rotator				FixedViewRot;
var		float				CameraScale, CurrentCameraScale, CameraScaleTime;	/** multiplier to default camera distance */
var		float				CameraScaleMin, CameraScaleMax;
var		float				SlopeBoostFriction;									// which materials allow slope boosting

/** view bob properties */
var		globalconfig float	Bob;
var		float				LandBob;
var		float				JumpBob;
var		float				AppliedBob;
var		float				bobtime, LadderTime;
var		int					LadderFootstepIndex;
var		vector				WalkBob;
var		vector				PunchAngle, PunchAngleVel;

var		hmserialize cipher BYTE	HealthMax;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var		hmserialize cipher BYTE	ArmorMax;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var		int					iImpactLocation;
var		repnotify vector	ImpactLocation0, ImpactLocation1, ImpactLocation2, ImpactLocation3, ImpactLocation4, ImpactLocation5, ImpactLocation6, ImpactLocation7;
var		BYTE				ImpactDirection, BloodSpurtFlags;

/// 2006.3.13. deif :) [�ں�����Ʈ��԰� ���� �� ����� ��ġ]
/// ���� �� ���ϰ� �� eye height�� ������ ���� �� ���� (-24)
var		float				BaseEyeHeightWhenCrouched;
var		float				LastUpdateEyeHeight;

var		globalconfig bool	bNoDetailMeshes;
var		globalconfig int	LODBias;

var enum EWeaponHand
{
	HAND_Right,
	HAND_Left,
	HAND_Centered,
	HAND_Hidden
}WeaponHand;

var		class<avaPawnSoundGroup>	SoundGroupClass;
var		class<avaEmit_HitEffect>	SparkEmitterClass;
var		class<avaEmit_BloodSpray>	BloodEmitterClass, BloodEmitterClassTeen;

enum EHitEffectType
{
	HET_Default,
	HET_HelmetHit,
	HET_HeadShot,
	HET_KevlarHit	
};

/** replicated information on a hit we've taken */
struct native TakeHitInfo
{
	/** the amount of damage */
	var bool				bDamage;
	/** the location of the hit */
	var vector				HitLocation;
	/** how much momentum was imparted */
	var vector				Momentum;
	/** the damage type we were hit with */
	var class<DamageType>	DamageType;
	/** the bone that was hit on our Mesh (if any) */
	var int					HitBoneIndex;	
	/** Punch angle */
	var vector				PunchAngle;
	var int					DamagedBy;
	var EHitEffectType		HitEffect;
};

struct native DamagedKickBack
{
	var() float				MinAngleX;
	var() float				MaxAngleX;
	var() float				DirectionRandomX;
	var() float				DamageAmpX;

	var() float				MinAngleZ;
	var() float				MaxAngleZ;
	var() float				DirectionRandomZ;
	var() float				DamageAmpZ;
};

var()	DamagedKickBack		DamagedKickBackInfo[`MAX_SHOT_INFO]; 

var repnotify TakeHitInfo	LastTakeHitInfo;
var hmserialize bool		bLastTakeHitVisibility;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	/** �������� �ػ���� ��븦 �� �� �־��°� ? ( ���̴����� ���� ǥ���Ҷ� ��� ) */

/** stop considering LastTakeHitInfo for replication when world time passes this (so we don't replicate out-of-date hits when pawns become relevant) */
var float					LastTakeHitTimeout;

/**
 Ragdoll effect
 **/
// Impact effects 
struct native PawnImpactContext
{
	var	ParticleSystemComponent		ImpactEffectComponent;	
	var	float						LastImpactTime;
	var	AudioComponent				ImpactSoundComponent;
	var	AudioComponent				ImpactSoundComponent2; // @TODO: This could be turned into a dynamic array; but for the moment just 2 will do.
};

var const float					LastImpactTime;
var const float					ImpactReFireDelay;
var const float					LandSoundRetriggerTime;
var transient float				LastLandSoundPlayTime;


var array<PawnImpactContext>	Ragdoll_Impacts;

/*********************************************************************************************
 Armor
********************************************************************************************* */
var hmserialize bool			bHasHelmet;	// [!] 20070323 dEAthcURe|HM 'hmserialize'


/*********************************************************************************************
 Weapon / Firing
********************************************************************************************* */

/** Holds the class type of the current weapon attachment.  Replicated to all clients. */
var repnotify	avaWeapon	CurrentWeapon;

/** This holds the local copy of the current attachment.  This "attachment" actor will exist on all versions of the client. */
var	avaWeaponAttachment		CurrentWeaponAttachment;

var				float FireRateMultiplier; /** affects firing rate of all weapons held by this pawn. */

/*********************************************************************************************
* Pain
********************************************************************************************* */
const MINTIMEBETWEENPAINSOUNDS=0.35;
var			float		LastPainSound;
var float DeathTime;
var int LookYaw;	// The Yaw Used to looking around
var float RagdollLifespan;

/*********************************************************************************************
* Foot placement IK system
********************************************************************************************* */
var name			LeftFootBone, RightFootBone;
var name			LeftFootControlName, RightFootControlName;
var float			BaseTranslationOffset;
var float			CrouchTranslationOffset;
var float			OldLocationZ;
var	bool			bEnableFootPlacement;
var const float		ZSmoothingRate;
/** if the pawn is farther than this away from the viewer, foot placement is skipped */
var float MaxFootPlacementDistSquared;
/** cached references to skeletal controllers for foot placement */
var SkelControlFootPlacement LeftLegControl, RightLegControl;

/*********************************************************************************************
* Custom gravity support
********************************************************************************************* */
var float CustomGravityScaling;		// scaling factor for this pawn's gravity - reset when pawn lands/changes physics modes
var bool bNotifyStopFalling;		// if true, StoppedFalling() is called when the physics mode changes from falling

/*********************************************************************************************
* 3��Ī Animation �� Play �ϱ� ���� Blend 
*********************************************************************************************/
var avaAnimBlendByWeaponType	weaponTypeBlend;	// Weapon Type �� ���� Blend ��ü %%% Node Name �� WeaponTypeNode �̾�߸� �Ѵ�. 
var avaAnimBlendByEvent			eventBlend;			// Event �� ���� Animation Play �� ���� Blend ��ü (WeaponType �� �������̴�) %% Node Name �� eventBlend ���� �Ѵ�.
var avaAnimBlendByWeaponState	weaponStateBlend;	// Weapon State �� ���� Blend ��ü (State ��ȯ Animation Sequence Name �� �˾ƿ��� ���� ���ȴ�.) %% Node Name �� weaponStateNode �̾�� �Ѵ�.

var hmserialize repnotify byte WeaponState;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var byte		   PrvWeaponState;

/* Reload Animation �� Play �ϱ� ���� Replication Variable */
var hmserialize repnotify byte ReloadAnimPlayCount;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize repnotify byte PreReloadAnimPlayCount;
var hmserialize repnotify byte PostReloadAnimPlayCount;
/* PullPin Animation �� Play �ϱ� ���� Replication Variable */
var hmserialize repnotify byte PullPinAnimPlayCount;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
/* ������ ���� Animation �� Play �ϱ� ���� Replication Variable */
var hmserialize repnotify byte MountSilencerPlayCount;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
/* ������ ���� Animation �� Play �ϱ� ���� Replication Variable */
var hmserialize repnotify byte UnMountSilencerPlayCount;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

/* Night vision */
var hmserialize repnotify bool NightvisionActivated;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var float NightvisionActivatedTime;

/* Bullet trail */
//var avaBulletTrailComponent BulletTrailComponent;

/* Weapon Zoom */
var hmserialize repnotify byte WeaponZoomChange;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var bool bDamageHead;			// �ֱٿ� Head�� �������� �Ծ���
var bool bWallShot;				// Wall Shot �� ���� Damage�� �Ծ���

// Helmet ���󰡱� ������ ���� Network ������
var hmserialize vector			TOH_Momentum;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize	repnotify bool	TOH_Play;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var int				FootStepSoundLeft;

var BYTE					LastQuickVoiceMsg;

// For stronger encryption : deif 2008/2/3
var byte						DamageInfoTag;

/*********************************************************************************************
* PickUp �� Swap �ϱ� ���� Properties 
* HostMigration �� �ƴ϶�� PlayerController �� �־ �������.
* UI �� ǥ���ϰų�, HostMigration �� �Ϸ��� TouchedPickUp �� replication �Ǿ�� �Ѵ�.
*********************************************************************************************/
var avaPickUpProvider			TouchedPP;
var	array<avaPickUp>			TouchedPickUp;
`define MAX_TOUCHED_PICKUP 32
var avaPickup			TouchedPickUp_Rep[`MAX_TOUCHED_PICKUP];
var hmserialize repnotify int	TouchedPickup_Count;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

// ���� Type �� Character �� ���� �ɷ�ġ ��ȭ 
struct native WeaponTypeAmp								// Weapon Type �� ����ġ
{
	var	float		AmmoAmp;						// Ammo ����ġ
	var float		ReloadAmp;						// Reload �ð� ����ġ
	var float		EquipAmp;						// Equip �ð� ����ġ
	var float		DamageAmp;						// Damage ����ġ
	var float		RangeAmp;						// Hit Range ����ġ
	var float		SpeedAmp;						// Speed ����ġ

	structdefaultproperties
	{
		AmmoAmp				=	0.0
		ReloadAmp			=	1.0
		EquipAmp			=	1.0
		DamageAmp			=	1.0
		RangeAmp			=	1.0
		SpeedAmp			=	1.0
	}
};

struct native WeaponTypeAdd
{
	var float		ZoomSpreadAdd;					// +
	var float		UnZoomSpreadAdd;				// +
	var float		SpreadFallingAdd;				//
	var float		SpreadMovingAdd;				//
	var float		SpreadDuckingAdd;				//
	var float		SpreadSteadyAdd;				//	

	structdefaultproperties
	{
		ZoomSpreadAdd		=	0.0
		UnZoomSpreadAdd		=	0.0
		SpreadFallingAdd	=	0.0
		SpreadMovingAdd		=	0.0
		SpreadDuckingAdd	=	0.0
		SpreadSteadyAdd		=	0.0
	}
};

var WeaponTypeAmp	WeapTypeAmp[`MAX_WEAPON_TYPE];
var WeaponTypeAdd	WeapTypeAdd[`MAX_WEAPON_TYPE];

var hmserialize bool	bIsDash;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var float			MiniMapScale;

var float			AimSpeedPct;					// �����̵��������ġ
var float			CrouchAimSpeedPct;				// ���ؾɾ��̵��������ġ
var float			SprintPct;						// ������Ʈ�������ġ
var float			CrouchSprintPct;				// �ɾƼ�������Ʈ�������ġ

var	float			ChrBaseSpeedPct;				// �⺻�̵��ӵ��������� ��ġ�� Character �ɷ�ġ. Default : 1.0
var float			ChrAimSpeedPct;					// �����̵��ӵ���...
var float			ChrWalkSpeedPct;				// �ȱ��̵��ӵ���...
var float			ChrCrouchSpeedPct;				// �ɾ��̵��ӵ���...
var float			ChrCrouchAimSpeedPct;			// �ɾ������̵��ӵ���...
var float			ChrSprintSpeedPct;				// �뽬�̵��ӵ���...
var float			ChrCrouchSprintSpeedPct;		// �ɾƴ뽬�̵��ӵ���...

var float			FallingDamageAmp;				// �������� �Դ� Damage ��������. Default : 1.0

var float			HeadDefenceRate;				// [0.0~1.0]	0.0 �̸� �Ӹ��� ���� ������� Helmet �� ������ ������, 1.0 �̸� �Ӹ��� ���� �¾Ƶ� ����� �������� ����
var float			ProjectileVelAmp;				// ��ô�� ���� �ӵ� ��������
var float			ThrowableWeapReadyAmp;			// ��ô�� ���� �غ� �ð� ��������

var bool			bSightMode;						// ���ظ���ΰ�?

var float			SpeedPctByChrType;				// ������ Speed ����ġ.
var float			MaxSpeedByChrType;				// ������ MaxSpeed ����.

var rotator			LastRotation;
var float			LastRotationInterpolatedTime;

// ���� Player �� ������ �ִ� (1��Ī ������ ���� ������) Weapon ���� Class, 3��Ī���� ������ �ִ� ���⸦ ����ֱ� �����̴�.
`define POSSESSED_WEAPON_CNT	12

var repnotify avaWeapon			PossessedWeapon[`POSSESSED_WEAPON_CNT];
var avaWeapon					PrvPossessedWeapon[`POSSESSED_WEAPON_CNT];

const AngToUnrealAng	=		182.04167;

var bool						bRightHandedWeapon;	// ���� �����տ� ����� �޼տ� �����...

// �ڰ������� ���⿡ ���� Limit Yaw Angle
var hmserialize bool			bLimitYawAngle;		// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Yaw Angle ���� Flag
var	hmserialize float			MaxLimitYawAngle;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Yaw Angle �ִ� Limit ��
var hmserialize float			MinLimitYawAngle;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Yaw Angle �ּ� Limit ��

var hmserialize bool			bLimitPitchAngle;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	
var hmserialize float			MinLimitPitchAngle;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	
var hmserialize float			MaxLimitPitchAngle;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	
	
var	array< class<avaWeapon> >	DefaultWeapons;		// Character �� Default �� ����ִ� �����...

// Body�� �ٴ� ������ Item Struct
struct native ItemPart
{
	var name					Id;
	var name					SocketName;
	var string					MeshName;
	var StaticMeshComponent		ItemMesh;
	var ECharSlot				Slot;		//!< �������� �߰�(2007/01/22).
	var	float					MaxVisibleDistance;
};

/*	
	��ȭ�� Animation Play �� ���� �����̴�.
	EXC_None �� ��� ����ü ������ Animation �� Play ������, EXC_None �� �ƴѰ��
	����ü���� Animation �� �и��ؼ� Play ���� �ʴ´�.
*/
enum EBlendExclusiveType
{
	EXC_None,				// None Exclusive
	EXC_InstallHeavyWeapon,	// ��ȭ�� ��ġ��... 
	EXC_FixedHeavyWeapon,	// ��ġ�� ��ȭ�⸦ ����� ���
};

var	hmserialize repnotify	EBlendExclusiveType		HeavyWeaponType;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// 
var	avaFixedHeavyWeapon			GripHeavyWeapon;		// ����ִ� ��ġ ��ȭ��

/** replicated to we can see where remote clients are looking */
var	hmserialize int				RemoteViewYaw;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var	int							TypeID;

var bool						bIsLocalPawn;			// �׾Controller �� ������ Local Pawn ���� �ƴ��� �����ϱ� ���� Flag
var bool						bThirdPersonDeathCam;

// [2006/10/13 , YTS] hide all mesh Related with this pawn.
var hmserialize bool			bForceHidden;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var(Camera) vector		OffsetLow, OffsetMid, OffsetHigh;
var(Camera) vector		OffsetLow_Crouch, OffsetMid_Crouch, OffsetHigh_Crouch;

var(Camera) vector		Viewport_OffsetLow;
var(Camera) vector		Viewport_OffsetMid;
var(Camera) vector		Viewport_OffsetHigh;

var(Camera) vector		WorstLocOffset;

/** set on client when valid team info is received; future changes are then ignored unless this gets reset first
 * this is used to prevent the problem where someone changes teams and their old dying pawn changes color
 * because the team change was received before the pawn's dying
 */
var	bool				bEnableNightVision;				// true �̸� Night Vision �� ����� �� �ֽ��ϴ�.
var hmserialize BYTE	DogTagCnt;						// [!] 20070323 dEAthcURe|HM 'hmserialize'
var float				QuickChatUpdateTime;			// ���������� QuickChat�� ��ģ �ð�

var avaWeapon			SpecialInventory;				// ������ �ִ� Ư����ǰ(C4, NukeCase ���)

//	 Take Hit Effect Properites with Ragdoll 
var bool				bBlendOutTakeHitPhysics;		/** if set, blend Mesh PhysicsWeight to 0.0 in C++ and call TakeHitBlendedOut() event when finished */
var float				fTakeHitPhysicsMultiflier;		
var float				fComboHitPhysicsMultiflier;
var float				fTakeHitPhysicsStartWeight;
var float				TakeHitPhysicsBlendOutSpeed;	/** speed at which physics is blended out when bBlendOutTakeHitPhysics is true (amount subtracted from PhysicsWeight per second) */
var PhysicsAsset		RagdollPhysicsAsset;			/** physics asset used while in ragdoll */
var PhysicsAsset		TakeHitPhysicsAsset;			/** physics asset used when playing physics based hit animations */

var bool				bIsInScreen;					// 
var repnotify bool		bTargetted;						// �д����� �־Ȱ濡 ���ؼ� Targetting �Ǿ���...
var bool				bDetected;
var float				LastDetectedTime;
var	repnotify BYTE		TargettedCnt;					//
var float				TargettedTime;					// �д����� �־Ȱ濡 ���ؼ� Targetting �� �ð�...
var PlayerController	LocalPC;						// Local Player Controller... ���� ���� �� �ƴѵ�....��.��
var bool				bUseBinocular;					// true �̸� �־Ȱ� �������....

//	Battle Stress Status System ����
var	float				StressDecTime;
var	int					StressValue;
var BYTE				StressLevel;
var int					DefaultTeam;

var float				LastKillTime;					// ���������� ���� ���� �ð�.... avaKilledIconMessage���� ä���ش�...

var vector				HeadTestOffset;
var vector				HeadViewOffset;
var vector				HeadViewCollision;

var bool				bDrawGrenadeIndicator;			// TRUE �̸� Grenade Indicator �� ǥ�����ش�.
var bool				bDrawNotifyTargetted;			// TRUE �̸� �ڽ��� Targetting �Ǿ����� �̴ϸʰ� �����ʿ� ǥ�����ش�.

struct native DamageData
{
	var	BYTE	Damage;
	var BYTE	Reserve1;
	var	BYTE	BoneIndex;
	var BYTE	Reserve2;
};

var	BYTE				EncryptKey;

var float					fLastFireTime;
// Look At Control
var() SkeletalMeshComponent	EyeBallComp;
var SkelControlLookAt		LookAtControl, LeftEyeControl, RightEyeControl;
var bool					bEnableAutoLookAtControl;	// Auto Look At Control Enable 

var		TriggerVolume		BombVolume;				//
var		avaUseVolume		TouchedUseVolume;		//

var		array< SequenceObject >		DogTagPickUpEvents;

cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061031 dEAthcURe|HM
	#endif
	
	virtual UBOOL TryJumpUp(FVector Dir, FVector Destination, DWORD TraceFlags, UBOOL bNoVisibility);
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual INT calcMoveFlags();
	virtual ETestMoveResult FindJumpUp(FVector Direction, FVector &CurrentPosition);	
	FLOAT GetGravityZ();
	void setPhysics(BYTE NewPhysics, AActor *NewFloor, FVector NewFloorV);
	virtual FVector CalculateSlopeSlide(const FVector& Adjusted, const FCheckResult& Hit);
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );	

	// camera
	virtual void UpdateEyeHeight(FLOAT DeltaSeconds);

	// Dash �� ���ؼ� ����.
	virtual FLOAT MaxSpeedModifier();	

	UBOOL UseFootPlacementThisTick();
	void EnableFootPlacement(UBOOL bEnabled);
	void DoFootPlacement(FLOAT DeltaSeconds);
	void DecayPunchAngle(FLOAT DeltaSeconds);
	void DecayPunchAngle_Inner(FLOAT DeltaSeconds);
	void UpdateEyeHeight2( FLOAT DeltaSeconds );
	void UpdateFootStepSound( FLOAT DeltaSeconds );
	UBOOL TickLookAtControl();

	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData);

	// ������ Max Speed ������ ���ؼ�...
protected:
	virtual void CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant);

	virtual void GetBoundingCylinder(FLOAT& CollisionRadius, FLOAT& CollisionHeight);
	virtual void physicsRotation(FLOAT deltaTime, FVector OldVelocity);

	void	EncrpytTakeGunDamage( FDamageData DamageData,class AController* instigateBy );
}

// Optimize ������ Properties
//		-	PossessedWeapon	Replication �� �ʿ䰡 ����...

replication
{
	// replicated properties
	if ( bNetOwner && bNetDirty )
		bHasHelmet, TouchedPickUp_Rep, TouchedPickup_Count, BombVolume, TouchedPP; 

	if ( bNetDirty )
		CurrentWeapon, PossessedWeapon,
		ImpactLocation0, ImpactLocation1, ImpactLocation2, ImpactLocation3, ImpactLocation4, ImpactLocation5, ImpactLocation6, ImpactLocation7, ImpactDirection, BloodSpurtFlags,
		ReloadAnimPlayCount, 
		PreReloadAnimPlayCount,
		PostReloadAnimPlayCount,
		PullPinAnimPlayCount, 
		MountSilencerPlayCount, 
		UnMountSilencerPlayCount,
		WeaponState, 
		TOH_Momentum, TOH_Play,
		Armor_Stomach, 
		bLimitYawAngle,	MaxLimitYawAngle, MinLimitYawAngle,	bLimitPitchAngle, MinLimitPitchAngle, MaxLimitPitchAngle, HeavyWeaponType, GripHeavyWeapon,
		HealthMax, ArmorMax, 
		DogTagCnt,
		bTargetted, StressLevel, 
		TargettedCnt, EncryptKey;

	if (bNetDirty && WorldInfo.TimeSeconds < LastTakeHitTimeout)
		LastTakeHitInfo, bLastTakeHitVisibility;

	if (bNetOwner && bNetDirty)
		NightvisionActivated, WeaponZoomChange;

    if ( !bNetOwner )
		RemoteViewYaw, bIsDash;
}

native final function PhysicalMaterial GetPawnPhysMaterial( int BodyIndex );

native final simulated function vector GetViewOffset( float DeltaTime, const out Rotator ViewRotation );
simulated native function GetBaseViewOffsets( float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High );
simulated native function Client_RequestTakeGunDamage( DamageData data, controller instigateBy );
simulated native function DecryptTakeGunDamage( DamageData data, controller instigateBy );

// Weapon Bob Native Code ��
simulated native function vector WeaponBobNative(float BobDamping, float JumpDamping);

simulated function avaPlayerReplicationInfo GetValidPRI()
{
	return  ( PlayerReplicationInfo != None ) ? avaPlayerReplicationInfo( PlayerReplicationInfo ) :
			( DrivenVehicle != None && DrivenVehicle.PlayerReplicationInfo != None ) ? avaPlayerReplicationInfo( DrivenVehicle.PlayerReplicationInfo ) :
			NONE;
}

simulated function avaPlayerController GetValidController()
{
	return	( Controller != None ) ? avaPlayerController( Controller ) :
			( DrivenVehicle != None && DrivenVehicle.Controller != None ) ? avaPlayerController( DrivenVehicle.Controller ) :
			None;
}

// AddStress �� ReleaseMaxStress ���� Code �� Pawn�� TickSpecial���� �ֽ��ϴ�.
// �Ʒ� Code �� �����ϽǶ� ���� �����ؾ� �մϴ�....
function AddStress( int Stress )
{
	StressValue		+=	Stress;
	StressValue		=	Clamp( StressValue, 0, 100 );

	if ( StressValue <= 30 )		StressLevel = 0;
	else if ( StressValue <= 85 )	StressLevel = 1;
	else
	{
		if ( StressLevel != 2 )
		{
			StressLevel = 2;
			StressDecTime = 6.0;
		}
	}
}

function ReleaseMaxStress()
{
	StressValue = 50;
	StressLevel = 1;
}

simulated function int GetStressLevel()
{
	return StressLevel;
}

unreliable server function RequestAddStress( int Stress )
{
	AddStress( Stress );
}

simulated function Tick( float DeltaTime )
{
	Super.Tick( DeltaTime );
}

exec simulated function TestStressLevel( int Level )
{
	StressLevel			=	Level;
	StressDecTime		=	60;
}

simulated function InitRagdollEffect()
{
	local int i;
	local PhysicalMaterial PhysMat;
	local PawnImpactContext Item;	

	for (i=0; i<Mesh.PhysicsAsset.BodySetup.Length; ++i)
	{
		PhysMat = GetPawnPhysMaterial(i);	
		Ragdoll_Impacts[i] = Item;

		if(PhysMat.ImpactEffect != None)
		{			
			Ragdoll_Impacts[i].ImpactEffectComponent = new(Outer) class'ParticleSystemComponent';
			AttachComponent(Ragdoll_Impacts[i].ImpactEffectComponent);
			Ragdoll_Impacts[i].ImpactEffectComponent.bAutoActivate = FALSE;  
			Ragdoll_Impacts[i].ImpactEffectComponent.SetTemplate(PhysMat.ImpactEffect);
		}

		if(PhysMat.ImpactSound != None)
		{
			Ragdoll_Impacts[i].ImpactSoundComponent = new(Outer) class'AudioComponent';
			AttachComponent(Ragdoll_Impacts[i].ImpactSoundComponent);
			Ragdoll_Impacts[i].ImpactSoundComponent.SoundCue = PhysMat.ImpactSound;

			Ragdoll_Impacts[i].ImpactSoundComponent2 = new(Outer) class'AudioComponent';
			AttachComponent(Ragdoll_Impacts[i].ImpactSoundComponent2);
			Ragdoll_Impacts[i].ImpactSoundComponent2.SoundCue = PhysMat.ImpactSound;
		}		
	}
}

simulated function SetBaseEyeheight()
{
	BaseEyeheight = bIsCrouched ? Default.BaseEyeHeightWhenCrouched : Default.BaseEyeheight;
}

simulated function Client_UpdateNightvision()
{	
	//`log( "Client_UpdateNightvision"@NightvisionActivated );
	NightvisionActivatedTime = WorldInfo.TimeSeconds;
	SoundGroupClass.static.PlayNightvisionSound( Self );
}

simulated function Client_WeaponZoomModeChange()
{
	SoundGroupClass.static.PlayWeaponZoomSound( Self );
}

event StoppedFalling()
{
	CustomGravityScaling = 1.0;
	bNotifyStopFalling = false;
}

/** PoweredUp()
returns true if pawn has game play advantages, as defined by specific game implementation
*/
function bool PoweredUp()
{
	return ( (DamageScaling > 1) || (FireRateMultiplier > 1) );
}

/** InCombat()
returns true if pawn is currently in combat, as defined by specific game implementation.
*/
function bool InCombat()
{
	return (WorldInfo.TimeSeconds - LastPainSound < 1) && !PhysicsVolume.bPainCausing;
}

/**
SuggestJumpVelocity()
returns true if succesful jump from start to destination is possible
returns a suggested initial falling velocity in JumpVelocity
Uses GroundSpeed and JumpZ as limits
*/
native function bool SuggestJumpVelocity(out vector JumpVelocity, vector Destination, vector Start);

`devexec simulated function togglefeet()
{
	bEnableFootPlacement = !bEnableFootPlacement;
}

simulated function ApplyWeaponSightMode( bool bMode )
{
	bSightMode = bMode;
}

simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	local avaWeapon w;
	super.PlayWeaponSwitch( OldWeapon, NewWeapon );
	// Weapon �� Change �Ǿ���...
	// �ӵ� ����ġ ����
	w = avaWeapon(Weapon);
	if ( w != None )
	{
		
		// ������ �⺻ �̵� �ӵ��� Character �� Skill �� ��������..
		GroundSpeed			=	w.BaseSpeed					* ChrBaseSpeedPct			* WeapTypeAmp[w.WeaponType].SpeedAmp;	// �⺻�ӵ�
		AimSpeedPct			=	w.AimSpeedPct				* ChrAimSpeedPct;													// ���ؽ� ����ġ
		WalkingPct			=	w.WalkSpeedPct				* ChrWalkSpeedPct;													// �ȱ�� ����ġ
		CrouchedPct			=	w.CrouchSpeedPct			* ChrCrouchSpeedPct;												// �ɾ��̵��� ����ġ
		CrouchAimSpeedPct	=	w.CrouchAimSpeedPct			* ChrCrouchAimSpeedPct;												// �ɾƼ� ���� �̵��� ����ġ
		SprintPct			=	w.SprintSpeedPct			* ChrSprintSpeedPct;												// ������Ʈ�� ����ġ
		CrouchSprintPct		=	w.CrouchSprintSpeedPct		* ChrCrouchSprintSpeedPct;											// �ɾƼ� ������Ʈ�� ����ġ
	}
}

`devexec function ShowCompDebug()
{
	bComponentDebug = !bComponentDebug;
}

simulated function PostBeginPlay()
{
	local rotator R;

	SpawnTime	=	WorldInfo.TimeSeconds;
	
	Super.PostBeginPlay();

	if (Mesh != None)
	{
		BaseTranslationOffset = Mesh.Translation.Z;
		CrouchTranslationOffset = Mesh.Translation.Z + CylinderComponent.CollisionHeight - CrouchHeight;		
	}

	// Zero out Pitch and Roll
	R.Yaw = Rotation.Yaw;
	SetRotation(R);

	if ( WorldInfo.NetMode != NM_DedicatedServer )
	{
		if ( class'avaPlayerController'.Default.PawnShadowMode == SHADOW_None )
		{
			if ( Mesh != None )
			{
				Mesh.CastShadow = false;
				Mesh.bCastDynamicShadow = false;
			}
		}		
	}

	// add to local HUD's post-rendered list
	ForEach LocalPlayerControllers(LocalPC)
	{
		if ( avaHUD(LocalPC.MyHUD) != None )
		{
			avaHUD(LocalPC.MyHUD).AddPostRenderedActor(self);
		}
	}

	if ( Role == ROLE_Authority )
	{
		// EncryptKey �� �߱��Ѵ�....
		EncryptKey	=	Rand(10);
	}

	if ( !WorldInfo.GRI.GameClass.Default.bTeamGame )
		SoundGroupClass.static.PlaySpawnSound( self );
}

simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	if (SkelComp == Mesh)
	{
		weaponTypeBlend = avaAnimBlendByWeaponType( mesh.Animations.FindAnimNode( 'WeaponTypeNode' ) );
		if ( weaponTypeBlend == None )	`warn( "Could not find weaponTypeBlend" );
		LeftLegControl = SkelControlFootPlacement(Mesh.FindSkelControl(LeftFootControlName));
		RightLegControl = SkelControlFootPlacement(Mesh.FindSkelControl(RightFootControlName));		
	}	
}

simulated function vector WeaponBob(float BobDamping, float JumpDamping )
{
	//Local Vector WBob;

	//WBob = BobDamping * WalkBob;
	//WBob.Z = (0.45 + 0.55 * BobDamping) * WalkBob.Z;
	//WBob.Z += LandBob;

	// add some weapon bob based on jumping
	//if ( Velocity.Z > 0 )
	//{
	//	WBob.Z -= 0.007 * FMin(Velocity.Z,300);
	//}
	//return WBob;

	Local Vector WBob;

	WBob = BobDamping * WalkBob;
	WBob.Z = (0.45 + 0.55 * BobDamping)*WalkBob.Z + JumpDamping *(LandBob + JumpBob);
	return WBob;
}

/** TurnOff()
Freeze pawn - stop sounds, animations, physics, weapon firing
*/
simulated function TurnOff()
{
	super.TurnOff();	
}

event EncroachedBy(Actor Other)
{
	//local avaPawn P;
	//// don't get telefragged by non-vehicle ragdolls and pawns feigning death
	//P = avaPawn(Other);
	//if (P == None && (P.Physics != PHYS_RigidBody))
	//{
	//	Super.EncroachedBy(Other);
	//}
}

function gibbedBy(actor Other)
{
	if ( Role < ROLE_Authority )
		return;
	if ( Pawn(Other) != None )
		Died(Pawn(Other).Controller, class'avaDmgType_Encroached', Location);
	else
		Died(None, class'avaDmgType_Encroached', Location);
}

simulated function PreRender(Canvas Canvas)
{
}

function AddVelocity( vector NewVelocity, vector HitLocation, class<DamageType> DamageType, optional TraceHitInfo HitInfo )
{
	if (!bIgnoreForces && !IsZero(NewVelocity))
	{
		Super.AddVelocity(NewVelocity, HitLocation, DamageType, HitInfo);
	}
}

simulated function TakeFallingDamage()
{
	local avaPlayerController PC;
	local float EffectiveSpeed;
	local float Damage;

	if (Velocity.Z < -0.5 * MaxFallSpeed)
	{
		if ( Role == ROLE_Authority )
		{
			MakeNoise(1.0);
			if (Velocity.Z < -1 * MaxFallSpeed)
			{
				EffectiveSpeed = Velocity.Z;
				if (TouchingWaterVolume())
				{
					EffectiveSpeed += 100;
				}
				if (EffectiveSpeed < -1 * MaxFallSpeed)
				{
					Damage = -150 * ( EffectiveSpeed + MaxFallSpeed ) / MaxFallSpeed;
					Damage *= FallingDamageAmp;
					TakeDamage( Damage, None, Location, vect(0,0,0), class'DmgType_Fell');

					// if on local client, play damage shake
					PC = avaPlayerController(Controller);
					
					if (PC != None )
					{
						if ( bIsDash )	PC.ClientCancelDash();
						if ( LocalPlayer(PC.Player) != None )
							PC.DamageShake(FMin(1, -1 * EffectiveSpeed / MaxFallSpeed));
					}
				}
	        }
		}
	}
	else if (Velocity.Z < -1.4 * JumpZ)
		MakeNoise(0.5);
}

// toss out a weapon
function TossWeapon(Vector TossVel)
{
	avaWeapon(Weapon).ThrowWeapon( true );
}

function DropDogTag();

function bool Died(Controller Killer, class<DamageType> damageType, vector HitLocation, optional bool bScoreKill = true,optional class<Weapon> weaponBy)
{
	avaGame( WorldInfo.Game ).ActivateKillEvent( Killer, Controller );

	StopFiring();

	if ( Killer != None && Killer.GetTeamNum() != GetTeamNum() )
		DropDogTag();

	if ( GripHeavyWeapon != None )
		GripHeavyWeapon.UserDied( self );


	avaPlayerReplicationInfo( PlayerReplicationInfo ).LastDeathTime = WorldInfo.GRI.ElapsedTime;

	Super.Died( Killer, damageType, HitLocation, bScoreKill, weaponBy );

	return true;
}

function bool StopFiring()
{
	return StopWeaponFiring();
}

function bool StopWeaponFiring()
{
	local int i;
	local bool bResult;
	local avaWeapon avaWeap;

	avaWeap = avaWeapon(Weapon);

	if ( avaWeap != None )
	{
		if ( Weapon.IsFiring() )
		{
			avaWeap.ClientEndFire(0);
			avaWeap.ClientEndFire(1);
			avaWeap.ServerStopFire(0);
			avaWeap.ServerStopFire(1);
		}
		bResult = true;
		//avaWeap.ClientStopZoom(true);
	}

	for (i = 0; i < InvManager.PendingFire.length; i++)
	{
		if (InvManager.PendingFire[i] > 0)
		{
			bResult = true;
			InvManager.PendingFire[i] = 0;
		}
	}

	return bResult;
}

simulated function float GetEyeHeight()
{
	if ( !IsLocallyControlled() )
		return BaseEyeHeight;
	else
		return EyeHeight;
}

function PlayVictoryAnimation();

simulated function StopPlayFiring();

function PlayTakeHit(vector HitLoc, int Damage, class<DamageType> damageType)
{
	if( WorldInfo.TimeSeconds - LastPainSound < MinTimeBetweenPainSounds )
		return;	
}

simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	// ��ȭ�⸦ ��ġ �߰ų� ��ġ�� ��ȭ�⸦ �����. Rotation ���� �ʴ´�.
	if ( HeavyWeaponType != EXC_None )
	{
		NewRotation.Yaw = (MaxLimitYawAngle + MinLimitYawAngle)/2;
	}

	//if ( Physics == PHYS_Ladder )
	//{
	//	NewRotation = OnLadder.Walldir;
	//}
	//else 
	if ( (Physics == PHYS_Walking) || (Physics == PHYS_Falling) || Physics == PHYS_Ladder )
	{
		NewRotation.Pitch = 0;
	}

	SetRotation(NewRotation);
}

/* UpdateEyeHeight()
* Update player eye position, based on smoothing view while moving up and down stairs, and adding view bobs for landing and taking steps.
* Called every tick only if bUpdateEyeHeight==true.
*/
simulated event UpdateEyeHeight( float DeltaTime )
{
	// UpdateEyeHeight �ø��� �Լ���� UpdateFootStepSound �Լ��� Native �� �̵��Ͽ����ϴ�!
}

/* GetPawnViewLocation()
Called by PlayerController to determine camera position in first person view.  Returns
the location at which to place the camera
*/
simulated function Vector GetPawnViewLocation()
{
	//`log(WorldInfo.TimeSeconds@"PawnViewLoc"@(Location.Z+EyeHeight)@Location.Z@EyeHeight);
	/*if ( bUpdateEyeHeight )*/	
	return Location + ((CylinderComponent.CollisionHeight + EyeHeight)  - CylinderComponent.CollisionHeight ) * vect(0,0,1) + WalkBob;
	/*else
		return Location + BaseEyeHeight * vect(0,0,1);*/
}

/* EndViewTarget
	Called by Camera when this actor becomes its ViewTarget */
simulated event EndViewTarget( PlayerController PC )
{
	if ( PC != None && PC.IsLocalPlayerController() )
	{
		SetForcedLodLevel( 0 );
		SetFootstepSoundSpatialization( true );
		bEnableFootPlacement = true;
	}
}

simulated function SetFootstepSoundSpatialization( bool bEnable )
{	
}

simulated function SetWeaponVisibility(bool bWeaponVisible )
{
	local avaWeapon Weap;
	local int		i;

	Weap = avaWeapon(Weapon);

	if (Weap != None)
	{
		Weap.ChangeVisibility(bWeaponVisible);
	}

	for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	{
		if ( PossessedWeapon[i] != None && PossessedWeapon[i].WeaponAttachment != None)
		{
			PossessedWeapon[i].WeaponAttachment.ChangeVisibility( !bWeaponVisible );
		}
	}

	//if (CurrentWeaponAttachment != None )
	//{
	//	CurrentWeaponAttachment.ChangeVisibility();
	//}
}

// [2006/10/12, YTS] variant of SetWeaponVisibility() above
simulated function SetWeaponHidden( bool bHide )
{
	Local avaWeapon Weap;
	Local int i;
	
	Weap = avaWeapon(Weapon);
	if( Weap != None )
		Weap.ChangeVisibility( bForceHidden ? false : !bHide );

	for( i = 0 ; i < `POSSESSED_WEAPON_CNT ; i++ )
	{
		if( PossessedWeapon[i] != None &&  PossessedWeapon[i].WeaponAttachment != None )
		{
			PossessedWeapon[i].ChangeVisibility( bForceHidden ? false : !bHide );
			PossessedWeapon[i].WeaponAttachment.ChangeVisibility( bForceHidden ? false : !bHide );
		}
	}
}

simulated function SetForcedLodLevel( int nLevel )
{
	local int i;
	// �ڱ� ĳ���� �� �� LOD �������� �ʵ���....
	for( i = 0 ; i < Mesh.Attachments.Length ; ++i )
	{
		if ( StaticMeshComponent(Mesh.Attachments[i].Component) != None )			StaticMeshComponent(Mesh.Attachments[i].Component).ForcedLodModel = nLevel;
		else if ( SkeletalMeshComponent(Mesh.Attachments[i].Component) != None )	SkeletalMeshComponent(Mesh.Attachments[i].Component).ForcedLodModel = nLevel;
	}
	Mesh.ForcedLodModel = nLevel;
}

/** sets whether or not the owner of this pawn can see it */
simulated function SetMeshVisibility(bool bVisible)
{
	local int i;
	local PrimitiveComponent Primitive;


	
	// Server ���� Character �� ���ڱ� ��Ÿ���� ������ ��� ���� Code
	//if ( bVisible && IsFirstPerson() )
	//{
	//	`warn( "avaPawn::SetMeshVisibility Visible Is True But First Person " @self );
	//	ScriptTrace();
	//	return;
	//}

	//if ( bVisible )
	//{
	//	CurrentCameraScale	= 1.0;
	//	CameraScaleTime		= 0.0;
	//	CameraScale			= 9.0;
	//}
	//else
	//{
	//	CurrentCameraScale	= default.CurrentCameraScale;
	//	CameraScale			= default.CameraScale;
	//}

	// Handle the main player mesh
	if (Mesh != None)
	{
		if ( bVisible == FALSE && class'avaOptionSettings'.static.GetDefaultObject().GetWorldShadow() == FALSE )
			Mesh.bPauseAnims = TRUE;
		else
			Mesh.bPauseAnims = FALSE;

		for( i = 0 ; i < Mesh.Attachments.Length ; ++i )
		{
			Primitive = PrimitiveComponent(Mesh.Attachments[i].Component);
			if ( Primitive != None )
			{
				// HelmetCollision is not visual component. hidden always.
				if( bForceHidden ||  ( avaCharacter(Self).Helmet != None  && (avaCharacter(Self).Helmet.CollisionComponent == Primitive) ) )
					Primitive.SetHidden(true);
				else
					Primitive.SetOwnerNoSee( !bVisible );
			}
		}
		if( bForceHidden )
			Mesh.SetHidden(true);
		else
			Mesh.SetOwnerNoSee( !bVisible );	
	}

	// Handle any weapons they might have
	SetWeaponVisibility( bForceHidden ? false : !bVisible);	

	if ( bVisible )
	{
		Mesh.ForceSkelUpdate();
	}
}

`devexec function FixedView(string VisibleMeshes)
{
	local bool bVisibleMeshes;
	local float fov;

	if (WorldInfo.NetMode == NM_Standalone)
	{
		if (VisibleMeshes != "")
		{
			bVisibleMeshes = ( VisibleMeshes ~= "yes" || VisibleMeshes~="true" || VisibleMeshes~="1" );

			if (VisibleMeshes ~= "default")
				bVisibleMeshes = !IsFirstPerson();

			avaCharacter(Self).SetMeshVisibility(bVisibleMeshes);
		}

		if (!bFixedView)
			CalcCamera( 0.0f, FixedViewLoc, FixedViewRot, fov );

		bFixedView = !bFixedView;
		`log("FixedView:" @ bFixedView);
	}
}

function DoComboName( string ComboClassName );
function bool InCurrentCombo()
{
	return false;
}

//=============================================================================
// Armor interface.
function bool ProcessArmor( out int inDamage, out int armor, float absorb, out int AbsorbDamage )
{
	AbsorbDamage	= Min( int(inDamage * absorb * CurrentArmorParam), armor );
	if ( armor > AbsorbDamage )		armor -= AbsorbDamage;
	else							armor  = 0;
	inDamage		-= AbsorbDamage;
	return (AbsorbDamage>0);	
}

function bool ProcessStomachArmor( out int inDamage, out BYTE armor, float absorb, out int AbsorbDamage )
{
	AbsorbDamage	= Min( int(inDamage * absorb * CurrentArmorParam), armor );
	if ( armor > AbsorbDamage )		armor -= AbsorbDamage;
	else							armor  = 0;
	inDamage		-= AbsorbDamage;
	return (AbsorbDamage>0);	
}

function bool AdjustDamageEx( out int inDamage, EShotInfo shotInfo, Controller instigatedBy, class<DamageType> damageType, out int AbsorbDamage )
{
	local class<avaDamageType>	aDmgType;
	local vector				v1,v2;
	local Rotator				r1,r2;
	local bool					bReducedByArmor;

	bReducedByArmor = false;

	//CheckHitInfo( HitInfo, Mesh, Normal(Momentum), HitLocation );
	//shotInfo = Check_ShotInfo( HitInfo );

	if (shotInfo == SI_Head)
	{
		if (bHasHelmet && class<avaDmgType_Gun>(damageType) != None )
		{
			bReducedByArmor = ProcessArmor( inDamage, Armor_Head, Absorb_Head, AbsorbDamage );
		}
		inDamage *= 3;
	}
	else
	{
		///@@@@@@@@@@@@@@@@ ��ȹ test
		///@@@@��ȹ part ��û ; ���� ���ֱ� =_=; deif 2006/7/19
		if ( class<avaDmgType_Gun>(damageType) != None )
			bReducedByArmor = ProcessStomachArmor( inDamage, Armor_Stomach, Absorb_Stomach, AbsorbDamage );

		if (shotInfo == SI_Chest )									inDamage *= 1.25;
		else if (shotInfo == SI_Stomach)							inDamage *= 1.5;
		else if (shotInfo == SI_LeftLeg || shotInfo == SI_RightLeg)	inDamage *= 0.7;
	}

	// BackStab �� Damage Modifier 2006/3/15 by OZ
	// HitLocation �� �̿��ؼ� BackStab �� ����ϸ� ������ ��������.
	// Ÿ������ Rotation �� �̿��Ѵ�.
	aDmgType = class<avaDamageType>(DamageType);

	if ( aDmgType != None )
	{
		if ( aDmgType.default.BackStabModifier > 0.0 )
		{
			r1 = GetViewRotation();
			r2 = instigatedBy.Pawn.GetViewRotation();
			r1.pitch = 0;
			r2.pitch = 0;
			v1 = vector( r1 );
			v2 = vector( r2 );
				//Location - hitlocation;
		
			// �Ĺ� 60�� �̳��� ��� Backstab ���� ó���Ѵ�.
			if ( normal(v1) dot normal(v2) > 0.8 )
			{
				inDamage *= aDmgType.default.BackStabModifier;
			}
		}
	}

	return bReducedByArmor;
}

//=============================================================================

simulated function SetHand(EWeaponHand NewWeaponHand)
{
	WeaponHand = NewWeaponHand;
	if ( avaWeapon(Weapon) != none )
		avaWeapon(Weapon).SetHand(WeaponHand);
}

function bool GiveHealth(int HealAmount, int HealMax)
{
	if (Health < HealMax)
	{
		Health = Min(HealMax, Health + HealAmount);
		return true;
	}
	return false;
}

/**
 * Overridden to return the actual player name from this Pawn's
 * PlayerReplicationInfo (PRI) if available.
 */
function String GetDebugName()
{
	// return the actual player name from the PRI if available
	if (PlayerReplicationInfo != None)
	{
		return "";
	}
	// otherwise return the formatted object name
	return GetItemName(string(self));
}

simulated function avaPhysicalMaterialProperty GetPhysicalMaterialBelow()
{
	local vector HitLocation, HitNormal, TraceStart;
	local TraceHitInfo HitInfo;	

	TraceStart = Location - (GetCollisionHeight() * vect(0,0,1));
	Trace(HitLocation, HitNormal, TraceStart - vect(0,0,32), TraceStart, false,, HitInfo);
	if (HitInfo.PhysMaterial == None)
		HitInfo.PhysMaterial = class'avaWeaponAttachment'.static.GetDefaultPhysicalMaterial();

	if (HitInfo.PhysMaterial == None) return None;
	
//	`log( "MaterialBelow"@HitInfo.PhysMaterial );
	return avaPhysicalMaterialProperty(HitInfo.PhysMaterial.GetPhysicalMaterialProperty(class'avaPhysicalMaterialProperty'));	
}

event PlayJumpSound()
{
	local avaPhysicalMaterialProperty PhysicalProperty;	

	//`log( "PlayJumpSound" );
	PhysicalProperty = GetPhysicalMaterialBelow();		

	if (PhysicalProperty != None)
	{
		PlaySound( PhysicalProperty.JumpSound, false, true );
	}
}

event PlayLandSound()
{
	local avaPhysicalMaterialProperty PhysicalProperty;		

	//`log( "PlayLandSound" );

	PhysicalProperty = GetPhysicalMaterialBelow();		

	if (PhysicalProperty != None)
	{
		if (WorldInfo.TimeSeconds - LastLandSoundPlayTime > LandSoundRetriggerTime)
		{
			PlaySound( PhysicalProperty.LandSound, false, true );

			LastLandSoundPlayTime = WorldInfo.TimeSeconds;
		}
	}
}

simulated event PlayFootStepSound(int FootDown,optional bool bManual)
{	
	local avaPhysicalMaterialProperty PhysicalProperty;		
	local Emitter AnEmitter;

	if ( bManual == false && ( IsFirstPerson() || VSize( Velocity ) < 10 )  )		return;

	if (Physics == PHYS_Ladder)
	{
		if (OnLadder != None)
		{
			if (FootDown == 0 && OnLadder.FootstepLeftSound != none)
			{
				PlaySound( OnLadder.FootstepLeftSound, true, true,,, true );				
			}
			else if (FootDown == 1 && OnLadder.FootstepRightSound != none)
			{
				PlaySound( OnLadder.FootstepRightSound, true, true,,, true );				
			}		
		}

		return;
	}

	PhysicalProperty = GetPhysicalMaterialBelow();		

	if (PhysicalProperty != None)
	{
		if (FootDown == 0 && PhysicalProperty.FootstepLeftSound != none)
		{
			PlaySound( PhysicalProperty.FootstepLeftSound, true, true,,, true );			
		}
		else if (FootDown == 1 && PhysicalProperty.FootstepRightSound != none)
		{
			PlaySound( PhysicalProperty.FootstepRightSound, true, true,,, true );
		}

		if ( PhysicalProperty.FootstepParticle != None && EffectIsRelevant(Location, false, 2000 ) )
		{
			AnEmitter = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetFootstepEmitter(PhysicalProperty.FootstepParticle, Mesh.GetBoneLocation( FootDown == 0 ? 'Bip01_L_Toe0' : 'Bip01_R_Toe0' , 0 ), Rotation);
			AnEmitter.ParticleSystemComponent.ActivateSystem();
			AnEmitter.ParticleSystemComponent.SetOcclusionGroup( Mesh );
			AnEmitter.SetTimer( 3, FALSE, 'HideSelf' );
		}
	}
}

simulated function bool EffectIsRelevant(vector SpawnLocation, bool bForceDedicated, optional float CullDistance )
{
	if ( WorldInfo.NetMode == NM_DedicatedServer )
		return bForceDedicated;

	if ( WorldInfo.NetMode == NM_ListenServer )
	{
		if ( bForceDedicated )
			return true;
		if ( IsHumanControlled() && IsLocallyControlled() )
			return true;
	}
	else if ( IsHumanControlled() )
	{
		return true;
	}

	if ( (SpawnLocation != Location) || WorldInfo.TimeSeconds - LastRenderTime < 1.0 )
	{
		if ( LocalPC.ViewTarget != None && ( LocalPC.Pawn == self || CheckMaxEffectDistance(LocalPC, SpawnLocation, CullDistance) ) )
		{
			return true;
		}
	}
	return false;
}


simulated function PlayZeroLatencyEffect( ImpactInfo Impact, Controller InstigateBy )
{
	local avaWeap_BaseGun		InstigateByWeapon;
	local class<avaDamageType>	avaDamage;
	local float					Damage;
	local EShotInfo				ShotInfo;
	Damage = 50;
	// ���� ���� �� ���!
	// FriendlyFireType �� 2�� ��쿡�� Gun Damage �� �ޱ� ������..
	if ( avaGameReplicationInfo(WorldInfo.GRI).FriendlyFireType != 2 && avaPlayerController( InstigateBy ).IsSameTeam( self) )
		return;

	InstigateByWeapon				= avaWeap_BaseGun( InstigateBy.Pawn.Weapon );
	LastTakeHitInfo.HitLocation		= Impact.HitLocation;
	LastTakeHitInfo.Momentum		= InstigateByWeapon.InstantHitMomentum[InstigateByWeapon.CurrentFireMode] * vector( instigateBy.Pawn.GetViewRotation() );
	LastTakeHitInfo.DamageType		= InstigateByWeapon.GetInstantHitDamageTypes(InstigateByWeapon.CurrentFireMode);
	LastTakeHitInfo.HitBoneIndex	= Mesh.MatchRefBone( Impact.HitInfo.BoneName );
	LastTakeHitInfo.PunchAngle		= GetPunchAngle();
	LastTakeHitInfo.bDamage			= Damage > 0 ? true : false;
	LastTakeHitInfo.DamagedBy		= -1;

	ShotInfo = Check_ShotInfo( Impact.HitInfo.BoneName );

	if ( ShotInfo == SI_Head )		LastTakeHitInfo.HitEffect = bHasHelmet ? HET_HelmetHit : HET_HeadShot;
	else							LastTakeHitInfo.HitEffect = Armor_Stomach > 0 ? HET_KevlarHit : HET_Default;
	
	avaDamage = class<avaDamageType>(LastTakeHitInfo.DamageType);
	LastTakeHitTimeout = WorldInfo.TimeSeconds + ( (avaDamage != None) ? avaDamage.static.GetHitEffectDuration(self, Damage)
									: class'avaDamageType'.static.GetHitEffectDuration(self, Damage) );
	// play clientside effects
	PlayTakeHitEffects( false );
}

/**
 * Responsible for playing any death effects, animations, etc.
 *
 * @param 	DamageType - type of damage responsible for this pawn's death
 *
 * @param	HitLoc - location of the final shot
 */
simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local Vector		ApplyImpulse, ShotDir;	
	local TraceHitInfo	HitInfo;
	//local PlayerController PC;
	//local bool bPlayersRagdoll;
	//local class<avaDamageType> avaDamageType;	
	bReplicateMovement = false;
	bTearOff = true;
	bPlayedDeath = true;

	HitDamageType = DamageType; // these are replicated to other clients
	TakeHitLocation = HitLoc;

	if ( WorldInfo.NetMode == NM_DedicatedServer )
	{
		GotoState('Dying');
		return;
	}

	//	20m �̳����� �Ʊ��� �״� ���� ���� Stress Value +35
	if ( LocalPC != None && LocalPC.Pawn != None && bIsInScreen && default.DefaultTeam == LocalPC.GetTeamNum() )
	{
		if ( VSize( Location - LocalPC.Pawn.Location ) * `UU2METER > 20.0 )
			RequestAddStress( 35 );
	}

	// Ragdoll Play �� ���ؼ� PhysicsInstance �� Collision Group �� ������ �ʿ䰡 �ִ�.
	// avaSkeletalMeshComponent( Mesh ).SetPhysicsAssetCollisionGroup( 0 );
	Mesh.SetPhysicsAsset(RagdollPhysicsAsset,TRUE);
	Mesh.SetRBChannel(RBCC_Untitled1);
	Mesh.SetRBCollidesWithChannel(RBCC_Pawn,false);
	Mesh.SetRBCollidesWithChannel(RBCC_EffectPhysics,false);
	Mesh.SetRBCollidesWithChannel(RBCC_GameplayPhysics,true);
	Mesh.SetRBCollidesWithChannel(RBCC_Default,true);
	Mesh.SetRBCollidesWithChannel(RBCC_Untitled1,true);
	Mesh.SetRBDominanceGroup(23);

	bBlendOutTakeHitPhysics = false;
	
	if ( avaPlayerController(LocalPC) != none && avaPlayerController(LocalPC).bDisableRagdoll )
	{
		GotoState('Dying');
		PlayDyingSoundEx( class<avaDamageType>(DamageType) );
		Destroy();
	}
	else if ( InitRagdoll() )
	{
		Mesh.MinDistFactorForKinematicUpdate = 0.f;

		InitRagdollEffect();

		Mesh.PhysicsWeight = 1.0;
		Mesh.bUpdateKinematicBonesFromAnimation = false;
		Mesh.bUpdateJointsFromAnimation = false;
		Mesh.bPauseAnims	= true;
		
		Mesh.SetNotifyRigidBodyCollision(true);
		// Ragdoll ���¿����� Animation �� �������� �ʴ´�.... but Medic �� ���� Ʋ���� ���̴�.
		// Mesh.SetAnimTreeTemplate( None );

		if ( TearOffMomentum != vect(0,0,0) )
		{
			CheckHitInfo( HitInfo, Mesh, Normal(TearOffMomentum), TakeHitLocation );
			if ( HitInfo.BoneName != '' )
			{				
				ShotDir = normal(TearOffMomentum);
				ApplyImpulse = ShotDir * DamageType.default.KDeathImpulse;

				// If not moving downwards - give extra upward kick
				if ( Velocity.Z > -10 )
				{
					ApplyImpulse += Vect(0,0,1)*DamageType.default.KDeathUpKick;
				}
				Mesh.AddImpulse(ApplyImpulse, TakeHitLocation, HitInfo.BoneName, true);				
			}
		}
		GotoState('Dying');
		PlayDyingSoundEx( class<avaDamageType>(DamageType) );
	}
	else
	{
		PlayDyingSoundEx( class<avaDamageType>(DamageType) );
		Destroy();
	}
}

simulated event Destroyed()
{
	local int i;
	Super.Destroyed();	
	for (i = 0; i < Attached.length; i++)
	{
		if (Attached[i] != None)
		{
			Attached[i].PawnBaseDied();
		}
	}
	
	if ( avaHUD(LocalPC.MyHUD) != None )
		avaHUD(LocalPC.MyHUD).RemovePostRenderedActor(self);

	eventBlend		 = none;
	weaponStateBlend = none;
	weaponTypeBlend  = none;
}

function AddDefaultInventory()
{
	//Controller.ClientSwitchToBestWeapon();
}

simulated `devexec function TestHW()
{
	//local avaFixedHeavyWeapon P;

	//if ( fixedHW == None )
	//{
	//	ForEach AllActors(class'avaFixedHeavyWeapon', P)
	//	{
	//		if ( P.TryToEnter( self ) )
	//			fixedHW = P;
	//	}
	//}
	//else
	//{
	//	if ( fixedHW.UserLeave( self ) )
	//		fixedHW = None;
	//}
}

/**
 *	Calculate camera view point, when viewing this pawn.
 *
 * @param	fDeltaTime	delta time seconds since last update
 * @param	out_CamLoc	Camera Location
 * @param	out_CamRot	Camera Rotation
 * @param	out_FOV		Field of View
 *
 * @return	true if Pawn should provide the camera point of view.
 */
simulated function bool CalcCamera( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	if (bFixedView)
	{
		out_CamLoc = FixedViewLoc;
		out_CamRot = FixedViewRot;
	}
	else
	{
		if ( !IsFirstPerson() )	// Handle BehindView
		{
			// spectator�� ���� ������ bind! -_-;
			//if (!IsLocallyControlled())
			//{
			//	GetActorEyesViewPoint( out_CamLoc, out_CamRot );
			//}

			CalcThirdPersonCam(fDeltaTime, out_CamLoc, out_CamRot, out_FOV);
			ApplyPunchAngle( out_CamRot );
		}
		else
		{
			if ( GripHeavyWeapon != None )	// ��ȭ�⸦ ��� ������ ��ȭ�⿡ Camera ��ġ�� ������� �Ѵ�.
			{
				GripHeavyWeapon.CalcViewPoint( self, fDeltaTime, out_CamLoc, out_CamRot, out_FOV );
				if ( IsFirstPerson() )	ApplyPunchAngle( out_CamRot );
			}
			else
			{
				// By default, we view through the Pawn's eyes..
				GetActorEyesViewPoint( out_CamLoc, out_CamRot );
			}
		}

		if ( avaWeapon(Weapon) != none)
			avaWeapon(Weapon).WeaponCalcCamera(fDeltaTime, out_CamLoc, out_CamRot);
	}
	return true;
}

simulated function ApplyPunchAngle( out Rotator out_Rotation )
{
	out_Rotation.Yaw	+= PunchAngle.y * AngToUnrealAng;
	out_Rotation.Pitch	+= PunchAngle.x * AngToUnrealAng;
	out_Rotation.Roll	+= PunchAngle.z * AngToUnrealAng; 
}

simulated function ApplyPunchAngleToWeapon( out Rotator out_Rotation )
{
	out_Rotation.Yaw	+= PunchAngle.y * AngToUnrealAng;
	out_Rotation.Pitch	+= PunchAngle.x * AngToUnrealAng;
	out_Rotation.Roll	+= PunchAngle.z * AngToUnrealAng; 
}

simulated event GetActorEyesViewPoint( out vector out_Location, out Rotator out_Rotation )
{
	//Controller.GetPlayerViewPoint( out_Location, out_Rotation );
	out_Location		= GetPawnViewLocation();
	out_Rotation		= GetViewRotation();
	ApplyPunchAngle( out_Rotation );
}

simulated function bool CalcThirdPersonCam( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	local vector CamStart, HitLocation, HitNormal, CamDir;

	CamStart = Location;
	CamStart.Z += GetCollisionHeight() + Mesh.Translation.Z;
	CamDir = Vector(out_CamRot) * GetCollisionRadius() * CurrentCameraScale;

	if ( (Health <= 0) )
	{
		// adjust camera position to make sure it's not clipping into world
		// @todo fixmesteve.  Note that you can still get clipping if FindSpot fails (happens rarely)
		FindSpot(GetCollisionExtent(),CamStart);
	}

	if (CurrentCameraScale < CameraScale)
	{
		CameraScaleTime += fDeltaTime;
		CurrentCameraScale += ( CameraScale - CurrentCameraScale ) * fDeltaTime / 2.0;
		CurrentCameraScale = FMin( CurrentCameraScale, CameraScale );
		
	}
	else if (CurrentCameraScale > CameraScale)
	{
		CameraScaleTime += fDeltaTime;
		CurrentCameraScale = ( default.CurrentCameraScale - CameraScale ) * CameraScaleTime/2.0;
		CurrentCameraScale = FMax( CurrentCameraScale, CameraScale );
	}
	if (CamDir.Z > GetCollisionHeight())
	{
		CamDir *= square(cos(out_CamRot.Pitch * 0.0000958738)); // 0.0000958738 = 2*PI/65536
	}
	out_CamLoc = CamStart - CamDir;
	if (Trace(HitLocation, HitNormal, out_CamLoc, CamStart, false, vect(12,12,12)) != None)
	{
		out_CamLoc = HitLocation;
	}
	return true;
}

/*
	���� �߻� ������ġ�� Return �Ѵ�. ��ġ ��ȭ�⸦ ����� ���� Ʋ���� ���ȴ�. 
*/
simulated function Vector GetWeaponStartTraceLocation(optional Weapon w)
{
	return GripHeavyWeapon == None ? GetPawnViewLocation() : GripHeavyWeapon.GetPawnViewLocation();
}

//=============================================
// Jumping functionality
function bool DoJump( bool bUpdating )
{
	if ( Super.DoJump(bUpdating) )
	{
		if ( !bUpdating )
		    PlayJumpSound();
		return true;
	}
	return false;
}

simulated event Landed(vector HitNormal, actor FloorActor)
{
	local vector ZVelocity;
	local float LandedMultiplier;

	Super.Landed(HitNormal, FloorActor);

	ZVelocity = vect(0,0,0);
	ZVelocity.Z = Velocity.Z;

	// adds impulses to vehicles and dynamicSMActors (e.g. KActors)
	LandedMultiplier = 4.0f; // 4.0f works well for landing on a Scorpion.
	if(DynamicSMActor(FloorActor) != none)
	{
		DynamicSMActor(FloorActor).StaticMeshComponent.AddImpulse(ZVelocity*LandedMultiplier,Location);
	}

	if ( Velocity.Z < -200 )
	{
		//OldZ = Location.Z;
		bJustLanded = bUpdateEyeHeight && Controller.LandingShake();
		if (bJustLanded)
		{
			Acceleration.X = 0;
			Acceleration.Y = 0;
			Velocity.X *= 0.1;
			Velocity.Y *= 0.1;
		}
	}
	if (InvManager != None)
	{
		InvManager.OwnerEvent('Landed');
	}
	if ( (Velocity.Z < -2*JumpZ) )
	{
		Velocity.X *= 0.1;
		Velocity.Y *= 0.1;
	}
	if (Health > 0 && !bHidden && (WorldInfo.TimeSeconds - SplashTime > 0.25))
	{
		PlayLandSound();
	}
	SetBaseEyeheight();
}

function PlayDyingSoundEx( class<avaDamageType> DamageType )
{	
	SoundGroupClass.Static.PlayDyingSound(self, DamageType);
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	//local int i,j;
	//local PrimitiveComponent P;
	//local string s;
	//local float xl,yl;
	//local class< avaWeaponAttachment > wc;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	HUD.Canvas.SetPos(24,out_YPos);
	HUD.Canvas.DrawText( "Location : " @Location@ "Rotation :" @Rotation@ "RemoteViewYaw" @RemoteViewYaw@ "EyeHeight" @EyeHeight@ "LastRenderTime" @LastRenderTime );
    out_YPos += out_YL;

	//HUD.Canvas.SetPos(24,out_YPos);
	//HUD.Canvas.DrawText( "Translation : " @Mesh.Translation.Z );
 //   out_YPos += out_YL;

	HUD.Canvas.SetPos(24,out_YPos);
	HUD.Canvas.DrawText("Dash :" @bIsDash @(VSize(Velocity)*`UU2METER) @VSize(Velocity)@ "Crouch :" @bIsCrouched@ "Physics :" @Physics@" "@Velocity @Acceleration@ "Weapon State :" @weaponState );
	out_YPos += out_YL;

	//HUD.Canvas.SetPos(24,out_YPos);
	//HUD.Canvas.DrawText("ForcedLOD :" @Mesh.ForcedLodModel@ "Predicted LOD" @Mesh.PredictedLODLevel@ "LOD Bias" @Mesh.LODBias );
	//out_YPos += out_YL;

	////HUD.Canvas.SetPos(24,out_YPos);
	////HUD.Canvas.DrawText("Helmet_DamageThreshold :" @Helmet_DamageThreshold );
	////out_YPos += out_YL;

	//for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	//{
	//	if ( PossessedWeapon[i] != None && PossessedWeapon[i].WeaponAttachment != None && PossessedWeapon[i].WeaponAttachment.bAttachmentState == true )
	//	{
	//		HUD.Canvas.SetPos(24,out_YPos);
	//		HUD.Canvas.DrawText( "Current Weapon : " @PossessedWeapon[i].WeaponAttachment );
	//		out_YPos += out_YL;

	//		wc = PossessedWeapon[i].WeaponAttachment.default.class;

	//		s = ""$wc;
	//		HUD.Canvas.SetPos(24,out_YPos);
	//		HUD.Canvas.DrawText( "Weapon Class: " @s );
	//		out_YPos += out_YL;
	//	}
	//}



	//if (!bComponentDebug)
	//	return;

	//Hud.Canvas.SetDrawColor(255,255,128,255);

	//for (i=0;i<Mesh.Attachments.Length;i++)
	//{
	//    HUD.Canvas.SetPos(4,out_YPos);

	//    s = ""$Mesh.Attachments[i].Component;
	//	Hud.Canvas.Strlen(s,xl,yl);
	//	j = len(s);
	//	while ( xl > (Hud.Canvas.ClipX*0.5) && j>10)
	//	{
	//		j--;
	//		s = Right(S,j);
	//		Hud.Canvas.StrLen(s,xl,yl);
	//	}

	//	HUD.Canvas.DrawText("Attachment"@i@" = "@Mesh.Attachments[i].BoneName@s);
	//    out_YPos += out_YL;

	//    P = PrimitiveComponent(Mesh.Attachments[i].Component);
	//    if (P!=None)
	//    {
	//		HUD.Canvas.SetPos(24,out_YPos);
	//		HUD.Canvas.DrawText("Component = "@P.Owner@P.HiddenGame@P.bOnlyOwnerSee@P.bOwnerNoSee);
	//		out_YPos += out_YL;

	//		s = ""$P;
	//		Hud.Canvas.Strlen(s,xl,yl);
	//		j = len(s);
	//		while ( xl > (Hud.Canvas.ClipX*0.5) && j>10)
	//		{
	//			j--;
	//			s = Right(S,j);
	//			Hud.Canvas.StrLen(s,xl,yl);
	//		}

	//		HUD.Canvas.SetPos(24,out_YPos);
	//		HUD.Canvas.DrawText("Component = "@s);
	//		out_YPos += out_YL;
	//	}
	//}

	//out_YPos += out_YL*2;
	//HUD.Canvas.SetPos(24,out_YPos);

	//out_YPos += out_YL;
}

/** force the player to ragdoll, automatically getting up when the body comes to rest
 * (basically, force activate the feign death code)
 */


/**
 * Check on various replicated data and act accordingly.
 */
simulated event ReplicatedEvent(name VarName)
{
	local int i;
	// If CurrentWeaponAttachmentClass has changed, the player has switched weapons and
	// will need to update itself accordingly.
	if ( VarName == 'CurrentWeapon' )					WeaponAttachmentChanged();	
	else if (VarName == 'ImpactLocation0' && Controller == None)	PlayImpactEffect(ImpactLocation0,0);
	else if (VarName == 'ImpactLocation1' && Controller == None)	PlayImpactEffect(ImpactLocation1,1);
	else if (VarName == 'ImpactLocation2' && Controller == None)	PlayImpactEffect(ImpactLocation2,2);
	else if (VarName == 'ImpactLocation3' && Controller == None)	PlayImpactEffect(ImpactLocation3,3);
	else if (VarName == 'ImpactLocation4' && Controller == None)	PlayImpactEffect(ImpactLocation4,4);
	else if (VarName == 'ImpactLocation5' && Controller == None)	PlayImpactEffect(ImpactLocation5,5);
	else if (VarName == 'ImpactLocation6' && Controller == None)	PlayImpactEffect(ImpactLocation6,6);
	else if (VarName == 'ImpactLocation7' && Controller == None)	PlayImpactEffect(ImpactLocation7,7);	
	else if (VarName == 'ReloadAnimPlayCount')			Client_PlayReloadAnimation( EBT_Reload );
	else if (VarName == 'PreReloadAnimPlayCount')		Client_PlayReloadAnimation( EBT_PreReload );
	else if (VarName == 'PostReloadAnimPlayCount' )		Client_PlayReloadAnimation( EBT_PostReload );
	else if (VarName == 'PullPinAnimPlayCount' )		Client_PlayPullPinAnimation();
	else if (VarName == 'MountSilencerPlayCount' )		Client_PlayMountSilencerAnimation();
	else if (VarName == 'UnMountSilencerPlayCount' )	Client_PlayUnMountSilencerAnimation();	
	else if (VarName == 'WeaponState' )					Client_ChangeWeaponState();
	else if (VarName == 'NightvisionActivated')			Client_UpdateNightvision();
	else if (VarName == 'WeaponZoomChange' )			Client_WeaponZoomModeChange();
	else if (VarName == 'TOH_Play' )					Client_TakeOffHelmet();
	else if (VarName == 'PossessedWeapon' )				ChangePossessedWeapon();
	else if (VarName == 'HeavyWeaponType' )				ResetEventBlend();
	else if (VarName == 'bTargetted' || VarName == 'TargettedCnt' )			UpdateTargetted();
	else if (VarName == 'LastTakeHitInfo')
	{
		// ���� �� ���� ����!
		// Local Player Controller �� Player ID �� ���ٸ� ���� �� ���ݾ�...
		if ( LastTakeHitInfo.DamagedBy != avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo ).PlayerID )
			PlayTakeHitEffects();

		LastPainTime = WorldInfo.TimeSeconds;		
	}
	else if (VarName == 'TouchedPickup_Count' )
	{
		TouchedPickup.Length = TouchedPickup_Count;
		for(i = 0 ; i < TouchedPickup_Count ; i++ )
			TouchedPickup[i] = TouchedPickup_Rep[i];
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function Client_ChangeWeaponState()
{
	// Play Change Weapon State Animation
	local Name		SequenceName;
	if ( PrvWeaponState != WeaponState && weaponStateBlend != None && eventBlend != None )
	{
		if ( weaponStateBlend.bAutoPlayAnimWhenChangeState )
		{
			SequenceName = weaponStateBlend.GetStateChangeSequenceName( PrvWeaponState, WeaponState );
			if ( SequenceName != '' )	eventBlend.PlayAnimByName( CurrentWeaponAttachment.AnimPrefix, SequenceName, false );
		}
	}
	PrvWeaponState = WeaponState;
	if ( CurrentWeaponAttachment != None )	CurrentWeaponAttachment.ChangeWeaponState( WeaponState );
}

// Host �� Weapon �� State �� ����Ǿ����� �˷��ش�.
function ChangeWeaponState( int i )
{
	WeaponState = i;
	Client_ChangeWeaponState();
}

simulated function PlayAnimByEvent( EBlendEvent event, optional bool bLoop )
{
	if ( eventBlend != None )	eventBlend.PlayAnimByEvent( event, bLoop );
}

simulated function StopAnimByEvent( EBlendEvent event )
{
	if ( eventBlend != None )	eventBlend.StopEvent( event );
}

simulated function Client_PlayMountSilencerAnimation()
{
	PlayAnimByEvent( EBT_MountSilencer );
}

simulated function Client_PlayUnMountSilencerAnimation()
{
	PlayAnimByEvent( EBT_UnMountSilencer );
}

`devexec simulated function PlayMountSilencerAnimation( bool bMount )
{
	if ( bMount )
	{
		Client_PlayMountSilencerAnimation();
		++MountSilencerPlayCount;
	}
	else			
	{
		Client_PlayUnMountSilencerAnimation();
		++UnMountSilencerPlayCount;
	}
}

function PlayPullPinAnimation()
{
	Client_PlayPullPinAnimation();
	PullPinAnimPlayCount++;
}

simulated function Client_PlayPullPinAnimation()
{
	PlayAnimByEvent( EBT_PullPin );
}

simulated function Client_PlayReloadAnimation( EBlendEvent eventType )
{
	PlayAnimByEvent( eventType );
}

`devexec simulated function PlayReloadAnimation( EBlendEvent eventType )
{
	Client_PlayReloadAnimation( eventType );
	
	switch( eventType )
	{
	case EBT_Reload:		++ReloadAnimPlayCount;			break;
	case EBT_PreReload:		++PreReloadAnimPlayCount;		break;
	case EBT_PostReload:	++PostReloadAnimPlayCount;		break;
	}
}

simulated function Client_TakeOffHelmet();

simulated function TakeOffHelmet( vector momentum )
{
	TOH_Momentum	=	momentum;
	TOH_Play		=	true;
	Client_TakeOffHelmet();
}

/**
 * Called when a pawn's weapon has fired and is responsibile for
 * delegating the creation off all of the different effects.
 *
 * bViaReplication denotes if this call in as the result of the
 * flashcount/flashlocation being replicated.  It's used filter out
 * when to make the effects.
 */

simulated function WeaponFired( bool bViaReplication, optional vector HitLocation)
{
	if ( Role == ROLE_Authority )
		avaPlayerReplicationInfo( PlayerReplicationInfo ).AddFireCount( avaWeapon( Weapon ).WeaponType );

	if (CurrentWeaponAttachment != None)
	{
		if ( IsFirstPerson() )
		{
			CurrentWeaponAttachment.FirstPersonFireEffects(Weapon, HitLocation);
			if ( Mesh.bCastHiddenShadow == true )
				CurrentWeaponAttachment.ThirdPersonFireEffects(HitLocation);
		}
		else
		{
			CurrentWeaponAttachment.ThirdPersonFireEffects(HitLocation);
		}

		//`log( "WeaponFired#2"@Role@bViaReplication@HitLocation );

		/// ������ Role_Authority && IsLocallyControlled()���µ�, Authority�� ���� ������ impact play�� �ϵ���!! ���ƴ�.
		if ( HitLocation != Vect(0,0,0) && (Role==ROLE_Authority || IsLocallyControlled() || bViaReplication))
		{
			CurrentWeaponAttachment.PlayImpactEffects(Weapon,HitLocation);
		}
	}
}

function ClearFlashLocation( Weapon Who )
{

}

simulated function SetImpactLocation( vector pos, optional bool bBackFace, optional bool bBloodSpurt )
{
	//NetUpdateTime	= WorldInfo.TimeSeconds - 1; // Force replication

	if (iImpactLocation == 0)			ImpactLocation0 = pos;
	else if (iImpactLocation == 1)		ImpactLocation1 = pos;
	else if (iImpactLocation == 2)		ImpactLocation2 = pos;
	else if (iImpactLocation == 3)		ImpactLocation3 = pos;
	else if (iImpactLocation == 4)		ImpactLocation4 = pos;
	else if (iImpactLocation == 5)		ImpactLocation5 = pos;
	else if (iImpactLocation == 6)		ImpactLocation6 = pos;
	else if (iImpactLocation == 7)		ImpactLocation7 = pos;

	if ( bBackFace )	ImpactDirection = ImpactDirection | (1<<iImpactLocation);
	else				ImpactDirection = ImpactDirection & ~(1<<iImpactLocation);

	if ( bBloodSpurt )	BloodSpurtFlags = BloodSpurtFlags | (1<<iImpactLocation);
	else				BloodSpurtFlags = BloodSpurtFlags & ~(1<<iImpactLocation);

	PlayImpactEffect( pos, iImpactLocation );

	iImpactLocation = (iImpactLocation+1) % 8;
}

simulated function PlayImpactEffect( vector pos, int i )
{
	if (CurrentWeaponAttachment != None)
	{
		CurrentWeaponAttachment.PlayImpactEffects2( Weapon, pos, false, (ImpactDirection & (1<<i)) == (1<<i), (BloodSpurtFlags & (1<<i)) == (1<<i), i );
	}
}

simulated function WeaponStoppedFiring( bool bViaReplication )
{
	if (CurrentWeaponAttachment != None)
	{
		if ( !IsFirstPerson() )
		{
			CurrentWeaponAttachment.StopThirdPersonFireEffects();
		}
		else
			CurrentWeaponAttachment.StopFirstPersonFireEffects(Weapon);
	}
}

/**
 * Called when a weapon is changed and is responsible for making sure
 * the new weapon respects the current pawn's states/etc.
 */

simulated function WeaponChanged(avaWeapon NewWeapon)
{
	// Make sure the new weapon respects behindview

	if (NewWeapon.Mesh!=None)
		NewWeapon.ChangeVisibility(IsFirstPerson());
}

// Event Animation �� Play �ϱ� ���� Animation Node �� ã�´�.
simulated function ResetEventBlend()
{
	local avaAnimBlendByExclusiveAnim	excAnimBlend;
	eventBlend = None;
	if ( HeavyWeaponType == EXC_None )
	{
		if ( CurrentWeaponAttachment != None )
		{
			eventBlend = avaAnimBlendByEvent( weaponTypeBlend.FindChildAnimNodeByClass( CurrentWeaponAttachment.AttachmentWeaponType, class'avaAnimBlendByEvent' ) );
			if ( CurrentWeaponAttachment.AnimPrefix != '' )
				weaponTypeBlend.InitAnimSequence( CurrentWeaponAttachment.AnimPrefix );

			if ( eventBlend != none )
			{
				weaponStateBlend = None;
				weaponStateBlend = avaAnimBlendByWeaponState( eventBlend.FindChildAnimNodeByClass( 0, class'avaAnimBlendByWeaponState' ) );
			}
		}
	}
	else
	{
		excAnimBlend	= avaAnimBlendByExclusiveAnim( mesh.Animations.FindAnimNode( 'ExclusiveAnim' ) );
		eventBlend		= avaAnimBlendByEvent( excAnimBlend.FindChildAnimNodeByClass( HeavyWeaponType, class'avaAnimBlendByEvent' ) );
		weaponStateBlend = None;
		`log( "avaPawn.ResetEventBlend" @excAnimBlend @eventBlend );
	}

	// Show `log if Failed
	//if ( eventBlend == None )
	//{
	//	`log( "Cannot Find EventBlend" @HeavyWeaponType @CurrentWeaponAttachment.WeaponType );
	//}
}

//==========================================================================
// 3��Ī �� Animation �� ���� Weapon Type �� Return �� �ش�. 2006/1/27 oz
//==========================================================================
simulated event function EBlendWeaponType GetWeaponAttachmentType()
{
	if ( CurrentWeaponAttachment != none )
	{
		return CurrentWeaponAttachment.AttachmentWeaponType;
	}
	return WBT_None;
}

/**
 * Temp Code - Adjust the Weapon Attachment
 */
`devexec function AttachAdjustMesh(string cmd)
{
	local string c,v;
	local vector t,s,l;
	local rotator r;
	local float sc;

	c = left(Cmd,InStr(Cmd,"="));
	v = mid(Cmd,InStr(Cmd,"=")+1);

	t  = CurrentWeaponAttachment.Mesh.Translation;
	r  = CurrentWeaponAttachment.Mesh.Rotation;
	s  = CurrentWeaponAttachment.Mesh.Scale3D;
	sc = CurrentWeaponAttachment.Mesh.Scale;
	l  = CurrentWeaponAttachment.MuzzleFlashLight.Translation;

	if (c~="x")  t.X += float(v);
	if (c~="ax") t.X =  float(v);
	if (c~="y")  t.Y += float(v);
	if (c~="ay") t.Y =  float(v);
	if (c~="z")  t.Z += float(v);
	if (c~="az") t.Z =  float(v);

	if (c~="r")   R.Roll  += int(v);
	if (c~="ar")  R.Roll  =  int(v);
	if (c~="p")   R.Pitch += int(v);
	if (c~="ap")  R.Pitch =  int(v);
	if (c~="w")   R.Yaw   += int(v);
	if (c~="aw")  R.Yaw   =  int(v);

	if (c~="scalex") s.X = float(v);
	if (c~="scaley") s.Y = float(v);
	if (c~="scalez") s.Z = float(v);

	if (c~="scale") sc = float(v);

	if (c~="lx") l.X = float(v);
	if (c~="ly") l.Y = float(v);
	if (c~="lz") l.Z = float(v);

	CurrentWeaponAttachment.Mesh.SetTranslation(t);
	CurrentWeaponAttachment.Mesh.SetRotation(r);
	CurrentWeaponAttachment.Mesh.SetScale(sc);
	CurrentWeaponAttachment.Mesh.SetScale3D(s);
	CurrentWeaponAttachment.MuzzleFlashLight.SetTranslation(l);

	`log("#### AdjustMesh ####");
	`log("####    Translation :"@CurrentWeaponAttachment.Mesh.Translation);
	`log("####    Rotation    :"@CurrentWeaponAttachment.Mesh.Rotation);
	`log("####    Scale3D     :"@CurrentWeaponAttachment.Mesh.Scale3D);
	`log("####    scale       :"@CurrentWeaponAttachment.Mesh.Scale);
	`log("####	 light       :"@CurrentWeaponAttachment.MuzzleFlashLight.Translation);
}


simulated function EShotInfo Check_ShotInfo( name b )
{
	/*
	SI_Generic,
	SI_Head,
	SI_Stomach,
	SI_Chest,
	SI_Arm,
	SI_Leg,
	*/	

	if (b == 'Bip01_HeadNub' || b == 'Bip01_Head') return SI_Head;
	if (b == 'Bip01_L_UpperArm' || b == 'Bip01_L_Forearm' || b == 'Bip01_L_Hand') return SI_LeftArm;
	if (b == 'Bip01_R_UpperArm' || b == 'Bip01_R_Forearm' || b == 'Bip01_R_Hand') return SI_RightArm;
	if (b == 'Bip01_L_Clavicle' || b == 'Bip01_R_Clavicle' || b == 'Bip01_Spine3' || b == 'Bip01_Neck')
		return SI_Chest;
	if (b == 'Bip01_Spine' || b == 'Bip01_Spine1' || b == 'Bip01_Spine2') 
		return SI_Stomach;
	if (b == 'Bip01_L_Calf' || b == 'Bip01_L_Thigh' || b == 'Bip01_L_Foot' || b == 'Bip01_L_Toe0' || b == 'Bip01_L_Toe0Nub') 
		return SI_LeftLeg;	
	if (b == 'Bip01_R_Calf' || b == 'Bip01_R_Thigh' || b == 'Bip01_R_Foot' || b == 'Bip01_R_Toe0' || b == 'Bip01_R_Toe0Nub') 
		return SI_RightLeg;	

	return SI_Generic;	
}

simulated function SwitchWeapon(byte NewGroup)
{
	if (avaInventoryManager(InvManager) != None)
	{
		avaInventoryManager(InvManager).SwitchWeapon(NewGroup);
	}
}

reliable client function ClientSwitchWeapon(byte NewGroup)
{
	SwitchWeapon(NewGroup);
}

// ���� �������� Damage
function TakeDrowningDamage()
{
	TakeDamage(5, None, Location + GetCollisionHeight() * vect(0,0,0.5)+ 0.7 * GetCollisionRadius() * vector(Controller.Rotation), vect(0,0,0), class'avaDmgType_Drowned');
}


/**
 * Check to see if we are in 1st or 3rd person and pass along the proper effect location
 */
simulated function vector GetEffectLocation()
{
	local vector SocketLocation;
	local rotator SocketRotation;

	if ( IsFirstPerson() )
		return avaWeapon(Weapon).GetEffectLocation(); //avaWeapon(Weapon).Mesh.Bounds.Origin + (vect(45,0,0) >> Rotation);
	else if ( (SkeletalMeshComponent(CurrentWeaponAttachment.Mesh) != None) && (CurrentWeaponAttachment.MuzzleFlashSocket != '') )
	{
		SkeletalMeshComponent(CurrentWeaponAttachment.Mesh).GetSocketWorldLocationAndRotation( CurrentWeaponAttachment.MuzzleFlashSocket, SocketLocation, SocketRotation);
		return SocketLocation;
	}
	else
	{
		return CurrentWeaponAttachment.Mesh.Bounds.Origin + (vect(45,0,0) >> Rotation);
	}
}

// Controller �� LocalController �̰� bBehindView �� false �̸� FirstPerson �̴�...
simulated function bool IsFirstPerson()
{	
	if ( LocalPC != None )
	{
		if ( LocalPC.ViewTarget == self && LocalPC.UsingFirstPersonCamera() )
			return true;
	}
	return false;
	//local PlayerController PC;
	//ForEach LocalPlayerControllers(PC)
	//{
	//	if ( (PC.ViewTarget == self) && PC.UsingFirstPersonCamera() )
	//		return true;
	//}
	//return false;
}


/** moves the camera in or out one */
simulated function AdjustCameraScale(bool bIn)
{
	if ( !IsFirstPerson() )
	{
		CameraScale = FClamp(CameraScale + (bIn ? -1.0 : 1.0), CameraScaleMin, CameraScaleMax);
	}
}

`devexec function FlushDebug()
{
	FlushPersistentDebugLines();
}

simulated event TornOff()
{
	local class<avaDamageType> avaDamage;

   	Super.TornOff();	

	avaDamage = class<avaDamageType>(HitDamageType);

	if ( avaDamage != None)
	{
		avaDamage.Static.PawnTornOff(self);
	}
}

simulated function Client_Reset();

simulated function ParticleSystemComponent GetImpactParticleSystemComponent( ParticleSystem PS_Type )
{
	return avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetImpactParticleSystemComponent( PS_Type );
}

simulated function Emitter GetImpactEmitter( ParticleSystem PS_Type, vector SpawnLocation, rotator SpawnRotation )
{
	return avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetImpactEmitter( PS_Type, SpawnLocation, SpawnRotation );
}


simulated function SpawnParticleEffectOnPawn( ParticleSystem PS, PrimitiveComponent HitComponent, name BoneName, vector HitLocation, rotator Rot )
{
	local SkeletalMeshComponent SMC;
	local ParticleSystemComponent PSC;
	local vector BoneRelativeLocation;
	local rotator BoneRelativeRotation;
	local Emitter AnEmitter;

	SMC = SkeletalMeshComponent(HitComponent);

	if( SMC != none )
	{
		PSC = GetImpactParticleSystemComponent( PS );		

		SMC.TransformToBoneSpace(BoneName, HitLocation, rot, BoneRelativeLocation, BoneRelativeRotation);
		SMC.AttachComponent(PSC, BoneName, BoneRelativeLocation, BoneRelativeRotation);
		PSC.SetLODLevel(avaGameReplicationInfo(WorldInfo.GRI).GetLODLevelToUse(PSC.Template, HitLocation) );
		PSC.ActivateSystem();
		PSC.SetOcclusionGroup( Mesh );
	}
	else
	{
		// if we can show gore then we can show what ever effect is on the physicalMaterial
		AnEmitter = GetImpactEmitter(PS, HitLocation, rot);				
		//AnEmitter.SetBase(self);
		AnEmitter.ParticleSystemComponent.SetLODLevel(avaGameReplicationInfo(WorldInfo.GRI).GetLODLevelToUse(AnEmitter.ParticleSystemComponent.Template, HitLocation) );
		AnEmitter.ParticleSystemComponent.ActivateSystem();
		AnEmitter.SetTimer(3, FALSE, 'HideSelf');
		AnEmitter.ParticleSystemComponent.SetOcclusionGroup( Mesh );
	}
}

event bool EncroachingOn( actor Other )
{
	if ( Pawn(Other) != None )
		return false;
	return Super.EncroachingOn( Other );
}

simulated function CalcHeadPos( out vector out_Loc, out rotator out_Rot )
{
	local rotator	RTemp;
	local quat		Q; 
	RTemp.Pitch		=	65536 / 2;
	RTemp.Yaw		=	65536 / 4;
	RTemp.Roll		=	-65536 / 4;
	Q = QuatFromRotator( RTemp );
	out_Rot = QuatToRotator( QuatProduct( Mesh.GetBoneQuaternion( 'Bip01_Head' ), Q )  );
	out_Loc = Mesh.GetBoneLocation( 'Bip01_Head' );
}

simulated State Dying
{
	ignores OnAnimEnd, Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling, BreathTimer;

	event bool EncroachingOn(Actor Other)
	{
		// don't abort moves in ragdoll
		return false;
	}

	simulated function Client_Reset()
	{
		Destroy();
	}

	simulated function Tick( float DeltaTime )
	{
		local float RagDollBlendTime;
		if ( Mesh.PhysicsWeight < 1.0 )
		{
			RagDollBlendTime = 1.0;
			Mesh.PhysicsWeight += ( DeltaTime * RagDollBlendTime );
			if ( Mesh.PhysicsWeight == 1.0 )
				Mesh.PhysicsWeight = 1.0;
		}
	}

	event Timer()
	{
		//SetCollision(false,false);

		/*local PlayerController PC;
		local bool bBehindAllPlayers;
		local vector ViewLocation;
		local rotator ViewRotation;

		if ( !PlayerCanSeeMe() )
		{
			Destroy();
			return;
		}
		// go away if not viewtarget
		//@todo steve - use drop detail, get rid of backup visibility check
		bBehindAllPlayers = true;
		ForEach LocalPlayerControllers(PC)
		{
			if ( (PC.ViewTarget == self) || (PC.ViewTarget == Base) )
			{
				if ( LifeSpan < 3.5 )
					LifeSpan = 3.5;
				SetTimer(2.0, false);
				return;
			}

			PC.GetPlayerViewPoint( ViewLocation, ViewRotation );
			if ( ((Location - ViewLocation) dot vector(ViewRotation) > 0) )
			{
				bBehindAllPlayers = false;
				break;
			}
		}
		if ( bBehindAllPlayers )
		{
			Destroy();
			return;
		}
		SetTimer(2.0, false);*/
	}

	simulated function StopPlayFiring() {}

	/**
	*	Calculate camera view point, when viewing this pawn.
	*
	* @param	fDeltaTime	delta time seconds since last update
	* @param	out_CamLoc	Camera Location
	* @param	out_CamRot	Camera Rotation
	* @param	out_FOV		Field of View
	*
	* @return	true if Pawn should provide the camera point of view.
	*/
	simulated function bool CalcCamera( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
	{
		local vector	v, HitLoc, HitNorm;
		local rotator	r;
		local actor		HitActor;
		
		if ( bIsLocalPawn && bThirdPersonDeathCam == false )
		{
			CalcHeadPos( v, r );
			HitActor = Trace( HitLoc, HitNorm, v + ( HeadTestOffset >> r ) , v + vector(r) * 4, false, HeadViewCollision );
			if ( HitActor != None && HitActor.bWorldGeometry )
			{
				if ( LocalPC != None )
				{
					bThirdPersonDeathCam = true;
					avaPlayerController( LocalPC ).CloseDeadScene();
				}
				CalcDeathThirdPersonCam(fDeltaTime, out_CamLoc, out_CamRot, out_FOV);
				return true;
			}
			out_CamLoc = v + ( HeadViewOffset  >> r );
			out_CamRot = r;
		}
		else
		{
			CalcDeathThirdPersonCam(fDeltaTime, out_CamLoc, out_CamRot, out_FOV);
		}
		return true;
		//return 
	}

	simulated function bool CalcDeathThirdPersonCam( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
	{
		local vector CamStart, HitLocation, HitNormal, CamDir;
		local vector Center;

		Center = Mesh.GetBoneLocation( 'Bip01' );
		CamStart = Center;
		//CamStart.Z += 1.5 * 40 + Mesh.Translation.Z;
		CamDir = Vector(out_CamRot) * GetCollisionRadius() * CurrentCameraScale;

		if (CurrentCameraScale < CameraScale)
		{
			CameraScaleTime += fDeltaTime;
			CurrentCameraScale += ( CameraScale - CurrentCameraScale ) * fDeltaTime / 2.0;
			CurrentCameraScale = FMin( CurrentCameraScale, CameraScale );
			
		}
		else if (CurrentCameraScale > CameraScale)
		{
			CameraScaleTime += fDeltaTime;
			CurrentCameraScale = ( default.CurrentCameraScale - CameraScale ) * CameraScaleTime/2.0;
			CurrentCameraScale = FMax( CurrentCameraScale, CameraScale );
		}
		//if (CamDir.Z > GetCollisionHeight())
		//{
		//	CamDir *= square(cos(out_CamRot.Pitch * 0.0000958738)); // 0.0000958738 = 2*PI/65536
		//}
		out_CamLoc = CamStart - CamDir;
		if (Trace(HitLocation, HitNormal, out_CamLoc, CamStart, false, vect(12,12,12)) != None)
		{
			out_CamLoc = HitLocation;
		}

		out_CamRot = Rotator( Center - out_CamLoc );


		return true;
	}

	simulated event Landed(vector HitNormal, Actor FloorActor)
	{
		local vector BounceDir;

		if( Velocity.Z < -500 )
		{
			BounceDir = 0.5 * (Velocity - 2.0*HitNormal*(Velocity dot HitNormal));
			TakeDamage( (1-Velocity.Z/30), Controller, Location, BounceDir, class'DmgType_Crushed');
		}
	}

	simulated function TakeDamage( int Damage, Controller InstigatedBy, Vector HitLocation,
						Vector Momentum, class<DamageType> damageType, optional TraceHitInfo HitInfo, optional class<Weapon> weaponBy )
	{
		local Vector				shotDir,ApplyImpulse,BloodMomentum;
		local class<avaDamageType>	avaDamage;
		local EShotInfo				shotInfo;
		local int					bloodsize;

		if ( InstigatedBy != None || EffectIsRelevant(Location, true, 0) )
		{
			avaDamage = class<avaDamageType>(DamageType);			

			CheckHitInfo( HitInfo, Mesh, Normal(Momentum), HitLocation );
			if ( avaDamage != None )
				avaDamage.Static.SpawnHitEffect(self, Damage > 0, Momentum, HitInfo.BoneName, HitLocation);
			if ( DamageType.default.bCausesBlood
				&& ((PlayerController(Controller) == None) || (WorldInfo.NetMode != NM_Standalone)) )
			{
				BloodMomentum = Momentum;
				if ( BloodMomentum.Z > 0 )
					BloodMomentum.Z *= 0.5;

				shotInfo = Check_ShotInfo( HitInfo.BoneName );
				if ( shotInfo == SI_Head )									bloodsize	= 0;
				else if ( shotInfo == SI_Stomach || shotInfo == SI_Chest )	bloodsize	= 1;
				else														bloodsize	= 2;
				
				if ( class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() )
					SpawnParticleEffectOnPawn( BloodEmitterClass.default.BloodEmitterTemplate[bloodsize], None, HitInfo.BoneName, HitLocation, rotator(-BloodMomentum) );	
				else
					SpawnParticleEffectOnPawn( BloodEmitterClassTeen.default.BloodEmitterTemplate[bloodsize], None, HitInfo.BoneName, HitLocation, rotator(-BloodMomentum) );	

				//SpawnParticleEffectOnPawn( BloodEmitterClass.default.EmitterTemplate, HitInfo.HitComponent, HitInfo.BoneName, HitLocation, rotator(-BloodMomentum) );
				//if ( class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() )
				//	SpawnParticleEffectOnPawn( BloodEmitterClass.default.EmitterTemplate, None, HitInfo.BoneName, HitLocation, rotator(-BloodMomentum) );
				//else
				//	SpawnParticleEffectOnPawn( BloodEmitterClassTeen.default.EmitterTemplate, None, HitInfo.BoneName, HitLocation, rotator(-BloodMomentum) );

			}

			if( (Physics != PHYS_RigidBody) || (Momentum == vect(0,0,0)) || (HitInfo.BoneName == '') )
				return;
			
			shotDir = Normal(Momentum);
			ApplyImpulse = (DamageType.Default.KDeathImpulse * shotDir);

			if( (avaDamage != None) && avaDamage.Default.bThrowRagdoll && (Velocity.Z > -10) )
			{
				ApplyImpulse += Vect(0,0,1)*DamageType.default.KDeathUpKick;
			}

			// AddImpulse() will only wake up the body for the bone we hit, so force the others to wake up
			Mesh.WakeRigidBody();
			Mesh.AddImpulse(ApplyImpulse, HitLocation, HitInfo.BoneName, true);
		}
	}

	simulated function ChangeViewTarget()
	{
		if ( LocalPC != None )
		{
			if ( bIsLocalPawn == false && LocalPC.ViewTarget == self )
			{
				avaPlayerController(LocalPC).ServerInitViewPlayer();
			}
		}
	}

	simulated function BeginState(Name PreviousStateName)
	{
		CustomGravityScaling = 1.0;
		DeathTime = WorldInfo.TimeSeconds;
		SetCollision(false,false);
		CylinderComponent.SetActorCollision(false, false);
		if ( Mesh != None )
		{
			Mesh.SetTraceBlocking(false, true);
			Mesh.SetActorCollision(false, true);
		}

		if ( bTearOff && (WorldInfo.NetMode == NM_DedicatedServer) )
			LifeSpan = 1.0;
		else
		{
			// 1.0 �� �Ŀ� Collision �� ����...
			// Why?	���� �� � Volume �ȿ� �־����� Ȯ���� �ʿ䰡 �ִ�. �ٷ� �������� Ȯ�� �� ����� ����...
			//		�� ���� Listen Server ���� ��ü�� ����������� Volume �ȿ� �ִ� ������ �Ǵ��� ������.....
			// SetTimer(1.0, false);

			SetTimer( `DEADVIEWTARGETCHANGETIME, false, 'ChangeViewTarget');

			// Respawn �� ������ ����Ÿ���̶�� RagDoll �� ���ְ� �׷��� �ʴٸ� Ragdoll �� ������ �ʴ´�.
			// Physics �� ������ ������ �������� �������� RagDoll �� ���ܵ��� �ʵ��� �Ѵ�.
			LifeSpan = 15.0;

			/*if ( avaGameReplicationInfo( WorldInfo.GRI ).bReinforcement )
				LifeSpan = RagDollLifeSpan;
			else
				LifeSpan = 0.0;*/


			//`log( "avaPawn.Dying.BeginState" @LifeSpan );
		}

		if ( Controller != None )
		{
			if ( Controller.bIsPlayer )
				Controller.PawnDied(self);
			else
				Controller.Destroy();
		}

		//`log( "avaPawn.Dying.BeginState" @LifeSpan );
	}
}

/* native������ PunchAngle�� �ٷ� �����ϴµ� */
simulated function SetPunchAngle( vector x )
{
	PunchAngle = x;	
}

/* native������ PunchAngle�� �ٷ� �����ϴµ� */
simulated function vector GetPunchAngle()
{
	return PunchAngle;
}

// DiedEx �� ���� ȣ���ؾ� �ȴ�...
function PlayerChangedTeam()
{
	Died( None, class'DmgType_Suicided', Location );
}

// ���� Health �� ��´�.
function DoDamage( int Damage )
{
	Health -= Damage;
}

function PlayHitEx(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo, bool bReducedByArmor, class<Weapon> weaponBy, EShotInfo shotInfo, int AbsorbDamage );

function bool GetNearPawn( float Distance, bool bSameTeam, out array<avaPawn> PawnList, optional int nMinimumCnt = 0 )
{
	local avaPawn	P;
	local int		nCnt;
	nCnt = 0;
	foreach VisibleActors( class'avaPawn', P)
	{
		if ( P == self )									continue;
		//if ( VSize( Location - P.Location ) > Distance )	continue;
		if ( bSameTeam == IsSameTeam( P )  )
		{
			PawnList[PawnList.length] = P;
			++nCnt;
		}
		if ( nMinimumCnt > 0 && nCnt >= nMinimumCnt )
			return true;
	}
	if ( nMinimumCnt > 0 && nCnt < nMinimumCnt )	
		return false;
	if ( nCnt > 0 )	return true;
	return false;
}

reliable server event function RequestEncryptTakeGunDamage( DamageData data, controller instigateBy )
{
	DecryptTakeGunDamage( data, instigateBy );
}

//reliable server 
event function RequestTakeGunDamage( BYTE HitDamage, controller instigateBy, name BoneName )
{
	local TraceHitInfo			HitInfo;
	local avaWeapon				InstigateByWeapon;
	local float					ArmorRatio;
	local class<avaDamageType>	DmgType;

	// �츮 ���ӿ��� Love Shot �� ����....
	if ( instigateBy.Pawn == None || instigateBy.Pawn.Health <= 0 )		return;

	InstigateByWeapon		= avaWeapon( instigateBy.Pawn.Weapon );

	HitInfo.BoneName	= BoneName;

	if ( avaWeap_BaseGun( InstigateByWeapon ) != None )
	{
		ArmorRatio	= avaWeap_BaseGun( InstigateByWeapon ).bulletType.default.ArmorRatio;
		DmgType		= avaWeap_BaseGun( InstigateByWeapon ).GetInstantHitDamageTypes(InstigateByWeapon.CurrentFireMode);

		// Damage Hack ���� �ǽɹ��� ���� Damage �� ������...
		if ( avaWeap_BaseGun( InstigateByWeapon ).GetExpectableMaxDamage() < HitDamage )
		{
			return;
		}
	}
	else
	{
		ArmorRatio  = 0.0;
		DmgType		= class<avaDamageType>( InstigateByWeapon.InstantHitDamageTypes[InstigateByWeapon.CurrentFireMode] );
	}
	TakeGunDamage( HitDamage, 
				   ArmorRatio,
				   instigateBy,
				   HitInfo,
				   InstigateByWeapon.InstantHitMomentum[InstigateByWeapon.CurrentFireMode] * vector( instigateBy.Pawn.GetViewRotation() ), // * ImpactInfo.RayDir
				   DmgType,
				   InstigateByWeapon.Class );
}

function TakeGunDamage( float HitDamage, float ArmorParam, controller InstigatingController, TraceHitInfo HitInfo, vector HitMomentum, class<DamageType> damageType, class<Weapon> Gun )
{
	//CheckHitInfo( HitInfo, Mesh, Normal(impact.RayDir), impact.HitLocation );

	if ( Check_ShotInfo( HitInfo.BoneName ) == SI_Generic) return;			

	CurrentArmorParam = ArmorParam;

	//TakeDamage( HitDamage, InstigatingController, Impact.HitLocation, HitMomentum, damageType, HitInfo, Gun );
	TakeDamage( HitDamage, InstigatingController, Mesh.GetBoneLocation( HitInfo.BoneName ), HitMomentum, damageType, HitInfo, Gun );	
}

function TakeDamage( int Damage, 
					 Controller instigatedBy, 
					 Vector hitlocation,
					 Vector momentum, 
					 class<DamageType> damageType, 
					 optional TraceHitInfo HitInfo, 
					 optional class<Weapon> weaponBy)
{
	local int			actualDamage;
	local Controller	Killer;
	local bool			bReducedByArmor;
	local EShotInfo		shotInfo;
	local int			AbsorbDamage;
	local array<avaPawn>	FriendlyPawn;
	local int				PrvHealth;

	if ( Role < ROLE_Authority )	return;
	if ( Health <= 0 )				return;

	PrvHealth = Health;

	if ( damagetype == None )									DamageType = class'DamageType';
	if ( (Physics == PHYS_None) && (DrivenVehicle == None) )	SetMovementPhysics();

	if (Physics == PHYS_Walking && damageType.default.bExtraMomentumZ)
		momentum.Z = FMax(momentum.Z, 0.4 * VSize(momentum));

	if ( instigatedBy == self )
		momentum *= 0.6;

	momentum = momentum/Mass;

	ActualDamage = Damage;
	WorldInfo.Game.ReduceDamage(ActualDamage, self, instigatedBy, DamageType);

	// HitInfo�� ���ٸ� Check �ؾ� ���� ������...????
	if ( HitInfo.BoneName == '' )
		CheckHitInfo( HitInfo, Mesh, Normal(Momentum), HitLocation );

	shotInfo = Check_ShotInfo( HitInfo.BoneName );

	AbsorbDamage = 0;
	if ( weaponBy != None )
	{
		// Weapon �� Gun �� �ƴ϶�� WallShot�� ��ȿ!
		if ( class<avaWeap_BaseGun>(WeaponBy) == None || class<avaWeapon>( weaponBy ).default.WeaponType == WEAPON_RPG )
			bWallShot = false;

		if ( class<avaWeapon>( weaponBy ).default.WeaponType != WEAPON_GRENADE && class<avaWeapon>( weaponBy ).default.WeaponType != WEAPON_RPG )
		{
			bReducedByArmor = AdjustDamageEx(ActualDamage, shotInfo, instigatedBy, DamageType, AbsorbDamage );
			// BS3 �����ϻ� ���....
			//if ( class<avaWeapon>( weaponBy ).default.WeaponType != WEAPON_SNIPER && StressLevel == 2 && Health > 1 && actualDamage > Health )
			//	ActualDamage = Health - 1;
		}
		else
		{
			///@@@@@@@@@@@@@@@@ ��ȹ test
			///@@@@��ȹ part ��û ; ���� ���ֱ� =_=; deif 2006/7/19	
			// Damage �� �����̶� ������ Helmet �� ���󰣴�... ���� ��û =_=; oz99 2007/2/6							
			if ( ActualDamage > 0 )
			{
				TakeOffHelmet( Momentum/5.0 );
				if ( Armor_Stomach > ActualDamage )	Armor_Stomach -= ActualDamage;
				else								Armor_Stomach =  0;
			}
		}
	}
	else
	{
		bWallShot = false;
	}


	if ( actualDamage > 0 && bIsDash && Controller != None )	
		avaPlayerController( Controller ).ClientCancelDash();	

	// call Actor's version to handle any SeqEvent_TakeDamage for scripting
	// Pawn �� TakeDamage Code �� �����Ѵ�.
	Super( Actor ).TakeDamage(actualDamage,instigatedBy,hitlocation,momentum,damageType,HitInfo,weaponBy);

	DoDamage( actualDamage );

	// �ڽ��� DMG�� ������ ���ҵ� HP�� �縸ŭ Stress Value ����
	AddStress( actualDamage );

	if ( HitLocation == vect(0,0,0) )
		HitLocation = Location;

	//if ( DamageType.default.bCausedByWorld && (instigatedBy == None || instigatedBy == controller) && LastHitBy != None )
	//	Killer = LastHitBy;
	//else if ( instigatedBy != None )
	Killer = instigatedBy;

	PlayHitEx(actualDamage,InstigatedBy, hitLocation, damageType, Momentum, HitInfo, bReducedByArmor, weaponBy, shotInfo, AbsorbDamage);

	if ( Health <= 0 )
	{
		// Play Sound - "Man Down!", "Enemy Down!" - 1067 means 20m
		if ( GetNearPawn( `AUTORADIOMSG_DISTANCE, true, FriendlyPawn, 2 ) == true )
		{
			if ( avaPlayerReplicationInfo(  PlayerReplicationInfo ).bSquadLeader )
				avaPlayerController(FriendlyPawn[0].Controller).RaiseAutoMessage( AUTOMESSAGE_LDDown, false );
			else
				avaPlayerController(FriendlyPawn[0].Controller).RaiseAutoMessage( AUTOMESSAGE_ManDown, false );
		}

		// ���� ���� �ƴ� ��쿡�� KillCam �� �����ִ��� ��ġ���� �Ѵ�...
		if ( instigatedBy != None && instigatedBy.Pawn != None && !avaPlayerController( instigatedBy ).IsSameTeam(self) )
		{
			// Headshot���� ���� ��쿡�� KillCam ����!
			if (VSize(Location - instigatedBy.Pawn.Location) > 5 * 52.0)
			{
				// ���� ���� �����... ���� ���� ������ �����ִ� ����...
				StartKillCam( Controller, instigatedBy.Pawn, true );
				
				if (bDamageHead ||  class<avaDmgType_Explosion>( damageType ) != None )
				{
					// ������ ���� �����... ���� ���� ������ �����ִ� ����...
					StartKillCam( Killer, self, false );
				}
			}

			// enemy down �� ������ �����ش�...
			avaPlayerController( instigatedBy ).RaiseAutoMessage( AUTOMESSAGE_EnemyDown, false );
		}
		TearOffMomentum = momentum;		
		
		Died(Killer, damageType, HitLocation,, weaponBy);		
	}
	else
	{
		if ( Health < 25 && PrvHealth >= 25 )
			avaPlayerController(Controller).PlayDamageBreathSound();

		// deif �ǰݽ� �ӵ� 50% ���� (QA ���� �ǰ� �ݿ�) 2006-04-08
		Velocity.x *= 0.5;
		Velocity.y *= 0.5;

		//AddVelocity( momentum, HitLocation, DamageType, HitInfo );
		if ( Controller != None )
		{
			Controller.NotifyTakeHit(instigatedBy, HitLocation, actualDamage, DamageType, Momentum);
		}
		if (DrivenVehicle != None)
		{
			DrivenVehicle.NotifyDriverTakeHit(InstigatedBy, HitLocation, actualDamage, DamageType, Momentum);
		}
		//if ( instigatedBy != None && instigatedBy != controller )
		//{
		//	LastHitBy = instigatedBy;
		//}
	}
	MakeNoise(1.0);
	bWallShot	=	false;
}

function StartKillCam( Controller C, Actor Target, bool bIsKiller )
{
	local PlayerController	PC;
	local Actor				Viewtarget;
	
	avaPlayerController(C).StartKillCam( Target, bIsKiller );

	Viewtarget =  C.Pawn;
	if ( Viewtarget == None )	
		return;

	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		if ( C == PC )	continue;

		if ( PC.Viewtarget == Viewtarget )
			avaPlayerController( PC ).StartKillCam( Target, bIsKiller );
	}
}

simulated function PlayTakeHitEffects( optional bool bCheckRelevant = true );

`devexec simulated function TestTakeHit()
{
	PlayTakeHitRagDoll( Vect(0,100,0), Vect(0,0,0), 32 );
}

simulated function PlayTakeHitRagDoll( vector Momentum, vector Hitlocation, int BoneIndex )
{
	local float DamageMultiflier;

	if( avaPlayerController(LocalPC) != none && avaPlayerController(LocalPC).bDisablePlayTakeHitRagdoll )
		return;

	if ( WorldInfo.TimeSeconds - LastRenderTime > TakeHitPhysicsBlendOutSpeed )
		return;

	if ( bBlendOutTakeHitPhysics == true )	DamageMultiflier = fComboHitPhysicsMultiflier;
	else									DamageMultiflier = fTakeHitPhysicsMultiflier;

	if (Mesh.PhysicsAssetInstance != None)
	{
	}
	else
	{
		Mesh.SetPhysicsAsset(TakeHitPhysicsAsset);
		Mesh.SetHasPhysicsAssetInstance(true);
		Mesh.PhysicsWeight = fTakeHitPhysicsStartWeight;
		Mesh.bUpdateKinematicBonesFromAnimation = true;
		Mesh.bUpdateJointsFromAnimation = true;
		Mesh.SetComponentRBFixed(false);
	}

	Mesh.AddImpulse( Momentum * DamageMultiflier , Hitlocation, Mesh.GetBoneNameByIndex( BoneIndex ) );
	bBlendOutTakeHitPhysics = true;
}

simulated event TakeHitBlendedOut()
{
	Mesh.PhysicsWeight = 0.0;
	Mesh.SetHasPhysicsAssetInstance(false);
	Mesh.bUpdateKinematicBonesFromAnimation = false;
	Mesh.bUpdateJointsFromAnimation = false;
	bBlendOutTakeHitPhysics=false;
}

/**
	Mesh Visibility ���� �Լ���
**/

/** 
	BecomeViewTarget �� ȣ�� �Ǵ� ���
	1. Camera �� SetViewTarget �� ȣ�� �� ���
	2. Camera �� ���� ��� Native ���� SetViewTarget �� ȣ�� �� ���
	3. Pawn �� PostBeginPlay ���� Role �� ROLE_AutonomousProxy �� ���
	4. Controller �� WaitingForPawn State ���� Tick �� Pawn �� None �� ���

**/

// ������ ��� Spectator �� Pawn �� ���� BecomeViewTarget �� ���´�.
// input �� Spectator �� Controller �̴�.
simulated event BecomeViewTarget( PlayerController PC )
{
	local avaPlayerController avaPC;

	bUpdateEyeHeight = true;


	if ( PC != None && PC.Pawn != None && PC.Pawn != Self )
	{
		`warn( "********* avaPawn.BecomeViewTarget PlayerController already has valid Pawn" @PC @PC.Pawn @Self );
	}

	if ( Controller != None && PC == Controller && Controller.IsLocalPlayerController() )
	{
		bIsLocalPawn = true;
	}

	if ( PC != None && PC.IsLocalPlayerController() )
	{
		SetForcedLodLevel( 1 );
		SetFootstepSoundSpatialization( false );
		bEnableFootPlacement = false;
	}

	// ������ ���
	if ( PC.Role == ROLE_Authority )
	{	
		// �ٸ� ����� Controller �� ���ؼ� BecomeViewTarget �� ȣ�� �� ����̴�.
		if ( PC.RemoteRole != ROLE_SimulatedProxy && PC.RemoteRole != ROLE_None )	
			return;
	}

//	`log( "avaPawn.BecomeViewTarget" @PC.Role @PC.RemoteRole );

	avaPC = avaPlayerController(PC);

	if ( avaPC.MyHUD != None )
		avaHUD( avaPC.MyHUD ).PawnSpawned();

	if ( avaPC != None )
	{
		avaCharacter(Self).SetMeshVisibility(avaPC.bBehindView);
	}
	else
	{
		avaCharacter(Self).SetMeshVisibility(!IsFirstPerson());
	}
}

simulated event PlayEffect_EjectBullet()
{
	if (CurrentWeaponAttachment != None)
	{
		if ( !WorldInfo.bDropDetail )
		{
			if ( IsFirstPerson() )
			{
				if ( avaWeap_BaseGun(Weapon) != None )
					avaWeap_BaseGun(Weapon).EjectBullet();
			}
			else
			{
				CurrentWeaponAttachment.EjectBullet();
			}
		}
	}
}


function AddPickUpProvider( avaPickUpProvider PP )
{
	TouchedPP = PP;
}

function RemovePickUpProvider( avaPickUpProvider PP )
{
	if ( TouchedPP == PP )
		TouchedPP = None;
}
/************************************************************************************
	SwappedPick ó�� ���� Function
***********************************************************************************/
function AddPickUp( avaPickUp P )
{
	Local int FindIndex;

	FindIndex = TouchedPickup.Find(P);
	if( FindIndex < 0)
	{
		if( TouchedPickup.Length < `MAX_TOUCHED_PICKUP )
		{
			TouchedPickUp[ TouchedPickup_Count ] = P;
			TouchedPickUp_Rep[ TouchedPickup_Count ] = P;
			TouchedPickup_Count++;
		}
		else
			`warn("can't contain more TouchedPickup Items. increase 'MAX_TOUCHED_PICKUP'");
	}
}

function RemovePickUp( avaPickUp P )
{
	local int idx;
	idx = TouchedPickUp.Find( P );
	if ( idx < 0 )	return;
	TouchedPickUp.Remove( idx, 1 );
//	TouchedPickUp_Rep.Remove( idx,1 );
	TouchedPickup_Count--;
}

/************************************************************************************
	Pawn ���� �浹������ Damage �� ���� �ʱ� ���ؼ�...
	UT������ Vehicle �� �浹������ Damage �� �ֱ����ؼ� ������ ����...
	2006/3/22 by OZ
***********************************************************************************/
function CrushedBy(Pawn OtherPawn)
{
}

/************************************************************************************
	Quick voice chat �� ����ȣ Animation �� Play �ϱ� ���� Function
	2006/3/22 by OZ
***********************************************************************************/
simulated function PlayQVCAnim(int QVC_Anim )
{
	if ( QVC_Anim < 0 )	return;
	LastQuickVoiceMsg = QVC_Anim;
	PlayAnimByEvent( EBT_QVC );
	if ( CurrentWeapon != None )	CurrentWeapon.RaiseQuickVoice( QVC_Anim );
	if ( avaPlayerController( LocalPC ).IsSameTeam( self ) && (WorldInfo.NetMode == NM_Standalone || LocalPC.Pawn != self ))
	{
		avaPlayerController(LocalPC).SetChatCam( self );
	}
}

//==================================================================================
// Dash ���� Code, Dash �� ��� ���� ���¿� ���� ������ ���� �ִ�.
//==================================================================================
reliable server function NotifyDoDash( bool bFlag )
{
	bIsDash = bFlag;
	avaWeapon(Weapon).ForceDoDash( bFlag );
	avaPlayerReplicationInfo( Controller.PlayerReplicationInfo ).SprintStat( bFlag );
	avaGame(WorldInfo.Game).TrackSprintStat( Controller.PlayerReplicationInfo, bFlag );
}

simulated function bool CanDoDash()
{
	return ( Weapon.IsInState( 'Active' ) &&			// Weapon �� State �� Active State �̾�߸� �Ѵ�.
			 Physics == PHYS_Walking &&					// Pawn �� Physics State �� PHYS_Walking �̾�߸� �Ѵ�.
			 !bIsWalking &&								// Pawn �� Crouched �� Walking �����̸� Dash �� �� ����.
			 avaWeapon( Weapon ).IsAvailableDash() );	// Weapon �� ���� Dash �� �����ؾ� �Ѵ�. ex) Zoom ���¿����� Dash �Ұ�
}

// Local Player �� ������ Version �̴�....
simulated function bool DoDash( bool bUpdating )
{
	if ( bUpdating && CanDoDash() )
	{
		if ( bIsDash == false )
			ForceDoDash( true );
	}
	else 
	{
		if ( bIsDash == true )
			ForceDoDash( false );
	}
	return true;
}

simulated function ForceDoDash( bool bFlag )
{
	bIsDash = bFlag;
	NotifyDoDash( bIsDash );
	avaWeapon(Weapon).ForceDoDash( bIsDash );
}

// Ladder Volume ���� ������ Dash �� ��ҵȴ�.
function ClimbLadder(LadderVolume L)
{
	Super.ClimbLadder( L );
	if ( Role == ROLE_Authority && bIsDash)
		avaPlayerController( Controller ).ClientCancelDash();
}

event PhysicsVolumeChange( PhysicsVolume NewVolume )
{
	Super.PhysicsVolumeChange( NewVolume );
	if ( NewVolume.bWaterVolume && bIsDash )
		avaPlayerController( Controller ).ClientCancelDash();
}

// ����(�ڰ�����...)�� ���� Yaw ȸ������ ������ ���� Override 2006/06/05 by OZ
simulated function ProcessViewRotation( float DeltaTime, out rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	Super.ProcessViewRotation( DeltaTime, out_ViewRotation, out_DeltaRot );

	if ( bLimitYawAngle ) // Yaw Angle �� ���ѵȴٸ�...
		out_ViewRotation = avaPlayerController(Controller).LimitViewYawRotation( out_ViewRotation, MinLimitYawAngle, MaxLimitYawAngle );
	if ( bLimitPitchAngle )
		out_ViewRotation = avaPlayerController(Controller).LimitViewPitchRotation( out_ViewRotation, MinLimitPitchAngle, MaxLimitPitchAngle );
}

function SetHeavyWeaponType( EBlendExclusiveType hwType )
{
	HeavyWeaponType = hwType;
	ResetEventBlend();
}

function InstallHeavyWeapon( EBlendExclusiveType hwType, optional float MinYaw, optional float MaxYaw, optional float MinPitch, optional float MaxPitch )
{
	SetHeavyWeaponType( hwType );

	if ( HeavyWeaponType != EXC_None )
	{
		LimitYawAngle( true, MinYaw, MaxYaw );
		LimitPitchAngle( true, MinPitch, MaxPitch );
	}
	else
	{
		LimitYawAngle( false );
		LimitPitchAngle( false );
	}
}

function LimitYawAngle( bool bEnable, optional float Min, optional float Max )
{
	bLimitYawAngle = bEnable;
	MinLimitYawAngle = Min;
	MaxLimitYawAngle = Max;
}

function LimitPitchAngle( bool bEnable, optional float Min, optional float Max )
{
	bLimitPitchAngle = bEnable;
	MinLimitPitchAngle = Min;
	MaxLimitPitchAngle = Max;
}

function SetRemoteViewYaw( int NewRemoteViewYaw )
{
	// RemoteViewYaw �� ��ġ ��ȭ�⸦ ����� ��쿡�� ����Ѵ�.
	if ( GripHeavyWeapon != None )
		RemoteViewYaw = NewRemoteViewYaw;
}


reliable client function GrippedHeavyWeapon( Weapon weap )
{
	if ( weap != None )	
	{
		InvManager.SetCurrentWeapon( weap );
	}
	else			
	{
		InvManager.SwitchToBestWeapon( true );
	}
}

//=================================================================================================
// Weapon Attachment ���� Function ��....
//=================================================================================================
function ChangeWeaponAttachment( avaWeapon w, bool bAttach )
{
	if ( !bAttach )
	{
		if ( CurrentWeapon != w )	
			return;
		else
		{
			CurrentWeapon = None;
			return;
		}
	}
	CurrentWeapon = w;
	if ( WorldInfo.NetMode != NM_DedicatedServer )
		WeaponAttachmentChanged();
}

simulated function WeaponAttachmentChanged()
{	
	if ( CurrentWeapon != None && CurrentWeaponAttachment == CurrentWeapon.WeaponAttachment )	return;
	if ( CurrentWeaponAttachment != None )
	{
		// Order of this bracelet is important
		CurrentWeaponAttachment.ChangeAttachmentState( false );
		if( bForceHidden )
			CurrentWeaponAttachment.ChangeVisibility(false);
		CurrentWeaponAttachment = None;
	}
		
	if ( CurrentWeapon != None )
	{
		if ( CurrentWeapon.WeaponAttachment != None )
		{
			CurrentWeaponAttachment = CurrentWeapon.WeaponAttachment;
			bRightHandedWeapon		= CurrentWeaponAttachment.bRightHandedWeapon;

			CurrentWeaponAttachment.ChangeAttachmentState( true );
			if( bForceHidden )
				CurrentWeaponAttachment.ChangeVisibility(false);

			ResetEventBlend();

			// None�̿��� ���â �ߴ� �κ� ����.(2007/04/24)
			PlayAnimByEvent( EBT_BringUp );

			WeaponState = 0;
			PrvWeaponState = 0;
		}
	}
	//`log( "avaPawn.WeaponAttachmentChanged" @CurrentWeaponAttachment );
}

function AddWeapon( avaWeapon w )
{
	local int i;
	for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	{
		if ( PossessedWeapon[i] == None )
		{
			PossessedWeapon[i] = w;
			//NetUpdateTime = WorldInfo.TimeSeconds - 1;
			ChangePossessedWeapon();
			return;
		}
	}
	`warn( "avaPawn.AddWeapon but Too many Weapon" @w );
}

function RemoveWeapon( avaWeapon w )
{
	local int i;

	for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	{
		if ( PossessedWeapon[i] == w )
		{
			PossessedWeapon[i] = None;
			//NetUpdateTime = WorldInfo.TimeSeconds - 1;
			ChangePossessedWeapon();
			return;
		}
	}
	`warn( "avaPawn.RemoveWeapon but cannot find weapon" @w );
}

simulated function ChangePossessedWeapon()
{
	local int i;	

	for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	{
		if ( PossessedWeapon[i] != PrvPossessedWeapon[i] )
		{
			if ( PossessedWeapon[i] != None )	
			{
				AddPossessedWeapon( PossessedWeapon[i] );
			}
			
			if ( PrvPossessedWeapon[i] != None )
			{
				RemovePossessedWeapon( PrvPossessedWeapon[i] );
			}

			PrvPossessedWeapon[i] = PossessedWeapon[i];
		}
	}
	
	if (bTargetted)
	{
		MarkSeeThroughGroupIndex( DefaultTeam + 3 );
	}
	else
	{
		MarkSeeThroughGroupIndex( DefaultTeam + 1 );
	}
}

simulated function AddPossessedWeapon( avaWeapon w )
{
	local avaWeaponAttachment	attachment;

	if( w.bSpecialInventory == true )
		SpecialInventory = w;

	if ( w.WeaponAttachment != None )
	{
		`warn( "avaPawn.AddPossessedWeapon But weapon has already attachment" @w );
		return;
	}

	attachment = Spawn( w.default.AttachmentClass, self );
	Assert(attachment != None);
	if( attachment == None )
		return;
	attachment.AttachTo( self, w );
	w.WeaponAttachment = attachment;
	
	// AddPossessedWeapon ���� WeaponAttachmentChanged �� ���� ȣ��Ǵ� ��찡 �ֱ� ������...
	if ( CurrentWeapon != None && CurrentWeapon == w && CurrentWeaponAttachment == None )
		WeaponAttachmentChanged();
	else if( bForceHidden && Attachment != None)
		Attachment.ChangeVisibility(false);

	UpdateWeaponStatusStr();
}

function UpdateWeaponStatusStr()
{
	local int		i;
	local string	BestStr;
	local int		BestPriority;
	BestPriority = -1;
	if ( Controller == None )	return;
	for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	{
		if ( PossessedWeapon[i] == None )			continue;
		if ( PossessedWeapon[i].StatusStr == "" )	continue;
		if ( BestPriority < PossessedWeapon[i].StatusPriority )
		{
			BestPriority = PossessedWeapon[i].StatusPriority;
			BestStr		 = PossessedWeapon[i].StatusStr;
		}
	}
	avaPlayerReplicationInfo( Controller.PlayerReplicationInfo ).SetWeaponStatusStr( BestStr );
}

simulated function RemovePossessedWeapon( avaWeapon w )
{
	if( w.bSpecialInventory == true )
		SpecialInventory = None;

	if ( w.WeaponAttachment == None )
	{
		`warn( "avaPawn.RemovePossessedWeapon But weapon has not attachment" @w );
		return;
	}
	w.WeaponAttachment.DetachFrom( self );
	w.WeaponAttachment.Destroy();
	w.WeaponAttachment = None;

	UpdateWeaponStatusStr();
}

// PointMan = 0, RifleMan = 1, Sniper = 2
simulated function int GetTypeID()	
{ 
	return TypeID; 
}

// Modifier ������ ���� function...
simulated function AddItemMesh( string MeshName, name SocketName, optional float MaxVisibleDistance );
//simulated function AddExtraMesh( string MeshName, optional float MaxVisibleDistance );
simulated function SetHelmet( string MeshName, optional string SkinName );
simulated function AddHelmetAccessory( string MeshName, name SocketName, optional float MaxVisibleDistance );

`devexec simulated function SetMiniMapScale( float Scale )
{
	MiniMapScale = Scale;
}

`devexec simulated function SetCameraDistance( float Scale )
{
	CameraScale = Scale;
}

simulated event vector GetCameraWorstCaseLoc()
{
	local vector WorstLocation;

	//WorstLocation = Location + (WorstLocOffset >> Rotation);
	WorstLocation = Mesh.GetBoneLocation( 'Bip01_Head', 0 ) + (WorstLocOffset >> LastRot);

	return WorstLocation;
}

simulated function float GetExposureCenterRegionScale()
{
	local float Scale;	

	if (CurrentWeapon != None)
		Scale = CurrentWeapon.GetExposureCenterRegionScale();

	if (Scale < 1)
		return Scale;	
	else
		return 1.0;
}

/** bLastTakeHitVisibility�� �������ֱ� ����. (������� ������ �������� �������� �� �� �־�����) */
function PlayHit(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo)
{
	Local Pawn P;
	Local bool bInSightInstigator;

	super.PlayHit(Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);

	bInSightInstigator = false;
	if( InstigatedBy != None && InstigatedBy.Pawn != None )
	{
		foreach InstigatedBy.Pawn.VisibleActors( class'Pawn', P)
		{
			if( P == Self )
				bInSightInstigator = true;
		}
	}
	bLastTakeHitVisibility = bInSightInstigator;
}

function AddDogTag()
{
	local	int						i;
	local	name					EventName;
	local	Sequence				GameSeq;
	local	bool					bPack;

	++DogTagCnt;
	EventName = 'PickDogTag';
	if ( DogTagCnt >= avaGameReplicationInfo(WorldInfo.GRI).DogTagPackCnt )
	{
		bPack		= true;
		DogTagCnt	= 0;
		EventName	= 'PickDogTagPack';
	}
	
	avaPlayerController( Controller ).GetDogTag( bPack );
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none && DogTagPickUpEvents.length == 0 )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_PickDogTag', true, DogTagPickUpEvents );
	}
	
	for ( i = 0 ; i < DogTagPickUpEvents.Length ; ++i )
		avaSeqEvent_PickDogTag( DogTagPickUpEvents[i] ).Trigger( EventName, Controller );
}

/**************************************************************************************************
	Functons For Targetting by Squad Leader
**************************************************************************************************/
simulated function SetTargetted( bool bActive )
{
	ReportTargetted( bActive );
}

reliable server function ReportTargetted( bool bActive )
{
	ClearTimer( 'ClearTargetted' );
	if ( bActive == true )
		SetTimer( `MAX_TARGETTIME, false, 'ClearTargetted' );		

	// Target �� Pawn �� �ٽ� Targetting �� �Ǿ���...
	if ( bTargetted == true && bActive == true )
		++TargettedCnt;

	bTargetted = bActive;
	UpdateTargetted();	
}

simulated function UpdateTargetted()
{
	if ( bTargetted == true )
	{
		TargettedTime = WorldInfo.TimeSeconds;
		if ( bDrawNotifyTargetted && IsLocallyControlled() )
			SoundGroupClass.static.PlayTargettedSound( self );
	}
	UpdateSeeThrough();
}

function ClearTargetted()
{
	bTargetted = false;
	UpdateTargetted();
}

simulated event UpdateSeeThrough()
{
	local int Flags;
	
	Flags = DefaultTeam + 1;
		
	if (bTargetted)
	{
		Flags += 2;
	}
	
	if (bDetected)
	{
		Flags += 4;
	}
	
	MarkSeeThroughGroupIndex( Flags );
}

simulated function MarkSeeThroughGroupIndex( int Index )
{
	local int i;	
	
	for ( i = 0 ; i < `POSSESSED_WEAPON_CNT ; ++ i )
	{
		if ( PossessedWeapon[i] != None && PossessedWeapon[i].WeaponAttachment != None)
		{
			PossessedWeapon[i].WeaponAttachment.MarkSeeThroughGroupIndex( Index );
		}
	}
}

simulated function UpdateKillTime()
{
	LastKillTime = WorldInfo.TimeSeconds;
}

simulated event byte GetTeamNum()
{
	local avaPlayerReplicationInfo	avaPRI;
	avaPRI = GetValidPRI();
	return ( avaPRI != None && avaPRI.Team != None ) ? avaPRI.Team.TeamIndex : 255;
}

// Volume....
function TouchBombVolume( TriggerVolume InBombVolume )
{
	BombVolume = InBombVolume;
}

function UnTouchBombVolume( TriggerVolume InBombVolume )
{
	if ( BombVolume == InBombVolume )
		BombVolume = None;
}

simulated function TouchUseVolume( avaUseVolume useVolume )
{
	TouchedUseVolume = useVolume;
}

simulated function UnTouchUseVolume( avaUseVolume useVolume )
{
	if ( useVolume == TouchedUseVolume )
		TouchedUseVolume = None;
}

simulated function TriggerVolume GetBombVolume()
{
	return BombVolume;
}

/** Kismet hook for kicking a Pawn out of a vehicle */
function OnExitVehicle(avaSeqAct_ExitVehicle Action)
{
	if (DrivenVehicle != None)
	{
		DrivenVehicle.DriverLeave(true);
	}
}

simulated event Rotator GetViewRotation()
{
	local avaPlayerController avaPC;
	avaPC = GetValidController();
	if ( avaPC != None )
		return avaPC.Rotation;
	else if ( Role < ROLE_Authority )
	{
		if ( LocalPC.Viewtarget == self )
			return LocalPC.BlendedTargetViewRotation;
	}
	return Rotation;
}

simulated function bool CanEnterVehicle()
{
	if ( !avaWeapon(Weapon).CanEnterVehicle() )	
		return false;
	return true;
}

//=================================================================================================
// �ش� Pawn �� ���� ������迡 �ش��ϴ� Pawn �ΰ��� Return ����....
//=================================================================================================
simulated event bool IsBIA()
{
	local int						BIAAccountID;
	local avaPlayerReplicationInfo	avaPRI;
	BIAAccountID = class'avaNetHandler'.static.GetAvaNetHandler().GetBIAAccountID();
	if ( BIAAccountID == 0 )
		return FALSE;
	avaPRI = GetValidPRI();
	return ( avaPRI != None ) ? avaPRI.AccountID == BIAAccountID : FALSE;
}

defaultproperties
{
	Components.Remove(Sprite)

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0020.000000
		CollisionHeight=+0045.000000
		BlockZeroExtent=false
	End Object
	CylinderComponent=CollisionCylinder

	ViewPitchMin=-15000
	ViewPitchMax=15000

	WalkingPct=+0.4
	CrouchedPct=+0.4
	SprintPct=+1.3
	CrouchSprintPct=1.0
	BaseEyeHeight=36.0

/// 2006.3.13. deif :) [�ں�����Ʈ��԰� ���� �� ����� ��ġ]
/// ���� �� ���ϰ� �� eye height�� ������ ���� �� ���� (-24)
	BaseEyeHeightWhenCrouched=12.0
	EyeHeight=32.0

	AirSpeed=+00400.000000
	GroundSpeed=+00400.0
	JumpZ=+00320.000000			/// sqrt( 2 * gravity * height )
	LadderSpeed=+160.0
	WaterSpeed=+00320.000000

	AccelRate=700

	CrouchHeight=33.0
	CrouchRadius=20.0
	WalkableFloorZ=0.78

	AlwaysRelevantDistanceSquared=+1960000.0
	InventoryManagerClass=class'avaInventoryManager'

	MeleeRange=+20.0
	bMuffledHearing=true

	Buoyancy=+000.99000000
	UnderWaterTime=+00020.000000
	bCanStrafe=True
	bCanSwim=true
	RotationRate=(Pitch=20000,Yaw=20000,Roll=20000)
	AirControl=+0.35
	bStasis=false
	bCanCrouch=true
	bCanClimbLadders=True
	bCanPickupInventory=True
	SightRadius=+12000.0


	SoundGroupClass=class'avaGame.avaPawnSoundGroup'
	BloodEmitterClass=class'avaGame.avaEmit_BloodSpray'
	BloodEmitterClassTeen=class'avaGame.avaEmit_BloodSprayTeen'	//Teen Version �� Blood Emitter 
	SparkEmitterClass=class'avaGame.avaEmit_Spark'

	MaxStepHeight=26.0
	MaxJumpHeight=45.0

	RagdollLifespan=18.0

	CurrentCameraScale	=1.0
	CameraScale			=5.0
	CameraScaleMin		=3.0
	CameraScaleMax		=40.0

	//==========================================================================
	// FootPlacement �� ������....
	LeftFootBone=Bip01_L_Foot
	RightFootBone=Bip01_R_Foot
	LeftFootControlName=LeftFootControl
	RightFootControlName=RightFootControl
	bEnableFootPlacement=true
	
	MaxFootPlacementDistSquared=56250000.0 // 7500 squared
	//==========================================================================

	CustomGravityScaling=1.0
	SlopeBoostFriction=0.2
	FireRateMultiplier=1.0

	MaxFallSpeed=+750.0
	LastPainSound=-1000.0

	bReplicateRigidBodyLocation=true

	WeaponState = 0

	bDamageHead=false

	bHasHelmet=true
	TOH_Play=false

	bCanWalkOffLedges=true /// ���� ���� ������ �� �ְ�

	FootStepSoundLeft=0

	SpeedPctByChrType	=	1.0
	MaxSpeedByChrType	=	400
	Armor_Stomach		=	40
	Armor_Head			=	9999
	HealthMax			=	100	
	ArmorMax			=	40
	Absorb_Stomach		=	0.5
	Absorb_Head			=	0.5

	bAlwaysRelevant		=	true

	/// Occluder�� ������� ���� ���Դϴ�. :)
	// Components
	/*Begin Object Class=avaBulletTrailComponent name=BulletTrailComponent0				
	End Object
	BulletTrailComponent=BulletTrailComponent0
	Components.Add(BulletTrailComponent0)	*/
	

	Helmet_DamageThreshold = 0	

	//bPushesRigidBodies=true

	ChrBaseSpeedPct				=	1.0
	ChrAimSpeedPct				=	1.0	
	ChrWalkSpeedPct				=	1.0
	ChrCrouchSpeedPct			=	1.0
	ChrCrouchAimSpeedPct		=	1.0
	ChrSprintSpeedPct			=	1.0
	ChrCrouchSprintSpeedPct		=	1.0

	FallingDamageAmp			=	1.0	
	MiniMapScale				=	1.0
	ProjectileVelAmp			=	1.0
	ThrowableWeapReadyAmp		=	1.0	

	OffsetLow = (X=-25,Y=15,Z=-30)
	OffsetMid = (X=-25,Y=15,Z=0)
	OffsetHigh = (X=5,Y=15,Z=30)

	OffsetLow_Crouch = (X=-25,Y=15,Z=-30)
	OffsetMid_Crouch = (X=-25,Y=15,Z=12.5)
	OffsetHigh_Crouch = (X=15,Y=15,Z=30)	
	
	//FOV 70
	Viewport_OffsetLow=(X=-15,Y=0,Z=0)
	Viewport_OffsetMid=(X=-15,Y=0,Z=0)
	Viewport_OffsetHigh=(X=-15,Y=0,Z=16)

	//FOV 90
	//Viewport_OffsetLow=(X=-2.5,Y=0,Z=0)
	//Viewport_OffsetMid=(X=0,Y=0,Z=0)
	//Viewport_OffsetHigh=(X=-2.5,Y=0,Z=16)

	WorstLocOffset=(X=-20,Y=-60,Z=50)
	
	bBlockActors		=	TRUE
//	bBlocksNavigation	=	TRUE	// AI-Vehicle�� ��ֹ��� �ν����� �ʵ��� �ϱ� ���� Off.(2007/07/27 ����)
	bEnableNightVision	=	TRUE

	QuickChatUpdateTime = -1

	LandSoundRetriggerTime = 0.3
	TakeHitPhysicsBlendOutSpeed = 1.5
	fTakeHitPhysicsMultiflier	= 0.25
	fComboHitPhysicsMultiflier	= 0.1
	fTakeHitPhysicsStartWeight	= 0.6
	//bTargetted = true
	HeadTestOffset	= (X=8,Y=0,Z=3)
	HeadViewOffset	= (X=4,Y=0,Z=3)
	HeadViewCollision = (X=6,Y=6,Z=6)

	bDrawGrenadeIndicator	=	TRUE	
	bDrawNotifyTargetted	=	TRUE
}
