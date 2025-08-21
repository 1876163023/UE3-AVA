class avaWeaponPawn_LeopardMachineGunner extends avaWeaponPawn;

function bool DriverEnter(Pawn P)
{
	avaVehicleWeapon(Weapon).DriverEnter();
	avaWeapon( P.Weapon ).ResetWeapon();
	avaPlayerController( P.Controller ).ClientCancelDash();
	return Super.DriverEnter(P);
}

event bool DriverLeave( bool bForceLeave )
{
	avaWeapon( Driver.Weapon ).EquipWeapon();
	avaVehicleWeapon(Weapon).DriverLeave();
	return Super.DriverLeave( bForceLeave );
}

simulated function AttachDriver( Pawn P )
{
	local avaPawn	avaP;
	// 앞으로 기관총 자리의 Driver는 충돌처리를 해야 한다.
	avaP = avaPawn(P);
	if (avaP != None)
	{
		avaP.SetWeaponVisibility(false);
		if (MyVehicle.bAttachDriver)
		{
			avaP.SetCollision(true, false, false);
			avaP.bCollideWorld = false;
			avaP.SetHardAttach(true);
			avaP.SetLocation(Location);
			avaP.SetPhysics(PHYS_None);

			MyVehicle.SitDriver(avaP, MySeatIndex);
		}
	}

//	Super.AttachDriver( P );

	//if ( ( P.Controller != None && P.IsLocallyControlled() ) )
	//{
	//	avaPlayerController( P.Controller ).ForceShoulderCam( true );
	//	avaPlayerController( P.Controller ).SetMouseSensitivityEx(0.3);
	//}
	//else if ( Controller != None && IsLocallyControlled() )
	//{
	//	avaPlayerController( Controller ).ForceShoulderCam( true );
	//	avaPlayerController( Controller ).SetMouseSensitivityEx(0.3);
	//}

	avaPawn( P ).OffsetMid	=	vect(-32,17,4);
	avaPawn( P ).SetHeavyWeaponType( EXC_FixedHeavyWeapon );
	avaPawn( P ).Mesh.SetTranslation( Vect(0,0,0) );
	avaPawn( P ).CurrentWeaponAttachment.ChangeAttachmentState( false );
	avaPawn( P ).PlayAnimByEvent( EBT_BringUp );

	// 탱크에 붙은 기관총에도 동이한 애니메이션이 적용되도록 한다.
	if ( avaVehicle_Leopard(MyVehicle) != None )
		avaVehicle_Leopard(MyVehicle).PlayAnimByEvent( EBT_BringUp );
}

simulated function DetachDriver( Pawn P )
{
	Super.DetachDriver( P );

	if ( (P.Controller != None && P.IsLocallyControlled()) )
	{
		avaPlayerController( P.Controller ).ForceShoulderCam( false );
		avaPlayerController( P.Controller ).SetMouseSensitivityEx(1);
		avaPawn(P).SetMeshVisibility(false);
	}
	else if ( IsLocallyControlled() )
	{
		avaPlayerController( Controller ).ForceShoulderCam( false );
		avaPlayerController( Controller ).SetMouseSensitivityEx(1);
		avaPawn(P).SetMeshVisibility(false);
	}

	avaPawn( P ).OffsetMid	=	avaPawn(P).default.OffsetMid;
	avaPawn( P ).SetHeavyWeaponType( EXC_None );
	if ( class'avaPawn'.default.Mesh != None )
		avaPawn( P ).Mesh.SetTranslation( class'avaPawn'.default.Mesh.Translation );
	avaPawn( P ).CurrentWeaponAttachment.ChangeAttachmentState( true );
}

simulated function DriverDied()
{
	// avaVehicleBase.EjectDriver()함수를 호출해서 밖으로 튕겨져 나가게 해준다.
	bShouldEject=true;
	Super.DriverDied();
}

defaultproperties
{
	MouseSensitivity = 0.3
	bForceShoulderCam = true
}