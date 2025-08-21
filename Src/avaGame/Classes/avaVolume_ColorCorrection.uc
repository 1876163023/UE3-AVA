class avaVolume_ColorCorrection extends Volume
	placeable
	native;


var()	float		Hue<ToolTip=-180 ~ 180 Deg>;
var()	float		Saturation<ToolTip=-100 ~ 100>;
var()	float		Lightness<ToolTip=-100 ~ 100>;
var()	float		Contrast<ToolTip=-128 ~ 127,Non-linear adjustment>;

var()	vector		Shadows<ToolTip=-1.0 ~ 1.0>;
var()	vector		Highlights<ToolTip=-0.2 ~ 5.0>;
var()	vector		MidTones<ToolTip=0.5 ~ 1.5>;
var()	float		Desaturation<ToolTip=0.0 ~ 1.0>;

var		float		MaximumWeight;
var()	float		FadeTime;

var()		EPixelFormat	PixelFormat<Tooltip=PF_A8R8G8B8-LessMemoryUsages But can't cover darkareas / PF_FloatRGBA-MoreMemoryUsages but can cover allareas/DO NOT USE ANOTHER FORMAT>;

cpptext
{
	virtual void PostEditChange( UProperty* PropertyThatChanged );
}

defaultproperties
{
	Hue=0.0
	Saturation=0.0
	Lightness=0.0
	Contrast=0.0

	Shadows=(X=0.0,Y=0.0,Z=0.0)
	Highlights=(X=1.0,Y=1.0,Z=1.0)
	MidTones=(X=1.0,Y=1.0,Z=1.0)
	Desaturation=0.0

	MaximumWeight=1.0
	FadeTime=3.0

	PixelFormat=PF_A8R8G8B8
}