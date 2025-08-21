class avaMissionNavPoint extends NavigationPoint
	native
	placeable;

var(Radar)	int			IconCode;
var(Radar)	bool		bShowDistance;
var(Radar)	ETeamType	TeamIndex;
var(Radar)	int			NameIndex;			//	avaLocalizedMessage 의 SpecialPlace 에 설정된 Index 입니다.var
var(Radar)	bool		bShowRadar;
var(Radar)	bool		bOnlyLeader;		//	Squad Leader 에게만 보이는 Point 이다...
var(Radar)	int			HUDIconCode;		//	쌍안경으로 볼 경우에 HUD 상에 표시되는 Icon Code 이다...

// @deprecated - avaHUD.PostBeginPlay()에서 Radar에 추가한다.
// function PostBeginPlay();

// Navigation Point를 원하는 Volume에 붙이면(Based) 같이 움직여야한다.
// 재확인 필요. (Editor에서 Base로 연결하면 같이 움직이지 않음)

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