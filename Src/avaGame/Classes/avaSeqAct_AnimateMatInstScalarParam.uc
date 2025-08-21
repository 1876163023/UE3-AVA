/*
	Material Instance Constant의 Scalar Parameter를 애니메이션 시키는 시퀀스 액션.

	2007/01/05	고광록
		UI작업으로 잠시 대기...
		SeqAct_WaitForLevelsVisible와 SeqAct_Timer를 참고해서 만들면 될 듯 싶다.
*/
class avaSeqAct_AnimateMatInstScalarParam extends SequenceAction
	native;

cpptext
{
	virtual void Activated()
	{
	}
	virtual UBOOL UpdateOp(FLOAT DeltaTime)
	{
		float rate;

		Time += DeltaTime;

		// 진행중인 경우.
		if(Time < PlayTime)
		{
			rate = Time / PlayTime;

			// running
			return FALSE;
		}

		// finish
		return TRUE;
	}
}

var() array<MaterialInstance>			CompletedMaterials;	//!< 애니메이션이 끝난 후의 완성된 재질.

var() array<MaterialInstanceConstant>	AnimatedMaterial;	//!< 애니메이션 할 재질.

/*! @brief ScalarValue를 설정할 부모 이름.
	@note
		단, AnimatedMaterial에서 사용하는 모든 MaterialInstanceConstant들의
		ScalarValue의 이름은 같아야 한다.
*/
var() Name								ScalarValueName;

var() float								BeginScalarValue;	//!< ScalarValue 시작값.
var() float								EndScalarValue;		//!< ScalarValue 종료값.

var() float								PlayTime;			//!< 애니메이션 시간.
var float								Time;				//!< 현재 시간.

defaultproperties
{
//	bCallHandler=false
	ObjName="Animate MatInstScalarParam"
	ObjCategory="Actor"

	BeginScalarValue	= 0;
	EndScalarValue		= 1;
	Time				= 0;
	PlayTime			= 1;

	InputLinks(0)=(LinkDesc="Start")
	OutputLinks(0)=(LinkDesc="End")

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="actor",PropertyName=actor, MaxVars=1)
}
