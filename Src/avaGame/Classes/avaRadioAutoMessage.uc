class avaRadioAutoMessage extends avaLocalMessage;

var localized string RadioAuto[12];

enum EAutoMessageType
{
	AUTOMESSAGE_Grenade,					// 1
	AUTOMESSAGE_Reloading,					// 2
	AUTOMESSAGE_Sniper,						// 3
	AUTOMESSAGE_ManDown,					// 4
	AUTOMESSAGE_FireInTheHole,				// 5
	AUTOMESSAGE_ImHit,						// 6
	AUTOMESSAGE_EnemyDown,					// 7	
	AUTOMESSAGE_LDDown,						// 8
	AUTOMESSAGE_ReconRequest,				// 9
	AUTOMESSAGE_ReconAccept,				// 10	
	AUTOMESSAGE_ChangeLD,					// 11	분대장이 변경되었음	
	AUTOMESSAGE_SelectTargetPoint,			// 12	분대장이 쌍안경을 이용해서 위치 지정	[목표지점으로 이동하라!]
};

static function SoundCue	GetSoundCue( int Index, avaPawn SenderPawn, PlayerController SelfPC )
{
	local int StressType;
	local class< avaLocalizedTeamPack >	LocalizedTeamPack;
	if ( SenderPawn == None )	return None;
	LocalizedTeamPack	= avaGameReplicationInfo( SelfPC.WorldInfo.GRI ).GetLocalizedTeamPack( avaPlayerController( SelfPC ), SenderPawn.GetTeamNum() );
	StressType			= SenderPawn.GetStressLevel();
	return LocalizedTeamPack.default.AutoSound[Index].Snd[StressType];
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

	`log( "avaRadioAutoMessage.ClientReceive" @index );


	if( index < 0 )
		return;	

	
	SenderPRI = avaPlayerReplicationInfo(RelatedPRI_1);
	SenderPawn = SenderPRI.GetAvaPawn();
	KeyIndex = GetMessageIndexOfKey( Key );
	DisplayMessage( SenderPawn, SenderPRI, P, default.RadioAuto[KeyIndex], Index );
}

defaultproperties
{		
	

	Begin Object class=avaLocalSound name=Message1
		Key="0"
		MsgIndex=AUTOMESSAGE_Grenade
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message2
		Key="1"
		MsgIndex=AUTOMESSAGE_Reloading
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message3
		Key="2"
		MsgIndex=AUTOMESSAGE_Sniper
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message4
		Key="3"
		MsgIndex=AUTOMESSAGE_ManDown
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message5
		Key="4"
		MsgIndex=AUTOMESSAGE_FireInTheHole
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message6
		Key="5"
		MsgIndex=AUTOMESSAGE_ImHit
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message7
		Key="6"
		MsgIndex=AUTOMESSAGE_EnemyDown
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
		bAddIndicator=false
	End Object

	Begin Object class=avaLocalSound name=Message8
		Key="7"
		MsgIndex=AUTOMESSAGE_LDDown
		MsgMain=QuickChat
		Shout(0)=1
		Shout(1)=1
		Shout(2)=1
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message9
		Key="8"
		MsgIndex=AUTOMESSAGE_ReconRequest
		MsgMain=QuickChat
		Shout(0)=0
		Shout(1)=0
		Shout(2)=0
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message10
		Key="9"
		MsgIndex=AUTOMESSAGE_ReconAccept
		MsgMain=CommandCenter
		Shout(0)=0
		Shout(1)=0
		Shout(2)=0
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message11
		Key="10"
		MsgIndex=AUTOMESSAGE_ChangeLD
		MsgMain=QuickChat
		Shout(0)=0
		Shout(1)=0
		Shout(2)=0
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
	End Object

	Begin Object class=avaLocalSound name=Message12
		Key="11"
		MsgIndex=AUTOMESSAGE_SelectTargetPoint
		MsgMain=QuickChat
		Shout(0)=0
		Shout(1)=0
		Shout(2)=0
		MsgTeamOnly(0)=1
		MsgTeamOnly(1)=1
		MsgTeamOnly(2)=1
	End Object

	MsgData(0)	=	Message1
	MsgData(1)	=	Message2
	MsgData(2)	=	Message3
	MsgData(3)	=	Message4
	MsgData(4)	=	Message5
	MsgData(5)	=	Message6
	MsgData(6)	=	Message7
	MsgData(7)	=	Message8
	MsgData(8)	=	Message9
	MsgData(9)	=	Message10
	MsgData(10)	=	Message11
	MsgData(11)	=	Message12

	//MsgData.Sort(Key)


	//MessageData.Add(( Key="0", MsgIndex=AUTOMESSAGE_Grenade, SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_Grenade_DE', SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_Grenade_RU')  )
	//MessageData.Add(( Key="1", MsgIndex=AUTOMESSAGE_Reloading,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_Reloading_DE',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_Reloading_RU'))
	//MessageData.Add(( Key="2", MsgIndex=AUTOMESSAGE_Sniper,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_Sniper_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_Sniper_Ru'))
	//MessageData.Add(( Key="3", MsgIndex=AUTOMESSAGE_ManDown,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_ManDown_DE',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_ManDown_RU'))
	//MessageData.Add(( Key="4", MsgIndex=AUTOMESSAGE_FireInTheHole,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_FireInTHeHole_DE',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_FireInTHeHole_RU'))
	//MessageData.Add(( Key="5", MsgIndex=AUTOMESSAGE_ImHit,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_ImHit_DE',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_ImHit_RU'))
	//MessageData.Add(( Key="6", MsgIndex=AUTOMESSAGE_EnemyDown,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_EnemyDown_DE',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_EnemyDown_RU'))
	//MessageData.Add(( Key="7", MsgIndex=AUTOMESSAGE_SetSignal))
	//MessageData.Add(( Key="8", MsgIndex=AUTOMESSAGE_SetTarget))

	//// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
	//MessageData.Sort(Key)
}