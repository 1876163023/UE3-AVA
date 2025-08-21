//=============================================================================
//  avaWeap_BaseMotar
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/06/01 by OZ
//		박격포 기본 Class
//		
//	ToDo.
//		1. 박격포 발사위치 맞출것 (Art Resouce 나온 후...)
//		2. Aiming 시 박격포 Animation? 혹은 Rotation?
//		3. 설치시 바닥각도에 의한 제한 적용하기										- 완료(2006/06/08)
//																					- 바닥의 각도를 그냥 normal 만 적용할 것인가 아니면 Instigator 의 방향성과 관련해서 적용할 것인가?
//																					- 일단 Floor 의 각도구하는 것은 했으나 진짜 적용할 것인가?
//																					- 오히려 Floor 의 각도와 상관없는 Max,Min 값이 정의되는것이 맞지 않을까?
//		4. 이전에 쐈던 각도 HUD 에 표시해주기										- 완료(2006/06/08)
//		5. Modeling 교체 및 각종 Property 들 제대로 적용하기 (Art Resouce 나온 후...)
//		6. 박격포탄이 떨어진 위치 미니맵상에 표시해주기 (미니맵이 나온 후...)	
//		7. 3인칭 Attachment 및 Animation Node 추가 작업하기, AnimTree 구성하기 (3인칭 Animation 작업 어떻게 할지 결정해야 함)
//		8. Network 에서 Test
//		
//=============================================================================
class avaWeap_BaseMotar extends avaWeapon;

var name	InstallAnimName;		// 박격포 설치 Animation Name
var name	UnInstallAnimName;		// 박격포 해체 Animation Name
var float	InstallAnimTime;		// 박격포 설치 시간
var float	UnInstallAnimTime;		// 박격포 해체 시간

var float	FireTimeDelay;			// 실제 Animation 이 Play 된 후 포탄이 나가기까지의 Delay
var float	MouseSensitivityRate;	// Weapon 에 의한 Mouse 감도 조절

var float	BasePitchAng;			// Pawn 의 0도는 BaseMotar의 이것에 해당한다. ET 는 60도.

var float	InstallFloorAng;		// 설치시 바닥의 각도
var float	MaxFloorAng;			// 설치가 가능한 바닥의 최대 각도 default property 에서 정의
var float	MinFloorAng;			// 설치가 가능한 바닥의 최소 각도 default porperty 에서 정의

var float	LimitYawAngle;			// default property 에서 정의된 Yaw Angle 제한
var float	LimitPitchAngleUp;		// default property 에서 정의된 Pitch Angle Up 제한
var float	LimitPitchAngleDown;	// default property 에서 정의된 Pitch Angle Down 제한
var float	MaxPitch;
var float	MinPitch;


var float	InstallYawAng;
var float	LimitMaxYawAng;
var float	LimitMinYawAng;

var float	LimitMaxPitchAng;
var float	LimitMinPitchAng;

var float	LastFireDisplayTime;	// default property 에 지정된 이전 샷의 각도를 보여줄 시간
var float	LastFireTime;			// 바로 전에 쐈던 시간
var float	LastFireYawAng;			// 바로 전에 쐈던 각도
var float	LastFirePitchAng;

simulated state Active
{
	// FireModeNum 이 1인 경우 Motar 를 설치한다.
	simulated function BeginFire(byte FireModeNum)
	{
		if ( !bDeleteMe && Instigator != None )
		{
			// Motar 를 설치 한다.
			if ( FireModeNum == 1 )
				DoInstall( true );
		}
	}
}

function DoInstall( bool bInstall )
{
	// ToDo. 설치가 가능한지 Check 해야 한다.
	// 바닥의 각도를 가지고 Check 하도록 한다.
	local float FloorAng;

	FloorAng = GetFloorAngle();
	// FloorAng 이 특정각도를 벗어나면 설치 불가
	if ( FloorAng > MaxFloorAng * 65535 / 360 || FloorAng < MinFloorAng * 65535 / 360 )	return;
	// Instigator 의 Physics 가 Walking 이어야 한다. Jump, Swim, Ladder 중 설치 불가
	if ( bInstall && Instigator.Physics != PHYS_Walking )	return;

	ForceInstall( bInstall );
	ClientInstall( bInstall );
}

