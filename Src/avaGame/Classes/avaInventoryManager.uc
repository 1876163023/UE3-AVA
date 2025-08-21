/**
 * avaInventoryManager
 * ava inventory definition
 *
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaInventoryManager extends InventoryManager
	config(Game) native;


`define		THROW_WEAPON_GROUP	4

/** if true, when you pickup a weapon, check to see if we should switch to it */
var config bool bAutoSwitchWeaponOnPickup;

/** if true, all weapons use no ammo */
var bool bInfiniteAmmo;

/** This struct defines ammo that is stored in inventory, but for which the pawn doesn't yet have a weapon for. */
struct native AmmoStore
{
	var	int				Amount;
	var class<avaWeapon> WeaponClass;
};

/** Stores the currently stored up ammo */
var array<AmmoStore> AmmoStorage;

/** Holds the last weapon used */
var Weapon PreviousWeapon;

/** When was the last adjustment made to the weapon list */

var float LastAdjustTime;

// {{ [+] 20070213 dEAthcURe|HM
var repnotify bool bHostMigrationComplete; 

var inventory	ReservedItemToRemove;

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bHostMigrationComplete' )
	{
		makePendingWeaponAsCurrent();
	}	
}

simulated function makePendingWeaponAsCurrent()
{
	local Inventory Inv;
	local Weapon DesiredWeapon;
	
	if(pendingWeaponClass != None) {
		Inv = FindInventoryType( pendingWeaponClass );
		if(Inv != None) {
			//`log("[dEAthcURe|avaInventoryManager|makePendingWeaponAsCurrent] desired weapon replaced with pended one");	//dEAthcURe|HM		
			DesiredWeapon = Weapon(inv);
			pendingWeaponClass = None;
			
			SetPendingWeapon(DesiredWeapon);
			
			if( Role < Role_Authority ) {
				//`log("[dEAthcURe|makePendingWeaponAsCurrent] --before ServerSetCurrentWeapon"); // dEAthcURe|HM
				ServerSetCurrentWeapon(DesiredWeapon);
			}
		}
		else {
			//`log("[dEAthcURe|avaInventoryManager|makePendingWeaponAsCurrent] No inventory found."); // dEAthcURe|HM
		}
	}	
}
// }} [+] 20070213 dEAthcURe|HM

replication
{
	// {{ dEAthcURe|HM
	if (Role==ROLE_Authority && bNetDirty && bNetOwner )
		bHostMigrationComplete;
	// }} dEAthcURe|HM
}

// Rifle 의 경우 가중치를 둔다.
// ToDo : Rifle 과 Pistol, Knife, Grenade 에 따라서 가중치를 추가할 것
simulated function float GetWeaponRatingFor( Weapon W )
{
	local float Rating;

	if ( !W.HasAnyAmmo() )	return 0.1;

	Rating = 1;
	// 일반 라이플이 최우선
	if ( avaWeap_BaseRifle(W) != None )			Rating += 0.5;
	if ( avaWeap_BaseSMG(W) != None )			Rating += 0.5;
	if ( avaWeap_BaseSniperRifle(W) != None )	Rating += 0.5;
	// 피스톨
	if ( avaWeap_BasePistol(W) != None )		Rating += 0.4;
	// 칼
	if ( avaWeap_BaseKnife(W) != None )			Rating += 0.3;
	
	// tend to stick with same weapon
	if ( !Instigator.IsHumanControlled() && IsActiveWeapon( W ) && (Instigator.Controller.Enemy != None) )
		Rating += 0.21;

	return Rating;
}

/**
 * returns the best weapon for this Pawn in loadout
 */
simulated function Weapon GetBestWeapon( optional bool bForceADifferentWeapon  )
{
	local Weapon	W, BestWeapon;
	local float		Rating, BestRating;

	ForEach InventoryActors( class'Weapon', W )
	{
		if( bForceADifferentWeapon &&
			IsActiveWeapon( W ) || avaWeapon(W).bNoSelectable == true )
		{
			continue;
		}

		Rating = GetWeaponRatingFor(W);
		if( BestWeapon == None || Rating > BestRating )
		{
			BestWeapon = W;
			BestRating = Rating;
		}
	}

	return BestWeapon;
}

/**
 * This function returns a sorted list of weapons, sorted by their InventoryWeight
 */
