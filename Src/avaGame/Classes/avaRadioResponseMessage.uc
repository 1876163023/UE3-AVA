class avaRadioResponseMessage extends avaLocalMessage
	config(game);

enum RadioReponseMessageType
{
	RADIORESPONSE_Roger,
	RADIORESPONSE_Negative,
	RADIORESPONSE_OnMyWay,
	RADIORESPONSE_Sorry,
	RADIORESPONSE_NiceShot,
	RADIORESPONSE_Thanks,
	RADIORESPONSE_Taunt,
	RADIORESPONSE_TauntKill
};

var localized string RadioResponse[7];
var localized string RadioResponse_Label_BroadCast;
var const Color RadioResponseColor;

var localized string TauntMsg[3];
var localized string AfterKillTauntMsg[3];


static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return default.RadioResponseColor;
}

static function SoundCue	GetSoundCue( int Index, avaPawn SenderPawn, PlayerController SelfPC )
{
	local int StressType;
	local class< avaLocalizedTeamPack >	LocalizedTeamPack;
	if ( SenderPawn == None )	return None;
	LocalizedTeamPack	= avaGameReplicationInfo( SelfPC.WorldInfo.GRI ).GetLocalizedTeamPack( avaPlayerController( SelfPC ), SenderPawn.GetTeamNum() );
	StressType			= SenderPawn.GetStressLevel();
	return LocalizedTeamPack.default.ResponseSound[Index].Snd[StressType];
}

static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local string					Key;
	local int						index;
	local avaPawn					Sender;
	local int						StressLvl;
	local string					Message;
	local avaPlayerReplicationInfo	SenderPRI;

	Key = Switch$"";
	index = GetMessageIndexOfKey(Key);
	SenderPRI = avaPlayerReplicationInfo(RelatedPRI_1);
	Sender = avaPlayerReplicationInfo( RelatedPRI_1).GetAvaPawn();
	if ( index == RADIORESPONSE_Taunt )
	{
		StressLvl = Sender.GetStressLevel();
		if ( P.WorldInfo.TimeSeconds - Sender.LastKillTime < 5.0 )	
		{
			Message = default.AfterKillTauntMsg[StressLvl];
			index	= RADIORESPONSE_TauntKill;
		}
		else														
			Message = default.TauntMsg[StressLvl];
	}
	else
	{
		Message = default.RadioResponse[ index ];
	}
	DisplayMessage( Sender, SenderPRI, P, Message, index );
}


defaultproperties
{
	RadioResponseColor=(R=170,G=170,B=170,A=220)

	

	Begin Object class=avaLocalSound name=Message1
		Key="1"
		MsgIndex=RADIORESPONSE_Roger
		AnimIndex=1
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message2
		Key="2"
		AnimIndex=7
		MsgIndex=RADIORESPONSE_Negative
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message3
		Key="3"
		MsgIndex=RADIORESPONSE_OnMyWay
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message4
		Key="4"
		AnimIndex=7
		MsgIndex=RADIORESPONSE_Sorry
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message5
		Key="5"
		AnimIndex=8
		MsgIndex=RADIORESPONSE_NiceShot
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message6
		Key="6"
		AnimIndex=8
		MsgIndex=RADIORESPONSE_Thanks
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message7
		Key="7"
		MsgIndex=RADIORESPONSE_Taunt
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=0	
		MsgTeamOnly(1)=0	
		MsgTeamOnly(2)=0	
		ShoutDistance	= 2500.0
	
	End Object

	Begin Object class=avaLocalSound name=Message8
		Key="8"
		MsgIndex=RADIORESPONSE_TauntKill
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=0	
		MsgTeamOnly(1)=0	
		MsgTeamOnly(2)=0	
		ShoutDistance	= 2500.0
	
	End Object


	MsgData(0)	=	Message1
	MsgData(1)	=	Message2
	MsgData(2)	=	Message3
	MsgData(3)	=	Message4
	MsgData(4)	=	Message5
	MsgData(5)	=	Message6
	MsgData(6)	=	Message7
	MsgData(7)	=	Message8

	//MessageData.Add(( Key="1", MsgIndex=RADIORESPONSE_Roger, AnimIndex=1, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Yes_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Yes_Ru'))
	//MessageData.Add(( Key="2", MsgIndex=RADIORESPONSE_Negative, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_No_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_No_Ru'))
	//MessageData.Add(( Key="3", MsgIndex=RADIORESPONSE_OnMyWay, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_OnMyWay_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_OnMyWay_Ru'))
	//MessageData.Add(( Key="4", MsgIndex=RADIORESPONSE_Sorry, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Sorry_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Sorry_Ru'))
	//MessageData.Add(( Key="5", MsgIndex=RADIORESPONSE_NiceShot, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Great_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Great_Ru'))
	//MessageData.Add(( Key="6", MsgIndex=RADIORESPONSE_Thanks, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Thanks_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Thanks_Ru'))
	//MessageData.Add(( Key="7", MsgIndex=RADIORESPONSE_Taunt1, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Taunt1_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Taunt1_Ru'))
	//MessageData.Add(( Key="8", MsgIndex=RADIORESPONSE_Taunt2, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Taunt2_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Taunt2_Ru'))
	//MessageData.Add(( Key="9", MsgIndex=RADIORESPONSE_Taunt3, SoundCue[0]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Taunt3_De',SoundCue[1]=SoundCue'avaQuickVoices.X_PersonalVoices.X_Taunt3_Ru'))

	//// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
	//MessageData.Sort(Key)
}