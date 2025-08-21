class avaUILabel extends avaUIImageBase
	native;

/** If true, this label will stop and reset and fading when it changes it's caption */
var(Widget) bool bResetFadeWhenChanged;

/** The string to display */
var(Widget) editinline const string Caption;

/** The Font to use */
var(Widget) editinlineuse const Font Font;

/** The color of this text */
var(Widget) editinline const linearcolor FontColor;

/** Alignment in the widget */
var(Widget) editinline EUIAlignment Justification;

enum EShadowDir
{
	ESD_None,
	ESD_UpperLeft,
	ESD_UpperMid,
	ESD_UpperRight,
	ESD_Left,
	ESD_Glow,
	ESD_Right,
	ESD_LowerLeft,
	ESD_LowerMid,
	ESD_LowerRight,
	ESD_MAX
};

/** If set, the font will be dawn with a shadow */
var(Widget) EShadowDir ShadowedType;

/** If the above is set, use this color for the shadow */
var(Widget) LinearColor ShadowColor;

/** These variables are workhorses.  We store the render positions for quick rerender */

/** NOTE: We use Right/Bottom as widget and height */
var private	const transient	float TextBounds[EUIWidgetFace.UIFACE_MAX];
var private const transient float FontScaling;
var private const transient int FontPageIndex;
var private const transient int DrawX;
var private const transient int DrawY;
var private const transient int LeftBoundary;
var private const transient int RightBoundary;

cpptext
{
	virtual void DrawText(FCanvas* Canvas, int XMod=0, int YMod=0);
	virtual void Render_Widget( FCanvas* Canvas );
	virtual void ResolveFacePosition( EUIWidgetFace Face );
	virtual void ResizeCaption();
	virtual void PostEditChange( UProperty* PropertyThatChanged );
}

native function SetCaption(string NewCaption);
native function SetFont(Font NewFont);
native function SetColor(LinearColor NewColor);


defaultproperties
{
	FontColor=(R=1.0,G=1.0,B=1.0,A=1.0)
}
