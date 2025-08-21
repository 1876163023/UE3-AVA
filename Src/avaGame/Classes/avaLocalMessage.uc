/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaLocalMessage extends LocalMessage
	abstract;

/* Play Sound & Animation for Complex Messages, and its helper */

struct MessageDataType
{
	var string			Key;
	var int				MsgIndex;
	var SoundCue		SoundCue[4];	// 0~1 : International		2~3 : Local
	var int				AnimIndex;

	structdefaultproperties
	{
		AnimIndex=-1
	}
};

//var const array<MessageDataType> MessageData;

var	const array<avaLocalSound>		MsgData;

static function ClientReceive( 
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local string MessageString;
	Local avaHUD HUD;

	MessageString = static.GetString(Switch, (RelatedPRI_1 == P.PlayerReplicationInfo), RelatedPRI_1, RelatedPRI_2, OptionalObject);
	
	HUD = avaHUD(P.MyHUD);
	// Commented Code from SuperClass(LocalMessage)
	//
	//if ( P.myHud != None )
	//	P.myHUD.LocalizedMessage( 
	//		Default.Class, 
	//		RelatedPRI_1,
	//		MessageString,
	//		Switch, 
	//		static.GetPos(Switch),
	//		static.GetLifeTime(Switch),
	//		static.GetFontSize(Switch, RelatedPRI_1, RelatedPRI_2, P.PlayerReplicationInfo),
	//		static.GetColor(Switch, RelatedPRI_1, RelatedPRI_2) );

	//if(IsConsoleMessage(Switch) && LocalPlayer(P.Player).ViewportClient != None)
	//	LocalPlayer(P.Player).ViewportClient.ViewportConsole.OutputText( MessageString );
	Assert(HUD != None);
	HUD.Message(RelatedPRI_1, MessageString, 'System');
}

static function SoundCue	GetSoundCue( int Index, avaPawn SenderPawn, PlayerController SelfPC );