simulated function GetWeaponList(out array<avaWeapon> WeaponList, optional bool bFilter, optional int GroupFilter, optional bool bNoEmpty)
{
	local avaWeapon Weap;
	local int i;
	ForEach InventoryActors( class'avaWeapon', Weap )
	{
		if ( Weap.bNoSelectable == true )	continue;
		if ( ReservedItemToRemove == Weap )	continue;
		if ( (!bFilter || Weap.InventoryGroup == GroupFilter) ) //&& ( !bNoEmpty || Weap.HasAnyAmmo()) )
		{
			if ( !Weap.HasAnyAmmo() && avaThrowableWeapon( Weap ) != None )
				continue;
			// Group 별로 Sort 해야 한다....
			if ( WeaponList.Length > 0 )
			{
				for ( i = 0 ; i < WeaponList.length ; i++ )
				{
					if ( WeaponList[i].InventoryGroup > Weap.InventoryGroup )
					{
						WeaponList.Insert(i,1);
						WeaponList[i] = Weap;
						break;
					}
				}
				if (i==WeaponList.Length)
				{
					WeaponList.Length = WeaponList.Length+1;
					WeaponList[i] = Weap;
				}
			}
			else
			{
				WeaponList.Length = 1;
				WeaponList[0] = Weap;
			}
		}
	}
}

/**
 * Handling switching to a weapon group
 */

simulated function SwitchWeapon(byte NewGroup)
{
	local avaWeapon CurrentWeapon;
	local array<avaWeapon> WeaponList;
	local int NewIndex;

	// Get the list of weapons

   	GetWeaponList(WeaponList,true,NewGroup);

	// Exit out if no weapons are in this list.

	if (WeaponList.Length<=0)
		return;

	CurrentWeapon = avaWeapon(PendingWeapon);
	if (CurrentWeapon == None)
	{
		CurrentWeapon = avaWeapon(Instigator.Weapon);
	}

	if (CurrentWeapon == none || CurrentWeapon.InventoryGroup != NewGroup)
	{
		// Changing groups, so activate the first weapon in the array

		NewIndex = 0;
	}
	else
	{
		// Find the current weapon's position in the list and switch to the one above it

		for (NewIndex=0;NewIndex<WeaponList.Length;NewIndex++)
		{
			if (WeaponList[NewIndex] == CurrentWeapon)
				break;
		}
		NewIndex++;
		if (NewIndex>=WeaponList.Length)		// start the beginning if past the end.
			NewIndex = 0;
	}

	// 가지고 있는 무기와 같은 Weapon 이다. 아무일도 하지 말자.
	if ( WeaponList[NewIndex] == CurrentWeapon )
		return;

	// Begin the switch process...
	SetCurrentWeapon(WeaponList[NewIndex]);
}

simulated function AdjustWeapon(int NewOffset)
{
	local Weapon CurrentWeapon;
	local array<avaWeapon> WeaponList;
	local int i,Index;


	if (LastAdjustTime > 0.f && WorldInfo.TimeSeconds - LastAdjustTime < 0.15)
	{
		return;
	}

	LastAdjustTime = WorldInfo.TimeSeconds;

	CurrentWeapon = avaWeapon(PendingWeapon);
	if (CurrentWeapon == None)
	{
		CurrentWeapon = avaWeapon(Instigator.Weapon);
	}

   	GetWeaponList(WeaponList,,,true);
	for (i=0;i<WeaponList.Length;i++)
	{
		if ( WeaponList[i] == CurrentWeapon )
		{
			Index = i;
			break;
		}
	}

   	Index += NewOffset;

   	if (Index<0)
   	{
   		Index = WeaponList.Length-1;
   	}

   	if ( Index >= WeaponList.Length )
   	{
   		Index = 0;
   	}

   	if ( Index >=0 )
   	{
		SetCurrentWeapon( WeaponList[Index] );
	}
}

/**
 * Switches to Previous weapon
 * Network: Client
 */
simulated function PrevWeapon()
{
	if ( avaWeapon(Pawn(Owner).Weapon) != None && avaWeapon(Pawn(Owner).Weapon).DoOverridePrevWeapon() )
		return;

	AdjustWeapon(-1);
}

/**
 *	Switches to Next weapon
 *	Network: Client
 */
simulated function NextWeapon()
{
	if ( avaWeapon(Pawn(Owner).Weapon) != None && avaWeapon(Pawn(Owner).Weapon).DoOverrideNextWeapon() )
		return;

	AdjustWeapon(+1);
}


