class avaUIArmorGauge extends avaUIProgressBar native;

var() string	DrawCode;
var() Font		DrawFont;
var() float		FontMargin[4];

var() float		ArmorRatio;
var() Color		HelmetColor;
var() Color		NoHelmetColor;

var transient byte RealDrawCode;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );		
}

defaultproperties
{
	Direction=AVAUIPROGRESSDirection_Down
	ArmorRatio = 1.0
}