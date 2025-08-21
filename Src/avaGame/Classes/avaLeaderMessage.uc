class avaLeaderMessage extends avaLocalMessage
	config(game);

var localized string LeaderCommand[20];
var localized string LeaderLabel;

var const Color LeaderCommandColor;

/* Sound Cue와 Localized String의 Index가 같도록 맞춰져있음. 
  따라서 메세지가 추가 또는 변경되면 사운드와 QuickChat 메세지가 어긋나므로 동기화 바람*/
enum ELeaderCommandType
{
	LEADERCOMMAND_NeedTarget,
	LEADERCOMMAND_NeedRecon,
	LEADERCOMMAND_FollowOrder,
	LEADERCOMMAND_KeepBase,
	LEADERCOMMAND_GoMission,
	LEADERCOMMAND_KillEm,
	LEADERCOMMAND_PlaceBomb,
	LEADERCOMMAND_OnPlacingBomb,
	LEADERCOMMAND_RemoveBomb,
	LEADERCOMMAND_OnRemovingBomb,
	LEADERCOMMAND_TakeArea,
	LEADERCOMMAND_AreaUnderAttack,
	LEADERCOMMAND_OnConvoyingObject,
	LEADERCOMMAND_ConvoyObject,
	LEADERCOMMAND_ObjectFound,
	LEADERCOMMAND_StopObject,
	LEADERCOMMAND_Assault,
	LEADERCOMMAND_Skirmish,
	LEADERCOMMAND_BreakThrough,
	LEADERCOMMAND_DefenseLine,
};

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return default.LeaderCommandColor;
}

static function SoundCue	GetSoundCue( int Index, avaPawn SenderPawn, PlayerController SelfPC )
{
	local int StressType;
	local class< avaLocalizedTeamPack >	LocalizedTeamPack;
	if ( SenderPawn == None )	return None;
	LocalizedTeamPack	= avaGameReplicationInfo( SelfPC.WorldInfo.GRI ).GetLocalizedTeamPack( avaPlayerController( SelfPC ), SenderPawn.GetTeamNum() );
	StressType			= SenderPawn.GetStressLevel();
	return LocalizedTeamPack.default.LeaderSound[Index].Snd[StressType];
}

static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	Local string					Key;
	Local avaMsgParam				Param;
	Local int						nCommand, nOperation;
	Local bool						bAttackTeam;
	local int						nParam;
	local int						index;
	local avaPawn					SenderPawn;
	local avaPlayerReplicationInfo	SenderPRI;
	local string					Message;
	local avaHUD					avaHUD;
	local bool						bInPlaySound;
	
	
	bInPlaySound = true;
	
	Param = avaMsgParam(OptionalObject);
	nParam = Param.IntParam[0];
	nCommand = Switch;
	nOperation = Param.IntParam[0];
	bAttackTeam = Param.BoolParam1;
	Key = nCommand $ "";
	if ( 7 <= nCommand&& nCommand <= 8)
		Key $= (nOperation$(bAttackTeam ? "A" : "D"));

	index		= GetMessageIndexOfKey(Key);
	SenderPRI	= avaPlayerReplicationInfo(RelatedPRI_1);
	SenderPawn	= SenderPRI.GetAvaPawn();
	avaHUD		= avaHUD(P.myHUD);

	if ( index == 0 && avaPlayerController(P).IsSameTeam( SenderPawn ) )	
	{
		if ( nParam == 8 )	// Target 설정 취소
		{
			Message = class'avaLocalizedMessage'.default.TargetCancel;
			bInPlaySound = false;
			avaHUD.AddTarget( None );
		}
		else
		{
			Message = default.LeaderCommand[index] $" "$class'avaHUD'.default.PlaceOfInterest[avaHUD.POIList[nParam].NameIndex];
			avaHUD.AddTarget( avaHUD.POIList[nParam], SenderPawn.GetTeamNum() );
		}
	}
	else				Message = default.LeaderCommand[index];
	DisplayMessage( SenderPawn, SenderPRI, P, Message, Index, bInPlaySound );
}

defaultproperties
{	
	LeaderCommandColor=(R=200,G=63,B=105,A=220)

	// nWaypointTeam $ nStrategy $ nWaypointNum	

	Begin Object class=avaLocalSound name=Message1
		Key="1"
		MsgIndex=LEADERCOMMAND_NeedTarget
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message2
		Key="2"
		MsgIndex=LEADERCOMMAND_NeedRecon
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message3
		Key="3"
		MsgIndex=LEADERCOMMAND_FollowOrder
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message4
		Key="4"
		MsgIndex=LEADERCOMMAND_KeepBase
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message5
		Key="5"
		MsgIndex=LEADERCOMMAND_GoMission
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message6
		Key="6"
		MsgIndex=LEADERCOMMAND_KillEm
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message7
		Key="70A"
		MsgIndex=LEADERCOMMAND_PlaceBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message8
		Key="80A"
		MsgIndex=LEADERCOMMAND_OnPlacingBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message9
		Key="70D"
		MsgIndex=LEADERCOMMAND_RemoveBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message10
		Key="80D"
		MsgIndex=LEADERCOMMAND_OnRemovingBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message11
		Key="71A"
		MsgIndex=LEADERCOMMAND_TakeArea
		Shout(2)=1
	End Object


	Begin Object class=avaLocalSound name=Message12
		Key="81A"
		MsgIndex=LEADERCOMMAND_AreaUnderAttack
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message13
		Key="72A"
		MsgIndex=LEADERCOMMAND_OnConvoyingObject
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message14
		Key="82A"
		MsgIndex=LEADERCOMMAND_ConvoyObject
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message15
		Key="72D"
		MsgIndex=LEADERCOMMAND_ObjectFound
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message16
		Key="82D"
		MsgIndex=LEADERCOMMAND_StopObject
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message17
		Key="73A"
		MsgIndex=LEADERCOMMAND_Assault
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message18
		Key="83A"
		MsgIndex=LEADERCOMMAND_Skirmish
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message19
		Key="74A"
		MsgIndex=LEADERCOMMAND_BreakThrough
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message20
		Key="84A"
		MsgIndex=LEADERCOMMAND_DefenseLine
		Shout(2)=1
	End Object

	MsgData(0)	= Message1
	MsgData(1)	= Message2
	MsgData(2)	= Message3
	MsgData(3)	= Message4
	MsgData(4)	= Message5
	MsgData(5)	= Message6
	MsgData(6)	= Message7
	MsgData(7)	= Message8
	MsgData(8)	= Message9
	MsgData(9)	= Message10
	MsgData(10)	= Message11
	MsgData(11)	= Message12
	MsgData(12)	= Message13
	MsgData(13)	= Message14
	MsgData(14)	= Message15
	MsgData(15)	= Message16
	MsgData(16)	= Message17
	MsgData(17)	= Message18
	MsgData(18)	= Message19
	MsgData(19)	= Message20


	// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
	//MessageData.Sort(Key)
}