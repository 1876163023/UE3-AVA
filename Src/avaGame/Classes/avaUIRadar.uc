class avaUIRadar extends UIObject native;

enum EUIRadarShape
{
	UIRADARSHAPE_RECTANGLE,
	UIRADARSHAPE_CIRCLE
};

var()							Surface					Image;
var()							array<TextureCoordinates>	BackgroundCoordinates;			// each teamcolor
var()							array<Color>				BackgroundColor;					// each teamcolor
var()							Float						BackgroundOpacity;
var()							color					BombColor, MissionObjectColor; 
var()							float					Hysteresis, ZoomScale;
var()							Font					Font;
var()							Font					TextFont;
var()							Material				MapMaterial;
var()							float					MapOffset[2];
var()							float					MapSize[2];
var()							bool					bClamp;
var Texture2D DefaultTexture;
var transient MaterialInstanceConstant					Map;
var transient Texture2D									LastMinimapTexture;
var transient					float					Scale;
var(Test)						vector					TestLocation;

//struct native RadarIconInfo
//{
//	var() int Code;
//	var() Surface Image;
//	var() color IconColor;
//	var() TextureCoordinates NormalPosCoordinates;
//	var() TextureCoordinates UpPosCoordinates;
//	var() TextureCoordinates DownPosCoordinates;
//
//	var(Anim) bool bAnimated;
//	var(Anim) int AnimatedImageCount;
//	var(Anim) float AnimationPeriod;
//
//	structdefaultproperties
//	{
//		bAnimated=false
//		AnimatedImageCount=1
//		AnimationPeriod=2.0
//	}
//};
//
//var(Icon)							array<RadarIconInfo>	Icons;
var()								int						Editor_IconCode;
var()								EUIRadarShape			RadarShape;
var()								bool					bDrawOnlySlotNum;
var()								Vector2D				SlotNumSize;

var(Icon)							float					BlinkPeriod;
var(Icon)							float					DamageDisplayTime;

// [2006/11/05 YTS] Outside Indicator ( Out Icon ) 
var(OutIcon)							bool					bShowOutIcon;
var(OutIcon)							Surface					OutIcon;
var(OutIcon)							TextureCoordinates		OutIconCoord;
var(OutIcon)							bool					bIgnoreOutIconCoord;
var(OutIcon)							bool					bIgnoreOutIconExtent;
var(OutIcon)							IntPoint				OutIconExtent;
var(OutIcon)							Color					OutIconColor;
var(OutIcon)							float					RotationOffsetDegree;

var(Icon)							array<int>				ClassCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper>;
var(Icon)							array<int>				FriendCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper>;
var(Icon)							array<int>				EnemyCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper>;
var(Icon)							array<int>				FriendDamagedCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper/>;
var(Icon)							array<int>				EnemyDamagedCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper/>;
var(Icon)							int						FriendDeadCode;
var(Icon)							int						EnemyDeadCode;

var(Targetted)						int						TargettedCode;
var(Targetted)						float					TargettedMinScale;
var(Targetted)						float					TargettedMaxScale;
var(Targetted)						float					TargettedWaveDuration;

//var(Icon)							class<NavigationPoint>	MissionNavPointClass;
//var(Icon)							int						MissionNavPointCode;
//var(Icon)							class<avaPickUp>		MissionObjectClass;
//var(Icon)							int						MissionObjectCode;

//var(Icon)							Color					FriendColor;
//var(Icon)							int						FriendCode;				
//var(Icon)							Color					FriendDeadColor;
//var(Icon)							int						FriendDeadCode;
//
//var(Icon)							Color					EnemyColor;
//var(Icon)							int						EnemyCode;
//var(Icon)							Color					EnemyDeadColor;
//var(Icon)							int						EnemyDeadCode;

var(Icon)							float					ReinforcementTime;
var(Icon)							bool					bOverrideReinforcementTime;

var(Icon)							bool					bShowLocalPlayer;
var(Icon)							int						LocalPlayerCode;

var(Icon)							int						QuickChatCode;
var(Icon)							float					QuickChatDisplayTime;

var(Icon)							int						SignalCode;
var(Icon)							float					SignalDisplayTime;

//var()								Color					FriendlyColor;
//var()								Color					EnemyColor;


// 분대장에 의해서 Target 된 놈을 표시하기 위한 Properties
var(targetted)						float					TargetDuration;
var(targetted)						float					TargetMaxScale;
var(targetted)						float					MinTargetHeight;
var(targetted)						float					MaxTargetHeight;
var(targetted)						float					TargetHeightSpacing;

var(targetted)						float					DefaultTargetHeight;
var(targetted)						int						POIIconCode;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
	virtual const FIconCodeInfo* GetRadarIconInfo( const AavaHUD* HUD, int code );
	virtual void PostEditChange( UProperty* PropertyThatChanged );

