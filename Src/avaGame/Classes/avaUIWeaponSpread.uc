class avaUIWeaponSpread extends avaUIProgressBar native;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );				
	void Render_Progress( FCanvas* Canvas, USurface* Tex, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL );
	void Render_Bar( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Ratio, FLOAT RatioL, USurface* Surface, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& DrawColor );
}

defaultproperties
{
	Direction=AVAUIPROGRESSDirection_Down
	BackgroundColor=(R=128,G=128,B=128,A=128)		 	
}