class avaUIFullScreenMap extends UIObject
	native;

var	transient	Surface						LatestMapTexture;
var(Map) float								ZoomScale;
var(Map) TextureCoordinates					MapTexCoord;
var(Map) bool								bOverrideMapTexCoord;
var(Map) bool								bShowAlways;
var(Map) float								MapRotationDegree;
var(Map) bool								bUseLargeMap;

var avaUIRadar								UIRadar;				// UIRadar와 같은 아이콘 정보를 쓸것임.

var(Icon)	bool							bDrawOnlySlotNum;
var(Icon)	Vector2D						SlotNumSize;

var(Icon)	bool							bShowFriendPlayer;
var(Icon)	bool							bShowEnemyPlayer<ToolTip=EnemyPlayers can be shown when visible>;
var(Icon)	bool							bShowRadarActor;
var(Icon)	bool							bShowDeadPlayer;
var(Icon)	bool							bShowLocalPlayer;
var(Icon)	bool							bShowFriendPlayerName;
var(Icon)	bool							bShowQuickChat;
var(Icon)	Vector2D						FriendPlayerNameOffset;
var(Icon)	float							RotateDegAngle<Axis-origin>;
var(Icon)	Font							Font;
var(Icon)	Color							FriendPlayerNameColor;

var(Icon)	Vector2D						SpecialInvNameOffset;
var(Icon)	Color							SpecialInvNameColor;

var(Icon)	Texture2D						VisibleRegionTexture;
var(Icon)	TextureCoordinates				VisibleRegionTexCoord;
var(Icon)	Vector2D						VisibleRegionRotateAxis;
var(Icon)	float							VisibleRegionRotateOffsetDegAngle;

var(Icon)	bool							bOverrideMissionNavPoint;
var(Icon)	class<NavigationPoint>			MissionNavPointClass;
var(Icon)	int								MissionNavPointIconCode;


var(Icon)							array<int>				ClassCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper>;
var(Icon)							array<int>				FriendCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper>;
var(Icon)							array<int>				EnemyCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper>;
var(Icon)							array<int>				FriendDamagedCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper/>;
var(Icon)							array<int>				EnemyDamagedCodes<ToolTip=/0-PointMan/1-RifleMan/2-Sniper/>;
var(Icon)							int						FriendDeadCode;
var(Icon)							int						EnemyDeadCode;
var(Icon)							int						LocalPlayerCode;
var(Icon)							int						QuickChatCode;

var(Interp) transient Interp Vector			InterpLocalPlayerIconScaler;
var(Interp) transient Interp Vector			InterpBIAIconScaler;

var private Texture2D						DefaultWhiteTexture;

var(Icon)	float							DefaultScale;


var(Targetted)						int						TargettedCode;
var(Targetted)						float					TargettedMinScale;
var(Targetted)						float					TargettedMaxScale;
var(Targetted)						float					TargettedWaveDuration;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
	virtual void Render_Widget( FCanvas* Canvas );
	void Render_Object( FCanvas* Canvas, const AavaHUD* HUD, const FVector WorldLocation, const INT IconCode, FLOAT Alpha = 1.0f ,FColor Color = FColor(255,255,255,255), UBOOL bOverrideColor = FALSE, UBOOL bAdjustRotator = FALSE, FLOAT RotateDegree = 0, FVector Scaler=FVector(1.0,1.0,1.0) );
	UBOOL CalcPosInMap( FVector& WorldLocation , const FLOAT RotDegAnle);
}

defaultproperties
{
	ZoomScale=1.0f
	DefaultWhiteTexture=Texture2D'EngineResources.WhiteSquareTexture'

	bShowFriendPlayer = true
	bShowEnemyPlayer = true
	bShowRadarActor = true
	bShowDeadPlayer = true
	bShowLocalPlayer = true
	bShowFriendPlayerName = true
	bShowQuickChat = true

	RotateDegAngle = 0.0
	Font=Font'EngineFonts.SmallFont'

	FriendPlayerNameOffset=(X=0,Y=-20)
	FriendPlayerNameColor=(R=192,G=192,B=192,A=255)

	SpecialInvNameOffset=(X=0,Y=5)
	SpecialInvNameColor=(R=255,G=255,B=255,A=255)

	MissionNavPointClass=class'avaMissionNavPoint'
	MissionNavPointIconCode=-1

	InterpLocalPlayerIconScaler=(X=1.0,Y=1.0)
	InterpBIAIconScaler=(X=1.0,Y=1.0)

	ClassCodes(0)	=	-1
	ClassCodes(1)	=	-1
	ClassCodes(2)	=	-1

	FriendCodes(0)	=	-1
	FriendCodes(1)	=	-1
	FriendCodes(2)	=	-1

	EnemyCodes(0)	=	-1
	EnemyCodes(1)	=	-1
	EnemyCodes(2)	=	-1

	FriendDamagedCodes(0)	=	-1
	FriendDamagedCodes(1)	=	-1
	FriendDamagedCodes(2)	=	-1

	EnemyDamagedCodes(0)	=	-1
	EnemyDamagedCodes(1)	=	-1
	EnemyDamagedCodes(2)	=	-1


	FriendDeadCode	=	-1
	EnemyDeadCode	=	-1	
	LocalPlayerCode	=	-1
	QuickChatCode	=	-1

	DefaultScale	=	1.0

	TargettedCode			=	2
	TargettedMinScale		=	0.5f
	TargettedMaxScale		=	2.0f
	TargettedWaveDuration	=	1.0f
}