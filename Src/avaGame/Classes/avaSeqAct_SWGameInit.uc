/// �̸����� �ٸ��� "SeqAct"�� ������ �ʴ´�.
/// ������ �ܼ��� ������ ���� ����.

class avaSeqAct_SWGameInit extends SequenceAction;

`include(avaGame/avaGame.uci)

/// ������ �����Ѱ�.
var() bool			bReinforcement;

var() int			ReInforcementFreq[`MAX_TEAM];	// 0 �� EU ���� Time, 1 �� NRF ���� Time 

/// ���� ���� �ð�.
var() int			RoundTimeLimit;
/// ������ ���� �ð�. 0 �̸� ������ ����.
var() int			PracticeTimeLimit;

/// �� ����� ����� ��ΰ�?
/// bReinforcement �� �����ų� �����ų� ��ȿ��.
var() int			MaxLives;

var() EMissionType	MissionType;

var() int			SpawnAllowTime;

var() ETeamType		AttackTeam;

var() float			BaseScore;			// �ʺ� ���� Score

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
