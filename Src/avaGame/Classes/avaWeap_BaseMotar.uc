//=============================================================================
//  avaWeap_BaseMotar
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/06/01 by OZ
//		�ڰ��� �⺻ Class
//		
//	ToDo.
//		1. �ڰ��� �߻���ġ ����� (Art Resouce ���� ��...)
//		2. Aiming �� �ڰ��� Animation? Ȥ�� Rotation?
//		3. ��ġ�� �ٴڰ����� ���� ���� �����ϱ�										- �Ϸ�(2006/06/08)
//																					- �ٴ��� ������ �׳� normal �� ������ ���ΰ� �ƴϸ� Instigator �� ���⼺�� �����ؼ� ������ ���ΰ�?
//																					- �ϴ� Floor �� �������ϴ� ���� ������ ��¥ ������ ���ΰ�?
//																					- ������ Floor �� ������ ������� Max,Min ���� ���ǵǴ°��� ���� ������?
//		4. ������ ���� ���� HUD �� ǥ�����ֱ�										- �Ϸ�(2006/06/08)
//		5. Modeling ��ü �� ���� Property �� ����� �����ϱ� (Art Resouce ���� ��...)
//		6. �ڰ���ź�� ������ ��ġ �̴ϸʻ� ǥ�����ֱ� (�̴ϸ��� ���� ��...)	
//		7. 3��Ī Attachment �� Animation Node �߰� �۾��ϱ�, AnimTree �����ϱ� (3��Ī Animation �۾� ��� ���� �����ؾ� ��)
//		8. Network ���� Test
//		
//=============================================================================
class avaWeap_BaseMotar extends avaWeapon;

var name	InstallAnimName;		// �ڰ��� ��ġ Animation Name
var name	UnInstallAnimName;		// �ڰ��� ��ü Animation Name
var float	InstallAnimTime;		// �ڰ��� ��ġ �ð�
var float	UnInstallAnimTime;		// �ڰ��� ��ü �ð�

var float	FireTimeDelay;			// ���� Animation �� Play �� �� ��ź�� ����������� Delay
var float	MouseSensitivityRate;	// Weapon �� ���� Mouse ���� ����

var float	BasePitchAng;			// Pawn �� 0���� BaseMotar�� �̰Ϳ� �ش��Ѵ�. ET �� 60��.

var float	InstallFloorAng;		// ��ġ�� �ٴ��� ����
var float	MaxFloorAng;			// ��ġ�� ������ �ٴ��� �ִ� ���� default property ���� ����
var float	MinFloorAng;			// ��ġ�� ������ �ٴ��� �ּ� ���� default porperty ���� ����

var float	LimitYawAngle;			// default property ���� ���ǵ� Yaw Angle ����
var float	LimitPitchAngleUp;		// default property ���� ���ǵ� Pitch Angle Up ����
var float	LimitPitchAngleDown;	// default property ���� ���ǵ� Pitch Angle Down ����
var float	MaxPitch;
var float	MinPitch;


var float	InstallYawAng;
var float	LimitMaxYawAng;
var float	LimitMinYawAng;

var float	LimitMaxPitchAng;
var float	LimitMinPitchAng;

var float	LastFireDisplayTime;	// default property �� ������ ���� ���� ������ ������ �ð�
var float	LastFireTime;			// �ٷ� ���� ���� �ð�
var float	LastFireYawAng;			// �ٷ� ���� ���� ����
var float	LastFirePitchAng;

simulated state Active
{
	// FireModeNum �� 1�� ��� Motar �� ��ġ�Ѵ�.
	simulated function BeginFire(byte FireModeNum)
	{
		if ( !bDeleteMe && Instigator != None )
		{
			// Motar �� ��ġ �Ѵ�.
			if ( FireModeNum == 1 )
				DoInstall( true );
		}
	}
}

