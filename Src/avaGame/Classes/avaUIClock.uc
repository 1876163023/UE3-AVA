class avaUIClock extends avaUISimpleText native;

var transient float LastValue;
var transient bool bAlarm;
var transient float AlarmStartTime;
var(Clock) float AlarmSeconds, AlarmBlinkingPeriod;
var(Clock) color NormalColor, AlarmColor;
var(Clock) bool	 bFormattedTime;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );

	virtual UBOOL UpdateString();	

	void DecideColor( FLOAT Time, UBOOL bAlarm );
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