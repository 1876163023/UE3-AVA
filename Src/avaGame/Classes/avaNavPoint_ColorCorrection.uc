//
// [2006/11/30 ю╠еб╫д] ColorCorrection NavPoint
//

class avaNavPoint_ColorCorrection extends NavigationPoint
	placeable
	native;

var() float Hue<ToolTip=-180 ~ 180 Deg>;			
var() float Saturation<ToolTip=-100 ~ 100>;
var() float Lightness<ToolTip=-100 ~ 100>;
var() float Contrast<ToolTip=-128 ~ 127>;
var() vector Shadows<ToolTip=-1.0 ~ 1.0>;
var() vector Highlights<ToolTip=-0.2 ~ 5.0>;
var() vector MidTones<ToolTip=0.5 ~ 1.5>;
var() float Desaturation<ToolTip=0.0 ~ 1.0>;

var() float FalloffStartDistance;
var() float FalloffEndDistance;
var() EPixelFormat PixelFormat<Tooltip=PF_A8R8G8B8-LessMemoryUsages But can't cover darkareas / PF_FloatRGBA-MoreMemoryUsages but can cover allareas / DO NOT USE ANOTHER FORMAT>;
var float MaximumWeight;			/**< MaxWeight is fixed. (Not implement yet) */

cpptext
{
	virtual void PostEditChange( UProperty* PropertyThatChanged );
}

defaultproperties
{
	Begin Object NAME=CollisionCylinder
		CollisionRadius=+0.0
		CollisionHeight=+0.0
	End Object

	Begin Object Name=Sprite
		Sprite=Texture2D'EngineResources.LookTarget'
	End Object


	Hue=0.0
	Saturation=0.0
	Lightness=0.0
	Contrast=0.0

	Shadows=(X=0.0,Y=0.0,Z=0.0)
	Highlights=(X=1.0,Y=1.0,Z=1.0)
	MidTones=(X=1.0,Y=1.0,Z=1.0)
	Desaturation=0.0

	FalloffStartDistance=500.0
	FalloffEndDistance=700.0
	MaximumWeight=1.0

	PixelFormat=PF_A8R8G8B8
	bNotBased=true
}