function DoInstall( bool bInstall )
{
	// ToDo. ��ġ�� �������� Check �ؾ� �Ѵ�.
	// �ٴ��� ������ ������ Check �ϵ��� �Ѵ�.
	local float FloorAng;

	FloorAng = GetFloorAngle();
	// FloorAng �� Ư�������� ����� ��ġ �Ұ�
	if ( FloorAng > MaxFloorAng * 65535 / 360 || FloorAng < MinFloorAng * 65535 / 360 )	return;
	// Instigator �� Physics �� Walking �̾�� �Ѵ�. Jump, Swim, Ladder �� ��ġ �Ұ�
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
	// �ڰ����� ��ġ �� ��� ������ �� ������ ������ �����Ѵ�.
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
		PC.SetAlwaysCrouch( true );												// ��ġ�ÿ��� Character �� �ɴ´�.
	}
	else
	{
		PC.IgnoreMoveInput( false );
		Pawn.InstallHeavyWeapon( EXC_None );
		PC.SetMouseSensitivityEx();
		PC.SetAlwaysCrouch( false );											// ��ġ�� ������ Character �� �Ͼ�� �Ѵ�.
	}
}

// Motar ��ġ�� State
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
		// NextState �� MotarInstall �� �ƴ� ��쵵 ���� �� �ִٸ� ó���� ��� �Ѵ�.
		ClearTimer( 'Installed' );
		if ( Role == ROLE_Authority && NextState != 'MotarInstall' )
			SetUpMotar( false );
	}

	// �ڰ����� ��ġ�ϰ� �������� ���� ��ü�� �Ұ����ϴ�.
	simulated function bool DenyClientWeaponSet()	{	return true;	}
}	

// Motar ��ü�� State
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

	// �ڰ����� ��ü�ϰ� ���� ���� ���� ��ü�� �Ұ����ϴ�.
	simulated function bool DenyClientWeaponSet()	{	return true;	}
}

