class avaUIProgressBar extends UIObject native;

struct native GaugeFadeInfo
{
	var float Value;
	var float Alpha;
};
var(Test) float TestRatio;
var(Fade) float FadeTime;
var(Fade) color FadeColor;
var(Alert) color AlertColor;
var(Alert) float AlertStartRatio;
var transient float Ratio;

var transient array<GaugeFadeInfo> FadeInfos;

enum AVAUIPROGRESSDirection
{
	AVAUIPROGRESSDirection_Left,
	AVAUIPROGRESSDirection_Right,
	AVAUIPROGRESSDirection_Up,
	AVAUIPROGRESSDirection_Down
};

var(Direction) AVAUIPROGRESSDirection Direction;
var(Color) color BackgroundColor, ProgressColor;
var(Images) Surface Image, BackgroundImage;
var(Images) TextureCoordinates ImageCoordinates, BackgroundCoordinates;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );	
	void UpdateFadeInfos();	
	void Render_Progress( FCanvas* Canvas, USurface* Tex, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL );
	void Render_Bar( FCanvas* Canvas, FLOAT Ratio, FLOAT RatioL, USurface* Surface, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& DrawColor );
}

defaultproperties
{	
	FadeColor = (R=255, G=100, B=100, A=255)		 	
	Direction=AVAUIPROGRESSDirection_Right
	BackgroundColor=(R=255,G=255,B=255,A=128)
	ProgressColor=(R=0,G=128,B=0,A=128)	
	Image=Texture2D'EngineResources.WhiteSquareTexture'
	BackgroundImage=Texture2D'EngineResources.WhiteSquareTexture'
	AlertColor = (R=255, G=0, B=0, A=255)		 	
	AlertStartRatio = 0.3
	TestRatio = 0.6
	FadeTime = 0.1
}