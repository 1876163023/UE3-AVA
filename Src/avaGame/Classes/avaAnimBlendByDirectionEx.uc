/**
// 8 방향 Animation 을 지원하기 위한 Blend
//
//		7	0	1
//		6		2
//		5	4	3
//
*/

class avaAnimBlendByDirectionEx extends avaAnimBlendBase
	native;

/** Allows control over how quickly the directional blend should be allowed to change. */
var()	float			DirDegreesPerSecond;

/** In radians. Between -PI and PI. 0.0 is running the way we are looking. */
var		float			DirAngle;

var (Animations)	bool	bAdjustRateByVelocity;
var (Animations)	float	fBasicVelocity;

cpptext
{
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}

defaultproperties
{
	Children(0)=(Name="Forward",Weight=1.0)
	Children(1)=(Name="ForwardRight")
	Children(2)=(Name="Right")
	Children(3)=(Name="BackwardRight")
	Children(4)=(Name="Backward")
	Children(5)=(Name="BackwardLeft")
	Children(6)=(Name="Left")
	Children(7)=(Name="ForwardLeft")
	bFixNumChildren=true

	DirDegreesPerSecond	=	360.0
	fBasicVelocity		=	400.0
}