simulated function float GetFloorAngle()
{
	local vector HitLocation, HitNormal;	
	local rotator view;

	Instigator.Trace(HitLocation, HitNormal, Instigator.Location + Vect(0,0,-1) * 100.0, Instigator.Location );

	view = Instigator.GetViewRotation();
	view.Pitch = 0;
	
	return Rotator( ( HitNormal Cross Vector( view ) ) Cross HitNormal ).Pitch;
}

reliable client function ClientInstall( bool bInstall )
{
	ForceInstall( bInstall );
}

simulated function ForceInstall( bool bInstall )
{
	if ( bInstall )	GotoState( 'MotarInstalling' );
	else			GotoState( 'MotarUnInstalling' );
}

function SetUpMotar( bool bSetUp )
{
	local avaPlayerController	PC;
	local avaPawn				Pawn;
	local rotator				ViewRot;

	PC		=	avaPlayerController( Instigator.Controller );
	Pawn	=	avaPawn( Instigator );
	// 박격포를 설치 한 경우 움직일 수 없으며 각도를 제한한다.
	if ( bSetUp )
	{
		if ( !PC.IsMoveInputIgnored() )	PC.IgnoreMoveInput( true );
		
		ViewRot = Pawn.GetViewRotation();

		InstallYawAng		= ViewRot.Yaw;

		LimitMaxYawAng	= ( InstallYawAng + LimitYawAngle * 65535 / 360 );
		LimitMinYawAng	= ( InstallYawAng - LimitYawAngle * 65535 / 360 );
		LimitMaxPitchAng= min( MaxPitch * 65535 / 360 ,InstallFloorAng + LimitPitchAngleUp * 65535 / 360 );
		LimitMinPitchAng= max( -MinPitch * 65535 / 360, InstallFloorAng - LimitPitchAngleDown * 65535 / 360 );

		Pawn.InstallHeavyWeapon( EXC_InstallHeavyWeapon, LimitMinYawAng, LimitMaxYawAng, LimitMinPitchAng, LimitMaxPitchAng );
		PC.SetMouseSensitivityEx( MouseSensitivityRate );
		PC.SetAlwaysCrouch( true );												// 거치시에는 Character 가 앉는다.
	}
	else
	{
		PC.IgnoreMoveInput( false );
		Pawn.InstallHeavyWeapon( EXC_None );
		PC.SetMouseSensitivityEx();
		PC.SetAlwaysCrouch( false );											// 거치가 끝나면 Character 가 일어서야 한다.
	}
}

// Motar 설치중 State
simulated state MotarInstalling
{
	ignores DoInstall;

	simulated function BeginState( name PrevState )
	{
		InstallFloorAng = GetFloorAngle();
		if ( Role == ROLE_Authority )	SetUpMotar( true );
		PlayWeaponAnimation( InstallAnimName, InstallAnimTime );
		SetTimer( InstallAnimTime, false, 'Installed' );
	}

	simulated function Installed()
	{
		GotoState( 'MotarInstall' );
	}

	simulated function EndState( name NextState )
	{
		// NextState 가 MotarInstall 이 아닌 경우도 있을 수 있다면 처리해 줘야 한다.
		ClearTimer( 'Installed' );
		if ( Role == ROLE_Authority && NextState != 'MotarInstall' )
			SetUpMotar( false );
	}

	// 박격포를 설치하고 있을때는 무기 교체가 불가능하다.
	simulated function bool DenyClientWeaponSet()	{	return true;	}
}	

// Motar 해체중 State
simulated state MotarUnInstalling
{
	ignores DoInstall;

	simulated function BeginState( name PrvState )
	{
		PlayWeaponAnimation( UnInstallAnimName, UnInstallAnimTime );
		SetTimer( UnInstallAnimTime, false, 'UnInstalled' );
	}

	simulated function UnInstalled()
	{
		GotoState( 'Active' );
	}

	simulated function EndState( name NextState )
	{
		ClearTimer( 'UnInstalled' );
		if ( Role == ROLE_Authority )
			SetUpMotar( false );
	}

	// 박격포를 해체하고 있을 때는 무기 교체가 불가능하다.
	simulated function bool DenyClientWeaponSet()	{	return true;	}
}

