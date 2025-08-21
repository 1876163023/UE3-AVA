class avaHudPercBar extends avaUIObject
	native;

var(Widget) float			Percentage;
var(Widget) bool			bStretch;		// Stretch Fill 을 할 것인가?
var(Widget) EUIWidgetFace	FixedFace;		// 어떤 축을 고정할 것인가?

var	TextureCoordinates		CoordinatesOrigin;

var instanced avaUIImage	PerBarFill;
var instanced avaUIImage	PerBarBackground;

cpptext
{	
	virtual void PostEditChange( UProperty* PropertyThatChanged );
}

defaultproperties
{
	Begin Object Class=avaUIImage Name=iPerBarFill
		WidgetTag=iPerBarFill
	End Object
	PerBarFill=iPerBarFill

	Begin Object Class=avaUIImage Name=iPerBarBack
		WidgetTag=iPerBarBack
	End Object
	PerBarBackground=iPerBarBack
}

