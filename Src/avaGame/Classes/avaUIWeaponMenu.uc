class avaUIWeaponMenu extends UIObject native;

enum EUIWeaponMenu_LinkType
{
	WEAPONLINK_Rows,
	WEAPONLINK_Columns,
};

var() float Padding[2]<ToolTip=Padding is used as HorizontalPadding for All LinkType>;
var() float RowSpacing<ToolTip=RowSpacing is used as ColumnSpacing for WEAPONLINK_Columns>;
var() float CategoryIndexXL<ToolTip=CategoryIndexXL is used as CategoryIndexYL for WEAPONLINK_Columns>;
var(Font) Font WeaponIconFont, WeaponNameFont, CategoryIndexFont;
var(Color) color SelectedColor[4], DrawColor, ShadowColor;
var(Background) TextureCoordinates BackgroundCoordinates;
var(Background) float BackgroundLeft, BackgroundRight;
var(Background) float SelectedBackgroundOffset[2];
var(Background) bool bShouldTileBackground;
var(Background) Surface Background;
var() float NameOffset[2];
var() float MinSizeX;
var(Test) string TestIcon, TestWeaponName;
var() float IconOffset;
var(Fade) float FadeOutTime;
var(Fade) float MaxOpacity;
var(Fade) float ItemFadeInTime, ItemFadeOutTime;
var(Fade) float MenuActiveTime;

var() EUIWeaponMenu_LinkType			WeaponLinkType;


/** 같은 무기 항목을 컬럼으로 배열할때 특성화된 구현 */
var(ColumnSpecified) Vector2D		WeaponItemExtentLarge, WeaponItemExtentSmall;
var(ColumnSpecified) Font			WeaponIconFontLarge, WeaponIconFontSmall;
var(ColumnSpecified) Font			WeaponNameFontLarge, WeaponNameFontSmall;
var(ColumnSpecified) Color			BackgroundColor[2]<ToolTip=/TeamColor EU:0, NRF:1/>;
var(ColumnSpecified) float			BackgroundOpacity;
var(ColumnSpecified) Vector2D		NumberOffset <ToolTip=adjust a position of rightbottom number>;
var(ColumnSpecified) Vector2D		NameShadowOffset;
var(ColumnSpecified) Color			NameShadowColor;
var(ColumnSpecified) Surface		SelectBackgroundImage;
var(ColumnSpecified) TextureCoordinates SelectBackgroundImageCoord;

var	private	Texture2D				DefaultWhiteTexture;

struct native WeaponDrawInfo
{
	var int			Group;
	var float		XL;
	var float		Offset;
	var byte		Icon;
	var private native pointer Name{TCHAR};
	var float		Alpha;
	var int			MaintenanceRate;
	var avaWeapon	Weapon;

	structcpptext
	{
		FWeaponDrawInfo() : Group(0), XL(0.f), Offset(0.f), Icon(0), Name(NULL), Alpha(0.f), MaintenanceRate(0), Weapon(NULL) {}
	}
};

var transient array<WeaponDrawInfo> Weapons;
var transient float		RowAlpha[16];
var transient int		RowCount[16];
var transient Weapon	PreviousWeapon;
var transient float		MenuTTL;
var transient avaPawn	PreviousPawn;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
	
	void Render_Row( FCanvas* Canvas, FLOAT X, FLOAT Y, INT CategoryIndex );
	void Render_Column( FCanvas* Canvas, FLOAT X, FLOAT Y, INT CategoryIndex, INT OwnerTeam, UBOOL bSelectedCategory );

	void Render_Background( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT TXL, FLOAT X0, FLOAT XL, FLOAT Alpha );
	void Render_Background_Column( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT TYL, FLOAT Y0, FLOAT YL, FLOAT Alpha );

	void UpdateWeaponInfo( FWeaponDrawInfo& );

	void UpdateData();
}

defaultproperties
{	
	RowSpacing = 2	
	CategoryIndexXL = 16
	CategoryIndexFont = TrueTypeFont'GameFonts.Tiny11'
	WeaponIconFont = TrueTypeFont'GameFonts.UI.UILarge'
	WeaponNameFont = TrueTypeFont'GameFonts.Tiny11'	
	ShadowColor = (R=0,G=0,B=0,A=255)
	TestIcon = "d"
	TestWeaponName = "Test weapon"
	MaxOpacity=1.0
	FadeOutTime=0.2
	ItemFadeInTime=0.1
	ItemFadeOutTime=0.1
	MenuActiveTime=2.0

	BackgroundColor(0)=(R=255,G=255,B=0,A=255)
	BackgroundColor(1)=(R=0,G=255,B=255,A=255)
	BackgroundOpacity=1.0
	DefaultWhiteTexture=Texture2D'EngineResources.WhiteSquareTexture'
	WeaponLinkType = WEAPONLINK_Columns;
	WeaponItemExtentLarge=(X=75,Y=50)
	WeaponItemExtentSmall=(X=60,Y=40)
	NameShadowColor=(A=255,R=255,G=255,B=255)

	SelectedColor(0)=(A=255,R=255,G=255,B=255)
	SelectedColor(1)=(A=255,R=255,G=255,B=0)
	SelectedColor(2)=(A=255,R=255,G=0,B=0)
	SelectedColor(3)=(A=255,R=128,G=128,B=128)
	
}