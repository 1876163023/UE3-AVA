class avaUIAmmoGauge extends avaUIProgressBar native;

var()	bool	bReloadGuage;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );				
}

defaultproperties
{
	Direction=AVAUIPROGRESSDirection_Down
	BackgroundColor=(R=128,G=128,B=128,A=128)		 	
}