class avaUIClassIcon extends UIObject
	native;

struct native ClassIconInfo
{
	var() Surface				Image;
	var() TextureCoordinates	NormalCoord;
	var() TextureCoordinates	HighlightCoord;
};

var() ClassIconInfo				IconInfo[2];	// Team º° Icon Info 
var() int						ClassIdx;
var() Color						NormalColor;
var() Color						HighlightColor;

var() bool						bCurrentClass;


cpptext
{
	void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	ClassIdx = 0
}