// Motar 를 Install 한 상태이다.
simulated state MotarInstall
{
	// FireModeNum 이 1인 경우 Motar 를 설치한다.
	simulated function BeginFire(byte FireModeNum)
	{
		if ( !bDeleteMe && Instigator != None )
		{
			// Motar 가 설치된 경우에만 발사가 가능하다.
			if ( FireModeNum == 1 )
			{
				DoInstall( false );
				return;
			}

			if( !bDeleteMe && Instigator != None )
			{
				Global.BeginFire(FireModeNum);
				// in the active state, fire right away if we have the ammunition
				if( PendingFire(FireModeNum) && HasAmmo(FireModeNum) )
				{
					SendToFiringState(FireModeNum);
				}
			}
		}
	}

	simulated function EndState( name NextState )
	{
		// 어떤 경우 MotarUnInstalling State 로 가는게 아니라 다른 State 로 가는 경우가 발생할 수도 있다.
		// 죽어서 InActive State 로 가는 경우?
		// WeaponFiring 로 가는 경우도 있다.
		if ( Role == ROLE_Authority && NextState != 'WeaponFiring' )
		{
			if ( NextState != 'MotarUnInstalling' )
				SetUpMotar( false );
		}
	}

	// 박격포를 설치하고 있을때는 무기 교체가 불가능하다.
	simulated function bool DenyClientWeaponSet()
	{
		return true;
	}
}

// Firing State
simulated state WeaponFiring
{
	simulated function RefireCheckTimer()
	{
		GotoState('MotarInstall');
	}

	simulated function BeginState( Name PreviousStateName )
	{
		// Fire the first shot right away
		FireAmmunition();
		TimeWeaponFiring( CurrentFireMode );
	}

	simulated function EndState( Name NextStateName )
	{
		// Set weapon as not firing
		ClearFlashCount();
		ClearFlashLocation();
		ClearTimer('RefireCheckTimer');
		if ( Role == ROLE_Authority && NextStateName != 'MotarInstall' )
		{
			SetUpMotar( false );
		}
	}
}

simulated function Projectile ProjectileFire()
{
	IncrementFlashCount();
	// 실제로 Projectile 을 만들 Timer 를 걸어주자.
	if ( FireTimeDelay != 0 )
		SetTimer( FireTimeDelay, false, 'SpawnProjectile' );
	else
		SpawnProjectile();
	return None;
}

// 실제로 발사를 한다.
simulated function SpawnProjectile()
{
	local vector		RealStartLoc, AimDir;
	local Projectile	SpawnedProjectile;
	local rotator		Rot;

	Rot					=	Instigator.GetViewRotation();

	LastFireTime		=	WorldInfo.TimeSeconds;
	LastFireYawAng		=	( Rot.Yaw & 65535 ) * 360 / 65535;
	LastFirePitchAng	=	( Rot.Pitch & 65535 ) * 360 / 65535;

	if( Role == ROLE_Authority )
	{
		RealStartLoc = GetPhysicalFireStartLoc();
		AimDir = Vector(GetAdjustedAim( RealStartLoc ));
		SpawnedProjectile = Spawn(GetProjectileClass(), Self,, RealStartLoc);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( AimDir );
		}
	}
}

simulated function Rotator GetAdjustedAim( vector StartFireLoc )
{
    local rotator r;
    r = Instigator.GetViewRotation();
    r.Pitch += BasePitchAng * 65535/360.0;
    r = Normalize( r );
    return r;
}

