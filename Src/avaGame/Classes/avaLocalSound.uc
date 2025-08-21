class avaLocalSound extends Object;

var string			Key;
var int				MsgIndex;
var int				AnimIndex;

var int				Shout[3];			//	Stress Level 별 Shout유무 
var int				MsgTeamOnly[3];		//	1 이면 Message 는 Team 에게만 보인다...
var float			ShoutDistance;		//

var	bool			bAddIndicator;		// Message 가 표시될때 Indicator 가 같이 표시된다...
var name			MsgMain;			// Message 의 주체...

defaultproperties
{
	AnimIndex		= -1
	ShoutDistance	= 1067.0
	MsgMain			= QuickChat
	bAddIndicator	= true
	Shout(0)		= 0
	Shout(1)		= 0
	Shout(2)		= 0
	MsgTeamOnly(0)	= 1
	MsgTeamOnly(1)	= 1
	MsgTeamOnly(2)	= 1
}