class avaMemberMessage extends avaLocalMessage
	config(game);

var localized string MemberCommand[20];

var const Color MemberColor;

enum EMemberCommandType
{
	MEMBERCOMMAND_NeedTarget,
	MEMBERCOMMAND_NeedRecon,
	MEMBERCOMMAND_FollowOrder,
	MEMBERCOMMAND_KeepBase,
	MEMBERCOMMAND_GoMission,
	MEMBERCOMMAND_KillEm,
	MEMBERCOMMAND_PlaceBomb,
	MEMBERCOMMAND_OnPlacingBomb,
	MEMBERCOMMAND_RemoveBomb,
	MEMBERCOMMAND_OnRemovingBomb,
	MEMBERCOMMAND_TakeArea,
	MEMBERCOMMAND_AreaUnderAttack,
	MEMBERCOMMAND_OnConvoyingObject,
	MEMBERCOMMAND_ConvoyObject,
	MEMBERCOMMAND_ObjectFound,
	MEMBERCOMMAND_StopObject,
	MEMBERCOMMAND_Assault,
	MEMBERCOMMAND_Skirmish,
	MEMBERCOMMAND_BreakThrough,
	MEMBERCOMMAND_DefenseLine,
};

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return default.MemberColor;
}

static function SoundCue	GetSoundCue( int Index, avaPawn SenderPawn, PlayerController SelfPC )
{
	local int StressType;
	local class< avaLocalizedTeamPack >	LocalizedTeamPack;
	if ( SenderPawn == None )	return None;
	LocalizedTeamPack	= avaGameReplicationInfo( SelfPC.WorldInfo.GRI ).GetLocalizedTeamPack( avaPlayerController( SelfPC ), SenderPawn.GetTeamNum() );
	StressType			= SenderPawn.GetStressLevel();
	return LocalizedTeamPack.default.MemberSound[Index].Snd[StressType];
}



