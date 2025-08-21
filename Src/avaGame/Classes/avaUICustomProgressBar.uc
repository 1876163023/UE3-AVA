class avaUICustomProgressBar extends avaUIProgressBar native;

var FLOAT XPos, YPos, XSize, YSize;

function event SetFadeInfos( array<GaugeFadeInfo> InFadeInfos )
{
	FadeInfos = InFadeInfos;
}

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
	void Render_Bar( FCanvas* Canvas, FLOAT Ratio, FLOAT RatioL, USurface* Surface, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& DrawColor );
	void Render_Progress( FCanvas* Canvas, USurface* Tex, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL );
	void UpdateFadeInfos();
	
}