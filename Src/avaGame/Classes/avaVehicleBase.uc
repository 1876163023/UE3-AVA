/*
	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.

	2007/05/10	 ����
		UTVehicleBase�� �״�� �ۿ�.
*/

class avaVehicleBase extends SVehicle
	abstract
	native
	notplaceable;

/*!
	���� UT�ʿ����� UTUIScene_VehicleHud�� �̿�������
	AVA������ Driver�� avaHUD(PlayerController(Owner).myHUD)�� �̿��Ͽ�
	ó���ؾ� �� �� �ʹ�.
*/
//	var UTUIScene_VehicleHud PawnHudScene;

/** Holds the tag of the panel that parents the various widgets of the hud scene.  This panel will be altered removed
    when the scene goes out of focus. */

/** If true the driver will be ejected if he leaves*/
var bool bShouldEject;

//! ���콺 �̵� �ӵ��� �����Ѵ�.(2007/10/23 ����)
var float MouseSensitivity;

//! ������ 3��Ī �������� �ϴ��� ���� ����.
var bool bForceShoulderCam;

/*********************************************************************************************
 HUD
********************************************************************************************* */

var Texture2D HudIcons;
var TextureCoordinates HudCoords;



replication
{
	if ( bNetDirty )
		bShouldEject;
}

simulated function SwitchWeapon(byte NewGroup)
{
	ServerChangeSeat(NewGroup-1);
}

/**
request change to adjacent vehicle seat
*/
simulated function AdjacentSeat(int Direction, Controller C)
{
	ServerAdjacentSeat(Direction, C);
}

/**
request change to adjacent vehicle seat
*/
reliable server function ServerAdjacentSeat(int Direction, Controller C);

/**
 * Called when a client is requesting a seat change
 *
 * @network	Server-Side
 */
reliable server function ServerChangeSeat(int RequestedSeat);

/**
 * AI - Returns the best firing mode for this weapon
 */
function byte ChooseFireMode()
{
	if (avaWeapon(Weapon) != None)
	{
		return avaWeapon(Weapon).BestMode();
	}
	return 0;
}

/**
 * AI - An AI controller wants to fire
 *
 * @Param 	bFinished	unused
 */

function bool BotFire(bool bFinished)
{
//! @comment avaBot�� ���̰� ���� ����Ѵ�.
/*
	local UTBot Bot;

	Bot = UTBot(Controller);
	if (Bot != None && Bot.ScriptedFireMode != 255)
*/

	// AIController�� ��쿡�� ������ 0���� �����ǰ� �Ѵ�.(2007/11/13)
	if ( AIController(Controller) != None )
		StartFire(0);
	else
		StartFire(ChooseFireMode());

	return true;
}

/**
 * @Returns the scale factor to apply to damage affecting this vehicle
 */
function float GetDamageScaling()
{
	if (Driver != None)
	{
		return (Driver.GetDamageScaling() * Super.GetDamageScaling());
	}
	else
	{
		return Super.GetDamageScaling();
	}
}

/**
 * @Returns true if the AI needs to turn towards a target
 */
function bool NeedToTurn(vector Targ)
{
//! @comment avaVehicleWeapon �߰��� �ּ� Ǯ ����.
/*!
	local avaVehicleWeapon VWeapon;

	// vehicles can have weapons that rotate independently of the vehicle, so check with the weapon instead
	VWeapon = avaVehicleWeapon(Weapon);
	if (VWeapon != None)
	{
		return !VWeapon.IsAimCorrect();
	}
	else
	{
		return Super.NeedToTurn(Targ);
	}
*/

	return false;
}


/**
 * Called on both the server an owning client when the player leaves the vehicle.  We want to make sure
 * any active weapon is shut down
 */

simulated function DriverLeft()
{
	if( WorldInfo.NetMode != NM_Client && 
		avaPlayerReplicationInfo(Driver.PlayerReplicationInfo) != None && 
		Driver.PlayerReplicationInfo.bHasFlag && avaPawn(Driver) != None )
	{
//		avaPawn(Driver).HoldGameObject(avaPlayerReplicationInfo(Driver.PlayerReplicationInfo).GetFlag());
	}
	if(bShouldEject)
	{
		EjectDriver();
		bShouldEject=false; // so next driver doesn't get ejected.
	}

	Super.DriverLeft();
	StopFiringWeapon();
}

