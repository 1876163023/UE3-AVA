class avaUIVoteMenu extends UIObject
	native;


var()				bool		bUseShadow;
var(Presentation)	Font		Font;
var(Presentation)	Color		DefaultColor;


var() array<string>			TestMessages;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
}


defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDSmall'
	DefaultColor=(R=255,G=255,B=255,A=255)
	bUseShadow	= true
}