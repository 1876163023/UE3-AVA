class avaUITopPlayerInfo extends UIObject native;

var(Global)		bool				bDrawAbsoluteRank;


var(Background)	Surface				BackgroundImage;
var(Background)	TextureCoordinates	BackgroundCoord;
var(Background)	Color				BackgroundImageColor;


var(Name)		Font				NameFont;
var(Name)		Color				NameNormalColor;
var(Name)		Color				NameSelectedColor;

var(Name)		Font				NumberFont;
var(Name)		Font				SelectedNumberFont;
var(Name)		Color				NumberColor;


var(Name)		Surface				SelectedImage;
var(Name)		TextureCoordinates	SelectedCoord;
var(Name)		Color				SelectedImageColor;
var(Name)		Vector2D			SelectedImagePos;
var(Name)		Vector2D			SelectedImageSize;

var(Name)		Vector2D			DrawPos;
var(Name)		int					DrawCnt;		// 홀수 이어야 한다....

var(Spacing)	float				ColumnSpacing;
var(Spacing)	float				NameSpacing;
var(Spacing)	float				PointSpacing;

var				int					PrevScore;
var				float				UpdateScoreTime;
var(Name)		CurveEdPresetCurve	UpdateScoreCurve;

var(Name)		float				FontScalerMaxTime;

var(test)		string				TestName;
var(test)		int					TestStartNum;

cpptext
{
	void	DrawPlayerName( FCanvas* Canvas, const int Number, const BOOL bSelected, const INT Level, const INT Score1, const INT Score2, const TCHAR* PlayerName, float X, float Y );
	void	DrawBG( FCanvas* Canvas, float X, float Y, float XL, float YL );
	virtual void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	NameFont			=	TrueTypeFont'GameFonts.Tiny11'
	NumberFont			=	TrueTypeFont'GameFonts.Tiny11'
	SelectedNumberFont	=	TrueTypeFont'GameFonts.Tiny11'
	NameNormalColor		=	(R=255,G=255,B=255,A=255)
	NameSelectedColor	=	(R=255,G=255,B=255,A=255)	
	NumberColor			=	(R=255,G=255,B=255,A=255)	
	DrawCnt				=	1	

	DrawPos				=	(X=10)
	ColumnSpacing		=	15
	NameSpacing			=	10
	PointSpacing		=	150
	TestStartNum		=	1
	FontScalerMaxTime	=	0.3
}