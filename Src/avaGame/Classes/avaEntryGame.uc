/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaEntryGame extends avaGame;




function StartMatch()
{}

// Parse options for this game...
event InitGame( string Options, out string ErrorMessage )
{}

auto State MatchPending
{
	function RestartPlayer( Controller aPlayer )
	{
	}

	function Timer()
    {
    }

    function BeginState(Name PreviousStateName)
    {
		bWaitingToStartMatch = true;
		avaGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
    }

	function EndState(Name NextStateName)
	{
		avaGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
	}
}
