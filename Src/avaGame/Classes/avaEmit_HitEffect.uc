/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaEmit_HitEffect extends avaReplicatedEmitter;

var repnotify name BoneName;

replication
{
	if ( Role == ROLE_Authority )
		BoneName;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'BoneName' )
	{
		if ( (Pawn(Base) != None) && (Pawn(Base).Mesh != None) )
		{
			AttachTo(Pawn(Base), BoneName);
		}
	}
	else
		super.ReplicatedEvent(VarName);
}

simulated function AttachTo( Pawn P, name NewBoneName )
{
	BoneName = NewBoneName;
	if (WorldInfo.NetMode == NM_DedicatedServer || NewBoneName == '')
	{
		SetBase(P);
	}
	else
	{
		SetBase(P,, P.Mesh, BoneName);
	}
}

simulated function PawnBaseDied()
{
	if (ParticleSystemComponent != None)
	{
		ParticleSystemComponent.DeactivateSystem();
	}
}

defaultproperties
{
	BoneName=INVALID // so name of 'None' gets replicated and triggers ReplicatedEvent() on clients
}