/** AllAmmo()
All weapons currently in inventory have ammo increased to max allowed value.  Super weapons will only have their ammo amount changed if
bAmmoForSuperWeapons is true.
*/
function AllAmmo(optional bool bAmmoForSuperWeapons)
{
	local Inventory Inv;

	for( Inv=InventoryChain; Inv!=None; Inv=Inv.Inventory )
		if ( (avaWeapon(Inv)!=None) && (bAmmoForSuperWeapons || !avaWeapon(Inv).bSuperWeapon) )
			avaWeapon(Inv).Loaded(true);
}


/**
 * SetCurrentWeapon starts a weapon change.  It calls SetPendingWeapon and then if it's called
 * on a remote client, tells the server to begin the process.
 *
 * @param	DesiredWeapon		The Weapon to switch to
 */

reliable client function SetCurrentWeapon( Weapon DesiredWeapon, optional bool bForced ) // [!] 20070214 dEAthcURe|HM // original reliable client function SetCurrentWeapon( Weapon DesiredWeapon)
{
	local Weapon PrevWeapon;
	local Inventory Inv; // [+] 20070213 dEAthcURe|HM
	
	// {{ [+] 20070213 dEAthcURe|HM
	if(pendingWeaponClass != None) {		
		Inv = FindInventoryType( pendingWeaponClass );		
		if(Inv != None) {
			//`log("[dEAthcURe|avaInventoryManager|SetCurrentWeapon] desired weapon replaced with pended one");			
			DesiredWeapon = Weapon(inv);
			pendingWeaponClass = None;
		}		
	}
	// }} [+] 20070213 dEAthcURe|HM

	PrevWeapon = Instigator.Weapon;

	// PrevWeapon 이 Weapon 을 바꾸기를 거부한다면 PendingWeapon 에 넣지 말아야 한다...
	// 만약 PendingWeapon 에 넣으려면 PutDown 을 제대로 구현해야 한다...
	if(!bForced) { // [+] 20070214 dEAthcURe|HM
	if ( PrevWeapon != None && PrevWeapon.DenyClientWeaponSet() )	return;
	} // [+] 20070214 dEAthcURe|HM

	SetPendingWeapon(DesiredWeapon);

	// If we are a remote client, make sure the Server Set's its pending weapon

	if( Role < Role_Authority )
	{
		`log( "avaInventoryManager.SetCurrentWeapon" @DesiredWeapon );
		ScriptTrace();
		ServerSetCurrentWeapon(DesiredWeapon);
	}
}


/**
 * Accessor for the server to begin a weapon switch on the client.
 *
 * @param	DesiredWeapon		The Weapon to switch to
 */

reliable client function ClientSetCurrentWeapon(Weapon DesiredWeapon)
{
	SetPendingWeapon(DesiredWeapon);
}

/**
 * When a client-switch begins on a remote-client, the server needs to be told to
 * start the process as well.  SetCurrentWeapon() makes that call.
 *
 * NETWORK - This function should *ONLY* be called from a remote client's SetCurrentWeapon()
 * function.
 *
 * @param	DesiredWeapon		The Weapon to switch to
 */

reliable server function ServerSetCurrentWeapon(Weapon DesiredWeapon)
{
	SetPendingWeapon(DesiredWeapon);
}

/**
 * This is the work-horse of the weapon switch.  It will set a new pending weapon
 * and tell the weapon to begin the switch.  If the call to Weapon.TryPutdown() returns
 * false, it means that the weapon can't switch at the moment and has deferred it until later
 *
 * @param	DesiredWeapon		The Weapon to switch to
 */

simulated function SetPendingWeapon( Weapon DesiredWeapon )
{
	local avaWeapon PrevWeapon;

	PrevWeapon = avaWeapon( Instigator.Weapon );
	/*
	if ( Instigator.PlayerReplicationInfo != None )
	{
		`log("################## SetPendingWeapon "@Instigator.PlayerReplicationInfo.PlayerName);
		`log("###   Owner			:"@Owner);
		`log("###   Role				:"@Role);
		`log("###   Desired Weapon	:"@DesiredWeapon);
		`log("###   Current Weapon   :"@PrevWeapon);
		`log("###   Pending Weapon 	:"@PendingWeapon);
	}
	*/
	// We only work with UTWeapons

	// Detect that a weapon is being reselected.  If so, notify that weapon.

	if ( DesiredWeapon != None && DesiredWeapon == Instigator.Weapon )
	{
		//`log("###   *** RESELECTED **");

		if (PendingWeapon != None)
		{
			PendingWeapon = None;
		}
		//else
		//{
		//	PrevWeapon.ServerReselectWeapon();
		//}

		//// If this weapon is ready to fire, there is no reason to perform the whole switch logic.
		//PrevWeapon.Activate();
		//if (!PrevWeapon.bReadyToFire())
		//{
		//	PrevWeapon.Activate();
		//}
		
		// {{ 20070214 dEAthcURe|HM
		PrevWeapon.Activate();
		//`log("[dEAthcURe|avaInventoryManager::SetPendingWeapon] PrevWeapon.Activate()");
		// }} 20070214 dEAthcURe|HM
	}
	else
	{
		PendingWeapon = DesiredWeapon;

		// if there is an old weapon handle it first.
		if( PrevWeapon != None && !PrevWeapon.bDeleteMe && !PrevWeapon.IsInState('Inactive') )
		{
			PrevWeapon.TryPutDown();
			/*
			if ( !PrevWeapon.TryPutdown() )
			{
				`log("###    ->>>>>>>>>>> Weapon defered putting itself down until it's done firing");
			}
			*/
		}
		else
		{
			//`log("###    ->>>>>>>>>>> Immediately called Change Weapon");

			// We don't have a weapon, force the call to ChangedWeapon
			ChangedWeapon();
		}
	}
}


