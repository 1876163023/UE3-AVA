class avaRadioCommandMessage extends avaLocalMessage
	config(game);

/* Sound Cue와 Localized String의 Index가 같도록 맞춰져있음. 
  따라서 메세지가 추가 또는 변경되면 사운드와 QuickChat 메세지가 어긋나므로 동기화 바람*/
enum ERadioCommandType
{
	RADIOCOMMAND_GO,
	RADIOCOMMAND_EnermySpotted,
	RADIOCOMMAND_StickTogether,
	RADIOCOMMAND_NeedReinforcements,
	RADIOCOMMAND_NeedCoveringFire,
	RADIOCOMMAND_AllClear,
	RADIOCOMMAND_Smoke,
	RADIOCOMMAND_NeedSniperHere,
};

var localized string RadioCommand[8];
var const Color RadioCommandColor;

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return default.RadioCommandColor;
}

static function SoundCue	GetSoundCue( int Index, avaPawn SenderPawn, PlayerController SelfPC )
{
	local int StressType;
	local class< avaLocalizedTeamPack >	LocalizedTeamPack;
	if ( SenderPawn == None )	return None;
	LocalizedTeamPack	= avaGameReplicationInfo( SelfPC.WorldInfo.GRI ).GetLocalizedTeamPack( avaPlayerController( SelfPC ), SenderPawn.GetTeamNum() );
	StressType			= SenderPawn.GetStressLevel();
	return LocalizedTeamPack.default.CommandSound[Index].Snd[StressType];
}

static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{

	local avaPlayerReplicationInfo	SenderPRI;
	local string					Key;
	local int						index;
	local avaPawn					SenderPawn;
	local int						KeyIndex;
	Key = Switch$"";
	index = GetIndexOfKey(Key);
	if( index < 0 )
		return;	
	SenderPRI = avaPlayerReplicationInfo(RelatedPRI_1);
	SenderPawn = SenderPRI.GetAvaPawn();
	KeyIndex = GetMessageIndexOfKey( Key );
	DisplayMessage( SenderPawn, SenderPRI, P, default.RadioCommand[KeyIndex], Index );
}

defaultproperties
{
	RadioCommandColor=(R=220,G=220,B=220,A=220)

	Begin Object class=avaLocalSound name=Message1
		Key="1"
		MsgIndex=RADIOCOMMAND_GO
		AnimIndex=0
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message2
		Key="2"
		MsgIndex=RADIOCOMMAND_EnermySpotted
		AnimIndex=4
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message3
		Key="3"
		MsgIndex=RADIOCOMMAND_StickTogether
		AnimIndex=6
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message4
		Key="4"
		AnimIndex=2
		MsgIndex=RADIOCOMMAND_NeedReinforcements
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message5
		Key="5"
		AnimIndex=5
		MsgIndex=RADIOCOMMAND_NeedCoveringFire
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message6
		Key="6"
		MsgIndex=RADIOCOMMAND_AllClear
		AnimIndex=3
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message7
		Key="7"
		AnimIndex=8
		MsgIndex=RADIOCOMMAND_Smoke
		Shout(2)=1
		
	End Object

	Begin Object class=avaLocalSound name=Message8
		Key="8"
		AnimIndex=5
		MsgIndex=RADIOCOMMAND_NeedSniperHere
		Shout(2)=1
		
	End Object

	MsgData(0)	=	Message1
	MsgData(1)	=	Message2
	MsgData(2)	=	Message3
	MsgData(3)	=	Message4
	MsgData(4)	=	Message5
	MsgData(5)	=	Message6
	MsgData(6)	=	Message7
	MsgData(7)	=	Message8



	//MessageData.Add(( Key="1", MsgIndex=RADIOCOMMAND_GO, AnimIndex=0, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_Gogogo_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_Gogogo_Ru'))
	//MessageData.Add(( Key="2", MsgIndex=RADIOCOMMAND_EnermySpotted, AnimIndex=4, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_EnemySpotted_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_EnemySpotted_Ru'))
	//MessageData.Add(( Key="3", MsgIndex=RADIOCOMMAND_StickTogether, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_StickTogether_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_StickTogether_Ru'))
	//MessageData.Add(( Key="4", MsgIndex=RADIOCOMMAND_NeedReinforcements, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_NeedReinforcement_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_NeedReinforcement_Ru'))
	//MessageData.Add(( Key="5", MsgIndex=RADIOCOMMAND_NeedCoveringFire, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_NeedCoveringFire_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_NeedCoveringFire_Ru'))
	//MessageData.Add(( Key="6", MsgIndex=RADIOCOMMAND_AllClear, AnimIndex=3, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_AllClear_De', SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_AllClear_Ru'))
	//MessageData.Add(( Key="7", MsgIndex=RADIOCOMMAND_Smoke, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_SmokeEm_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_SmokeEm_Ru'))
	//MessageData.Add(( Key="8", MsgIndex=RADIOCOMMAND_NeedSniperHere, SoundCue[0]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_NeedSniper_De',SoundCue[1]=SoundCue'avaQuickVoices.Z_TacticalVoices.Z_NeedSniper_Ru'))

	// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
	//MessageData.Sort(Key)
}