static function PlaySoundByIndex(int index, avaPawn SenderPawn, PlayerController SelfPC, bool bShout )
{
	
	local AudioComponent	AC;
	local SoundCue			sc;
	
	if( index < 0 )
		return;

	sc = GetSoundCue( index, SenderPawn, SelfPC );

	`log( "avaLocalMessage.GetSoundCue" @sc );
	/* Playing Sound if it exists*/
	if( sc != None)
	{
		if ( bShout == FALSE )
		{
			AC = SelfPC.WorldInfo.CreateAudioComponent( sc, false, true,,,, AudioChannel_Voice );
			AC.bAllowSpatialization = false;
			AC.bAutoDestroy = true;
			AC.Play();	
		}
		else
		{
			avaPlayerController(SelfPC).PlaySoundEx( sc, 
													 SenderPawn, 
													 vect(0,0,0), 
													 true, 
													 false, 
													 AudioChannel_Voice );
		}
	}
}


// 코드 중간중간의 Default.PlayMessage 구문은 defaultproperties에서
// 미리 재생할 SoundCue를 추가하지 않으면 추가한 결과가 반영되지 않음을 의미
//static function Play(string Key, PlayerReplicationInfo SenderPRI, PlayerController SelfPC)
//{
//	local avaPawn			Sender;
//	local AudioComponent	AC;
//	local int				index;
//	local SoundCue			sc;
//
//	index = GetIndexOfKey(Key);
//	if( index < 0 )
//		return;
//
//	/*Playing Animation if it exists*/
//	if (SenderPRI != None )
//	{
//		Sender = avaPlayerReplicationInfo( SenderPRI).GetAvaPawn();
//		if ( Sender != None && default.MsgData[Index].AnimIndex != -1)
//			Sender.PlayQVCAnim( default.MsgData[Index].AnimIndex);
//	}
//
//	sc = GetSoundCue( index, Sender, SelfPC );
//	/* Playing Sound if it exists*/
//	if( sc != None)
//	{
//		`log( "avaLocalMessage.PlaySoundByIndex"  @sc );
//		AC = SelfPC.WorldInfo.CreateAudioComponent( sc, false, true,,,, AudioChannel_Voice );
//		AC.bAllowSpatialization = false;
//		AC.bAutoDestroy = true;
//		AC.Play();	
//	}
//}

static function DisplayMessage( avaPawn SenderPawn, avaPlayerReplicationInfo SenderPRI, PlayerController Listener, coerce string str, int Index, optional bool bInPlaySound = true )
{
	//	bAddIndicator
	local int						StressType;
	local avaHUD					avaHUD;
	local bool						bSameTeam;
	local bool						bPlaySound;
	local vector					ListenerLoc;

	StressType = SenderPawn.GetStressLevel();

	if ( avaPlayerController( Listener ).IsSameTeam( SenderPawn ) )	
		bSameTeam = true;

	avaHUD = avaHUD(Listener.myHUD);

	// Shout 인 경우에는 팀과 상관없이 Sound는 출력된다...
	// Shout 인 경우에는 팀 Check와 상관없이 거리가 멀면 Message 도 출력되지 않는다...
	// Shout 가 1이면 외치는 것이다.. Shout 거리보다 멀기 때문에 Pass
	if ( Listener.ViewTarget != None )	ListenerLoc = Listener.ViewTarget.Location;
	else								ListenerLoc = Listener.Location;

	/*Playing Animation if it exists*/
	if ( SenderPawn != None && default.MsgData[Index].AnimIndex != -1)
		SenderPawn.PlayQVCAnim( default.MsgData[Index].AnimIndex);

	if ( default.MsgData[Index].Shout[StressType] == 1 && VSize( ListenerLoc - SenderPawn.Location ) > default.MsgData[Index].ShoutDistance )	
		return;

	if ( ( default.MsgData[Index].Shout[StressType] == 1 || bSameTeam == true ) && bInPlaySound == true )
	{
		bPlaySound = true;
	}

	if ( bPlaySound )
	{
		PlaySoundByIndex(index, SenderPawn, Listener, bool( default.MsgData[Index].Shout[StressType] ) );
		if ( default.MsgData[Index].bAddIndicator )
			avaHUD.AddQuickChatIndicator( SenderPawn, !bool( default.MsgData[Index].MsgTeamOnly[StressType] ) );
	}

	// 같은 Team 이 아니기 때문에 Pass
	if ( default.MsgData[Index].MsgTeamOnly[StressType] == 1 && bSameTeam == false )
	{
		return;
	}


	avaHUD.Message( SenderPRI, str, default.MsgData[Index].MsgMain );
	
}

static function int GetIndexOfKey(string Key)
{
	local int i;

	for ( i = 0 ; i < default.MsgData.length ; ++ i )
	{
		if ( default.MsgData[i].Key == Key )
			return i;
	}
	return -1;
	//Local int First, Last, Mid;	

	//First = 0;
	//Last = default.MsgData.Length - 1;
	//
	//while( First <= Last )
	//{
	//	Mid = (First + Last) / 2;
	//	if( default.MsgData[Mid].Key == Key )
	//		return Mid;

	//	if( default.MsgData[Mid].Key > Key )
	//		Last = Mid - 1;
	//	else
	//		First = Mid + 1;		
	//}

	//return -1;
}

static function int GetMessageIndexOfKey(string Key)
{
	Local int i;
	i = GetIndexOfKey(Key);
	return i < 0 ? -1 : default.MsgData[i].MsgIndex;
}

// information about Announcements

static function byte AnnouncementLevel(byte MessageIndex)
{
	return 1;
}

static function bool IsRewardAnnouncement(byte MessageIndex)
{
	return false;
}

static function Name AnnouncementSound(byte MessageIndex)
{
	return '';
}

static function AddAnnouncement(avaAnnouncer Announcer, byte MessageIndex)
{
	local avaQueuedAnnouncement NewAnnouncement, A;

	NewAnnouncement = Announcer.Spawn(class'avaQueuedAnnouncement');
	NewAnnouncement.AnnouncementClass = Default.Class;
	NewAnnouncement.MessageIndex = MessageIndex;

	// default implementation is just add to end of queue
	if ( Announcer.Queue == None )
	{
		NewAnnouncement.nextAnnouncement = Announcer.Queue;
		Announcer.Queue = NewAnnouncement;
	}
	else
	{
		for ( A=Announcer.Queue; A!=None; A=A.nextAnnouncement )
		{
			if ( A.nextAnnouncement == None )
			{
				A.nextAnnouncement = NewAnnouncement;
				break;
			}
		}
	}
}

static function PrecacheGameAnnouncements(avaAnnouncer Announcer, class<GameInfo> GameClass);

