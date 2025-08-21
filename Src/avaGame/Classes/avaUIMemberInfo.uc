class avaUIMemberInfo extends UIObject native 
	dependson(avaUISimpleText)
	dependson(avaUIProgressBar);

// 개인의 정보를 표시하기 위한 variable...
// 4. Slot Number
// 8. Current Weapon Info
// 9. Ammo Count & Reload Count

// General Information
var(general)	int				TeamIndex;
var(general)	float			ColumnSpacing;					

// Background Information...
struct native BackGroundInfo
{
	var()	Surface					BackgorundImage;
	var()	TextureCoordinates		BackgroundCoordinate;
	var()	Color					BackgroundColor;

	structdefaultproperties
	{
		BackgorundImage	=	Texture2D'EngineResources.WhiteSquareTexture'
		BackgroundColor	=	(R=255,G=255,B=255,A=255)
	}
};

var()	BackGroundInfo								BGInfo[3];			//	0 : Normal 1 : Disabled 2 : Selected

// Player Name Information
var(PlayerName)	Font								PlayerNameFont;
var(PlayerName) Vector2D							PlayerNameOffset;
var(PlayerName)	Color								PlayerNameColor;
var(PlayerName) Color								LeaderPlayerNameColor;
var(PlayerName) AVAUIAlign							PlayerNameHorizontalAlign;
var(PlayerName) AVAUIAlign							PlayerNameVerticalAlign;
var(PlayerName)	bool								bUsePlayerNameShadow;
var(PlayerName) Color								PlayerNameShadowColor;
var(PlayerName) Vector2D							PlayerNameShadowOffset;

struct native	FadeInfoArray
{
	var	array<GaugeFadeInfo>	FadeInfos;	
};

// Health Gauge Information
var(HealthBar)	bool								bDrawHealthBar;
var(HealthBar)	instanced	avaUICustomProgressBar	HealthBar;	
var(HealthBar)	Vector2D							HealthBarOffset;
var(HealthBar)	Vector2D							HealthBarSize;
var				FadeInfoArray						HealthBarFadeInfos[16];

// Armor Gauge Information
var(ArmorBar)	bool								bDrawArmorBar;
var(ArmorBar)	instanced	avaUICustomProgressBar	ArmorBar;
var(ArmorBar)	Vector2D							ArmorBarOffset;
var(ArmorBar)	Vector2D							ArmorBarSize;
var				FadeInfoArray						ArmorBarFadeInfos[16];

// Weapon Information
var(WeaponInfo) bool								bDrawWeaponInfo;
var(WeaponInfo) Font								WeaponInfoFont;
var(WeaponInfo) Vector2D							WeaponInfoOffset;
var(WeaponInfo)	Color								WeaponInfoColor;
var(WeaponInfo) AVAUIAlign							WeaponInfoHorizontalAlign;
var(WeaponInfo) AVAUIAlign							WeaponInfoVerticalAlign;
var(WeaponInfo)	bool								bUseWeaponInfoShadow;
var(WeaponInfo) Color								WeaponInfoShadowColor;
var(WeaponInfo) Vector2D							WeaponInfoShadowOffset;

var(ClassInfo)	bool								bDrawClassInfo;
var(ClassInfo)	Vector2D							ClassInfoOffset;
var(ClassInfo)	Vector2D							ClassInfoSize;
var(ClassInfo)	array< int >						ClassIconIndex;

var(UseActionInfo)	bool							bDrawUseAction;
var(UseActionInfo)	Vector2D						UseActionIconOffset;
var(UseActionInfo)	Vector2D						UseActionIconSize;
var(UseActionInfo)	array< int >					UseActionIconIndex;

var(Status)		Font								StatusFont;
var(Status)		Vector2D							StatusOffset;
var(Status)		Color								StatusColor;


// SlotNum Information
var(SlotNum)	Vector2D							SlotNumOffset;
var(SlotNum)	Vector2D							SlotNumSize;
var(SlotNum)	AVAUIAlign							SlotNumHorizontalAlign;
var(SlotNum)	AVAUIAlign							SlotNumVerticalAlign;

// for test field
var(test)	int										TestCnt;
var(test)	int										TestSelectedCol;
var(test)	int										TestDisabledCol;
var(test)	string									TestPlayerName;
var(test)	string									TestWeaponCode;
var(test)	int										TestClassNum;
var(test)	int										TestUseActionNum;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
	virtual void UpdateHealthBarFadeInfo( int nSlot, float InRatio, float FadeOutTime );
	virtual void UpdateArmorBarFadeInfo( int nSlot, float InRatio, float FadeOutTime );

	void	DrawBG( FCanvas* Canvas, int BGType, float X, float Y, float XL, float YL );
	void	DrawPlayerName( FCanvas* Canvas,const BOOL bSquadLeader, const INT Level, const TCHAR* PlayerName, float X, float Y, float XL, float YL, float AddAlpha );
	void	DrawHealthBar( FCanvas* Canvas, int nSlot, float Ratio, float X, float Y, float XL, float YL );
	void	DrawArmorBar( FCanvas* Canvas, int nSlot, float Ratio, float X, float Y, float XL, float YL );
	void	DrawWeaponInfo( FCanvas* Canvas, const TCHAR* WeaponInfo, float X, float Y, float XL, float YL, float AddAlpha, bool bDead );

	void	DrawClassInfo( FCanvas* Canvas, const int ClassNum, float X, float Y, float XL, float YL , float AddAlpha);
	void	DrawUseActionInfo( FCanvas* Canvas, const int UseAction, float X, float Y, float XL, float YL , float AddAlpha);
}

defaultproperties
{
	TestCnt				=	8
	TestSelectedCol		=	1
	TestDisabledCol		=	2
	TestPlayerName		=	"ABCDEFGHIJ"
	TestWeaponCode		=	"V"

	PlayerNameFont				=	TrueTypeFont'GameFonts.Tiny11'	
	PlayerNameHorizontalAlign	=	AVAUIALIGN_LeftOrTop
	PlayerNameVerticalAlign		=	AVAUIALIGN_LeftOrTop

	WeaponInfoFont				=	TrueTypeFont'GameFonts.Tiny11'	
	WeaponInfoHorizontalAlign	=	AVAUIALIGN_LeftOrTop
	WeaponInfoVerticalAlign		=	AVAUIALIGN_LeftOrTop

	StatusFont			=	TrueTypeFont'GameFonts.Tiny11'	

	bDrawHealthBar				=	true
	Begin Object class=avaUICustomProgressBar Name=iHealthBar
	End Object
	HealthBar	= iHealthBar

	bDrawArmorBar				=	false
	Begin Object class=avaUICustomProgressBar Name=iArmorBar
	End Object
	ArmorBar	= iArmorBar

	SlotNumOffset		=	(x=0,y=0)
	SlotNumSize			=	(x=19,y=19)

	ClassIconIndex(0)	=	24
	ClassIconIndex(1)	=	25
	ClassIconIndex(2)	=	26
	ClassInfoSize		=	(x=19,y=19)
}