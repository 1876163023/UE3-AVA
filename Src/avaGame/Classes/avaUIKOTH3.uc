class avaUIKOTH3 extends avaUIClock native;

var() int TeamIndex;

var transient int LastNumPlayersInside;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );

	virtual UBOOL UpdateString();	
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'	
	NormalColor = (R=140, G=255, B=47, A=255)		 
	AlarmColor = (R=255, G=0, B=0, A=255)		 
	AlarmSeconds = 30
	AlarmBlinkingPeriod = 0.7
	bFormattedTime = true
}