simulated function ClientWeaponSet(Weapon NewWeapon, bool bOptionalSet)
{
	local Weapon OldWeapon;

	OldWeapon = Instigator.Weapon;

	// If no current weapon, then set this one
	if( OldWeapon == None || OldWeapon.bDeleteMe || OldWeapon.IsInState('Inactive') )
	{
		SetCurrentWeapon(NewWeapon);
		return;
	}

	if( OldWeapon == NewWeapon )
	{
		return;
	}

	if( bOptionalSet )
	{
		if( OldWeapon.DenyClientWeaponSet() ||
			(Instigator.IsHumanControlled() && PlayerController(Instigator.Controller).bNeverSwitchOnPickup) )
		{
			LastAttemptedSwitchToWeapon = NewWeapon;
			return;
		}
	}

	SetCurrentWeapon(NewWeapon);
	//if( PendingWeapon == None || !PendingWeapon.HasAnyAmmo() || PendingWeapon.GetWeaponRating() < NewWeapon.GetWeaponRating() )
	//{
	//	// Compare switch priority and decide if we should switch to new weapon
	//	if( !Instigator.Weapon.HasAnyAmmo() || Instigator.Weapon.GetWeaponRating() < NewWeapon.GetWeaponRating() )
	//	{
	//		SetCurrentWeapon(NewWeapon);
	//		return;
	//	}
	//}
	//NewWeapon.GotoState('Inactive');
}

/**
 * Handle AutoSwitching to a weapon
 */

simulated function bool AddInventory( Inventory NewItem, optional bool bDoNotActivate )
{
	local bool bResult;

	if (Role == ROLE_Authority)
	{
		if ( avaWeapon(NewItem).bNoSelectable == true )
			bDoNotActivate = true;

		bResult = super.AddInventory(NewItem, bDoNotActivate);	
	}

	//if (bResult && avaWeapon(NewItem) != None)
	//{
	//	avaWeapon(NewItem).SetLightEnvironment( avaCharacter(Instigator).LightEnvironment );

	//	// Check to see if we need to give it any extra ammo the pawn has picked up

	//	for (i=0;i<AmmoStorage.Length;i++)
	//	{
	//		if (AmmoStorage[i].WeaponClass == NewItem.Class)
	//		{
	//			avaWeapon(NewItem).AddAmmo(AmmoStorage[i].Amount);
	//			AmmoStorage.Remove(i,1);
	//			break;
	//		}
	//	}

	//	if (!bDoNotActivate && bAutoSwitchWeaponOnPickup && Instigator != None && PlayerController(Instigator.Controller) != None)
	//	{
	//		if ( (avaWeapon(Instigator.Weapon) == None || avaWeapon(Instigator.Weapon).Priority < avaWeapon(NewItem).Priority)
	//			&& (Instigator.Weapon == None || !Instigator.Weapon.IsFiring()) )
	//		{
	//			avaWeapon(NewItem).ClientWeaponSet(true);
	//		}
	//	}
	//}
	return bResult;
}


