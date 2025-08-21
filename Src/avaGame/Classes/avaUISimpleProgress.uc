class avaUISimpleProgress extends UIObject
	native;

enum EUISimpleProgressType
{
	SIMPLE_PROGRESS_TYPE_STRING,
	SIMPLE_PROGRESS_TYPE_BOX,
	SIMPLE_PROGRESS_TYPE_FLOW,
};

enum EUISimpleProgressDirectionType
{
	SIMPLE_PROGRESS_DIR_INCREASE,
	SIMPLE_PROGRESS_DIR_DECREASE,
	SIMPLE_PROGRESS_DIR_NO_DIRECTION,
};

var() float						MaxElement;
var() float						CurrentPos;
var() float						TargetPos;
var() Color						DrawColor;
var() EUISimpleProgressType		ProgressType;
var() float						Attenuation<ToolTip=AttenuationFactor between 0.0f and 1.0f>;

var private float				LatestRenderTime;

var(StringProgress) Font			DrawFont;
var(StringProgress) string			StringElement;
var(BoxProgress) float				BoxSize;
var(FlowProgress) float				Fuzziness<ToolTip=FlowProgress seems to be randomized, 0.0(Stable) ~ 1.0(UnStable)>;

var private Texture2D			DefaultWhiteTexture;

cpptext
{
	virtual void Render_Widget( FCanvas *Canvas );
	virtual void UpdateProgress() {}
	void SetTargetPos( FLOAT NewTargetPos );
	UBOOL IsMoving( EUISimpleProgressDirectionType& Dir);
}

defaultproperties
{
	DrawFont=Font'EngineFonts.SmallFont'
	MaxElement=10.0
	CurrentPos=2.0
	DrawColor=(A=255,R=255,G=255,B=255)
	ProgressType=SIMPLE_PROGRESS_TYPE_BOX
	BoxSize=5
	DefaultWhiteTexture=Texture2D'EngineResources.WhiteSquareTexture'
	Attenuation=10.0
}