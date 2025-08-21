
class avaUIBackGroundImage extends UIObject
	native;

struct native BackGroundImageInfo
{
	var()	float					XPos,YPos,Width,Height;
	var()	TextureCoordinates		TextureCoord;
	var()	Color					ImageColor;

	structdefaultproperties
	{
		XPos=0
		YPos=0
		Width=100
		Height=100
		ImageColor=(R=255,G=255,B=255,A=255)
	}
};

var(BackGround)	array<BackGroundImageInfo>	BGInfos;
var(BackGround) Surface						BGImage;
var(BackGround)	int							TeamIndex;

var(BackGround) int							TestTeamIndex;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	TeamIndex	=	-1
}

