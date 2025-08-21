/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
/** activates/deactivates infinite ammo for the target(s) */
class avaSeqAct_InfiniteAmmo extends SequenceAction;

var() bool bInfiniteAmmo;

event Activated()
{
	local SeqVar_Object ObjVar;
	local Pawn P;
	local avaInventoryManager InvManager;

	foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Target")
	{
		P = Pawn(ObjVar.GetObjectValue());
		if (P != None)
		{
			InvManager = avaInventoryManager(P.InvManager);
			if (InvManager != None)
			{
				InvManager.bInfiniteAmmo = bInfiniteAmmo;
			}
		}
	}
}

defaultproperties
{
	ObjName="Infinite Ammo"
	bInfiniteAmmo=true
	bCallHandler=false
}
