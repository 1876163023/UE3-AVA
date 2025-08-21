class avaUIDeathLog extends UIObject native;

var() float LifeTime, DimTime;
var() Font Font;
var() Font IconFont;
var() float LineSpacing;
var() int IconFontCutY;
var(Shadow) bool bDropShadow;
var(Shadow) int ShadowX, ShadowY;

var() int	IconSpacing;
var() bool	bDrawOnlySlotNum;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.SmallFont'
	IconFont = TrueTypeFont'GameFonts.UI.UISmall'
	LifeTime=5.0
	DimTime=1.5
	LineSpacing=3
	bDrawOnlySlotNum=false
}