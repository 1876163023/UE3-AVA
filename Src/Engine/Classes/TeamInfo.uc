//=============================================================================
// TeamInfo.
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class TeamInfo extends ReplicationInfo
	native
	nativereplication;

var hmserialize databinding localized string TeamName;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var databinding int Size; //number of players on this team in the level
var hmserialize databinding float Score;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize databinding int TeamIndex;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var databinding color TeamColor;

cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061128 dEAthcURe|HM
	#endif
	
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	// Variables the server should send to the client.
	if( bNetDirty && (Role==ROLE_Authority) )
		Score, TeamIndex;

	if ( bNetInitial && (Role==ROLE_Authority) )
		TeamName;
}

function bool AddToTeam( Controller Other )
{
	local Controller P;
	local bool bSuccess;

	// make sure loadout works for this team
	if ( Other == None )
	{
		`log("Added none to team!!!");
		return false;
	}

	Size++;
	Other.PlayerReplicationInfo.Team = self;
	Other.PlayerReplicationInfo.NetUpdateTime = WorldInfo.TimeSeconds - 1;

	bSuccess = false;
	if ( Other.IsA('PlayerController') )
		Other.PlayerReplicationInfo.TeamID = 0;
	else
		Other.PlayerReplicationInfo.TeamID = 1;

	while ( !bSuccess )
	{
		bSuccess = true;
		foreach WorldInfo.AllControllers(class'Controller', P)
		{
			if ( P.bIsPlayer && (P != Other)
				&& (P.PlayerReplicationInfo.Team == Other.PlayerReplicationInfo.Team)
				&& (P.PlayerReplicationInfo.TeamId == Other.PlayerReplicationInfo.TeamId) )
			{
				bSuccess = false;
				break;
			}
		}
		if ( !bSuccess )
		{
			Other.PlayerReplicationInfo.TeamID = Other.PlayerReplicationInfo.TeamID + 1;
		}
	}
	return true;
}

function RemoveFromTeam(Controller Other)
{
	Size--;
}

simulated function string GetHumanReadableName()
{
	return TeamName;
}

/* GetHUDColor()
returns HUD color associated with this team
*/
simulated function color GetHUDColor()
{
	return TeamColor;
}

/* GetTextColor()
returns text color associated with this team
*/
function color GetTextColor()
{
	return TeamColor;
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	NetUpdateFrequency=2
	TeamColor=(r=255,g=64,b=64,a=255)
}
