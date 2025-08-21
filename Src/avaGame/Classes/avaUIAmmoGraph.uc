class avaUIAmmoGraph extends UIObject native;

var() Surface Image;
var() float BlankU, BlankV, FullU, FullV;
var() int AmmoInImage;
var() float AmmoSize;
var() float SpacingX, SpacingY;
var() interp float Scroll;
var transient int Value;
var transient bool bIsReloading;
var() color BlankColor, FullColor;
var(Background) color BackgroundImageColor;
var(Background) Surface BackgroundImage;
var(Background) TextureCoordinates BackgroundImageCoordinates;
var(Background) float BackgroundImageTop;
var(Background) bool bShouldTileBackground;
var() float Padding[2];
var(Fade) float FadeTime;
var(Test) int TestValue, TestMaxAmmo;

var transient array<float> Alphas;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );

	virtual void Render_Widget( FCanvas* Canvas );

	void Render_Column( FCanvas* Canvas, FLOAT X, FLOAT Y, INT N, INT Blank, INT Index, INT AlphaStart );
	void Render_Ammo( FCanvas* Canvas, FLOAT Alpha, FLOAT X, FLOAT Y, FLOAT YL );
	void Render_Background( FCanvas* Canvas, FLOAT Y );
}

defaultproperties
{
	AmmoInImage = 5
	SpacingX = 7
	SpacingY = 5
	BlankColor=(R=25,G=25,B=25,A=255)
	FullColor=(R=255,G=255,B=255,A=255)	
	BackgroundImageColor=(R=255,G=255,B=255,A=255)
	FadeTime = 0.1
}