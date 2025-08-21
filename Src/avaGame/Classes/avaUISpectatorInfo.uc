class avaUISpectatorInfo extends avaUISimpleText
	native;

var int	SpectatorLevel;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
	virtual void Render_Text( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT ScaleX = 1.f, FLOAT ScaleY = 1.f);
	virtual UBOOL UpdateString();		
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'
	DrawColor=(R=255,G=255,B=255,A=255)
	bDropShadow=true
}