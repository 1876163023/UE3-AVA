/*=============================================================================
  avaFixedHeavyWeapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/21 by OZ

	고정중화기 class

			Vehicle 로 부터 상속 받아서 구현하려고 했으나 Vehicle 의 Code 가 방대하고
			재사용할 수 있는 Code 가 얼마 없는것 같아서 새로 만듬

	1.Location 은 두종류가 있을 수 있다.
	하나는 3인칭에서의 Pawn 이 위치하게 될 Location 과, 1인칭에서 Camera 가 위치하게 될 Location 이다.
	1인칭에서 Camera 가 위치하게 될 Location 의 경우 무기의 회전축으로 하면 될 것 같다.


	ToDo.
		1.Collision 처리
			Physics 를 PHYS_RigidBody 나 PHYS_Interpolation 으로 설정하지 않으면
			cylindercomponent 에 touch 하는 순간 untouch 가 다시 된다.

			CollideActor 를 false 로 하면 BlockActor 를 true 로 해도 Blocking 이 되지 않는것 같다.


		2.Use Key 를 이용해서 잡을 수 있도록....
		3.Network 고려...
		4.예외처리....
		5.IgnoreMoveInput 을 true 로 해도 Crouch 가 가능하다... 막아야 한다.
	
=============================================================================*/
class avaFixedHeavyWeapon extends Actor
	native
	placeable;

var array< avaPawn >					TouchList;		//
var	avaPawn								User;			// 중화기를 사용하고 있는 Pawn
var	MeshComponent						WeaponMesh;		// 중화기용 Mesh
var	CylinderComponent					cylinderComponent;

var class<avaWeap_BaseFixedHeavyWeapon>	WeaponClass;	// Logical 한 부분을 담당하는 Weapon Class
var Weapon								WeaponInst;

var Vector					ViewPoint;					// 1인칭에서 Camera 가 놓여있을 위치
var Vector					CamOffset;					// ViewPoint 에 대한 Offset
var float					CamDist;

// View Angle 을 제한할 것인지에 대한 Flag
var bool					bLimitYaw;
var bool					bLimitPitch;
var() float					LimitMaxPitch;
var() float					LimitMinPitch;
var() float					LimitMaxYaw;

var AnimNodeAimOffset		aimNode;
var AnimNodeSequence		SeqPlayer;

replication
{
	if (bNetDirty && Role == ROLE_Authority)
		User;
}


simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	SkeletalMeshComponent( WeaponMesh ).GetSocketWorldLocationAndRotation( 'ViewPoint', ViewPoint );
	aimNode		= AnimNodeAimOffset( SkeletalMeshComponent( WeaponMesh ).Animations.FindAnimNode( 'aimingNode' ) );	
	SeqPlayer	= AnimNodeSequence( SkeletalMeshComponent( WeaponMesh ).Animations.FindAnimNode( 'AnimSeqPlayer' ) );
//	`log( "avafixedheavyWeapon.PostBeginPlay" @ViewPoint @Location @Rotation );
}

simulated function Tick( float DeltaTime )
{
	local float yaw, pitch;
	if ( User == None  ) return;

	if ( User.Controller != None )
	{
		pitch = User.Controller.Rotation.Pitch;
		yaw	  = User.Controller.Rotation.Yaw;
	}
	else
	{
		pitch = ((User.RemoteViewPitch << 8) & 65535);
		yaw	  = User.RemoteViewYaw;
	}
	if ( pitch > 32768 )	pitch -= 65536;

	//@ToDo. devide by zero 가 나오는 경우가 있다. 수정할 것... 아마도 MaxLimitYawAngle 등등의 Replication 이 늦게 되는 경우인것 같음....
	aimNode.Aim.X = -0.5 + (yaw-User.MinLimitYawAngle)/(User.MaxLimitYawAngle-User.MinLimitYawAngle);
	aimNode.Aim.Y = -0.5 + (pitch-User.MinLimitPitchAngle)/(User.MaxLimitPitchAngle-User.MinLimitPitchAngle);
	aimNode.Aim.X *= 2.0;
	aimNode.Aim.Y *= 2.0;
}

/*
	중화기를 잡으려고 시도한다. 잡을 수 있다면 적절히 Setting 한 후 return true, 
*/
function bool TryToEnter(avaPawn P)
{
	if ( !CanEnter( P ) )	return FALSE;
	return UserEnter( P );
}


