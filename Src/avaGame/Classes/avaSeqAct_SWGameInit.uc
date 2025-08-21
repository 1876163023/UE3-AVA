/// 이름과는 다르게 "SeqAct"를 하지는 않는다.
/// 게임의 단순히 세팅을 위한 것임.

class avaSeqAct_SWGameInit extends SequenceAction;

`include(avaGame/avaGame.uci)

/// 증원이 가능한가.
var() bool			bReinforcement;

var() int			ReInforcementFreq[`MAX_TEAM];	// 0 은 EU 증원 Time, 1 은 NRF 증원 Time 

/// 한판 진행 시각.
var() int			RoundTimeLimit;
/// 연습판 진행 시각. 0 이면 연습판 없음.
var() int			PracticeTimeLimit;

/// 한 사람의 목숨이 몇개인가?
/// bReinforcement 가 켜지거나 꺼지거나 유효함.
var() int			MaxLives;

var() EMissionType	MissionType;

var() int			SpawnAllowTime;

var() ETeamType		AttackTeam;

var() float			BaseScore;			// 맵별 기준 Score

var() int			DogTagPackCnt;

var() array<SceneStateDataType> SceneStateData <ToolTip=currently availables : DefaultHUD/Spectator/Dead>;

var() int			MissionHelpIdx[2];


defaultproperties
{
	ObjCategory  = "Game Setting"
	ObjName      = "SWGame"
	bCallHandler = false

	bReinforcement			=	true
	ReInforcementFreq[0]	=	27
	ReInforcementFreq[1]	=	27


	RoundTimeLimit    = 90
	PracticeTimeLimit = 30

	MaxLives = 1

	InputLinks.Empty
	OutputLinks.Empty
	VariableLinks.Empty

	MissionHelpIdx[0]		=	-1
	MissionHelpIdx[1]		=	-1
}
