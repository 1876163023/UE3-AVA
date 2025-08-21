class avaUIPlayersSummary extends UIObject native dependson(avaUIGameString);

enum avaUIHAlignType		// Horizontal Align Type
{
	HAlign_Left,
	HAlign_Center,
	HAlign_Right,
};

enum avaUIVAlignType		// Vertical Align Type
{
	VAlign_Top,
	VAlign_Center,
	VAlign_Bottom,
};

// [2006/11/05 YTS, Add] innerstruct of ColumnInfo. storing Iconinfo.
struct native PlayerSummaryIconInfo
{
	var() Surface						Surface;
	var() IntPoint						DrawExtent;
	var() TextureCoordinates			TexCoord;
	var() bool							bIgnoreCoord;			// using entire area of surface
	var() bool							bIgnoreExtent;			// using surface extent directly
	var() Color							Color;
	structdefaultproperties
	{
		Surface=Texture2D'EngineResources.WhiteSquareTexture'
		Color=(R=255,G=255,B=255,A=255)
		bIgnoreCoord=true
		bIgnoreExtent=true
	}
};

// [2006/11/05 YTS, Modify] add some displayable flag & icon 
struct native ColumnInfo
{
	var() string					Label;
	var() avaUIPlayerSummaryType	SummaryType;
	var() float						Width;
	var() float						Spacing;
	var() avaUIHAlignType			HeaderHorizontalAlign;
	var() avaUIVAlignType			HeaderVerticalAlign;
	var() avaUIHAlignType			DaraHorizontalAlign;

	var() array<PlayerSummaryIconInfo>			IconInfo;
	var() bool									bShowIcon;
	var() bool									bShowString;
	var() bool									bClipString;

	structdefaultproperties
	{
		bShowIcon=false
		bShowString=true
		bClipString=false
	}
};

var(Header) Font						HeaderFont;			// Font For Header Control
var(Header) Color						HeaderColor[2]<ToolTip=/0-SameTeam/1-OppositeTeam/>;		// Color For Header Control
var(Column) Font						DataFont;			// Font For Data
var(Column) Color						DataColor[2]<ToolTip=/0-SameTeam/1-OppositeTeam/>;			// Color For Dat
var(Column) Color						DisabledColor[2]<ToolTip=/0-SameTeam/1-OppositeTeam/>;
var(Column) float						DataHeight;
var(Header) int							nTeam;				// whitch Team 
var(Header) float						Xpos, Ypos;
var(Header) float						Padding;
var(Header) float						RowSpacing;
var(Header) float						HeaderHeight;
var(Header) avaUIPlayerSummaryType		SortBy;
var(Header) array< ColumnInfo >			ColumnInfos;		// Colum Info Data
var()		bool						bOnlySpectator;		// Spectator 용인가?
var()		int							LimitCnt;			// 제한 Count...


var(Highlight)		Color				HighlightFontColor[2];
var(Highlight)		Color				HighlightBGColor[2];

var(Test) string						TestData;
var(Test) int							TestDataCnt;
var(Test) int							HightLightIdx;

struct native SummaryFieldValueInfo
{
	var transient init		string	FieldOrgValue;
	var transient init		string	FieldReplValue;
};

var private transient init	array<SummaryFieldValueInfo>	FieldLinearTable;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );

protected:
	FPlayerSummaryIconInfo* GetPlayerIconInfo( avaUIPlayerSummaryType Type, const AavaPlayerReplicationInfo* PRI, int OwnerTeam );
}

defaultproperties
{
	SortBy = AVAUISummary_Score
	TestData	= "Data"
	TestDataCnt = 1
	HeaderColor(0)=(R=255,G=255,B=255,A=255)
	HeaderColor(1)=(R=255,G=255,B=255,A=255)

	DataColor(0)=(R=255,G=255,B=255,A=255)
	DataColor(1)=(R=255,G=255,B=255,A=255)

	HighlightFontColor(0)=(R=255,G=255,B=255,A=255)
	HighlightFontColor(1)=(R=255,G=255,B=255,A=255)

	HighlightBGColor(0)=(R=255,G=255,B=255,A=255)
	HighlightBGColor(1)=(R=255,G=255,B=255,A=255)

	LimitCnt	= 0
}