// [2006/10/31, YTS]
// 자신이 맞아 죽거나 다른사람을 죽였을때 나타나는 메세지 처리
class avaUIKillMessage extends avaUIInfoMessage
	native;

var transient float				LatestUpdateTime;

var() Color						KilledColor;
var() Color						KillerColor;
var() float						DisplayDuration<ToolTip=Icons /Normal:0, HeadShot:1, Explosive:2/>;
var transient vector			KillCamLocation;
var	int							Level;

enum KillMessageIconType
{
	KILLMESSAGE_ICONTYPE_NORMAL,
	KILLMESSAGE_ICONTYPE_HEADSHOT,
	KILLMESSAGE_ICONTYPE_EXPLOSION,
	KILLMESSAGE_ICONTYPE_WALLSHOT,
	KILLMESSAGE_ICONTYPE_WALLHEADSHOT,
};

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
	virtual INT UpdateInfo();
	//virtual UBOOL UpdateString();

	//UBOOL Render_Icon( FCanvas* Canvas, FLOAT X, FLOAT Y );	
	//virtual void Render_Widget( FCanvas* Canvas );

	virtual void Render_Text( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT ScaleX = 1.f, FLOAT ScaleY = 1.f );

	BOOL	CheckViewtargetKiller( const FKillMessageInfo& MsgInfo, const AavaPlayerController* PlayerOwner );
	BOOL	CheckViewtargetVictim( const FKillMessageInfo& MsgInfo, const AavaPlayerController* PlayerOwner );
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'
	DrawColor=(R=255,G=255,B=255,A=255)
	KilledColor=(R=255,G=255,B=255,A=255)
	KillerColor=(R=255,G=255,B=255,A=255)

	Icons(KILLMESSAGE_ICONTYPE_NORMAL)=(Image=Texture2D'EngineResources.WhiteSquareTexture',Code=KILLMESSAGE_ICONTYPE_NORMAL,IconColor=(A=255,R=255,G=255,B=255),Coordinates=(U=0,V=0,UL=0,VL=0))
	Icons(KILLMESSAGE_ICONTYPE_HEADSHOT)=(Image=Texture2D'EngineResources.WhiteSquareTexture',Code=KILLMESSAGE_ICONTYPE_HEADSHOT, IconColor=(A=255,R=255,G=255,B=255),Coordinates=(U=0,V=0,UL=0,VL=0))
	Icons(KILLMESSAGE_ICONTYPE_EXPLOSION)=(Image=Texture2D'EngineResources.WhiteSquareTexture',Code=KILLMESSAGE_ICONTYPE_EXPLOSION,IconColor=(A=255,R=255,G=255,B=255),Coordinates=(U=0,V=0,UL=0,VL=0))
	Icons(KILLMESSAGE_ICONTYPE_WALLSHOT)=(Image=Texture2D'EngineResources.WhiteSquareTexture',Code=KILLMESSAGE_ICONTYPE_WALLSHOT,IconColor=(A=255,R=255,G=255,B=255),Coordinates=(U=0,V=0,UL=0,VL=0))
	Icons(KILLMESSAGE_ICONTYPE_WALLHEADSHOT)=(Image=Texture2D'EngineResources.WhiteSquareTexture',Code=KILLMESSAGE_ICONTYPE_WALLHEADSHOT,IconColor=(A=255,R=255,G=255,B=255),Coordinates=(U=0,V=0,UL=0,VL=0))

	bDropShadow=true
	DisplayDuration=1.5
}