// CrossHair 를 그려준다.
simulated function DrawWeaponCrosshair( Hud HUD )
{
	local int		i;
	local rotator	CurRot;
	// Grid 와 Text 를 찍기 위한 변수값들
	local int		GridX, GridY, GridW, GridH, GridX2;
	local float		XL, YL;
	local int		DisplayAng;
	// For Draw Yaw-Grid
	local int		StartYawAng, StartYawGridAng, MaxYawAng, MinYawAng;
	local float		CHYawStartX, CHYawStartY;			// CrossHair 시작 좌표...
	// For Draw Pitch-Grid
	local int		StartPitchAng, StartPitchGridAng, MaxPitchAng, MinPitchAng;
	local float		CHPitchStartX1, CHPitchStartY,	CHPitchStartX2;		//

	// Constant Value
	local float		CanvasXRate, CanvasYRate;
	local int		YawGPP, PitchGPP;
	local int		YawGridMax, PitchGridMax, YawGridLeft, YawGridRight;
	local int		PitchGridWidthBig, PitchGridWidthSmall, PitchGridHeight, YawGridHeightBig, YawGridHeightSmall;

	YawGPP				= 3;	// Yaw Grid Per Pixel
	YawGridHeightBig	= 27;
	YawGridHeightSmall	= 13;
	YawGridMax			= 90;	// Yaw Grid 는 양 90 도 각도를 보여준다
	YawGridLeft			= 73;
	YawGridRight		= 107;
	PitchGPP			= 5;	// Pitch Grid Per Pixel
	PitchGridMax		= 25;
	CanvasXRate			= HUD.Canvas.ClipX / 1024;
	CanvasYRate			= HUD.Canvas.ClipY / 768;

	// 설치되어 있지 않으면 CrossHair 는 보이지 않는다...
	 if ( !IsInState( 'MotarInstall' ) && !IsInState( 'WeaponFiring') )	return;

	CurRot = Instigator.GetViewRotation();

	HUD.Canvas.SetDrawColor( 255,255,255 );

	// For Draw Yaw Angle & Grid
	CHYawStartX = 0.5 * HUD.Canvas.ClipX - YawGridMax * YawGPP * CanvasXRate;
	CHYawStartY = 0.5 * HUD.Canvas.ClipY;

	StartYawAng = ( ( CurRot.Yaw & 65535 ) * 360 / 65535 ) - YawGridMax;	// 현재 Yaw 의 90도 +,- 를 Display 한다.
	if ( StartYawAng < 0 )	StartYawAng += 360;

	StartYawGridAng = ( ( StartYawAng / 5 ) + 1 ) * 5;				// 5도를 기준으로 눈금을 그려준다.

	HUD.Canvas.SetPos( CHYawStartX, CHYawStartY );
	HUD.Canvas.DrawRect( YawGridLeft * YawGPP * CanvasXRate, 2 );

	HUD.Canvas.SetPos( CHYawStartX + YawGridRight * YawGPP * CanvasXRate, CHYawStartY );
	HUD.Canvas.DrawRect( YawGridLeft * YawGPP * CanvasXRate, 2 );

	for ( i = 0 ; i < YawGridMax * 2 ; i += 5 )
	{
		GridX = CHYawStartX + ( StartYawGridAng - StartYawAng + i ) * 3 * CanvasXRate;
		GridW = 1;

		if ( StartYawGridAng - StartYawAng + i > YawGridLeft && StartYawGridAng - StartYawAng + i < YawGridRight ) continue;


		if ( ( ( StartYawGridAng + i ) % 15 ) == 0 )	//큰눈금
		{
			GridY = CHYawStartY - YawGridHeightBig * CanvasYRate;
			GridH = YawGridHeightBig * CanvasYRate + 1;

			if ( i < 70 || i > 105 )
			{
				DisplayAng = StartYawGridAng + i;
				if ( DisplayAng < 0 )			DisplayAng += 360;
				else if ( DisplayAng >= 360 )	DisplayAng -= 360;

				HUD.Canvas.TextSize( DisplayAng, XL, YL ); // Clipped!
				HUD.Canvas.SetPos( GridX - XL/2, GridY - YL  );
				HUD.Canvas.DrawText( DisplayAng );
			}
		}
		else
		{
			GridY = CHYawStartY - YawGridHeightSmall * CanvasYRate;
			GridH = YawGridHeightSmall * CanvasYRate + 1;
		}
		HUD.Canvas.SetPos( GridX, GridY  );
		HUD.Canvas.DrawRect( GridW, GridH );
	}

	// For Draw Pitch Angle & Grid
	CHPitchStartX1 = CHYawStartX + 75 * YawGPP * CanvasXRate;;
	CHPitchStartX2 = CHYawStartX + 105 * YawGPP * CanvasYRate;;
	CHPitchStartY = ( 0.5 * HUD.Canvas.ClipY ) + ( PitchGridMax * PitchGPP * CanvasYRate);
		
	StartPitchAng = ( ( CurRot.Pitch & 65535 ) * 360 / 65535 ) - PitchGridMax;	// 현재 Pitch 의 25도 +,- 를 Display 한다.
	if ( StartPitchAng < 0 )	StartPitchAng += 360;
	StartPitchGridAng = ( ( StartPitchAng / 5 )  ) * 5;						// 5도를 기준으로 눈금을 그려준다.


	HUD.Canvas.SetPos( CHPitchStartX1, CHPitchStartY - 50 * 5 * HUD.Canvas.ClipY / 768 );
	HUD.Canvas.DrawRect( 2, 50 * 5 * HUD.Canvas.ClipY / 768 );

	HUD.Canvas.SetPos( CHPitchStartX2, CHPitchStartY - 50 * 5 * HUD.Canvas.ClipY / 768 );
	HUD.Canvas.DrawRect( 2, 50 * 5 * HUD.Canvas.ClipY / 768 );


	PitchGridWidthBig		= 20;
	PitchGridWidthSmall		= 13;
	PitchGridHeight			= 1;

	for ( i = 50 ; i > 0 ; i -= 5 )
	{
		GridY = CHPitchStartY - ( StartPitchGridAng - StartPitchAng + i ) * 5 * HUD.Canvas.ClipY / 768;
		GridH = PitchGridHeight;
		if ( ( ( StartPitchGridAng + i ) % 10 ) == 0 )
		{
			GridX = CHPitchStartX1;
			GridW = PitchGridWidthBig * CanvasXRate;
			GridX2= CHPitchStartX2 - GridW;

			// Draw Angle
			DisplayAng = StartPitchGridAng + i + BasePitchAng;
			if ( DisplayAng >= 360 )	DisplayAng -= 360;
			else if ( DisplayAng < 0 )	DisplayAng += 360;

			HUD.Canvas.TextSize( DisplayAng, XL, YL ); // Clipped!
			HUD.Canvas.SetPos( (CHPitchStartX1 + CHPitchStartX2)/2 - XL/2, GridY - YL/2  );
			HUD.Canvas.DrawText( DisplayAng );
		}
		else
		{
			GridX = CHPitchStartX1;
			GridW = PitchGridWidthSmall * CanvasXRate;
			GridX2= CHPitchStartX2 - GridW;
		}
		HUD.Canvas.SetPos( GridX, GridY  );
		HUD.Canvas.DrawRect( GridW, GridH );

		HUD.Canvas.SetPos( GridX2, GridY );
		HUD.Canvas.DrawRect( GridW, GridH );
	}

	// 제한각도를 표시해준다.
	HUD.Canvas.SetDrawColor( 255, 255, 0 );
	MaxYawAng = ( LimitMaxYawAng & 65535 ) * 360 / 65535;
	MinYawAng = ( LimitMinYawAng & 65535 ) * 360 / 65535;

	if ( MaxYawAng < StartYawAng )	MaxYawAng += 360;
	if ( MinYawAng < StartYawAng )	MinYawAng += 360;

	if ( MinYawAng > StartYawAng + ( 90 - YawGridLeft ) )
	{
		HUD.Canvas.SetPos( CHYawStartX + (MinYawAng - StartYawAng - ( 90 - YawGridLeft )) * 3 * HUD.Canvas.ClipX / 1024,
						CHYawStartY - 19 * HUD.Canvas.ClipY / 768 );
		HUD.Canvas.DrawRect( 2, 28 * HUD.Canvas.ClipY / 768 );
	}

	if ( MaxYawAng < StartYawAng + 180 - ( 90 - YawGridLeft ) )
	{
		HUD.Canvas.SetPos( CHYawStartX + (MaxYawAng - StartYawAng + ( 90 - YawGridLeft )) * 3 * HUD.Canvas.ClipX / 1024,
						CHYawStartY - 19 * HUD.Canvas.ClipY / 768 );
		HUD.Canvas.DrawRect( 2, 28 * HUD.Canvas.ClipY / 768 );
	}

	MaxPitchAng = ( LimitMaxPitchAng & 65535 ) * 360 / 65535;
	MinPitchAng = ( LimitMinPitchAng & 65535 ) * 360 / 65535;

	if ( MinPitchAng < StartPitchAng ) MinPitchAng += 360;

	if ( MinPitchAng > StartPitchAng )
	{
		HUD.Canvas.SetPos( CHPitchStartX1- 4, CHPitchStartY - ( MinPitchAng - StartPitchAng) * 5 * HUD.Canvas.ClipY / 768 );
		HUD.Canvas.DrawRect( 9, 2 );
		HUD.Canvas.SetPos( CHPitchStartX2- 4, CHPitchStartY - ( MinPitchAng - StartPitchAng) * 5 * HUD.Canvas.ClipY / 768 );
		HUD.Canvas.DrawRect( 9, 2 );
	}

	if ( MaxPitchAng < StartPitchAng ) MaxPitchAng += 360;

	if ( MaxPitchAng < StartPitchAng + 50 )
	{
		HUD.Canvas.SetPos( CHPitchStartX1- 4, CHPitchStartY - ( MaxPitchAng - StartPitchAng) * 5 * HUD.Canvas.ClipY / 768 );
		HUD.Canvas.DrawRect( 9, 2 );
		HUD.Canvas.SetPos( CHPitchStartX2- 4, CHPitchStartY - ( MaxPitchAng - StartPitchAng) * 5 * HUD.Canvas.ClipY / 768 );
		HUD.Canvas.DrawRect( 9, 2 );
	}

	// 이전 샷의 각도를 보여준다.
	if ( WorldInfo.TimeSeconds - LastFireTime < LastFireDisplayTime )
	{
		//HUD.Canvas.SetDrawColor( 255, 0, 0 );
		HUD.Canvas.SetDrawColor( 255, 0, 0, (1.0 - (WorldInfo.TimeSeconds - LastFireTime)/LastFireDisplayTime) * 255 );

		if ( LastFireYawAng < StartYawAng )		LastFireYawAng += 360;
		if ( LastFirePitchAng < StartPitchAng )	LastFirePitchAng += 360;
		HUD.Canvas.SetPos( CHYawStartX + (LastFireYawAng - StartYawAng) * 3 * CanvasXRate,
						   CHYawStartY - 19 * CanvasYRate );
		HUD.Canvas.DrawRect( 2, 28 * CanvasYRate );

		if ( LastFirePitchAng < StartPitchAng + 50 && LastFirePitchAng > StartPitchAng )
		{
			HUD.Canvas.SetPos( CHPitchStartX1- 4, CHPitchStartY - ( LastFirePitchAng - StartPitchAng) * 5 * CanvasYRate );
			HUD.Canvas.DrawRect( 9, 2 );
			HUD.Canvas.SetPos( CHPitchStartX2- 4, CHPitchStartY - ( LastFirePitchAng - StartPitchAng) * 5 * CanvasYRate );
			HUD.Canvas.DrawRect( 9, 2 );
		}
	}
}


