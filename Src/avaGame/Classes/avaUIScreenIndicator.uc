class avaUIScreenIndicator extends UIObject
	native;

cpptext
{
	void Render_Widget( FCanvas* Canvas );
}

//enum IndicatorFaceType
//{
//	INDICATORFACE_TOP,
//	INDICATORFACE_RIGHT,
//	INDICATORFACE_BOTTOM,
//	INDICATORFACE_LEFT,
//};


struct native IndicatorIconInfo
{
	var() Color							TextColor;
	var() Color							IconColor;
	var() array<Surface>				Icon;
	var() array<TextureCoordinates>		Coord;
	var() Vector2D						DrawExtent;
	var() bool							bIgnoreExtent;

	structdefaultproperties
	{
		TextColor=(R=255,G=255,B=255,A=255)
		IconColor=(R=255,G=255,B=255,A=255)
		Icon(ISA_Top)=Texture2D'EngineResources.WhiteSquareTexture'			//Top
		Icon(ISA_Right)=Texture2D'EngineResources.WhiteSquareTexture'			//Right
		Icon(ISA_Bottom)=Texture2D'EngineResources.WhiteSquareTexture'			//Bottom
		Icon(ISA_Left)=Texture2D'EngineResources.WhiteSquareTexture'			//Left
		Icon(ISA_Center)=Texture2D'EngineResources.WhiteSquareTexture'			// Center
		Icon(ISA_None)=Texture2D'EngineResources.WhiteSquareTexture'			// None
		
		Coord(ISA_Top)=()			//Top
		Coord(ISA_Right)=()			//Right
		Coord(ISA_Bottom)=()			//Bottom
		Coord(ISA_Left)=()			//Left
		Coord(ISA_Center)=()			// Center
		Coord(ISA_None)=()			// None
		
		DrawExtent=(X=10,Y=10)
		bIgnoreExtent=true
	}
};

var() array<IndicatorIconInfo>			IndicatorIcons;
var() float								BlinkPeriod;		//second
var() bool								bShowDistance;
var() bool								bShowName;
var() string							WaypointName;
var() string							MissionObjectName;
var() Font								Font;

var(IndexInfo) const int				Indicator_Waypoint1;
var(IndexInfo) const int				Indicator_Waypoint2;
var(IndexInfo) const int				Indicator_QuickChatAll;
var(IndexInfo) const int				Indicator_QuickChatTeam;
var(IndexInfo) const int				Indicator_MissionObject;

var(IndexInfo) const int				ScreenArea_Top;
var(IndexInfo) const int				ScreenArea_Bottom;
var(IndexInfo) const int				ScreenArea_Left;
var(IndexInfo) const int				ScreenArea_Right;
var(IndexInfo) const int				ScreenArea_Center;
var(IndexInfo) const int				ScreenArea_None;

defaultproperties
{
	IndicatorIcons(INDICATORTYPE_Waypoint1)=(TextColor=(R=0,G=0,B=255,A=255))
	IndicatorIcons(INDICATORTYPE_Waypoint2)=(TextColor=(R=255,G=255,B=0,A=255))
	IndicatorIcons(INDICATORTYPE_QuickChat_All)=(TextColor=(R=255,G=0,B=255,A=255))
	IndicatorIcons(INDICATORTYPE_QuickChat_Team)=(TextColor=(R=0,G=255,B=0,A=255))
	IndicatorIcons(INDICATORTYPE_MissionObject)=(TextColor=(R=255,G=0,B=0,A=0))

	bShowName=true
	bShowDistance=true
	WaypointName="WP"
	MissionObjectName="MO"
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'
	BlinkPeriod=0.7

	Indicator_Waypoint1=INDICATORTYPE_Waypoint1
	Indicator_Waypoint2=INDICATORTYPE_Waypoint2
	Indicator_QuickChatAll=INDICATORTYPE_QuickChat_All
	Indicator_QuickChatTeam=INDICATORTYPE_QUickChat_Team
	Indicator_MissionObject=INDICATORTYPE_MissionObject

	ScreenArea_Top=ISA_Top
	ScreenArea_Bottom=ISA_Bottom
	ScreenArea_Left=ISA_Left
	ScreenArea_Right=ISA_Right
	ScreenArea_Center=ISA_Center
	ScreenArea_None=ISA_None
}