protected :
	void Render_Object( FCanvas* Canvas, const float xpos, const float ypos, const float sizex, const float sizey, const FIconCodeInfo* IconInfo, FLOAT Alpha = 1.0f, FLOAT UpdateTime = -1.0f, UBOOL bOverrideColor = FALSE, FLinearColor OverrideColor = FLinearColor(1.f,1.f,1.f) );
	void Render_Object( FCanvas* Canvas, const FVector& DeltaLocation, const FLinearColor& Color, const FRotator& ViewRotator, FLOAT Alpha = 1.0f );
	void Render_Object( FCanvas* Canvas, const FVector& DeltaLocation, const FIconCodeInfo* IconInfo, const FRotator& ViewRotator, FLOAT Alpha = 1.0f , FLOAT UpdateTime = -1.0f, UBOOL bOverrideColor = FALSE, FLinearColor OverrideColor = FLinearColor(1.f,1.f,1.f), UBOOL bAdjustRotator = FALSE, FLOAT RotateDegree = 0, FLOAT Scaler = 1.0 );
	void DrawPawnInHUD( FCanvas* Canvas, AavaPawn* Pawn, AavaPawn* PawnOwner, AavaPlayerController* PlayerOwner, const FVector& view_Location, const FRotator& view_Rotator,const BYTE OwnerTeam );	
	void DrawVehicleInHUD( FCanvas* Canvas, AavaVehicle* Vehicle, AavaPawn* PawnOwner, const AavaPlayerController* PlayerOwner, const FVector& view_Location, const FRotator& view_Rotator,const BYTE OwnerTeam );	
	void DrawTargetInHUD( FCanvas* Canvas, const AActor* PawnOwner, const AavaPlayerController* PlayerOwner, const AActor* actor, const FLOAT TargettedTime, const int IconCode, const FString str );
	UBOOL CalcPosInRadar( const FVector& pos, const FRotator& ViewRotator, FVector2D& resultPos, FVector2D& scaledPos,UBOOL &bOutside, FLOAT& DegAngle,UBOOL bClamp = FALSE, UBOOL bAlwaysClamp = FALSE );
}

defaultproperties
{
	Position=(Value[UIFACE_Right]=119,Value[UIFACE_Bottom]=119,ScaleType[UIFACE_Right]=EVALPOS_PixelOwner,ScaleType[UIFACE_Bottom]=EVALPOS_PixelOwner)
	Scale = 0.01
	Hysteresis = 64.0	
	DefaultTexture="EngineResources.WhiteSquareTexture"
	MissionObjectColor=(R=255,G=0,B=0,A=255)
	BombColor	=(R=255,G=0,B=0,A=255)
	Font		=TrueTypeFont'GameFonts.Tiny11'
	TextFont	=Font'EngineFonts.SmallFont'

	//Icons(0) = (Code=0,IconColor=(R=255,G=0,B=0,A=255))
	ZoomScale=1.0

	bShowOutIcon=true
	OutIcon=Texture2D'EngineResources.WhiteSquareTexture'
	OutIconCoord=(U=0,V=0,UL=1,VL=1)
	bIgnoreOutIconCoord=true
	bIgnoreOutIconExtent=true
	OutIconExtent=(X=8,Y=8)
	OutIconColor=(R=255,G=255,B=255,A=255)
	RotationOffsetDegree=0.0

	//FriendColor=(R=0,G=255,B=0,A=255)
	//FriendDeadColor=(R=50,G=255,B=50,A=255)
	//FriendCode=-1
	//FriendDeadCode=-1
	//EnemyColor=(R=255,G=255,B=255,A=255)
	//EnemyDeadColor=(R=255,G=255,B=255,A=200)
	//EnemyCode=-1
	//EnemyDeadCode=-1

	BackgroundCoordinates(0)=()
	BackgroundCoordinates(1)=()

	BackgroundColor(0)=(R=255,G=255,B=255,A=255)
	BackgroundColor(1)=(R=255,G=255,B=255,A=255)

	BackgroundOpacity=1.0

	RadarShape=UIRADARSHAPE_CIRCLE
	ReinforcementTime=10.0
	bOverrideReinforcementTime=false

	DamageDisplayTime=3.0
	BlinkPeriod=1.0

	FriendCodes(0)=-1
	FriendCodes(1)=-1
	FriendCodes(2)=-1
	EnemyCodes(0)=-1
	EnemyCodes(1)=-1
	EnemyCodes(2)=-1
	ClassCodes(0)=-1
	ClassCodes(1)=-1
	ClassCodes(2)=-1

	FriendDamagedCodes(0)=-1
	FriendDamagedCodes(1)=-1
	FriendDamagedCodes(2)=-1
	EnemyDamagedCodes(0)=-1
	EnemyDamagedCodes(1)=-1
	EnemyDamagedCodes(2)=-1

	FriendDeadCode=-1
	EnemyDeadCode=-1
	
	QuickChatCode=-1
	QuickChatDisplayTime=5.0

	//EnemyColor=(R=255,G=78,B=0,A=255)
	//FriendlyColor=(R=34,G=207,B=0,A=255)

	TargetDuration		= 0.3
	TargetMaxScale		= 4.0
	MinTargetHeight		= 32.0
	MaxTargetHeight		= 137.0
	TargetHeightSpacing = 16.0
	DefaultTargetHeight	= 64.0

	TargettedCode			=	2
	TargettedMinScale		=	0.5f
	TargettedMaxScale		=	2.0f
	TargettedWaveDuration	=	1.0f
}