/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// CheatManager
// Object within playercontroller that manages "cheat" commands
// only spawned in single player mode
//=============================================================================

class avaCheatManager extends CheatManager within PlayerController
	native;

`include(avaGame/avaGame.uci)

/* AllWeapons
	Give player all available weapons
*/
`devexec function AllWeapons()
{
	if( (WorldInfo.NetMode!=NM_Standalone) || (Pawn == None) )
		return;

	// Weapons			
}

/* AllAmmo
	Sets maximum ammo on all weapons
*/
`devexec function AllAmmo()
{
	if ( (Pawn != None) && (avaInventoryManager(Pawn.InvManager) != None) )
	{
		avaInventoryManager(Pawn.InvManager).AllAmmo(true);
		avaInventoryManager(Pawn.InvManager).bInfiniteAmmo = true;
	}
}


`devexec function KillBadGuys()
{
	local playercontroller PC;
	local avaPawn p;

	PC = avaPlayerController(Outer);

	if (PC!=none)
	{
		ForEach DynamicActors(class'avaPawn', P)
		{
			if ( !WorldInfo.GRI.OnSameTeam(P,PC) )
			{
				P.TakeDamage(20000,P.Controller, P.Location, Vect(0,0,0),class'avaDamageType');
			}
		}
	}
}

`devexec function RBGrav(float NewGravityScaling)
{
	WorldInfo.RBPhysicsGravityScaling = NewGravityScaling;
}