/**
EjectDriver() throws the driver out at high velocity
*/
simulated function EjectDriver()
{
	local float Speed;
	local rotator ExitRotation;

	if ( Driver == None )
	{
		return;
	}
	if ( PlayerController(Driver.Controller) != None )
	{
		ExitRotation = rotator(Velocity);
		ExitRotation.Pitch = -8192;
		Driver.Controller.SetRotation(ExitRotation);
	}
	Speed = VSize(Velocity);
	if (Speed < 2600 && Speed > 0)
	{
		Driver.Velocity = -0.6 * (2600 - Speed) * Velocity/Speed;
		Driver.Velocity.Z = 600;
	}
	else
	{
		Driver.Velocity = vect(0,0,600);
	}

	if ( avaPawn(Driver) != None )
	{
		avaPawn(Driver).CustomGravityScaling = 0.5;
		avaPawn(Driver).bNotifyStopFalling = true;
	}
}

/**
 * Shut down the weapon
 */
simulated function StopFiringWeapon()
{
	local avaWeapon Weap;

	Weap = avaWeapon(Weapon);
	if (Weap != none)
	{
		Weap.ForceEndFire();
	}
}

/** handles the driver pawn of the dead vehicle (decide whether to ragdoll it, etc) */
function HandleDeadVehicleDriver()
{
	local Pawn OldDriver;
	local avaVehicle VehicleBase;

	if (Driver != None)
	{
		VehicleBase = avaVehicle(GetVehicleBase());
		// if Driver wasn't visible in vehicle, destroy it
		if (VehicleBase != None && VehicleBase.bEjectKilledBodies)
		{
			// otherwise spawn dead physics body
			if (!bDriverIsVisible && PlaceExitingDriver())
			{
				Driver.StopDriving(self);
				Driver.DrivenVehicle = self;
			}
			Driver.TearOffMomentum = Velocity * 0.25;
			Driver.SetOwner(None);
			Driver.Died(None, (VehicleBase != None) ? VehicleBase.RanOverDamageType : class'DamageType', Driver.Location);
		}
		else
		{
			OldDriver = Driver;
			Driver = None;
			OldDriver.DrivenVehicle = None;
			OldDriver.Destroy();
		}
	}
}

/**
 * HoldGameObject() Attach GameObject to mesh.
 * @param 	GameObj 	Game object to hold
 */
//! �߰������� �����ϴ� (��߰���) ������Ʈ�� ���ٰ� �����Ѵ�.
//	simulated event HoldGameObject(UTCarriedObject GameObj);

/** TakeHeadShot()
 * @param	Impact - impact information (where the shot hit)
 * @param	HeadShotDamageType - damagetype to use if this is a headshot
 * @param	HeadDamage - amount of damage the weapon causes if this is a headshot
 * @param	AdditionalScale - head sphere scaling for head hit determination
 * @return		true if pawn handled this as a headshot, in which case the weapon doesn't need to cause damage to the pawn.
*/
function bool TakeHeadShot(const out ImpactInfo Impact, class<avaDamageType> HeadShotDamageType, int HeadDamage, float AdditionalScale, Controller InstigatingController)
{
	local avaPawn P;

	// if visible, damageable driver, check if got headshot
	if (bDriverIsVisible && DriverDamageMult > 0.0)
	{
		P = avaPawn(Driver);
		return (P != None);// && P.TakeHeadShot(Impact, HeadShotDamageType, HeadDamage, AdditionalScale, InstigatingController));
	}
	else
	{
		return false;
	}
}

/** Gets the powerlevel for this vehicle - primarily used for charge bars
 * @param PowerLevel (out) - how full the charge bar should be (0 to 1)
 * @return whether this vehicle's HUD should display a charge bar
 */
simulated function bool GetPowerLevel(out float PowerLevel)
{
	local avaWeapon Weap;

	Weap = avaWeapon(Weapon);
	if (Weap != None )//&& Weap.AmmoDisplayType >= EAWDS_BarGraph)
	{
//		PowerLevel = Weap.GetPowerPerc();
		return true;
	}
	else
	{
		return false;
	}
}

defaultproperties
{
	MouseSensitivity = 1.0
	SightRadius=12000.0
//	bCanBeAdheredTo=TRUE
//	bCanBeFrictionedTo=TRUE
//	HudIcons=Texture2D'UI_HUD.HUD.UI_HUD_BaseB'
}