function UserDied( avaPawn P )
{
	if ( User != P || User == None )	return;	// Error

	avaWeap_BaseFixedHeavyWeapon( WeaponInst ).SetInactive();
	P.GrippedHeavyWeapon( None );
	WeaponInst.Destroy();

	User.InstallHeavyWeapon( EXC_None );
	avaPlayerController(User.Controller).IgnoreMoveInput( false );
	P.GripHeavyWeapon	=	None;
	WeaponInst			=	None;
	User				=	None;
	NetUpdateTime		=	WorldInfo.TimeSeconds - 1;
}

/*
	중화기를 놓는다.
*/
function bool UserLeave(avaPawn P)
{
	if ( User != P || User == None )	return false;	// Error

	// Equip 중에 총을 바꾸는 경우 Bug 발생..... 예외처리 할것....
	if ( !WeaponInst.IsInState( 'Active' ) )	return false;
	
	avaWeap_BaseFixedHeavyWeapon( WeaponInst ).SetInactive();
	P.GrippedHeavyWeapon( None );
	WeaponInst.Destroy();

	User.InstallHeavyWeapon( EXC_None );
	avaPlayerController(User.Controller).IgnoreMoveInput( false );
	P.GripHeavyWeapon	=	None;
	WeaponInst			=	None;
	User				=	None;
	NetUpdateTime		=	WorldInfo.TimeSeconds - 1;
	return true;
}

/*
	중화기를 잡을 수 있는지를 Check 한다.
	ToDo.
		1. 중화기가 부서질수 있다면 부서지지 않았는지를 Check 해야 한다.
		2. 게임모드에서 거치 중화기를 사용할 수 없도록 한다면 Vehicle 처럼 Game 에 확인을 받아야 한다.
*/
function bool CanEnter(avaPawn P)
{
	local float		Incidence;
	if ( User != None || P.Controller == None ||!P.Controller.bIsPlayer )	return FALSE;
	if ( P.Physics != PHYS_Walking )										return FALSE;
	// TouchList 에 Pawn 이 없다.
	if ( TouchList.Find( P ) < 0 )											return FALSE;
	// TouchList 에 다른 Pawn 이 있다. 잡을 수 있는 공간확보 실패
	if ( TouchList.Length > 1 )												return FALSE;
	// Rotation Check
	// Use Point 가 있어야 할 것 같다.
	Incidence = vector( P.GetViewRotation() ) dot vector( Rotation );
	if ( Incidence < 0.5 )													return FALSE;
	return TRUE;
}

/*
	Pawn 이 중화기를 잡은 경우를 처리해준다.
*/
function bool UserEnter( avaPawn P )
{
	User = P;
	P.GripHeavyWeapon	=	Self;
	P.StopFiring();
	// Logical 한 무기를 User 에게 Setting 을 해준다
	WeaponInst = spawn( WeaponClass );
	avaWeap_BaseFixedHeavyWeapon( WeaponInst ).BaseWeap = self;
	avaWeapon( WeaponInst ).GiveToEx( P, false );

	//p.InvManager.SetCurrentWeapon( WeaponInst );
	P.GrippedHeavyWeapon( WeaponInst );

	// Pawn 의 위치는 Location 으로 하면 안된다...
	p.Velocity = vect(0,0,0);
	p.Acceleration = vect(0,0,0);

	// SetLocation 한 경우에 Invalid 한 Location 인 경우 실패한다.
	p.SetLocation( Location );

	p.InstallHeavyWeapon( EXC_FixedHeavyWeapon, 
						  Rotation.Yaw - LimitMaxYaw * 65535/360, 
						  Rotation.Yaw + LimitMaxYaw * 65535/360,
						  LimitMinPitch * 65535/360, 
						  LimitMaxPitch * 65535/360 );
	avaPlayerController(p.Controller).IgnoreMoveInput( true );

	//if ( P.IsLocallyControlled() )
	//{
	//	WeaponMesh.SetHidden( true );
	//}

	NetUpdateTime		=	WorldInfo.TimeSeconds - 1;
	return true;
}