simulated function DiscardInventory()
{
	if (Role==ROLE_Authority)
	{
		Super.DiscardInventory();
	}

	//local Inventory	Inv;
	//local vector	TossVelocity;
	//`log ( "DiscardInventory" );
	//ForEach InventoryActors(class'Inventory', Inv)
	//{
	//	if( Inv.bDropOnDeath && Instigator != None )
	//	{
	//		TossVelocity = vector(Instigator.GetViewRotation());
	//		TossVelocity = TossVelocity * ((Instigator.Velocity dot TossVelocity) + 500.f) + 250.f * VRand() + vect(0,0,250);
	//		Inv.DropFrom(Instigator.Location, TossVelocity);
	//	}
	//	else
	//	{
	//		Inv.Destroy();
	//	}
	//}
}

reliable server function RequestRemoveFromInventory(Inventory ItemToRemove)
{
	RemoveFromInventory( ItemToRemove );
}

simulated function RemoveFromInventory(Inventory ItemToRemove)
{
	if (Role==ROLE_Authority)
	{
		Super.RemoveFromInventory(ItemToRemove);
	}
}


function bool NeedsAmmo(class<avaWeapon> TestWeapon)
{
	local array<avaWeapon> WeaponList;
	local int i;

	// Check the list of weapons

   	GetWeaponList(WeaponList);
	for (i=0;i<WeaponList.Length;i++)
	{
		if ( WeaponList[i].Class == TestWeapon )	// The Pawn has this weapon
		{
			if ( WeaponList[i].AmmoCount < WeaponList[i].MaxAmmoCount )
				return true;
			else
				return false;
		}
	}

	// Check our stores.

	for (i=0;i<AmmoStorage.Length;i++)
	{
		if (AmmoStorage[i].WeaponClass == TestWeapon)
		{
			if ( AmmoStorage[i].Amount < TestWeapon.default.MaxAmmoCount )
				return true;
			else
				return false;
		}
	}

	return true;

}

/**
 * Called by the avaAmmoPickup classes, this function attempts to add ammo to a weapon.  If that
 * weapon exists, it adds it otherwise it tracks the ammo in an array for later.
 */

function AddAmmoToWeapon(int AmountToAdd, class<avaWeapon> WeaponClassToAddTo)
{
	local array<avaWeapon> WeaponList;
	local int i;

	// Get the list of weapons

   	GetWeaponList(WeaponList);
	for (i=0;i<WeaponList.Length;i++)
	{
		if ( WeaponList[i].Class == WeaponClassToAddTo )	// The Pawn has this weapon
		{
			WeaponList[i].AddAmmo(AmountToAdd);
			return;
		}
	}

	// Add to to our stores for later.

	for (i=0;i<AmmoStorage.Length;i++)
	{

		// We are already tracking this type of ammo, so just increment the ammount

		if (AmmoStorage[i].WeaponClass == WeaponClassToAddTo)
		{
			AmmoStorage[i].Amount += AmountToAdd;
			return;
		}
	}

	// Track a new type of ammo

	i = AmmoStorage.Length;
	AmmoStorage.Length = AmmoStorage.Length + 1;
	AmmoStorage[i].Amount = AmountToAdd;
	AmmoStorage[i].WeaponClass = WeaponClassToAddTo;

}

/**
 * Scans the inventory looking for any of type InvClass.  If it finds it it returns it, other
 * it returns none.
 */

function Inventory HasInventoryOfClass(class<Inventory> InvClass)
{
	local inventory inv;

	inv = InventoryChain;
	while(inv!=none)
	{
		if (Inv.Class==InvClass)
			return Inv;

		Inv = Inv.Inventory;
	}

	return none;
}

/**
 * Store the last used weapon for later
 */

simulated function ChangedWeapon()
{
	local int i;
	local avaWeapon Wep;

	PreviousWeapon = Pawn(Owner).Weapon;
	
	Super.ChangedWeapon();

	Wep = avaWeapon(Pawn(Owner).Weapon);

	// Clear out Pending fires if the weapon doesn't allow them

	if ( Wep!=none && Wep.bNeverForwardPendingFire )
	{
		for (i=0;i<PendingFire.Length;i++)
		{
			PendingFire[i] = 0;
		}
	}
}

