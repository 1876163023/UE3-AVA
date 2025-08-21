class avaUILevelIcon extends UIObject native;

var()	int		TestLevel;
var()	float	Oppacity;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
}

defaultproperties
{
	Oppacity = 1.0
}