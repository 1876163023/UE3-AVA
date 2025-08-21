/*
	Material Instance Constant�� Scalar Parameter�� �ִϸ��̼� ��Ű�� ������ �׼�.

	2007/01/05	����
		UI�۾����� ��� ���...
		SeqAct_WaitForLevelsVisible�� SeqAct_Timer�� �����ؼ� ����� �� �� �ʹ�.
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

		// �������� ���.
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

var() array<MaterialInstance>			CompletedMaterials;	//!< �ִϸ��̼��� ���� ���� �ϼ��� ����.

var() array<MaterialInstanceConstant>	AnimatedMaterial;	//!< �ִϸ��̼� �� ����.

/*! @brief ScalarValue�� ������ �θ� �̸�.
	@note
		��, AnimatedMaterial���� ����ϴ� ��� MaterialInstanceConstant����
		ScalarValue�� �̸��� ���ƾ� �Ѵ�.
*/
var() Name								ScalarValueName;

var() float								BeginScalarValue;	//!< ScalarValue ���۰�.
var() float								EndScalarValue;		//!< ScalarValue ���ᰪ.

var() float								PlayTime;			//!< �ִϸ��̼� �ð�.
var float								Time;				//!< ���� �ð�.

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
