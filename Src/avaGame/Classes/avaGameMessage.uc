class avaGameMessage extends GameMessage;

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if ( RelatedPRI_1 != None && avaPlayerReplicationInfo( RelatedPRI_1 ).bSilentLogin == true )
		return "";

	return Super.GetString( Switch, bPRI1HUD, RelatedPRI_1, RelatedPRI_2, OptionalObject );
}

static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local avaPlayerController avaP;
	Local string MessageString;
	Local avaHUD HUD;

	avaP = avaPlayerController(P);
	if ( avaP == None )
		return;

	MessageString = static.GetString(Switch, (RelatedPRI_1 == P.PlayerReplicationInfo), RelatedPRI_1, RelatedPRI_2, OptionalObject);
//	`Log("GameMessage = "$MessageString$", Switch = "$Switch);

	if(MessageString == "")
		return;

	HUD = avaHUD(P.MyHUD);
	Assert(HUD != None);
	HUD.Message(RelatedPRI_1, MessageString, 'System');
}