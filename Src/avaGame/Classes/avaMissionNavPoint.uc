class avaMissionNavPoint extends NavigationPoint
	native
	placeable;

var(Radar)	int			IconCode;
var(Radar)	bool		bShowDistance;
var(Radar)	ETeamType	TeamIndex;
var(Radar)	int			NameIndex;			//	avaLocalizedMessage �� SpecialPlace �� ������ Index �Դϴ�.var
var(Radar)	bool		bShowRadar;
var(Radar)	bool		bOnlyLeader;		//	Squad Leader ���Ը� ���̴� Point �̴�...
var(Radar)	int			HUDIconCode;		//	�־Ȱ����� �� ��쿡 HUD �� ǥ�õǴ� Icon Code �̴�...

// @deprecated - avaHUD.PostBeginPlay()���� Radar�� �߰��Ѵ�.
// function PostBeginPlay();

// Navigation Point�� ���ϴ� Volume�� ���̸�(Based) ���� ���������Ѵ�.
// ��Ȯ�� �ʿ�. (Editor���� Base�� �����ϸ� ���� �������� ����)

defaultproperties
{
	Begin Object Name=Sprite
		Sprite=Texture2D'EngineResources.LockLocation'
		Scale=5.0
	End Object

	NameIndex = -1
	bShowRadar = true
	bStatic=false
}