// Motar �� Install �� �����̴�.
simulated state MotarInstall
{
	// FireModeNum �� 1�� ��� Motar �� ��ġ�Ѵ�.
	simulated function BeginFire(byte FireModeNum)
	{
		if ( !bDeleteMe && Instigator != None )
		{
			// Motar �� ��ġ�� ��쿡�� �߻簡 �����ϴ�.
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
		// � ��� MotarUnInstalling State �� ���°� �ƴ϶� �ٸ� State �� ���� ��찡 �߻��� ���� �ִ�.
		// �׾ InActive State �� ���� ���?
		// WeaponFiring �� ���� ��쵵 �ִ�.
		if ( Role == ROLE_Authority && NextState != 'WeaponFiring' )
		{
			if ( NextState != 'MotarUnInstalling' )
				SetUpMotar( false );
		}
	}

	// �ڰ����� ��ġ�ϰ� �������� ���� ��ü�� �Ұ����ϴ�.
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
	// ������ Projectile �� ���� Timer �� �ɾ�����.
	if ( FireTimeDelay != 0 )
		SetTimer( FireTimeDelay, false, 'SpawnProjectile' );
	else
		SpawnProjectile();
	return None;
}

// ������ �߻縦 �Ѵ�.
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

// CrossHair �� �׷��ش�.
simulated function DrawWeaponCrosshair( Hud HUD )
{
	local int		i;
	local rotator	CurRot;
	// Grid �� Text �� ��� ���� ��������
	local int		GridX, GridY, GridW, GridH, GridX2;
	local float		XL, YL;
	local int		DisplayAng;
	// For Draw Yaw-Grid
	local int		StartYawAng, StartYawGridAng, MaxYawAng, MinYawAng;
	local float		CHYawStartX, CHYawStartY;			// CrossHair ���� ��ǥ...
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
	YawGridMax			= 90;	// Yaw Grid �� �� 90 �� ������ �����ش�
	YawGridLeft			= 73;
	YawGridRight		= 107;
	PitchGPP			= 5;	// Pitch Grid Per Pixel
	PitchGridMax		= 25;
	CanvasXRate			= HUD.Canvas.ClipX / 1024;
	CanvasYRate			= HUD.Canvas.ClipY / 768;

	// ��ġ�Ǿ� ���� ������ CrossHair �� ������ �ʴ´�...
	 if ( !IsInState( 'MotarInstall' ) && !IsInState( 'WeaponFiring') )	return;

	CurRot = Instigator.GetViewRotation();

	HUD.Canvas.SetDrawColor( 255,255,255 );

	// For Draw Yaw Angle & Grid
	CHYawStartX = 0.5 * HUD.Canvas.ClipX - YawGridMax * YawGPP * CanvasXRate;
	CHYawStartY = 0.5 * HUD.Canvas.ClipY;

	StartYawAng = ( ( CurRot.Yaw & 65535 ) * 360 / 65535 ) - YawGridMax;	// ���� Yaw �� 90�� +,- �� Display �Ѵ�.
	if ( StartYawAng < 0 )	StartYawAng += 360;

	StartYawGridAng = ( ( StartYawAng / 5 ) + 1 ) * 5;				// 5���� �������� ������ �׷��ش�.

	HUD.Canvas.SetPos( CHYawStartX, CHYawStartY );
	HUD.Canvas.DrawRect( YawGridLeft * YawGPP * CanvasXRate, 2 );

	HUD.Canvas.SetPos( CHYawStartX + YawGridRight * YawGPP * CanvasXRate, CHYawStartY );
	HUD.Canvas.DrawRect( YawGridLeft * YawGPP * CanvasXRate, 2 );

	for ( i = 0 ; i < YawGridMax * 2 ; i += 5 )
	{
		GridX = CHYawStartX + ( StartYawGridAng - StartYawAng + i ) * 3 * CanvasXRate;
		GridW = 1;

		if ( StartYawGridAng - StartYawAng + i > YawGridLeft && StartYawGridAng - StartYawAng + i < YawGridRight ) continue;


		if ( ( ( StartYawGridAng + i ) % 15 ) == 0 )	//ū����
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
		
	StartPitchAng = ( ( CurRot.Pitch & 65535 ) * 360 / 65535 ) - PitchGridMax;	// ���� Pitch �� 25�� +,- �� Display �Ѵ�.
	if ( StartPitchAng < 0 )	StartPitchAng += 360;
	StartPitchGridAng = ( ( StartPitchAng / 5 )  ) * 5;						// 5���� �������� ������ �׷��ش�.


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

	// ���Ѱ����� ǥ�����ش�.
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

	// ���� ���� ������ �����ش�.
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

	// Projectile ���� Properties
	WeaponFireTypes(0)		=	EWFT_Projectile
	WeaponProjectiles(0)	=	class'avaProj_Motar'
	FireOffset				=	(X=22,Y=10)					// Projectile �� ������ ��ġ

	// Weapon SkeletalMesh
	BaseSkelMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rif_1p"
	BaseAnimSetName			=	"Wp_Rif_M4.Ani_M4A1_1P"

	// Animation & Animation �ӵ� ���� Properties
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
	BaseSpeed			= 260	// �⺻�ӵ�
	AimSpeedPct			= 0.0	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct= 1	// �ɾƼ� ������Ʈ�� ����ġ

	// Ammo - Rifle ó�� Reload �� �� �ʿ䰡 ����
	AmmoCount		=	10
	MaxAmmoCount	=	10

	// ��� ������ ���������� �˼� �ֵ��� degree �� �ְ��Ѵ�.
	LimitYawAngle		=	45
	BasePitchAng		=	60
	LimitPitchAngleUp	=	30		// �߻� Limit Max ������ BasePitchAng + LimitPitchAngleUp + �ٴڰ����� ���ѵȴ�.
	LimitPitchAngleDown	=	20		// �߻� Limit Min ������ BasePitchAng - LimitPitchAngleDown + �ٴڰ����� ���ѵȴ�.

	MaxPitch			=	30		// LimitPitchAngleUp + �ٴڰ����� MaxPitch �� ������ ���Ѵ�
	MinPitch			=	20		// �ٴڰ��� - LimitPitchAngleDown �� MinPitch ���� ���� �� ����

	MaxFloorAng			=	20		// �ٴڰ����� MaxFloorAng ���� ũ�� ��ġ �� �� ����.
	MinFloorAng			=	-20		// �ٴڰ����� MinFloorAng ���� ������ ��ġ �� �� ����.

	MouseSensitivityRate=	0.2
	LastFireDisplayTime	=	10
}
