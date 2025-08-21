/*
	Check Point 위치 추가...		
*/

class avaUIOccupationBar extends UIObject native;

struct native BarInfo
{
	var()	TextureCoordinates		TextureCoord;
	var()	Color					ImageColor;
	var()	Surface					Image;

	structdefaultproperties
	{
		ImageColor=(R=255,G=255,B=255,A=255)
	}
};

struct native OccupationIndicatorInfo
{
	var()	float					YPos;
	var()	float					Width,Height;
	var()	TextureCoordinates		TextureCoord;
	var()	Color					ImageColor;
	var()	Surface					Image;

	structdefaultproperties
	{
		ImageColor=(R=255,G=255,B=255,A=255)
	}
};

var(BarInfo)	BarInfo							BackgroundBar;

var(BarInfo)	BarInfo							LeftBar;
var(BarInfo)	BarInfo							RightBar;


var(BarInfo)	OccupationIndicatorInfo			CheckPoint;
var(BarInfo)	array<OccupationIndicatorInfo>	Indicator;
var(BarInfo)	OccupationIndicatorInfo			TargetPoint;
var(BarInfo)	bool							bUseDominance;

var(BarInfo)	bool							bDrawProgressRate;
var(BarInfo)	bool							bDrawProgressByPercentage;
var(BarInfo)	Font							ProgressRateFont;
var(BarInfo)	Vector2D						ProgressRateOffset;
var(BarInfo)	Vector2D						TargetProgressRateOffset;
var(BarInfo)	Color							ProgressRateColor;

var(test)		array<float>			TestCheckPoint;
var(test)		float					TestIndicatorPoint;
var(test)		int						TestIndicatorIdx;
var(test)		float					TestTargetPoint;
var(test)		string					TestRate;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
	virtual void RenderCheckPoint( FCanvas* Canvas, float Cur, float Max );
	virtual void RenderTargetPoint( FCanvas* Canvas, float Cur, float Max );
}

defaultproperties
{
	TestRate					=	"30000"
	ProgressRateFont			=	TrueTypeFont'GameFonts.Tiny11'
	ProgressRateColor			=	(R=255,G=255,B=255,A=255)
	bDrawProgressRate			=	false
	bDrawProgressByPercentage	=	false
	ProgressRateOffset			=	(x=0,y=0)
}
