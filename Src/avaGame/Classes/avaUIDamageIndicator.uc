class avaUIDamageIndicator extends UIObject native;

var() TextureCoordinates	TopIndicator;
var() Surface				Image;
var() Color					IndicatorColor;
var() float					LeftPos,	TopPos,	 TopWidth,  TopHeight;
var() bool					bDisplayDirection;		//	

var() float					DisplayTime;
var() float					BlendTime;

var transient float			LastRenderTime;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	LeftPos				=	0.0
	TopPos				=	0.0
	TopWidth			=	1.0
	TopHeight			=	0.2
	bDisplayDirection	=	true
}