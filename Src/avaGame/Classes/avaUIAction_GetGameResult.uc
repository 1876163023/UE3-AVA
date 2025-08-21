class avaUIAction_GetGameResult extends UIAction_GetValue;

var int WinnerTeam;
var bool bIsPlayerWinner;
var Object Player;

event Activated()
{
	Local PlayerController		PC, LocalPC;
	Local WorldInfo				WorldInfo;
	Local avaGameReplicationInfo GRI;
	
	// 'Player' Variable is an inacceptable type
	if( Player != None && (avaPlayerController(Player) == None && avaPawn(Player) == None) )
		return;

	if( GetOwnerScene() == None ) 
		return;

	// Get GameReplicationInfo
	WorldInfo = GetOwnerScene().GetWorldInfo();
	if( avaGameReplicationInfo(WorldInfo.GRI) == None )
		return;

	WinnerTeam = TEAM_Unknown;
	bIsPlayerWinner = false;

	GRI = avaGameReplicationInfo(WorldInfo.GRI);
	Assert(GRI != None);

	WinnerTeam = GRI.nWinTeam;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = PC;
		break;
	}

	if ( WinnerTeam < 0 )
	{
		if( 0 <= LocalPC.PlayerReplicationInfo.Team.TeamIndex  && LocalPC.PlayerReplicationInfo.Team.TeamIndex <= 1 )
			WinnerTeam = (GRI.Teams[TEAM_EU].Score > GRI.Teams[TEAM_USSR].Score ? TEAM_EU : TEAM_USSR);
	}
	
	if( Player == None )
	{
		bIsPlayerWinner = (LocalPC.PlayerReplicationInfo.Team.TeamIndex == WinnerTeam);
	}
	else if( avaPlayerController(Player) != None && avaPlayerController(Player).PlayerReplicationInfo != None)
	{
		bIsPlayerWinner = (PlayerController(Player).PlayerReplicationInfo.Team.TeamIndex == WinnerTeam);
	}
	else if ( avaPawn(Player) != None && avaPawn(Player).PlayerReplicationInfo != None )
	{
		bIsPlayerWinner = (avaPawn(Player).PlayerReplicationInfo.Team.TeamIndex == WinnerTeam);
	}
}

defaultproperties
{
	bCallHandler=false
	ObjCategory="UI"
	ObjName="ava GetGameResult"
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="WinnerTeam",PropertyName=WinnerTeam,bWriteable=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bIsPlayerWinner",PropertyName=bIsPlayerWinner,bWriteable=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="PlayerTarget",PropertyName=Player,bWriteable=true,MaxVars=1))
}