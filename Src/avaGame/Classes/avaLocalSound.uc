class avaLocalSound extends Object;

var string			Key;
var int				MsgIndex;
var int				AnimIndex;

var int				Shout[3];			//	Stress Level �� Shout���� 
var int				MsgTeamOnly[3];		//	1 �̸� Message �� Team ���Ը� ���δ�...
var float			ShoutDistance;		//

var	bool			bAddIndicator;		// Message �� ǥ�õɶ� Indicator �� ���� ǥ�õȴ�...
var name			MsgMain;			// Message �� ��ü...

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