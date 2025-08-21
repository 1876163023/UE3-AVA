/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
/** base class for gibs */
class avaGib extends Actor;

/** sound played when we hit a wall */
var SoundCue HitSound;

/** the component that will be set to the chosen mesh */
var StaticMeshComponent Mesh;

function Timer()
{
	local PlayerController PC;

	ForEach LocalPlayerControllers(PC)
	{
		if( pc.ViewTarget == self )
		{
			SetTimer(4.0, false);
			return;
		}
	}

	Destroy();
}

event BecomeViewTarget(PlayerController PC)
{
	SetHidden(True);
	LifeSpan = 0;
	SetTimer(4.0, false);
}

/**
 *	Calculate camera view point, when viewing this actor.
 *
 * @param	fDeltaTime	delta time seconds since last update
 * @param	out_CamLoc	Camera Location
 * @param	out_CamRot	Camera Rotation
 * @param	out_FOV		Field of View
 *
 * @return	true if Actor should provide the camera point of view.
 */
simulated function bool CalcCamera( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	if ( Physics != PHYS_None )
	{
		out_CamRot = Rotation;
	}
	out_CamLoc = Location;
	return false;
}

simulated event RigidBodyCollision( PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent,
					const out CollisionImpactData RigidCollisionData, int ContactIndex )
{
	if ( LifeSpan < 7.3 )
	{
		PlaySound(HitSound, true);
		Mesh.SetNotifyRigidBodyCollision(false);
	}
}

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=GibMesh
		BlockActors=false
		CollideActors=true
		BlockRigidBody=true
		CastShadow=false
		bCastDynamicShadow=false
		Scale=1.5
		bNotifyRigidBodyCollision=true
		ScriptRigidBodyCollisionThreshold=10.0
		//bUseHardwareScene=true
	End Object
	Components.Add(GibMesh)
	Mesh=GibMesh
	CollisionComponent=GibMesh

	TickGroup=TG_PostAsyncWork
	RemoteRole=ROLE_None
	Physics=PHYS_RigidBody
	bNoEncroachCheck=true
	bCollideActors=true
	bBlockActors=false
	bWorldGeometry=false
	bCollideWorld=true
	bProjTarget=true
	LifeSpan=8.0
	bGameRelevant=true

	//Mass=30

	//HitSound=SoundCue'A_Gameplay.A_Gameplay_GibSmallCue'
}
