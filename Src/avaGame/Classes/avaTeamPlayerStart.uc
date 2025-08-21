/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaTeamPlayerStart extends PlayerStart
	native;

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void Spawned();
}

// Players on different teams are not spawned in areas with the
// same TeamNumber unless there are more teams in the level than
// team numbers.
var() ETeamType			TeamNumber;			// what team can spawn at this start
var() byte				ManagerGroup;		// 0 for non group.
var() EWaypointTeamType	SquadIndex;			// what squad can spawn at this start

// sprites used for this actor in the editor, depending on which team it's on
var array<Texture2D> TeamSprites;

simulated function OnToggle(SeqAct_Toggle action)
{
	if ( ManagerGroup != 0 )
	{
		// manager group 에 속해 있는 녀석은 kismet 에서 독자적으로 조작하지 마라.
		`warn( "do not use kismet within managerGroup" );
	}

	super.OnToggle( action );
}


defaultproperties
{
	TeamSprites[0]=Texture2D'EnvyEditorResources.S_Player_Red'
	TeamSprites[1]=Texture2D'EnvyEditorResources.S_Player_Blue'
	TeamSprites[2]=Texture2D'EnvyEditorResources.S_Player_Gray'

	ManagerGroup = 0

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0017.000000
		CollisionHeight=+0046.000000
	End Object
}
