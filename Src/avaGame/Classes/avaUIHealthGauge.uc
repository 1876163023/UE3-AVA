class avaUIHealthGauge extends avaUIProgressBar native;

//struct native HealthGaugeCode
//{
//	var() class<avaCharacter> CharacterType;
//	var() string DrawCode;
//};
//
//
//var() string DrawCode;
//var() Font DrawFont;
//var() array<HealthGaugeCode> Codes;
//var() float FontMargin[4];
//
//var transient byte RealDrawCode;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );				
}

defaultproperties
{
	Direction=AVAUIPROGRESSDirection_Down
	BackgroundColor=(R=128,G=128,B=128,A=128)		 	
}