static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	Local string Key;
	Local avaMsgParam Param;
	Local int nCommand, nOperation;
	Local bool bAttackTeam;
	local int index;
	local avaPlayerReplicationInfo	SenderPRI;
	local avaPawn					SenderPawn;
	Param = avaMsgParam(OptionalObject);
	nCommand = Switch;
	nOperation = Param.IntParam[0];
	bAttackTeam = Param.BoolParam1;
	Key = nCommand $ "";
	if ( 7 <= nCommand&& nCommand <= 8)
		Key $= (nOperation$(bAttackTeam ? "A" : "D"));
	SenderPRI = avaPlayerReplicationInfo(RelatedPRI_1);
	SenderPawn = SenderPRI.GetAvaPawn();
	index	= GetMessageIndexOfKey(Key);

	`log( "avaMemberMessage" @index @nCommand @bAttackTeam @Key );

	DisplayMessage( SenderPawn, SenderPRI, P, default.MemberCommand[GetMessageIndexOfKey(Key)], Index );
}

defaultproperties
{
	MemberColor=(R=0,G=255,B=0,A=200)

	Begin Object class=avaLocalSound name=Message1
		Key="1"
		MsgIndex=MEMBERCOMMAND_NeedTarget
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message2
		Key="2"
		MsgIndex=MEMBERCOMMAND_NeedRecon
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message3
		Key="3"
		MsgIndex=MEMBERCOMMAND_FollowOrder
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message4
		Key="4"
		MsgIndex=MEMBERCOMMAND_KeepBase
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message5
		Key="5"
		MsgIndex=MEMBERCOMMAND_GoMission
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message6
		Key="6"
		MsgIndex=MEMBERCOMMAND_KillEm
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message7
		Key="70A"
		MsgIndex=MEMBERCOMMAND_PlaceBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message8
		Key="80A"
		MsgIndex=MEMBERCOMMAND_OnPlacingBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message9
		Key="70D"
		MsgIndex=MEMBERCOMMAND_RemoveBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message10
		Key="80D"
		MsgIndex=MEMBERCOMMAND_OnRemovingBomb
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message11
		Key="71A"
		MsgIndex=MEMBERCOMMAND_TakeArea
		Shout(2)=1
	End Object


	Begin Object class=avaLocalSound name=Message12
		Key="81A"
		MsgIndex=MEMBERCOMMAND_AreaUnderAttack
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message13
		Key="72A"
		MsgIndex=MEMBERCOMMAND_OnConvoyingObject
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message14
		Key="82A"
		MsgIndex=MEMBERCOMMAND_ConvoyObject
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message15
		Key="72D"
		MsgIndex=MEMBERCOMMAND_ObjectFound
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message16
		Key="82D"
		MsgIndex=MEMBERCOMMAND_StopObject
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message17
		Key="73A"
		MsgIndex=MEMBERCOMMAND_Assault
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message18
		Key="83A"
		MsgIndex=MEMBERCOMMAND_Skirmish
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message19
		Key="74A"
		MsgIndex=MEMBERCOMMAND_BreakThrough
		Shout(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message20
		Key="84A"
		MsgIndex=MEMBERCOMMAND_DefenseLine
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
				

	//MessageData.Add((Key="1", MsgIndex=MEMBERCOMMAND_AllMove, SoundCue[0]=SoundCue'avaQuickVoices.CommandVoices.CL_AllMove_1_De',SoundCue[1]=SoundCue'avaQuickVoices.CommandVoices.CL_AllMove_1_Ru'))
	//MessageData.Add((Key="2", MsgIndex=MEMBERCOMMAND_AllAttack, SoundCue[0]=SoundCue'avaQuickVoices.CommandVoices.CL_AllAttack_1_De',SoundCue[1]=SoundCue'avaQuickVoices.CommandVoices.CL_AllAttack_1_Ru'))
	//MessageData.Add((Key="3", MsgIndex=MEMBERCOMMAND_AllDefend, SoundCue[0]=SoundCue'avaQuickVoices.CommandVoices.CL_AllDefend_1_De',SoundCue[1]=SoundCue'avaQuickVoices.CommandVoices.CL_AllDefend_1_Ru'))
	//MessageData.Add((Key="4", MsgIndex=MEMBERCOMMAND_FallBack, SoundCue[0]=SoundCue'avaQuickVoices.CommandVoices.C_FallBack_De',SoundCue[1]=SoundCue'avaQuickVoices.CommandVoices.C_FallBack_Ru'))
	//MessageData.Add((Key="5", MsgIndex=MEMBERCOMMAND_FollowMe, AnimIndex=2,SoundCue[0]=SoundCue'avaQuickVoices.CommandVoices.C_Follow_De',AnimIndex=2,SoundCue[1]=SoundCue'avaQuickVoices.CommandVoices.C_Follow_Ru'))
	//MessageData.Add((Key="6", MsgIndex=MEMBERCOMMAND_HoldThisPosition, SoundCue[0]=SoundCue'avaQuickVoices.CommandVoices.C_HoldPosition_De',SoundCue[1]=SoundCue'avaQuickVoices.CommandVoices.C_HoldPosition_Ru'))
	//MessageData.Add((Key="70A", MsgIndex=MEMBERCOMMAND_PlaceBomb, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_PlaceBomb_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_PlaceBomb_Ru'))
	//MessageData.Add((Key="80A", MsgIndex=MEMBERCOMMAND_OnPlaceBomb, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_OnPlacingBomb_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_OnPlacingBomb_Ru'))
	//MessageData.Add((Key="70D", MsgIndex=MEMBERCOMMAND_RemoveBomb, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_RemoveBomb_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_RemoveBomb_Ru'))
	//MessageData.Add((Key="80D", MsgIndex=MEMBERCOMMAND_OnRemoveBomb, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_OnRemovingBomb_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_OnRemovingBomb_Ru'))
	//
	//MessageData.Add((Key="71A", MsgIndex=MEMBERCOMMAND_TakeArea, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_TakeArea_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_TakeArea_Ru'))
	//MessageData.Add((Key="81A", MsgIndex=MEMBERCOMMAND_AreaUnderAttack, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_AreaUnderAttack_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_AreaUnderAttack_Ru'))
	//MessageData.Add((Key="71D", MsgIndex=MEMBERCOMMAND_TakeArea_Dup, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_TakeArea_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_TakeArea_Ru'))
	//MessageData.Add((Key="81D", MsgIndex=MEMBERCOMMAND_AreaUnderAttack_Dup, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_AreaUnderAttack_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_AreaUnderAttack_Ru'))
	//MessageData.Add((Key="72A", MsgIndex=MEMBERCOMMAND_OnConvoyingObject, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_OnConvoyingObject_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_OnConvoyingObject_Ru'))
	//MessageData.Add((Key="82A", MsgIndex=MEMBERCOMMAND_ConvoyObject, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_ConvoyObject_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_ConvoyObject_Ru'))
	//MessageData.Add((Key="72D", MsgIndex=MEMBERCOMMAND_ObjectFound, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_ObjectFound_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_ObjectFound_Ru'))
	//MessageData.Add((Key="82D", MsgIndex=MEMBEROMMANND_StopObject, SoundCue[0]=SoundCue'avaGameVoices.LevelVoices.LV_StopObject_De',SoundCue[1]=SoundCue'avaGameVoices.LevelVoices.LV_StopObject_Ru'))
	//
	//// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
	//MessageData.Sort(Key)
}