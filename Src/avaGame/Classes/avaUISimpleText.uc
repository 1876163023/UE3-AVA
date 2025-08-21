class avaUISimpleText extends UIObject native; 

enum AVAUIAlign
{
	AVAUIALIGN_LeftOrTop,
	AVAUIALIGN_Center,
	AVAUIALIGN_RightOrBottom
};

var(Presentation) AVAUIAlign HorizontalAlign, VerticalAlign;
var(Presentation) Font Font;
var(Presentation) color DrawColor;
var(Shadow) bool bDropShadow;
var(Text) string Message;
var(Fade) float FadeInTime, FadeOutTime;
var(Fade) float FadeMax;
var transient float DrawXL, DrawYL;
var transient float TextXL, TextYL;
var transient int FadeMode; // -1ï¿?FadeOut 1ï¿?FadeIn
var transient bool bDirty;
var transient string OldMessage;

var(Background) bool bDrawBackground;
var(Background) Surface Background;
var(Background) TextureCoordinates LeftTopRounding, RightTopRounding, LeftBottomRounding, RightBottomRounding;
var(Background) TextureCoordinates TopBorder, LeftBorder, RightBorder, BottomBorder;
var(Background) bool bVSymmetry, bHSymmetry;
var(Background) TextureCoordinates Inner; 
var(Background) color BackgroundColor;
var(Background) float Padding[4];
var(Background) float MinSize[2];
var(Background) bool bShouldTileBackground;

var bool bDrawIcon;
var(Icon) surface Icon;
var(Icon) TextureCoordinates IconCoordinates;
var(Presentation) float IconSpacing;
var(Icon) color IconColor;
var(Presentation) EUIWidgetFace IconPosition;
var(Presentation) bool bDrawIndep;
var(Presentation) bool bSkipRender;

var(Interp) transient Interp Vector		InterpIconOffset;
var(Interp) transient Interp Vector		InterpIconScaler;
var(Interp) transient Interp Vector2D	InterpIconScaleAxis;
var(Interp) transient Interp float		InterpIconOpacity;

var(Text) bool bMessageShouldBeSaved;
var(Interp) transient Interp Vector		InterpTextOffset;
var(Interp) transient Interp Vector		InterpTextScaler;
var(Interp) transient Interp float		InterpTextOpacity;
var(Interp) transient Interp Vector2D	InterpTextScaleAxis;

// <=0ÀÌ¸é disabled
var float Hot_CurrentValue;
var(Hot) float Hot_Decay;

cpptext
{
	FLinearColor CalcWarmColor( const FLinearColor& SrcColor ) const;
	void CalculatePosition( FLOAT& X, FLOAT& Y, FLOAT XL, FLOAT YL );
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );

	virtual void Render_Widget( FCanvas* Canvas );	

	virtual UBOOL UpdateString() {return FALSE;}

	virtual void PostLoad();

	void Render_Background( FCanvas* Canvas, FLOAT XL, FLOAT YL, FLOAT& X, FLOAT& Y );

	virtual void Render_Text( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT ScaleX = 1.f, FLOAT ScaleY = 1.f);

	UBOOL Render_Icon( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL );	
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'
	DrawColor = (R=140, G=255, B=47, A=255)	
	BackgroundColor = (R=255, G=255, B=255, A=255)
	IconColor = (R=255, G=255, B=255, A=255)

	bDirty=true

	bDropShadow=true

	FadeMax = 1.0
	FadeInTime = 0.5
	FadeOutTime = 0.5

	HorizontalAlign = AVAUIALIGN_RightOrBottom
	VerticalAlign = AVAUIALIGN_Center

	bVSymmetry	=	true
	bHSymmetry	=	true
	bDrawIcon	=	false
	
	Hot_Decay = 0.05

	IconPosition=UIFACE_Left;

	InterpIconScaler=(X=1.0,Y=1.0,Z=1.0)
	InterpTextScaler=(X=1.0,Y=1.0,Z=1.0)

	InterpIconOpacity=1.0
	InterpTextOpacity=1.0

	InterpIconScaleAxis=(X=0.5,Y=0.5)
	InterpTextScaleAxis=(X=0.5,Y=0.5)

	Begin Object Class=avaUIEvent_SimpleTextChanged Name=SimpleTextChanged
	End Object
}