simulated function SwitchToPreviousWeapon()
{
	local array<avaWeapon>	WeaponList;
	local int				i;
	if ( PreviousWeapon != None )
	{
		GetWeaponList(WeaponList,false);

		for ( i = 0 ; i < WeaponList.length ; ++ i )
		{
			if ( PreviousWeapon == WeaponList[i] )
			{
				SetCurrentWeapon( PreviousWeapon );
				return;
			}
		}
	}

	WeaponList.length = 0;
	// 이전 Weapon 이 없다면 THROW_WEAPON_GROUP 이 선택된다...
	// THROW_WEAPON_GROUP 에 아무것도 없다면 NextWeapon 으로 간다...
	GetWeaponList(WeaponList,true,`THROW_WEAPON_GROUP);
	if ( WeaponList.length > 0 )
	{
		SwitchWeapon( `THROW_WEAPON_GROUP );
	}
	else
	{
		NextWeapon();
	}
}

// Weapon 을 지운다음에 Switch 할 지 여부를 Flag 로 둔다.
// Weapon 을 Swap 한 경우에는 Switch 하지 않는게 좋다.
simulated function RemoveFromInventoryEx(Inventory ItemToRemove, bool bSwitch )
{
	local Inventory Item;
	local bool		bFound;

	if( ItemToRemove != None )
	{
		// make sure we don't have other references to the item
		if( ItemToRemove == Instigator.Weapon )
		{
			Instigator.Weapon = None;
		}

		if( InventoryChain == ItemToRemove )
		{
			bFound = TRUE;
			InventoryChain = ItemToRemove.Inventory;
			InventoryChain.NetUpdateTime = WorldInfo.TimeSeconds - 1;
		}
		else
		{
			// If this item is in our inventory chain, unlink it.
			for(Item = InventoryChain; Item != None; Item = Item.Inventory)
			{
				if( Item.Inventory == ItemToRemove )
				{
					bFound = TRUE;
					Item.Inventory = ItemToRemove.Inventory;
					Item.NetUpdateTime = WorldInfo.TimeSeconds - 1;
					break;
				}
			}
		}

		if( bFound )
		{
			ItemToRemove.ItemRemovedFromInvManager();
			ItemToRemove.SetOwner(None);
			ItemToRemove.Inventory = None;
			ItemToRemove.SetInventoryManager( None );
			ItemToRemove.NetUpdateTime = WorldInfo.TimeSeconds - 1;
			//Client_RemoveFromInventory( ItemToRemove );
		}

		if ( bSwitch )
		{
			if ( Instigator.Controller != None)
			{
				Instigator.Controller.ClientSwitchToBestWeapon();
			}
		}
	}
}

// Owner 의 변화가 제대로 Replication 이 되지 않는다... 명시적으로 처리 해 준다...
//reliable client simulated function Client_RemoveFromInventory( Inventory ItemToRemove )
//{
//	ItemToRemove.SetOwner( None );
//}

/**
 * Cut and pasted here except call ClientWeaponSet on the weapon instead of forcing it.
 * This causes all of the "should I put down" logic to occur
 */

simulated function SwitchToBestWeapon( optional bool bForceADifferentWeapon )
{
	local Weapon BestWeapon;

	// if we don't already have a pending weapon,
	if( bForceADifferentWeapon || PendingWeapon == None || AIController(Instigator.Controller) != None )
	{
		// figure out the new weapon to bring up
		BestWeapon = GetBestWeapon( bForceADifferentWeapon );

		//! 같은 무기를 다시 설정할 필요는 없다.
		//! (Bot.ExecuteWhatToDoNext()에서 이 함수가 호출되어 Reload가 끝나지 못한다)
		if ( BestWeapon != None && BestWeapon != Instigator.Weapon )
			SetCurrentWeapon( BestWeapon );
	}
}

simulated function OwnerEvent(name EventName)
{
	local Inventory			Inv;
	if ( EventName == 'died' )
	{
		ForEach InventoryActors(class'Inventory', Inv)
		{
			if( Inv.bReceiveOwnerEvents  )
			{
				Inv.OwnerEvent(EventName);
			}
		}
	}
}

simulated function ReserveRemovedItem( Inventory ItemToRemove )
{
	ReservedItemToRemove = ItemToRemove;
	SetTimer( 1.0f, false, 'ClearReserveRemovedItem' ); 
}

simulated function ClearReserveRemovedItem()
{
	ReservedItemToRemove = None;
}

simulated function DrawHud( HUD H )
{

}

defaultproperties
{
	bMustHoldWeapon=true
	PendingFire(0)=0
	PendingFire(1)=0
	bOnlyRelevantToOwner=FALSE
	//bOnlyRelevantToOwner=FALSE
}
