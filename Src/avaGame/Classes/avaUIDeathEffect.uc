class avaUIDeathEffect extends UIObject native;

var()	Surface				EffectImage;
var()	TextureCoordinates	EffectCoordinates;
var()	Color	EffectColor;
var()	float	EffectTime;

var()	float	EffectMinAlpha;
var()	float	EffectMaxAlpha;

var()	float	TestAlpha;


var transient float	EffectStartTime;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
}

defaultproperties
{
	EffectColor = (R=255,G=0,B=0)
	EffectTime	= 5.0
	EffectImage = "EngineResources.WhiteSquareTexture"
	EffectCoordinates = (U=0.0,V=0.0,UL=1.0,VL=1.0)
	EffectMinAlpha	= 0
	EffectMaxAlpha	= 128
}