class avaUIGrenadeIndicator extends UIObject
	native;

var(Indicator) TextureCoordinates	TopIndicator;
var(Indicator) Surface				Image;
var(Indicator) Color				IndicatorColor;
var(Indicator) float				LeftPos, TopPos, TopWidth, TopHeight;
var(WeaponIcon)	Font				WeaponIconFont;
var(WeaponIcon)	Color				WeaponIconColor;

var(test)	String	TestStr;
var(test)	FLOAT	TestAngle;

cpptext
{
public:
	virtual void Render_Widget( FCanvas *Canvas );
	virtual void Render_Icon( FCanvas* Canvas, FString IconStr );
	virtual void Render_Indicator( FCanvas* Canvas, FLOAT Angle );
}

defaultproperties
{
	LeftPos				=	0.0
	TopPos				=	0.0
	TopWidth			=	1.0
	TopHeight			=	0.2
}