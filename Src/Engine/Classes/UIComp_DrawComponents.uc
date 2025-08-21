/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 *
 * +Added support for pulsing and fading the drawing components.  It adds an UpdateFade() function that
 * will return the current fade value giving the deltatime.  The child is responsible for called SetOpacity()
 * passing in whatever value is included.  This is possibly a temporary solution
 *
 */

class UIComp_DrawComponents extends UIComponent
//	within UIObject
	native(UIPrivate);

// FADING

enum EFadeType
{
	EFT_None,			// No fading, Opacity will
	EFT_Fading,			// Fading to a specific Alpha
//	EFT_Pulsing			// Pulsing between Alpha 0.0 and FadeTarget
	EFT_Loop,			// Looping infinitely
};

struct native ScaleRenderParameters
{
	var transient bool						bEnabled;
	var() transient Vector2D				ScaleAxis;
	var() transient float					ScaleDuration;
	var() transient CurveEdPresetCurve		ScalePresetCurve[EUIOrientation.UIORIENT_MAX]<ToolTip=/UIORIENT_Horizontal=0/UIORIENT_Vertical=1/>;

	var transient float						ScaleUpdateTime;

	structcpptext
	{
		void FScaleRenderParameters::Reset() { bEnabled = false; }

		static void ScaleParms( FVector2D& LeftTop, FVector2D& RightBottom, FVector2D AxisXY, FVector2D FactorXY )
		{
			// ��ȯ�ϱ� ���� �߽��� ��ũ����ǥ���� X,Y������ �̸� ���.
			//FLOAT CenterPosX = RenderParameters.DrawX + RenderParameters.DrawXL * AxisX;
			//FLOAT CenterPosY = RenderParameters.DrawY + RenderParameters.DrawYL * AxisY;
			FVector2D OriginExtent(RightBottom.X - LeftTop.X, RightBottom.Y - LeftTop.Y);
			FVector2D CenterPosXY = FVector2D(LeftTop.X + OriginExtent.X * AxisXY.X, 
											LeftTop.Y + OriginExtent.X * AxisXY.Y );

			// ��ǥ�� ��ȯ (ScaleAxis�� 0,0���� �ϰ� ������ Screen ��ǥ�� ����.) 
			FVector4 NewPos(-(AxisXY.X * OriginExtent.X ),			//left
							-(AxisXY.Y * OriginExtent.Y),			//top
							(1.0f - AxisXY.X) * OriginExtent.X,	//right
							(1.0f - AxisXY.Y) * OriginExtent.Y);	//bottom


			// ��ȯ�� ���¿��� �߽��� �������� �������� �ٲ۴�.
			FVector4 NewScaledPos( NewPos.X * FactorXY.X, NewPos.Y * FactorXY.Y, NewPos.Z * FactorXY.X, NewPos.W * FactorXY.Y );
			
			// �ٲ� �������� ����
			LeftTop.X = CenterPosXY.X + NewScaledPos.X;
			LeftTop.Y = CenterPosXY.Y + NewScaledPos.Y;
			RightBottom.X = CenterPosXY.X + NewScaledPos.Z;
			RightBottom.Y = CenterPosXY.Y + NewScaledPos.W;
		}
	}

	structdefaultproperties
	{
		bEnabled = false
	}
};

/** Where the fade is going */

var(Fading) transient float						FadeDuration;

var(Fading) transient EFadeType					FadeType;
var(Fading) transient CurveEdPresetCurve		FadePresetCurve;
var			transient float						FadeUpdateTime;
var(Debug)	transient bool						FadingTrigger;


///** Used to create a delta for fading */
//var transient float LastRenderTime;

///** Used for pulsing.  This is the rate at which it will occur */
//var transient float FadeRate;

/** How Much Longer until we reach the target Alpha */

/** �׷����� �͵��� Scaling�� �����ϵ����� ( �������� �̹���, ���ư��� �г� ��� ), ���� RenderBound�� ������� */
var(Scaling) transient ScaleRenderParameters	ScaleRenderParam;
var(Debug) transient bool						ScalingTrigger;

cpptext
{
	/**
	 * ���̵� ȿ������ ������Ʈ �Ѵ�.(Enable -> Active -> Pressed �� ���� ��Ÿ���� �ٲ� �Ҹ�)
	 * 
	 * @param	FadeAlpha - In: The current Alpha, Out: The New Alpha
	 * @return true if an update is needed
	 */
	UBOOL UpdateFade(FLOAT& FadeAlpha);
	UBOOL UpdateScaleRender(FRenderParameters& RenderParameters );

	/** ������ �÷��� FadingTrigger, ScalingTrigger�� ���� */
	void PostEditChange( FEditPropertyChain& PropertyThatChanged );

protected:
	FLOAT GetCurveValue( UCurveEdPresetCurve* PresetCurve, FLOAT Ratio );
}

function final native Fade( EFadeType itsFadeType, CurveEdPresetCurve PresetCurve, float Duration);

// @deprecated using EFT_Loop as a alternative
//function final native Pulse(optional float MaxAlpha=1.0, optional float MinAlpha=0.0, optional float PulseRate=1.0);
function final native ResetFade();

function final native ScaleRender( CurveEdPresetCurve PresetCurveHorz, CurveEdPresetCurve PresetCurveVert, FLOAT ScaleAxisX, FLOAT ScaleAxisY, FLOAT ScaleDuration );
function final native ResetScaleRender();

function native ApplyDrawCompStyle( UIStyle_Data StyleData );
function native RefreshDrawCompStyle();

/** OnFadeComplete will be called as soon as the fade has been completed */
delegate OnFadeComplete(UIComp_DrawComponents Sender);
delegate OnScaleRenderComplete(UIComp_DrawComponents Sender);

// within UIObject �� �����ϵ����ʾ� �ӽ÷� ���.
function final native UIObject GetOuterUUIObject() const;

defaultproperties
{
	FadeType = EFT_None
}
