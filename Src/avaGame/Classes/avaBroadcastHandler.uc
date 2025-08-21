//=============================================================================
// BroadcastHandler
//
// Message broadcasting is delegated to BroadCastHandler by the GameInfo.
// The BroadCastHandler handles both text messages (typed by a player) and
// localized messages (which are identified by a LocalMessage class and id).
// GameInfos produce localized messages using their DeathMessageClass and
// GameMessageClass classes.
//
// Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class avaBroadcastHandler extends BroadcastHandler
	config(Game);

/*
 Broadcast a localized message to Team
 Most messages deal with 0 to 2 related PRIs.
 The LocalMessage class defines how the PRI's and optional actor are used.
*/
event AllowBroadcastLocalizedTeam( Controller Sender, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	local PlayerController P;

	foreach WorldInfo.AllControllers(class'PlayerController', P)
	{
		if (P.PlayerReplicationInfo.Team == Sender.PlayerReplicationInfo.Team)
			BroadcastLocalized(Sender, P, Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
	}
}

event AllowBroadcast( Controller Sender, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	local PlayerController P;
	foreach WorldInfo.AllControllers(class'PlayerController', P)
	{
		BroadcastLocalized(Sender, P, Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
	}
}

// 죽은 사람이 보내는 경우에는 Ghost Chat 이 가능한지를 참조한다...
function BroadcastText( PlayerReplicationInfo SenderPRI, PlayerController Receiver, coerce string Msg, optional name Type )
{
	local bool bDeadSender;
	local bool bDeadReceiver;

	if ( SenderPRI.bOnlySpectator == true || SenderPRI.bIsSpectator == true )											bDeadSender = true;
	if ( Receiver.PlayerReplicationInfo.bOnlySpectator == true || Receiver.PlayerReplicationInfo.bIsSpectator == true )	bDeadReceiver = true;
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bEnableGhostChat == true || !bDeadSender )
	{
		Receiver.TeamMessage( SenderPRI, Msg, Type );
	}
	else
	{
		// Sender 가 죽었고, Ghost Chat 이 허용되지 않음
		if ( bDeadReceiver )
			Receiver.TeamMessage( SenderPRI, Msg, Type );
	}
}

defaultproperties
{
}