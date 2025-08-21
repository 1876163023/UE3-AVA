class avaUILocationInfo extends avaUISimpleText
	native;


cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
	virtual UBOOL UpdateString();		
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'
	DrawColor=(R=255,G=255,B=255,A=255)
	bDropShadow=true
}