class avaUIHostLoadingBar extends avaUIProgressBar native;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );				
}

defaultproperties
{
	Direction=AVAUIPROGRESSDirection_Right
	BackgroundColor=(R=128,G=128,B=128,A=128)		 	
}