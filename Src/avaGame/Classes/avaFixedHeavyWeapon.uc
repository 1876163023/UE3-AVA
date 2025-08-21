/*=============================================================================
  avaFixedHeavyWeapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/21 by OZ

	������ȭ�� class

			Vehicle �� ���� ��� �޾Ƽ� �����Ϸ��� ������ Vehicle �� Code �� ����ϰ�
			������ �� �ִ� Code �� �� ���°� ���Ƽ� ���� ����

	1.Location �� �������� ���� �� �ִ�.
	�ϳ��� 3��Ī������ Pawn �� ��ġ�ϰ� �� Location ��, 1��Ī���� Camera �� ��ġ�ϰ� �� Location �̴�.
	1��Ī���� Camera �� ��ġ�ϰ� �� Location �� ��� ������ ȸ�������� �ϸ� �� �� ����.


	ToDo.
		1.Collision ó��
			Physics �� PHYS_RigidBody �� PHYS_Interpolation ���� �������� ������
			cylindercomponent �� touch �ϴ� ���� untouch �� �ٽ� �ȴ�.

			CollideActor �� false �� �ϸ� BlockActor �� true �� �ص� Blocking �� ���� �ʴ°� ����.


		2.Use Key �� �̿��ؼ� ���� �� �ֵ���....
		3.Network ���...
		4.����ó��....
		5.IgnoreMoveInput �� true �� �ص� Crouch �� �����ϴ�... ���ƾ� �Ѵ�.
	
=============================================================================*/
class avaFixedHeavyWeapon extends Actor
	native
	placeable;

var array< avaPawn >					TouchList;		//
var	avaPawn								User;			// ��ȭ�⸦ ����ϰ� �ִ� Pawn
var	MeshComponent						WeaponMesh;		// ��ȭ��� Mesh
var	CylinderComponent					cylinderComponent;

var class<avaWeap_BaseFixedHeavyWeapon>	WeaponClass;	// Logical �� �κ��� ����ϴ� Weapon Class
var Weapon								WeaponInst;

var Vector					ViewPoint;					// 1��Ī���� Camera �� �������� ��ġ
var Vector					CamOffset;					// ViewPoint �� ���� Offset
var float					CamDist;

// View Angle �� ������ �������� ���� Flag
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

	//@ToDo. devide by zero �� ������ ��찡 �ִ�. ������ ��... �Ƹ��� MaxLimitYawAngle ����� Replication �� �ʰ� �Ǵ� ����ΰ� ����....
	aimNode.Aim.X = -0.5 + (yaw-User.MinLimitYawAngle)/(User.MaxLimitYawAngle-User.MinLimitYawAngle);
	aimNode.Aim.Y = -0.5 + (pitch-User.MinLimitPitchAngle)/(User.MaxLimitPitchAngle-User.MinLimitPitchAngle);
	aimNode.Aim.X *= 2.0;
	aimNode.Aim.Y *= 2.0;
}

/*
	��ȭ�⸦ �������� �õ��Ѵ�. ���� �� �ִٸ� ������ Setting �� �� return true, 
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
	��ȭ�⸦ ���´�.
*/
function bool UserLeave(avaPawn P)
{
	if ( User != P || User == None )	return false;	// Error

	// Equip �߿� ���� �ٲٴ� ��� Bug �߻�..... ����ó�� �Ұ�....
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
	��ȭ�⸦ ���� �� �ִ����� Check �Ѵ�.
	ToDo.
		1. ��ȭ�Ⱑ �μ����� �ִٸ� �μ����� �ʾҴ����� Check �ؾ� �Ѵ�.
		2. ���Ӹ�忡�� ��ġ ��ȭ�⸦ ����� �� ������ �Ѵٸ� Vehicle ó�� Game �� Ȯ���� �޾ƾ� �Ѵ�.
*/
function bool CanEnter(avaPawn P)
{
	local float		Incidence;
	if ( User != None || P.Controller == None ||!P.Controller.bIsPlayer )	return FALSE;
	if ( P.Physics != PHYS_Walking )										return FALSE;
	// TouchList �� Pawn �� ����.
	if ( TouchList.Find( P ) < 0 )											return FALSE;
	// TouchList �� �ٸ� Pawn �� �ִ�. ���� �� �ִ� ����Ȯ�� ����
	if ( TouchList.Length > 1 )												return FALSE;
	// Rotation Check
	// Use Point �� �־�� �� �� ����.
	Incidence = vector( P.GetViewRotation() ) dot vector( Rotation );
	if ( Incidence < 0.5 )													return FALSE;
	return TRUE;
}

/*
	Pawn �� ��ȭ�⸦ ���� ��츦 ó�����ش�.
*/
function bool UserEnter( avaPawn P )
{
	User = P;
	P.GripHeavyWeapon	=	Self;
	P.StopFiring();
	// Logical �� ���⸦ User ���� Setting �� ���ش�
	WeaponInst = spawn( WeaponClass );
	avaWeap_BaseFixedHeavyWeapon( WeaponInst ).BaseWeap = self;
	avaWeapon( WeaponInst ).GiveToEx( P, false );

	//p.InvManager.SetCurrentWeapon( WeaponInst );
	P.GrippedHeavyWeapon( WeaponInst );

	// Pawn �� ��ġ�� Location ���� �ϸ� �ȵȴ�...
	p.Velocity = vect(0,0,0);
	p.Acceleration = vect(0,0,0);

	// SetLocation �� ��쿡 Invalid �� Location �� ��� �����Ѵ�.
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
	Pawn �� ��ȭ�⸦ ���� ����� Camera �� ��ġ�� ȸ���� ����� �ش�.
*/
simulated function bool CalcViewPoint( avaPawn P, float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	//if ( User == None )	return false;	Local ������ User �� �ִ��� ������ �� �� ����... ��.��;
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
	Animation �� Play ���ش�...
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
	if ( avaPawn( Other ) == None )					return;	// Pawn �� �ƴϴ�.
	if ( TouchList.Find( avaPawn( Other ) ) >= 0 )	return;	// �̹� ��� �ִ� Pawn �̴�.
//	`log( "Touch" @Other );
	TouchList[ TouchList.Length ] = avaPawn( Other );
}

event UnTouch( Actor Other )
{
	local int nIdx;
	if ( avaPawn( Other ) == None )					return;	// Pawn �� �ƴϴ�.
	nIdx = TouchList.Find( avaPawn( Other ) );
//	`log( "Untouch" @Other );
	if ( nIdx >= 0 )	TouchList.Remove( nIdx, 1 );
}

defaultproperties
{
	Begin Object Class=SkeletalMeshComponent Name=MeshComponent0
		bUseAsOccluder = FALSE	
		SkeletalMesh=SkeletalMesh'WP_Heavy_M249.WP_Heavy_M249'	// ���� Class ��
		AnimSets(0)=AnimSet'WP_Heavy_M249.AnimSet'				// ���� Class ��
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