//simulated function SetPosition(avaPawn Holder)
//{
//	local vector DrawOffset, ViewOffset;
//	local avaPawn.EWeaponHand CurrentHand;
//	local rotator NewRotation;
//
//	if ( !Instigator.IsFirstPerson() )
//		return;
//
//	// Hide the weapon if hidden
//	CurrentHand = GetHand();
//	if ( CurrentHand == HAND_Hidden)
//	{
//		bHidden = true;
//		return;
//	}
//	bHidden = false;
//
//	// Adjust for the current hand
//	ViewOffset = PlayerViewOffset;
//	switch ( CurrentHand )
//	{
//		case HAND_Left:
//			ViewOffset.Y *= -1;
//			break;
//
//		case HAND_Centered:
//			ViewOffset.Y = 0;
//			break;
//	}
//
//	// Calculate the draw offset
//	if ( Holder.Controller == None )
//		DrawOffset = (ViewOffset >> Rotation) + Holder.GetEyeHeight() * vect(0,0,1);
//	else
//	{
//		DrawOffset.Z = Holder.GetEyeHeight();
//		if ( Holder.bWeaponBob )
//		{
//			DrawOffset += Holder.WeaponBob(BobDamping);
//		}
//
//		if ( avaPlayerController(Holder.Controller) != None )
//		{
//			DrawOffset += avaPlayerController(Holder.Controller).ShakeOffset >> Holder.Controller.Rotation;
//		}
//
//		DrawOffset = DrawOffset + ( ViewOffset >> Holder.Controller.Rotation );
//	}
//
//	// Adjust it in the world
//	SetLocation( Holder.Location + DrawOffset );
//
//	NewRotation = (Holder.Controller == None ) ? Holder.Rotation : Holder.Controller.Rotation;
//	if ( Holder.bWeaponBob )
//	{
//		// if bWeaponBob, then add some rotation lag
//		NewRotation.Yaw = LagRot(NewRotation.Yaw & 65535, Rotation.Yaw & 65535, MaxYawLag);
//		NewRotation.Pitch = LagRot(NewRotation.Pitch & 65535, Rotation.Pitch & 65535, MaxPitchLag);
//	}
////	SetRotation(NewRotation);
//}