/*
	Pawn 이 중화기를 잡은 경우의 Camera 의 위치와 회전을 계산해 준다.
*/
simulated function bool CalcViewPoint( avaPawn P, float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	//if ( User == None )	return false;	Local 에서는 User 가 있는지 없는지 알 수 없다... ㅡ.ㅡ;
	//SkeletalMeshComponent( WeaponMesh ).GetSocketWorldLocationAndRotation( 'ViewPoint', ViewPoint );
	out_CamLoc = ViewPoint;// + ( ViewPoint - Location ) >> Rotation;
	out_CamRot = P.Controller.Rotation;
	out_CamLoc += ( CamOffset >> Rotation ) - Vector( out_CamRot ) * CamDist;
	return true;
}

simulated function vector GetViewPoint()
{
	return ViewPoint + ( CamOffset >> Rotation ) - Vector( User.Controller.Rotation ) * CamDist;
}

/*
	Animation 을 Play 해준다...
*/
simulated function PlayAnimatin( name seqName, float fDesiredDuration, optional bool bLoop )
{
	local float DesiredRate;

	if ( SeqPlayer != None )
	{
		 if ( SeqPlayer.AnimSeq == None || SeqPlayer.AnimSeq.SequenceName != seqName )
			 SeqPlayer.SetAnim( seqName );
		if ( SeqPlayer.AnimSeq != None)
		{
			DesiredRate = (fDesiredDuration > 0.0) ? (SeqPlayer.AnimSeq.SequenceLength / fDesiredDuration) : 1.0;
			SeqPlayer.PlayAnim(bLoop, DesiredRate);
		}
	}
}

simulated function Vector GetPawnViewLocation()
{
	return ViewPoint + CamOffset - Vector( User.Controller.Rotation ) * CamDist;
}

event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	if ( avaPawn( Other ) == None )					return;	// Pawn 이 아니다.
	if ( TouchList.Find( avaPawn( Other ) ) >= 0 )	return;	// 이미 들어 있는 Pawn 이다.
//	`log( "Touch" @Other );
	TouchList[ TouchList.Length ] = avaPawn( Other );
}

event UnTouch( Actor Other )
{
	local int nIdx;
	if ( avaPawn( Other ) == None )					return;	// Pawn 이 아니다.
	nIdx = TouchList.Find( avaPawn( Other ) );
//	`log( "Untouch" @Other );
	if ( nIdx >= 0 )	TouchList.Remove( nIdx, 1 );
}

defaultproperties
{
	Begin Object Class=SkeletalMeshComponent Name=MeshComponent0
		bUseAsOccluder = FALSE	
		SkeletalMesh=SkeletalMesh'WP_Heavy_M249.WP_Heavy_M249'	// 하위 Class 용
		AnimSets(0)=AnimSet'WP_Heavy_M249.AnimSet'				// 하위 Class 용
		AnimTreeTemplate=AnimTree'WP_Heavy_M249.AnimTree'
		PhysicsAsset=PhysicsAsset'WP_Heavy_M249.WP_Heavy_M249_Physics'
		CastShadow=false
		
		bUpdateSkelWhenNotRendered=false
		Rotation=(Yaw=-16384)
		
		CollideActors=true
		BlockActors=true
		BlockZeroExtent=true
		BlockNonzeroExtent=true
		BlockRigidBody=true
		bHasPhysicsAssetInstance=true
		//PhysicsWeight=1.0

	End Object
	CollisionComponent=MeshComponent0
	WeaponMesh=MeshComponent0
	Components.Add(MeshComponent0)

	Begin Object Class=CylinderComponent Name=CollisionCylinder
		CollisionRadius=+0017.000000
		CollisionHeight=+0045.000000
		Translation=(Z=45.0)
		CollideActors=true
		BlockZeroExtent=false
		BlockNonzeroExtent=true
	End Object
	CylinderComponent=CollisionCylinder
	Components.Add(CollisionCylinder)

	bCollideActors	=	true
	bBlockActors	=	true
	Physics			=	PHYS_Interpolating
	
	bCollideWorld	=	true
	bProjTarget		=	true
	
	bLimitYaw		=	true
	bLimitPitch		=	true
	LimitMaxPitch	=	10
	LimitMinPitch	=	-10
	LimitMaxYaw		=	10

	CamOffset		=	(x=-7,z=5.5)

	WeaponClass		=	class'avaWeap_BaseFixedHeavyWeapon'
	bReplicateMovement	=	true
	bStatic				=	false
	bNetInitialRotation	=	true

	RemoteRole=ROLE_SimulatedProxy
}
