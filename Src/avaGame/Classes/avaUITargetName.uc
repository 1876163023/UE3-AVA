class avaUITargetName extends avaUISimpleText native;

var transient Pawn TrackedTarget;
var transient float TrackedTargetAlpha;
var() float MinDist, MaxDist;
var transient float LastRenderTime;
var() float RisingSpeed, FallingSpeed;
var() float VisibleStart, MaxAlpha;

var() bool	bPrevInvincibilityMode;

cpptext
{
	virtual UBOOL UpdateString();
}

defaultproperties
{
	MinDist = 5 
	MaxDist = 20
	RisingSpeed = 2.0
	FallingSpeed = 1.0
	Font = TrueTypeFont'GameFonts.SmallFont'
	DrawColor = (R=255,G=10,B=10,A=255)
	HorizontalAlign = AVAUIALIGN_Center
	VisibleStart = 0.5
	MaxAlpha = 255.0
}