defaultproperties
{
	InventoryGroup = 5

	// Projectile 관련 Properties
	WeaponFireTypes(0)		=	EWFT_Projectile
	WeaponProjectiles(0)	=	class'avaProj_Motar'
	FireOffset				=	(X=22,Y=10)					// Projectile 이 나가는 위치

	// Weapon SkeletalMesh
	BaseSkelMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rif_1p"
	BaseAnimSetName			=	"Wp_Rif_M4.Ani_M4A1_1P"

	// Animation & Animation 속도 관련 Properties
	WeaponFireAnim(0)	=	Fire
 	WeaponPutDownAnim	=	Down
	WeaponEquipAnim		=	BringUp
	WeaponIdleAnims(0)	=	Idle

	InstallAnimName		=	Reload
	InstallAnimTime		=	2.0
	UnInstallAnimName	=	Reload
	UnInstallAnimTime	=	2.0

	FireInterval(0)		=	2.0
	EquipTime			=	1.1333
	PutDownTime			=	0.0333

	FireTimeDelay		=	1.0

	// Velocity Limit Parameter of Pawn
	BaseSpeed			= 260	// 기본속도
	AimSpeedPct			= 0.0	// 조준시 보정치
	WalkSpeedPct		= 0.4	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	// Ammo - Rifle 처럼 Reload 를 할 필요가 없음
	AmmoCount		=	10
	MaxAmmoCount	=	10

	// 모든 각도는 직관적으로 알수 있도록 degree 로 넣게한다.
	LimitYawAngle		=	45
	BasePitchAng		=	60
	LimitPitchAngleUp	=	30		// 발사 Limit Max 각도는 BasePitchAng + LimitPitchAngleUp + 바닥각도로 제한된다.
	LimitPitchAngleDown	=	20		// 발사 Limit Min 각도는 BasePitchAng - LimitPitchAngleDown + 바닥각도로 제한된다.

	MaxPitch			=	30		// LimitPitchAngleUp + 바닥각도가 MaxPitch 를 넘지는 못한다
	MinPitch			=	20		// 바닥각도 - LimitPitchAngleDown 는 MinPitch 보다 작을 수 없다

	MaxFloorAng			=	20		// 바닥각도가 MaxFloorAng 보다 크면 설치 할 수 없다.
	MinFloorAng			=	-20		// 바닥각도가 MinFloorAng 보다 작으면 설치 할 수 없다.

	MouseSensitivityRate=	0.2
	LastFireDisplayTime	=	10
}
