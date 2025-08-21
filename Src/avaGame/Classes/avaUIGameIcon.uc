class avaUIGameIcon extends UIObject
	native;

enum avaUIGameIconType
{
	AVAUIGAMEICON_Armor,
	AVAUIGAMEICON_Helmet,
	AVAUIGAMEICON_Team,
	AVAUIGAMEICON_Armor_Heart,
};

struct native GameIconInfo
{
	var() Surface		Icon;
	var() Color			Color;
	var() TextureCoordinates Coord;
	var() bool			bIgnoreCoord;

	structdefaultproperties
	{
		Icon=Texture2D'EngineResources.WhiteSquareTexture'
		Color=(R=255,G=255,B=255,A=255)
		bIgnoreCoord=true
	}
};

var()		avaUIGameIconType			Binding;
var()		array<GameIconInfo>			GameIcons<ToolTip=/Armor Helmet Has 0 or Not 1 / Team EU 0 NRF 1 / Armor_Heart 0 Armor, 1 HeartOne, 2 HeartTwo />;
var(ArmorAndHearts)		float			DangerRate;
var(ArmorAndHearts)		Color			DangerColor[2];
var(ArmorAndHearts)		Color			NormalColor;
var(ArmorAndHearts)		float			DangerHeartBeat[2]<ToolTip=timeduration(ms) for each heart in danger. It has two heart image>;
var(ArmorAndHearts)		float			NormalHeartBeat[2]<ToolTip=timeduration(ms) for each heart in normal. it has two heart image>;

cpptext
{
	void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	DangerRate=0.3
	DangerColor(0)=(R=255,G=0,B=0,A=255)
	DangerColor(1)=(R=255,G=255,B=0,A=255)
	NormalColor=(R=255,G=255,B=255,A=255)

	DangerHeartBeat[0]=0.4
	DangerHeartBeat[1]=0.2

	NormalHeartBeat[0]=0.8
	NormalHeartBeat[1]=0.4

	GameIcons(0)=()
	GameIcons(1)=()
}